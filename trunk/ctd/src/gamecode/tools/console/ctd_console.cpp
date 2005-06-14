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
 #   date of creation:  13/06/2005
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

                                            ConsoleIH( EnConsole* p_console ) : 
                                             GenericInputHandler< EnConsole >( p_console ),
                                             _toggleEnable( false )
                                            {
                                                _retCode = KeyMap::get()->getKeyCode( "Return" );
                                                _autoCompleteCode = KeyMap::get()->getKeyCode( "Tab" );
                                            }
                                
        virtual                             ~ConsoleIH() {}

        //! Handle input events.
        bool                                handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
                                            {
                                                if ( ea.getEventType() == osgGA::GUIEventAdapter::KEYDOWN ) 
                                                {
                                                    if ( ea.getKey() == 92 ) // '^'
                                                    {
                                                        _toggleEnable = !_toggleEnable;
                                                        _p_userObject->enable( _toggleEnable );
                                                    }
                                                    else if ( _toggleEnable && ea.getKey() == _retCode )
                                                    {
                                                        _p_userObject->applyCmd( _p_userObject->_p_inputWindow->getText().c_str() );
                                                    }
                                                    else if ( ea.getKey() == _autoCompleteCode )
                                                    {
                                                        _p_userObject->autoCompleteCmd( _p_userObject->_p_inputWindow->getText().c_str() );
                                                    }
                                                    else if ( ea.getKey() == osgGA::GUIEventAdapter::KEY_Up )
                                                    {
                                                        _p_userObject->cmdHistory( true );
                                                    }
                                                    else if ( ea.getKey() == osgGA::GUIEventAdapter::KEY_Down )
                                                    {
                                                        _p_userObject->cmdHistory( false );
                                                    }
                                                }

                                                return false;
                                            }

        void                                resetToggle( bool en ) 
                                            {
                                                _toggleEnable = en;
                                            }

    protected:

        unsigned int                        _retCode;
        
        unsigned int                        _autoCompleteCode;

        bool                                _toggleEnable;
};

//! Implement and register the statistics entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( ConsoleEntityFactory );

EnConsole::EnConsole() :
_enable( false ),
_p_wnd( NULL ),
_p_inputWindow( NULL ),
_p_outputWindow( NULL ),
_p_inputHandler( NULL ),
_cmdHistoryIndex( 0 )
{
    // register entity in order to get updated per simulation step
    EntityManager::get()->registerUpdate( this, true );
    // register entity in order to get notifications
    EntityManager::get()->registerNotification( this, true );
}

EnConsole::~EnConsole()
{        
    CEGUI::WindowManager::getSingleton().destroyWindow( _p_wnd );

    if ( _p_inputHandler )
        _p_inputHandler->destroyHandler();

    // destroy the cmd registry singleton
    ConsoleCommandRegistry::get()->destroy();
}

void EnConsole::handleNotification( const EntityNotification& notify )
{
    // handle some notifications
    switch( notify.getId() )
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
    try
    {
        _p_wnd = CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/FrameWindow", CON_WND "mainWnd" );
        _p_wnd->setSize( CEGUI::Size( 0.75f, 0.4f ) );
        _p_wnd->setPosition( CEGUI::Point( 0.125f, 0.2f ) );
        _p_wnd->setAlpha( 0.7f );
        _p_wnd->setAlwaysOnTop( true );
        _p_wnd->setText( "Console" );
        _p_wnd->setMinimumSize( CEGUI::Size( 0.2f, 0.2f ) );
        _p_wnd->subscribeEvent( CEGUI::FrameWindow::EventCloseClicked, CEGUI::Event::Subscriber( EnConsole::onCloseFrame, this ) );

        _p_outputWindow = static_cast< CEGUI::MultiLineEditbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/MultiLineEditbox", CON_WND "output" ) );
        _p_outputWindow->setReadOnly( true );
        _p_outputWindow->setSize( CEGUI::Size( 0.96f, 0.7f ) );
        _p_outputWindow->setPosition( CEGUI::Point( 0.02f, 0.1f ) );
        _p_outputWindow->setAlpha( 0.7f );
        _p_wnd->addChildWindow( _p_outputWindow );

        _p_inputWindow = static_cast< CEGUI::Editbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Editbox", CON_WND "input" ) );
        _p_inputWindow->setSize( CEGUI::Size( 0.96f, 0.1f ) );
        _p_inputWindow->setPosition( CEGUI::Point( 0.02f, 0.8f ) );
        _p_inputWindow->setAlpha( 0.7f );
        _p_wnd->addChildWindow( _p_inputWindow );
    }
    catch ( CEGUI::Exception e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** Console: cannot setup dialog layout." << endl;
        log << "      reason: " << e.getMessage().c_str() << endl;
    }

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

void EnConsole::cmdHistory( bool prev )
{
    if ( !_cmdHistory.size() )
        return;

    if ( prev )
    {
        if ( _cmdHistoryIndex > 0 )
            _cmdHistoryIndex--;
    }
    else
    {
        if ( _cmdHistoryIndex < ( _cmdHistory.size() - 1 ) )
            _cmdHistoryIndex++;
    }

    CEGUI::String text( _cmdHistory[ _cmdHistoryIndex ] );
    _p_inputWindow->setText( text );
    _p_inputWindow->setCaratIndex( text.length() );
}

void EnConsole::autoCompleteCmd( const std::string& cmd )
{
    CEGUI::String text;

    // get matching candidates
    std::vector< std::string > candidates;
    unsigned int matchcnt = ConsoleCommandRegistry::get()->autoCompleteCmd( cmd, candidates );

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
    }
    else // we have one single match
    {
        text = *p_beg;
        _p_inputWindow->setText( text );
        _p_inputWindow->setCaratIndex( text.length() );
    }
}

void EnConsole::applyCmd( const std::string& cmd )
{
    CEGUI::String text = _p_outputWindow->getText();
    text += "> " + cmd + "\n";
    // dispatch the command
    text += dispatchCmdLine( cmd ) + "\n";
    _p_outputWindow->setText( text );
    _p_inputWindow->setText( "" );
    // set carat position in order to trigger text scrolling after a new line has been added
    _p_outputWindow->setCaratIndex( text.length() - 1 );

    // store the command in history
    if ( cmd.length() )
    {
        _cmdHistory.push_back( cmd );
        _cmdHistoryIndex = _cmdHistory.size();
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

        result += executeCmd( cmd );
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
    size_t marker = 0;
    string curstr;
    for ( size_t cnt = 0; cnt <= strsize; cnt++ ) // merge string areas noted by ""
    {
        if ( ( marker == 0 ) && ( cmdline[ cnt ] == '\"' ) )
        {
             marker = cnt;
        }
        else if ( cmdline[ cnt ] == '\"' )
        {
            string merge = cmdline.substr( marker + 1, cnt - marker - 1 ); // cut the "" from string
            args.push_back( merge );
            marker = 0;
        }
        else
        {
            if ( marker == 0 ) 
            {
                if ( cmdline[ cnt ] == ' ' || cmdline.length() == cnt )
                {
                    args.push_back( curstr );
                    curstr = "";
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

}

} // namespace CTD
