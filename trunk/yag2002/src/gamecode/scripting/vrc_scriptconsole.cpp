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
 # Small build-in utility for script debugging
 #
 #   date of creation:  04/11/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/


#include <vrc_main.h>
#include "vrc_scriptconsole.h"
#include <lua.h>

namespace vrc
{

ScriptConsole::ScriptConsole() :
 _p_scScriptWnd( NULL ),
 _p_scInputWindow( NULL ),
 _p_scOutputWindow( NULL )
{
}

ScriptConsole::~ScriptConsole()
{
    try
    {
        if ( _p_scScriptWnd )
            CEGUI::WindowManager::getSingleton().destroyWindow( _p_scScriptWnd );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "GameLogicScript: problem destroying script editor gui." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

bool ScriptConsole::scInitialize()
{
    CEGUI::DefaultWindow* p_parent = yaf3d::GuiManager::get()->getRootWindow();
    try
    {
        float framewidth = 0.4f, frameheight = 0.2f;
        _p_scScriptWnd = static_cast< CEGUI::FrameWindow* >( CEGUI::WindowManager::getSingleton().createWindow( ( CEGUI::utf8* )"TaharezLook/FrameWindow", "_script_editor_" ) );
        _p_scScriptWnd->setText( "logic script editor" );
        _p_scScriptWnd->setPosition( CEGUI::Point( 0.6f, 0.8f ) );
        _p_scScriptWnd->setSize( CEGUI::Size( framewidth, frameheight ) );
        _p_scScriptWnd->setSizingEnabled( true );
        _p_scScriptWnd->setAlpha( 0.7f );
        _p_scScriptWnd->setMinimumSize( CEGUI::Size( 0.2f, 0.1f ) );
        _p_scScriptWnd->setFont( YAF3D_GUI_CONSOLE );
        _p_scScriptWnd->setAlwaysOnTop( true );
        _p_scScriptWnd->show();

        _p_scOutputWindow = static_cast< CEGUI::MultiLineEditbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/MultiLineEditbox", "_script_editor_output_" ) );
        _p_scOutputWindow->setReadOnly( true );
        _p_scOutputWindow->setSize( CEGUI::Size( 0.96f, 0.7f ) );
        _p_scOutputWindow->setPosition( CEGUI::Point( 0.02f, 0.1f ) );
        _p_scOutputWindow->setFont( YAF3D_GUI_CONSOLE );
        _p_scOutputWindow->setAlpha( 0.7f );
        _p_scScriptWnd->addChildWindow( _p_scOutputWindow );

        _p_scInputWindow = static_cast< CEGUI::Editbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Editbox", "_script_editor_input_" ) );
        _p_scInputWindow->subscribeEvent( CEGUI::MultiLineEditbox::EventCharacterKey, CEGUI::Event::Subscriber( &vrc::ScriptConsole::scOnInputTextChanged, this ) );
        _p_scInputWindow->setSize( CEGUI::Size( 0.96f, 0.15f ) );
        _p_scInputWindow->setPosition( CEGUI::Point( 0.02f, 0.8f ) );
        _p_scInputWindow->setAlpha( 0.7f );
        _p_scOutputWindow->setFont( YAF3D_GUI_FONT8 );
        _p_scInputWindow->setFont( YAF3D_GUI_CONSOLE );
        _p_scScriptWnd->addChildWindow( _p_scInputWindow );

        p_parent->addChildWindow( _p_scScriptWnd );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "GameLogic: error creating script editor" << std::endl;
        log_error << "  reason: " << e.getMessage().c_str() << std::endl;
        return false;
    }

    scAddOutput( "Lua script console\n" LUA_VERSION "\n" );
    return true;
}

bool ScriptConsole::scOnInputTextChanged( const CEGUI::EventArgs& arg )
{
    CEGUI::KeyEventArgs& ke = static_cast< CEGUI::KeyEventArgs& >( const_cast< CEGUI::EventArgs& >( arg ) );
    if ( ke.codepoint == SDLK_RETURN )
    {
        // skip empty lines
        if ( !_p_scInputWindow->getText().length() )
            return true;

        scAddOutput( ">" + std::string( _p_scInputWindow->getText().c_str() ) );
        scProcessCmd( _p_scInputWindow->getText().c_str() );
        _p_scInputWindow->setText( "" );
        _p_scOutputWindow->setCaratIndex( _p_scOutputWindow->getText().length() );
    }

    return true;
}

void ScriptConsole::scShow( bool en )
{
    if ( !_p_scScriptWnd )
        return;

    if ( en )
        _p_scScriptWnd->show();
    else
        _p_scScriptWnd->hide();
}

void ScriptConsole::scAddOutput( const std::string& msg, bool timestamp )
{
    if ( _p_scOutputWindow )
    {
        std::string ts;
        
        if ( timestamp )
            ts = "[" + yaf3d::getFormatedTime() + "] ";

        _p_scOutputWindow->setText( _p_scOutputWindow->getText() + ts + msg );
        _p_scOutputWindow->setCaratIndex( _p_scOutputWindow->getText().length() );
    }
}

} // namespace vrc
