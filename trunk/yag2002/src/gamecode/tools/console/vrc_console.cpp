/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2006, A. Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU Lesser General Public 
 *  License version 2.1 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public 
 *  License along with this program; if not, write to the Free 
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 * 
 ****************************************************************/

/*###############################################################
 # entity for an in-game console
 #
 #   date of creation:  06/13/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_console.h"
#include "vrc_consoleiobase.h"
#include "vrc_consoleiogui.h"
#include "vrc_consoleiocin.h"
#include "vrc_cmdregistry.h"
#include "vrc_basecmd.h"

namespace vrc
{

//! NOTE: console command is not available in published version
#ifndef VRC_BUILD_PUBLISH
//! Implement and register the statistics entity factory
YAF3D_IMPL_ENTITYFACTORY( ConsoleEntityFactory )
#endif

EnConsole::EnConsole() :
_enable( false ),
_p_ioHandler( NULL ),
_cmdHistoryIndex( 0 ),
_shutdownInProgress( false ),
_shutdownCounter( 0 ),
_idleInProgress( false ),
_idleCounter( 0 ),
_p_log( NULL ),
_cwd( "/" )
{
    // register entity attribute
    getAttributeManager().addAttribute( "initialCmd", _initialCommand );
}

EnConsole::~EnConsole()
{

    if ( _p_ioHandler )
        _p_ioHandler->shutdown();

    if ( _p_log )
        closeLog();
}

void EnConsole::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle some notifications
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_MENU_ENTER:
            break;

        case YAF3D_NOTIFY_MENU_LEAVE:
            break;

        case YAF3D_NOTIFY_SHUTDOWN:

            yaf3d::EntityManager::get()->deleteEntity( this );
            break;

        case YAF3D_NOTIFY_NEW_LEVEL_INITIALIZED:

            if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server )
            {
                log_info << std::endl;
                log_out  << "================================" << std::endl;
                log_out  << "=== VRC server console ready ===" << std::endl;
                log_out  << "================================" << std::endl;
                log_out  << "" << std::endl;
            }
            break;

        default:
            ;
    }
}

void EnConsole::initialize()
{
    static bool alreadycreated = false;
    if ( alreadycreated )
    {
        log_error << "the console entity can be created only once for entire application run-time."
                                             << "you are trying to create a second instance!" << std::endl;
    }
    alreadycreated = true;

    if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server )
    {
        _p_ioHandler = new ConsoleIOCin( this );
    }
    //! NOTE the console is not enabled in published version!
#ifndef VRC_BUILD_PUBLISH
    else
    {
        _p_ioHandler = new ConsoleIOGui( this );
    }
#endif

    // register entity in order to get updated per simulation step
    yaf3d::EntityManager::get()->registerUpdate( this, true );
    // register entity in order to get notifications
    yaf3d::EntityManager::get()->registerNotification( this, true );

    // issue initial command if one exists
    if ( _initialCommand.length() )
        enqueueCmd( _initialCommand );
}

void EnConsole::enable( bool en )
{
    if ( _enable == en )
        return;

    _p_ioHandler->enable( en );
    _enable = en;
}

void EnConsole::triggerShutdown( float delay )
{
    _shutdownInProgress = true;
    _shutdownCounter = delay;
}

void EnConsole::triggerIdle( unsigned int steps )
{
    _idleCounter = steps;
    _idleInProgress = true;
}

bool EnConsole::createLog( const std::string& filename, bool append )
{
    if ( _p_log )
        return false;

    _p_log = new std::ofstream;
    std::ios_base::openmode mode = std::ios_base::out | std::ios_base::binary;
    if ( append )
        mode |= std::ios_base::app;

    _p_log->open( ( yaf3d::Application::get()->getMediaPath() + filename ).c_str(), mode );
    if ( !*_p_log )
    {
        delete _p_log;
        _p_log = NULL;
        return false;
    }

    std::string text = ( append ? "' appended on " : "' created on " );
    *_p_log << "# log file '" << filename << text << yaf3d::getTimeStamp() << std::endl;

    return true;
}

bool EnConsole::closeLog()
{
    if ( !_p_log )
        return false;

    _p_log->close();
    delete _p_log;
    _p_log = NULL;

    return true;
}

bool EnConsole::setCWD( const std::string& cwd )
{
    std::string newcwd = _cwd;

    if ( !cwd.size() )
    {
        newcwd = "/";
    }
    else
    {
        // cut slashes at end of path
        while ( ( newcwd.size() > 0 ) && ( newcwd[ newcwd.size() - 1 ] != '/' ) ) newcwd.erase( newcwd.size() - 1 );

        if ( cwd[ 0 ] == '/' )
        {
            newcwd = cwd;
        }
        else if ( cwd == ".." )
        {
            if ( newcwd.size() > 1 )
            {
                // cut slashes at end of path
                while ( ( newcwd.size() > 0 ) && ( newcwd[ newcwd.size() - 1 ] != '/' ) ) newcwd.erase( newcwd.size() - 1 );
                if ( newcwd.size() > 1 ) newcwd.erase( newcwd.size() - 1 );
            }
        }
        else if ( cwd == "." )
        {
            // do nothing
        }
        else
        {
            // currently we don't allow the usage of multiple ".." in path!
            if ( cwd[ 0 ] == '.' )
                return false;

            if ( newcwd.size() > 1 )
                newcwd += "/" + cwd;
            else
                newcwd += cwd;

        }
    }

    // now check if the new directory exists in file system
    if ( !yaf3d::checkDirectory( yaf3d::Application::get()->getMediaPath() + newcwd ) )
        return false;
 
    _cwd = newcwd;
    return true;
}

//! Get current working directory (relative to media path)
const std::string& EnConsole::getCWD()
{
    return _cwd;
}

// some methods for shell functionality 
//-------------------------------------
std::string EnConsole::cmdHistory( bool prev )
{
    if ( !_cmdHistory.size() )
        return "";

    std::string cmd = _cmdHistory[ _cmdHistoryIndex ];

    // update cmd index
    if ( prev )
        --_cmdHistoryIndex;
    else
        ++_cmdHistoryIndex;

    // clamp index
    if ( int( _cmdHistoryIndex ) < 0 )
        _cmdHistoryIndex = 0;
    else if ( _cmdHistoryIndex > ( _cmdHistory.size() - 1 ) )
        _cmdHistoryIndex = _cmdHistory.size() - 1;

    return cmd;
}

void EnConsole::autoCompleteCmd( const std::string& cmd )
{
    // get matching candidates
    std::vector< std::string > candidates;
    unsigned int matchcnt = ConsoleCommandRegistry::get()->getCmdCandidates( cmd, candidates );

    if ( !matchcnt ) // we have no match and no candidates
    {
        _p_ioHandler->output( "> type 'help' in order to get a complete list of possible commands.\n" );
        return;
    }

    std::vector< std::string >::iterator p_beg = candidates.begin(), p_end = candidates.end();
    if ( matchcnt > 1 ) // we have several candidates
    {
        std::string text( "> possible commands: " );
        for ( ; p_beg != p_end; ++p_beg )
        {
            text += ( *p_beg ) + "  ";
        }
        text += "\n";
        _p_ioHandler->output( text );

        // now auto-complete up to next unmatching character in candidates
        bool dobreak = false;
        size_t cnt = cmd.size();
        do
        {
            p_beg = candidates.begin(), p_end = candidates.end();
            char matchc = ( *p_beg )[ cnt ];
            ++p_beg;
            for ( ; p_beg != p_end; ++p_beg )
            {
                std::string curcmd = *p_beg;
                if ( ( cnt >= curcmd.length() ) || ( matchc != curcmd[ cnt ] ) )
                {
                    dobreak = true;
                    break;
                }
            }

            ++cnt;

        } while ( !dobreak );

        std::string cmdc = *candidates.begin();
        cmdc = cmdc.substr( 0, cnt - 1 );
        _p_ioHandler->setCmdLine( cmdc );
    }
    else // we have one single match
    {
        _p_ioHandler->setCmdLine( *p_beg );
    }
}

void EnConsole::enqueueCmd( std::string cmd )
{
    // enqueue the command, it will be executed on next update
    _cmdQueue.push( make_pair( cmd, false ) );
}

void EnConsole::issueCmd( std::string cmd )
{
    // enqueue the command, it will be executed on next update
    _issuedCmdQueue.push( make_pair( cmd, true ) );
}

void EnConsole::applyCmd( const std::string& cmd, bool hashcmd )
{
    std::string text( "> " + cmd + "\n" );
    // dispatch the command
    text += dispatchCmdLine( cmd );
    _p_ioHandler->output( text );
    _p_ioHandler->setCmdLine( "" );

    // store the command in history if it is not the exact same as before
    if ( hashcmd && cmd.length() )
    {
        if ( !_cmdHistory.size() )
        {
            _cmdHistory.push_back( cmd );
            _cmdHistoryIndex = 0;
        }
        else
        {
            if ( _cmdHistory.back() != cmd )
            {
                _cmdHistory.push_back( cmd );
                _cmdHistoryIndex = _cmdHistory.size() - 1;
            }
        }
    }
}

const std::string& EnConsole::dispatchCmdLine( const std::string& cmdline )
{
    static std::string result;
    result = "";

    std::vector< std::string > cmds;
    yaf3d::explode( cmdline, ";", &cmds ); // multiple commands can be given separated by semicolon 
    std::vector< std::string >::iterator p_beg = cmds.begin(), p_end = cmds.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        // clean up cmd from leading whitespaces
        std::string cmd = *p_beg;
        cmd.erase( 0, cmd.find_first_not_of( " " ) );

        // append to log if log is created
        if ( _p_log )
            *_p_log << ">" << cmd << std::endl;

        result += executeCmd( cmd );

        // append to log if log is created
        if ( _p_log )
        {
            *_p_log << result << std::endl;
            _p_log->flush();
        }
    }

    return result;
}

const std::string& EnConsole::executeCmd( const std::string& cmd )
{
    static std::string result;
    result = "";
    std::string lcmd = cmd + ";"; // append a semicolon for easiert parsing
    std::string command;
    std::string arguments;

    // parse and extract the command and its arguments
    std::size_t pos = lcmd.find_first_of( " " );
    command = ( pos == ( size_t )-1 ) ? cmd : lcmd.substr( 0, pos );
    ++pos;
    size_t posend = lcmd.find_first_of( ";", pos );
    if ( command.length() < ( posend - 1 ) )
        arguments = lcmd.substr( pos, posend - pos );

    BaseConsoleCommand* p_cmd = ConsoleCommandRegistry::get()->getCmd( command );
    if ( !p_cmd )
    {
        result = "* cannot find command '" + command + "'";
        return result;
    }

    // format the arguments into a list
    std::vector< std::string > args;
    parseArguments( arguments, args );
    // execute command
    result = p_cmd->execute( args );

    return result;
}

void EnConsole::parseArguments( const std::string& cmdline, std::vector< std::string >& args )
{
    // arguments are white space separated, except they are placed in "" like: "my server name"
    std::size_t strsize = cmdline.size();
    int  marker = -1;
    std::string curstr;
    for ( std::size_t cnt = 0; cnt <= strsize; ++cnt ) // merge string areas noted by ""
    {
        if ( ( marker == -1 ) && ( cmdline[ cnt ] == '\"' ) )
        {
             marker = cnt;
        }
        else if ( cmdline[ cnt ] == '\"' )
        {
            std::string mergeit = cmdline.substr( marker + 1, cnt - marker - 1 ); // cut the "" from string
            args.push_back( mergeit );
            marker = -1;
            // skip white spaces until next argument
            cnt = ( cnt < strsize ) ? cnt + 1 : cnt;
            cnt = cmdline.find_first_not_of( " ", cnt ); // skip white spaces until next argument
            if ( ( int )cnt < 0 ) // if end of line reached then we are finish parsing arguments
                break;
            cnt--;
        }
        else
        {
            if ( marker == -1 ) 
            {
                if ( ( ( cmdline[ cnt ] == ' ' ) || ( cmdline.length() == cnt ) ) && curstr.size() > 0 )
                {
                    args.push_back( curstr );
                    curstr = "";
                    cnt = cmdline.find_first_not_of( " ", cnt ); // skip white spaces until next argument
                    if ( ( int )cnt < 0 ) // if end of line reached then we are finish parsing arguments
                        break;
                    cnt--;
                }
                else
                {
                    curstr += cmdline[ cnt ];
                }
            }
        }
    }
}

void EnConsole::updateEntity( float deltaTime )
{
    // hanlde issued idle command
    if ( !_idleInProgress ) 
    {        
        // execute all issued commands from console
        {
            while ( !_issuedCmdQueue.empty() )
            {
                std::pair< std::string, bool >& cmd = _issuedCmdQueue.front();
                applyCmd( cmd.first, cmd.second );
                _issuedCmdQueue.pop();
                // if idle command is issued so we have to abort further command handling until the idle command is completed
                if ( _idleInProgress )
                    return;
            }
        }
        // execute all queued commands
        {
            while ( !_cmdQueue.empty() )
            {
                std::pair< std::string, bool >& cmd = _cmdQueue.front();
                applyCmd( cmd.first, cmd.second );
                _cmdQueue.pop();
                // if idle command is in queue so we have to abort further command handling until the idle command is completed
                if ( _idleInProgress )
                    return;
            }
        }
    }
    else
    {
        if ( _idleCounter == 0 )
            _idleInProgress = false;
        else
            _idleCounter--;
    }

    if ( _shutdownInProgress )
    {
        if ( _shutdownCounter < 0 )
            yaf3d::Application::get()->stop();
        else
            _shutdownCounter -= deltaTime;
    }
}

} // namespace vrc
