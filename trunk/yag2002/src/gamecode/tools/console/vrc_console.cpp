/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2007, A. Botorabi
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
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_console.h"
#include "ctd_cmdregistry.h"
#include "ctd_basecmd.h"

using namespace std;

namespace CTD
{

#define CON_WND    "_console_"

// Input handler for toggling console window
class ConsoleIH : public GenericInputHandler< EnConsole >
{
    public:

        explicit                            ConsoleIH( EnConsole* p_console ) : 
                                             GenericInputHandler< EnConsole >( p_console ),
                                             _toggleEnable( false )
                                            {
                                                _retCode = static_cast< int >( KeyMap::get()->getKeyCode( "Return" ) );
                                                _autoCompleteCode = static_cast< int >( KeyMap::get()->getKeyCode( "Tab" ) );
                                            }
                                
        virtual                             ~ConsoleIH() {}

        //! Handle input events.
        bool                                handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
                                            {
                                                const osgSDL::SDLEventAdapter* p_eventAdapter = dynamic_cast< const osgSDL::SDLEventAdapter* >( &ea );
                                                assert( p_eventAdapter && "invalid event adapter received" );
                                                SDLKey key = p_eventAdapter->getSDLKey();

                                                if ( p_eventAdapter->getEventType() == osgGA::GUIEventAdapter::KEYDOWN ) 
                                                {
                                                    if ( key == SDLK_F10 )
                                                    {
                                                        _toggleEnable = !_toggleEnable;
                                                        getUserObject()->enable( _toggleEnable );
                                                    }
                                                    else if ( _toggleEnable && key == _retCode )
                                                    {
                                                        getUserObject()->issueCmd( _p_userObject->_p_inputWindow->getText().c_str() );
                                                    }
                                                    else if ( key == _autoCompleteCode )
                                                    {
                                                        getUserObject()->autoCompleteCmd( _p_userObject->_p_inputWindow->getText().c_str() );
                                                    }
                                                    else if ( key == SDLK_UP )
                                                    {
                                                        getUserObject()->cmdHistory( true );
                                                    }
                                                    else if ( key == SDLK_DOWN )
                                                    {
                                                        getUserObject()->cmdHistory( false );
                                                    }
                                                }

                                                return false;
                                            }

        void                                resetToggle( bool en ) 
                                            {
                                                _toggleEnable = en;
                                            }

    protected:

        int                                 _retCode;
        
        int                                 _autoCompleteCode;

