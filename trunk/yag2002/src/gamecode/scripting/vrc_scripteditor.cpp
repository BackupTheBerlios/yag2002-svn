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
 #   author:            ali botorabi (boto)
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/


#include <vrc_main.h>
#include "vrc_scripteditor.h"
#include <lua.h>

namespace vrc
{

ScriptEditor::ScriptEditor() :
 _p_seScriptWnd( NULL ),
 _p_seInputWindow( NULL ),
 _p_seOutputWindow( NULL )
{
}

ScriptEditor::~ScriptEditor()
{
    try
    {
        if ( _p_seScriptWnd )
            CEGUI::WindowManager::getSingleton().destroyWindow( _p_seScriptWnd );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "GameLogicScript: problem destroying script editor gui." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

bool ScriptEditor::seInitialize()
{
    CEGUI::DefaultWindow* p_parent = yaf3d::GuiManager::get()->getRootWindow();
    try
    {
        float framewidth = 0.4f, frameheight = 0.2f;
        _p_seScriptWnd = static_cast< CEGUI::FrameWindow* >( CEGUI::WindowManager::getSingleton().createWindow( ( CEGUI::utf8* )"TaharezLook/FrameWindow", "_script_editor_" ) );
        _p_seScriptWnd->setText( "logic script editor" );
        _p_seScriptWnd->setPosition( CEGUI::Point( 0.6f, 0.8f ) );
        _p_seScriptWnd->setSize( CEGUI::Size( framewidth, frameheight ) );
        _p_seScriptWnd->setSizingEnabled( true );
        _p_seScriptWnd->setAlpha( 0.7f );
        _p_seScriptWnd->setMinimumSize( CEGUI::Size( 0.2f, 0.1f ) );
        _p_seScriptWnd->setFont( YAF3D_GUI_CONSOLE );
        _p_seScriptWnd->setAlwaysOnTop( true );
        _p_seScriptWnd->show();

        _p_seOutputWindow = static_cast< CEGUI::MultiLineEditbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/MultiLineEditbox", "_script_editor_output_" ) );
        _p_seOutputWindow->setReadOnly( true );
        _p_seOutputWindow->setSize( CEGUI::Size( 0.96f, 0.7f ) );
        _p_seOutputWindow->setPosition( CEGUI::Point( 0.02f, 0.1f ) );
        _p_seOutputWindow->setFont( YAF3D_GUI_CONSOLE );
        _p_seOutputWindow->setAlpha( 0.7f );
        _p_seScriptWnd->addChildWindow( _p_seOutputWindow );

        _p_seInputWindow = static_cast< CEGUI::Editbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Editbox", "_script_editor_input_" ) );
        _p_seInputWindow->subscribeEvent( CEGUI::MultiLineEditbox::EventCharacterKey, CEGUI::Event::Subscriber( &vrc::ScriptEditor::seOnInputTextChanged, this ) );
        _p_seInputWindow->setSize( CEGUI::Size( 0.96f, 0.15f ) );
        _p_seInputWindow->setPosition( CEGUI::Point( 0.02f, 0.8f ) );
        _p_seInputWindow->setAlpha( 0.7f );
        _p_seOutputWindow->setFont( YAF3D_GUI_FONT8 );
        _p_seInputWindow->setFont( YAF3D_GUI_CONSOLE );
        _p_seScriptWnd->addChildWindow( _p_seInputWindow );

        p_parent->addChildWindow( _p_seScriptWnd );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "GameLogic: error creating script editor" << std::endl;
        log_error << "  reason: " << e.getMessage().c_str() << std::endl;
        return false;
    }

    seAddOutput( "Lua script console\n" LUA_VERSION "\n" );
    return true;
}

bool ScriptEditor::seOnInputTextChanged( const CEGUI::EventArgs& arg )
{
    CEGUI::KeyEventArgs& ke = static_cast< CEGUI::KeyEventArgs& >( const_cast< CEGUI::EventArgs& >( arg ) );
    if ( ke.codepoint == SDLK_RETURN )
    {
        // skip empty lines
        if ( !_p_seInputWindow->getText().length() )
            return true;

        seAddOutput( ">" + std::string( _p_seInputWindow->getText().c_str() ) );
        seProcessCmd( _p_seInputWindow->getText().c_str() );
        _p_seInputWindow->setText( "" );
        _p_seOutputWindow->setCaratIndex( _p_seOutputWindow->getText().length() );
    }

    return true;
}

void ScriptEditor::seShow( bool en )
{
    if ( en )
        _p_seScriptWnd->show();
    else
        _p_seScriptWnd->hide();
}

void ScriptEditor::seAddOutput( const std::string& msg )
{
    if ( _p_seOutputWindow )
    {
        _p_seOutputWindow->setText( _p_seOutputWindow->getText() + msg );
        _p_seOutputWindow->setCaratIndex( _p_seOutputWindow->getText().length() );
    }
}

} // namespace vrc
