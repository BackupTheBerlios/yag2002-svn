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
 # gui-based io handler for an in-game console
 #
 #   date of creation:  09/09/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_console.h"
#include "ctd_consoleiogui.h"


namespace CTD
{

#define CON_WND             "_console_gui_"
#define KEY_ISSUE_CMD       "Return"
#define KEY_AUTOCOMPLETE    "Tab"

ConsoleIOGui::ConsoleIOGui( EnConsole* p_console ) : 
 ConsoleIOBase( p_console ),
 GenericInputHandler< EnConsole >( p_console ),
 _toggleEnable( false )
{
    _retCode = static_cast< int >( KeyMap::get()->getKeyCode( KEY_ISSUE_CMD ) );
    _autoCompleteCode = static_cast< int >( KeyMap::get()->getKeyCode( KEY_AUTOCOMPLETE ) );

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
        _p_wnd->subscribeEvent( CEGUI::FrameWindow::EventCloseClicked, CEGUI::Event::Subscriber( &CTD::ConsoleIOGui::onCloseFrame, this ) );

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
        log << Log::LogLevel( Log::L_ERROR ) << "*** Console GUI: cannot setup dialog layout." << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}
                                        
ConsoleIOGui::~ConsoleIOGui() 
{
}

bool ConsoleIOGui::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
    const osgSDL::SDLEventAdapter* p_eventAdapter = dynamic_cast< const osgSDL::SDLEventAdapter* >( &ea );
    assert( p_eventAdapter && "invalid event adapter received" );
    SDLKey key = p_eventAdapter->getSDLKey();

    if ( p_eventAdapter->getEventType() == osgGA::GUIEventAdapter::KEYDOWN ) 
    {
        if ( key == SDLK_F10 )
        {
            _toggleEnable = !_toggleEnable;
            _p_consoleEntity->enable( _toggleEnable );
        }
        else if ( _toggleEnable && key == _retCode )
        {
            _p_consoleEntity->issueCmd( _p_inputWindow->getText().c_str() );
        }
        else if ( key == _autoCompleteCode )
        {
            _p_consoleEntity->autoCompleteCmd( _p_inputWindow->getText().c_str() );
        }
        else if ( key == SDLK_UP )
        {
            std::string cmd = _p_consoleEntity->cmdHistory( true );
            CEGUI::String text( cmd );
            _p_inputWindow->setText( text );
            _p_inputWindow->setCaratIndex( text.length() );
        }
        else if ( key == SDLK_DOWN )
        {
            std::string cmd = _p_consoleEntity->cmdHistory( false );
            CEGUI::String text( cmd );
            _p_inputWindow->setText( text );
            _p_inputWindow->setCaratIndex( text.length() );
        }
    }

    return false;
}

void ConsoleIOGui::resetToggle( bool en ) 
{
    _toggleEnable = en;
}

void ConsoleIOGui::shutdown() 
{
    destroyHandler();
}

void ConsoleIOGui::enable( bool en )
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

void ConsoleIOGui::setCmdLine( const std::string& text )
{
    CEGUI::String cetext( text );
    _p_inputWindow->setText( cetext );
    _p_inputWindow->setCaratIndex( cetext.length() );
}

void ConsoleIOGui::output( const std::string& text )
{
    CEGUI::String cetext;
    cetext = _p_outputWindow->getText();
    cetext += CEGUI::String( text );
    _p_outputWindow->setText( cetext );
    _p_outputWindow->setCaratIndex( cetext.length() );
}

bool ConsoleIOGui::onCloseFrame( const CEGUI::EventArgs& arg )
{
    enable( false );
    resetToggle( false );
    return true;
}

} // namespace CTD