        bool                                _toggleEnable;
};

//! Implement and register the statistics entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( ConsoleEntityFactory );

EnConsole::EnConsole() :
_p_wnd( NULL ),
_p_inputWindow( NULL ),
_p_outputWindow( NULL ),
_enable( false ),
_p_inputHandler( NULL ),
_cmdHistoryIndex( 0 ),
_shutdownInProgress( false ),
_shutdownCounter( 0 ),
_idleInProgress( false ),
_idleCounter( 0 ),
_p_log( NULL ),
_cwd( "/" )
{
}

EnConsole::~EnConsole()
{        
    CEGUI::WindowManager::getSingleton().destroyWindow( _p_wnd );

    if ( _p_inputHandler )
        _p_inputHandler->destroyHandler();

    // destroy the cmd registry singleton
    ConsoleCommandRegistry::get()->destroy();

    if ( _p_log )
        closeLog();
}

void EnConsole::handleNotification( const EntityNotification& notification )
{
    // handle some notifications
    switch( notification.getId() )
    {
        case CTD_NOTIFY_MENU_ENTER:
            break;

        case CTD_NOTIFY_MENU_LEAVE:
            break;

        case CTD_NOTIFY_SHUTDOWN:

            EntityManager::get()->deleteEntity( this );
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
        log << Log::LogLevel( Log::L_ERROR ) << "the console entity can be created only once for entire application run-time."
                                             << "you are trying to create a second instance!" << std::endl;
    }
    alreadycreated = true;

    try
    {
        _p_wnd = CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/FrameWindow", CON_WND "mainWnd" );
        _p_wnd->setSize( CEGUI::Size( 0.75f, 0.4f ) );
        _p_wnd->setPosition( CEGUI::Point( 0.125f, 0.2f ) );
        _p_wnd->setAlpha( 0.7f );
        _p_wnd->setFont( CTD_GUI_CONSOLE );
        _p_wnd->setAlwaysOnTop( true );
        _p_wnd->setText( "command console" );
        _p_wnd->setMinimumSize( CEGUI::Size( 0.2f, 0.2f ) );
        _p_wnd->subscribeEvent( CEGUI::FrameWindow::EventCloseClicked, CEGUI::Event::Subscriber( &CTD::EnConsole::onCloseFrame, this ) );

        _p_outputWindow = static_cast< CEGUI::MultiLineEditbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/MultiLineEditbox", CON_WND "output" ) );
        _p_outputWindow->setReadOnly( true );
        _p_outputWindow->setSize( CEGUI::Size( 0.96f, 0.7f ) );
        _p_outputWindow->setPosition( CEGUI::Point( 0.02f, 0.1f ) );
        _p_outputWindow->setFont( CTD_GUI_CONSOLE );
        _p_outputWindow->setAlpha( 0.7f );
        _p_wnd->addChildWindow( _p_outputWindow );

        _p_inputWindow = static_cast< CEGUI::Editbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Editbox", CON_WND "input" ) );
        _p_inputWindow->setSize( CEGUI::Size( 0.96f, 0.1f ) );
        _p_inputWindow->setPosition( CEGUI::Point( 0.02f, 0.8f ) );
        _p_inputWindow->setAlpha( 0.7f );
        _p_inputWindow->setFont( CTD_GUI_CONSOLE );
        _p_wnd->addChildWindow( _p_inputWindow );
    }
    catch ( const CEGUI::Exception& e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** Console: cannot setup dialog layout." << endl;
        log << "      reason: " << e.getMessage().c_str() << endl;
    }

    // register entity in order to get updated per simulation step
    EntityManager::get()->registerUpdate( this, true );
    // register entity in order to get notifications
    EntityManager::get()->registerNotification( this, true );

    _p_inputHandler = new ConsoleIH( this );
}

bool EnConsole::onCloseFrame( const CEGUI::EventArgs& arg )
{
    enable( false );
    _p_inputHandler->resetToggle( false );
    return true;
}

void EnConsole::enable( bool en )
{
    if ( _enable == en )
        return;

    if ( en )
    {
        GuiManager::get()->getRootWindow()->addChildWindow( _p_wnd );
        _p_inputWindow->activate();
    }
    else
    {
        GuiManager::get()->getRootWindow()->removeChildWindow( _p_wnd );
        _p_inputWindow->deactivate();
    }

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

    _p_log->open( ( Application::get()->getMediaPath() + filename ).c_str(), mode );
    if ( !*_p_log )
    {
        delete _p_log;
        _p_log = NULL;
        return false;
    }

    std::string text = ( append ? "' appended on " : "' created on " );
    *_p_log << "# log file '" << filename << text << getTimeStamp() << std::endl;

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
    if ( !checkDirectory( Application::get()->getMediaPath() + newcwd ) )
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
void EnConsole::cmdHistory( bool prev )
{
    if ( !_cmdHistory.size() )
        return;

    CEGUI::String text( _cmdHistory[ _cmdHistoryIndex ] );
    _p_inputWindow->setText( text );
    _p_inputWindow->setCaratIndex( text.length() );

    // update cmd index
    if ( prev )
        _cmdHistoryIndex--;
    else
        _cmdHistoryIndex++;

    // clamp index
    if ( int( _cmdHistoryIndex ) < 0 )
        _cmdHistoryIndex = 0;
    else if ( _cmdHistoryIndex > ( _cmdHistory.size() - 1 ) )
        _cmdHistoryIndex = _cmdHistory.size() - 1;

}

void EnConsole::autoCompleteCmd( const std::string& cmd )
{
    CEGUI::String text;

    // get matching candidates
    std::vector< std::string > candidates;
    unsigned int matchcnt = ConsoleCommandRegistry::get()->getCmdCandidates( cmd, candidates );

    if ( !matchcnt ) // we have no match and no candidates
    {
        text = _p_outputWindow->getText();
        text += "> type 'help' in order to get a complete list of possible commands.\n";
        _p_outputWindow->setText( text );
        _p_outputWindow->setCaratIndex( text.length() );
        return;
    }

    std::vector< std::string >::iterator p_beg = candidates.begin(), p_end = candidates.end();
    if ( matchcnt > 1 ) // we have several candidates
    {
        text = _p_outputWindow->getText();
        text += "> possible commands: ";
        for ( ; p_beg != p_end; p_beg++ )
        {
            text += ( *p_beg ) + "  ";
        }
        text += "\n";
        _p_outputWindow->setText( text );
        _p_outputWindow->setCaratIndex( text.length() );

        // now auto-complete up to next unmatching character in candidates
        bool dobreak = false;
        size_t cnt = cmd.size();
        do
        {
            p_beg = candidates.begin(), p_end = candidates.end();
            char matchc = ( *p_beg )[ cnt ];
            p_beg++;
            for ( ; p_beg != p_end; p_beg++ )
            {
                std::string curcmd = *p_beg;
                if ( ( cnt >= curcmd.length() ) || ( matchc != curcmd[ cnt ] ) )
                {
                    dobreak = true;
                    break;
                }
            }

            cnt++;

        } while ( !dobreak );

        std::string cmdc = *candidates.begin();
        cmdc = cmdc.substr( 0, cnt - 1 );
        _p_inputWindow->setText( cmdc );
        _p_inputWindow->setCaratIndex( cmdc.length() );
    }
    else // we have one single match
    {
        text = *p_beg;
        _p_inputWindow->setText( text );
        _p_inputWindow->setCaratIndex( text.length() );
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
    CEGUI::String text = _p_outputWindow->getText();
    text += "> " + cmd + "\n";
    // dispatch the command
    text += dispatchCmdLine( cmd );
    _p_outputWindow->setText( text );
    _p_inputWindow->setText( "" );
    // set carat position in order to trigger text scrolling after a new line has been added
    _p_outputWindow->setCaratIndex( text.length() - 1 );

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
    static string result;
    result = "";

    vector< string > cmds;
    explode( cmdline, ";", &cmds ); // multiple commands can be given separated by semicolon 
    vector< string >::iterator p_beg = cmds.begin(), p_end = cmds.end();
    for ( ; p_beg != p_end; p_beg++ )
    {
        // clean up cmd from leading whitespaces
        string cmd = *p_beg;
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
    static string result;
    result = "";
    string lcmd = cmd + ";"; // append a semicolon for easiert parsing
    string command;
    string arguments;

    // parse and extract the command and its arguments
    size_t pos = lcmd.find_first_of( " " );
    command = ( pos == ( size_t )-1 ) ? cmd : lcmd.substr( 0, pos );
    pos++;
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
    size_t strsize = cmdline.size();
    int  marker = -1;
    string curstr;
    for ( size_t cnt = 0; cnt <= strsize; cnt++ ) // merge string areas noted by ""
    {
        if ( ( marker == -1 ) && ( cmdline[ cnt ] == '\"' ) )
        {
             marker = cnt;
        }
        else if ( cmdline[ cnt ] == '\"' )
        {
            string mergeit = cmdline.substr( marker + 1, cnt - marker - 1 ); // cut the "" from string
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
            Application::get()->stop();
        else
            _shutdownCounter -= deltaTime;
    }
}

} // namespace CTD
