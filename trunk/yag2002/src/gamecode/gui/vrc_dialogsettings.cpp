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
 # settings dialog control
 #
 #   date of creation:  04/05/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_menu.h"
#include "vrc_dialogsettings.h"
#include "vrc_dialogplayercfg.h"
#include "../sound/vrc_ambientsound.h"

namespace vrc
{

// some defines
#define SDLG_PREFIX                 "sd_"
#define PLAYER_CONFIG_GUI_LAYOUT    "gui/playerconfig.xml"

DialogGameSettings::DialogGameSettings( EnMenu* p_menuEntity ) :
_busy( false ),
_p_clickSound( NULL ),
_p_settingsDialog( NULL ),
_p_playerName( NULL ),
_p_serverName( NULL ),
_p_serverIP( NULL ),
_p_serverPort( NULL ),
_p_mouseSensivity( NULL ),
_mouseSensitivity( 1.0f ),
_p_keyMoveForward( NULL ),
_p_keyMoveBackward( NULL ),
_p_keyMoveLeft( NULL ),
_p_keyMoveRight( NULL ),
_p_keyCameraMode( NULL ),
_p_keyJump( NULL ),
_p_mouseInvert( NULL ),
_p_keyChatMode( NULL ),
_mouseInverted( false ),
_p_keyKeybEnglish( NULL ),
_p_keyKeybGerman( NULL ),
_p_resolution( NULL ),
_p_fullscreen( NULL ),
_p_wndscreen( NULL ),
_p_menuEntity( p_menuEntity )
{
}

DialogGameSettings::~DialogGameSettings()
{
    if ( _p_settingsDialog )
        CEGUI::WindowManager::getSingleton().destroyWindow( _p_settingsDialog );

    assert( !_busy && "this object must not be destroyed before the message box has been closed! see method onClickedOk" );
}

bool DialogGameSettings::initialize( const std::string& layoutfile )
{    
    _p_settingsDialog = yaf3d::GuiManager::get()->loadLayout( layoutfile, NULL, SDLG_PREFIX );
    if ( !_p_settingsDialog )
    {
        log_error << "*** DialogGameSettings: cannot find layout: " << layoutfile << std::endl;
        return false;
    }

    _p_settingsDialog->hide();

    try
    {
        // setup dialog
        _p_settingsDialog->subscribeEvent( CEGUI::FrameWindow::EventCloseClicked, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onClickedCancel, this ) );

        // setup ok button
        CEGUI::PushButton* p_btnok = static_cast< CEGUI::PushButton* >( _p_settingsDialog->getChild( SDLG_PREFIX "btn_ok" ) );
        p_btnok->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onClickedOk, this ) );

        // setup ok button
        CEGUI::PushButton* p_btnplayercfg = static_cast< CEGUI::PushButton* >( _p_settingsDialog->getChild( SDLG_PREFIX "btn_playercfg" ) );
        p_btnplayercfg->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onClickedPlayerConfig, this ) );

        // setup cancel button
        CEGUI::PushButton* p_btncancel = static_cast< CEGUI::PushButton* >( _p_settingsDialog->getChild( SDLG_PREFIX "btn_cancel" ) );
        p_btncancel->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onClickedCancel, this ) );

        // get player name text box
        _p_playerName = static_cast< CEGUI::Editbox* >( _p_settingsDialog->getChild( SDLG_PREFIX "text_playername" ) );

        // get tab control contents
        //-------------------------
        CEGUI::TabControl* p_tabctrl = static_cast< CEGUI::TabControl* >( _p_settingsDialog->getChild( SDLG_PREFIX "tab_ctrl" ) );
        p_tabctrl->subscribeEvent( CEGUI::TabControl::EventSelectionChanged, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onTabChanged, this ) );

        // get contents of pane Network
        //#############################
        CEGUI::TabPane*    p_paneNetworking = static_cast< CEGUI::TabPane* >( p_tabctrl->getTabContents( SDLG_PREFIX "pane_networking" ) );
        _p_serverName = static_cast< CEGUI::Editbox* >( p_paneNetworking->getChild( SDLG_PREFIX "nw_servername" ) );
        _p_serverIP   = static_cast< CEGUI::Editbox* >( p_paneNetworking->getChild( SDLG_PREFIX "nw_serverip" ) );
        _p_serverPort = static_cast< CEGUI::Editbox* >( p_paneNetworking->getChild( SDLG_PREFIX "nw_serverport" ) );

        // get contents of pane Control
        //#############################
        CEGUI::TabPane*    p_paneControl = static_cast< CEGUI::TabPane* >( p_tabctrl->getTabContents( SDLG_PREFIX "pane_control" ) );

        // key bindings
        //-------------
        _p_keyMoveForward  = static_cast< CEGUI::PushButton* >( p_paneControl->getChild( SDLG_PREFIX "btn_forward" ) );
        _p_keyMoveForward->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onClickedForward, this ) );

        _p_keyMoveBackward = static_cast< CEGUI::PushButton* >( p_paneControl->getChild( SDLG_PREFIX "btn_backward" ) );
        _p_keyMoveBackward->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onClickedBackward, this ) );

        _p_keyMoveLeft     = static_cast< CEGUI::PushButton* >( p_paneControl->getChild( SDLG_PREFIX "btn_left" ) );
        _p_keyMoveLeft->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onClickedLeft, this ) );

        _p_keyMoveRight    = static_cast< CEGUI::PushButton* >( p_paneControl->getChild( SDLG_PREFIX "btn_right" ) );
        _p_keyMoveRight->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onClickedRight, this ) );

        _p_keyJump         = static_cast< CEGUI::PushButton* >( p_paneControl->getChild( SDLG_PREFIX "btn_jump" ) );
        _p_keyJump->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onClickedJump, this ) );

        _p_keyCameraMode   = static_cast< CEGUI::PushButton* >( p_paneControl->getChild( SDLG_PREFIX "btn_camera" ) );
        _p_keyCameraMode->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onClickedCameraMode, this ) );

        _p_keyChatMode     = static_cast< CEGUI::PushButton* >( p_paneControl->getChild( SDLG_PREFIX "btn_chatmode" ) );
        _p_keyChatMode->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onClickedChatMode, this ) );
        //-------------

        // get contents of pane Keyboard
        //#############################
        CEGUI::TabPane*    p_paneKeyboard = static_cast< CEGUI::TabPane* >( p_tabctrl->getTabContents( SDLG_PREFIX "pane_keyboard" ) );

        _p_keyKeybEnglish = static_cast< CEGUI::Checkbox* >( p_paneKeyboard->getChild( SDLG_PREFIX "cb_english" ) );
        _p_keyKeybEnglish->subscribeEvent( CEGUI::Checkbox::EventCheckStateChanged, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onKeyboardEnglishChanged, this ) );

        _p_keyKeybGerman  = static_cast< CEGUI::Checkbox* >( p_paneKeyboard->getChild( SDLG_PREFIX "cb_german" ) );
        _p_keyKeybGerman->subscribeEvent( CEGUI::Checkbox::EventCheckStateChanged, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onKeyboardGermanChanged, this ) );

        //-------------
        // set callback for mouse sensivity scrollbar
        _p_mouseSensivity = static_cast< CEGUI::Scrollbar* >( p_paneControl->getChild( SDLG_PREFIX "sb_mousesensivity" ) );
        _p_mouseSensivity->subscribeEvent( CEGUI::Scrollbar::EventScrollPositionChanged, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onMouseSensitivityChanged, this ) );

        // setup invert mouse callback
        _p_mouseInvert = static_cast< CEGUI::Checkbox* >( p_paneControl->getChild( SDLG_PREFIX "cbx_mouseinvert" ) );

        // get contents of pane Display
        //#############################
        // fill up the resolution combobox
        CEGUI::TabPane*    p_paneDisplay = static_cast< CEGUI::TabPane* >( p_tabctrl->getTabContents( SDLG_PREFIX "pane_display" ) );

        // get fullscreen and windowed checkboxes
        _p_fullscreen = static_cast< CEGUI::RadioButton* >( p_paneDisplay->getChild( SDLG_PREFIX "rb_fullscreen" ) );
        _p_fullscreen->subscribeEvent( CEGUI::RadioButton::EventSelectStateChanged, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onFullscreenChanged, this ) );
        _p_wndscreen = static_cast< CEGUI::RadioButton* >( p_paneDisplay->getChild( SDLG_PREFIX "rb_windowed" ) );
        // get resolution combobox
        _p_resolution = static_cast< CEGUI::Combobox* >( p_paneDisplay->getChild( SDLG_PREFIX "cbox_resolution" ) );
        // enumerate possible screen resolutions
        std::vector< std::string > settings;
        yaf3d::enumerateDisplaySettings( settings, 16 ); // we take settings including above 16 color bits
        for ( size_t cnt = 0; cnt < settings.size(); ++cnt )
        {
            CEGUI::ListboxTextItem* p_item = new CEGUI::ListboxTextItem( settings[ cnt ].c_str() );
            _p_resolution->addItem( p_item );
        }
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "*** DialogGameSettings: cannot setup dialog layout." << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
    }

    // create player config dialog
    _playerConfigDialog = std::auto_ptr< DialogPlayerConfig >( new DialogPlayerConfig( this ) );
    _playerConfigDialog->initialize( PLAYER_CONFIG_GUI_LAYOUT );
    _playerConfigDialog->show( false );

    // setup all control contents
    setupControls();

    return true;
}

void DialogGameSettings::setClickSound( EnAmbientSound* p_sound )
{
    _p_clickSound = p_sound;
    assert( _playerConfigDialog.get() && "game settings dialog is not initialized!" );
    _playerConfigDialog->setClickSound( _p_clickSound );
}

bool DialogGameSettings::isDirty()
{
    // for easiness the settings are always dirty, a real check may be implemented later
    return true;
}

void DialogGameSettings::setupControls()
{
    // get current configuration settings
    //-----------------------------------
    {
        std::string cfg_playername;
        yaf3d::Configuration::get()->getSettingValue( VRC_GS_PLAYER_NAME, cfg_playername );
        // set player name
        _p_playerName->setText( cfg_playername );
    }

    // get network settings
    {
        std::string cfg_servername;
        yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_SERVER_NAME, cfg_servername );
        std::string cfg_serverip;
        yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_SERVER_IP, cfg_serverip );
        unsigned int cfg_serverport;
        yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_SERVER_PORT, cfg_serverport );

        _p_serverName->setText( cfg_servername );
        _p_serverIP->setText( cfg_serverip );
        std::stringstream portasstring;
        portasstring << cfg_serverport;
        _p_serverPort->setText( portasstring.str() );
    }

    // get key bindings
    {
        _keyBindingLookup.clear();
        std::string cfg_movecmd;
        yaf3d::Configuration::get()->getSettingValue( VRC_GS_KEY_MOVE_FORWARD, cfg_movecmd );
        _p_keyMoveForward->setText( cfg_movecmd.c_str() );
        _keyBindingLookup.push_back( std::make_pair( cfg_movecmd, _p_keyMoveForward ) );

        yaf3d::Configuration::get()->getSettingValue( VRC_GS_KEY_MOVE_BACKWARD, cfg_movecmd );
        _p_keyMoveBackward->setText( cfg_movecmd.c_str() );
        _keyBindingLookup.push_back( std::make_pair( cfg_movecmd, _p_keyMoveBackward ) );

        yaf3d::Configuration::get()->getSettingValue( VRC_GS_KEY_MOVE_LEFT, cfg_movecmd );
        _p_keyMoveLeft->setText( cfg_movecmd.c_str() );
        _keyBindingLookup.push_back( std::make_pair( cfg_movecmd, _p_keyMoveLeft ) );

        yaf3d::Configuration::get()->getSettingValue( VRC_GS_KEY_MOVE_RIGHT, cfg_movecmd );
        _p_keyMoveRight->setText( cfg_movecmd.c_str() );
        _keyBindingLookup.push_back( std::make_pair( cfg_movecmd, _p_keyMoveRight ) );

        yaf3d::Configuration::get()->getSettingValue( VRC_GS_KEY_JUMP, cfg_movecmd );
        _p_keyJump->setText( cfg_movecmd.c_str() );
        _keyBindingLookup.push_back( std::make_pair( cfg_movecmd, _p_keyJump ) );

        std::string cfg_mode;
        yaf3d::Configuration::get()->getSettingValue( VRC_GS_KEY_CAMERAMODE, cfg_mode );
        _p_keyCameraMode->setText( cfg_mode.c_str() );
        _keyBindingLookup.push_back( std::make_pair( cfg_mode, _p_keyCameraMode ) );

        yaf3d::Configuration::get()->getSettingValue( VRC_GS_KEY_CHATMODE, cfg_mode );
        _p_keyChatMode->setText( cfg_mode.c_str() );
        _keyBindingLookup.push_back( std::make_pair( cfg_mode, _p_keyChatMode ) );

        float  cfg_mousesensitivity;
        yaf3d::Configuration::get()->getSettingValue( VRC_GS_MOUSESENS, cfg_mousesensitivity );
        bool   cfg_mouseInverted;
        yaf3d::Configuration::get()->getSettingValue( VRC_GS_INVERTMOUSE, cfg_mouseInverted );
        // setup scrollbar position
        _p_mouseSensivity->setDocumentSize( VRC_GS_MAX_MOUSESENS );
        _p_mouseSensivity->setScrollPosition( cfg_mousesensitivity );
        // setup chekbox
        _p_mouseInvert->setSelected( cfg_mouseInverted );
    }

    // get keyboard settings
    {
        std::string cfg_keyboard;
        yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_KEYBOARD, cfg_keyboard );

        if ( cfg_keyboard == YAF3D_GS_KEYBOARD_ENGLISH )
        {
            _p_keyKeybEnglish->setSelected( true );
            _p_keyKeybGerman->setSelected( false );
        }
        else if ( cfg_keyboard == YAF3D_GS_KEYBOARD_GERMAN )
        {
            _p_keyKeybEnglish->setSelected( false );
            _p_keyKeybGerman->setSelected( true );
        }
        else
            log_error << "*** DialogGameSettings: invalid keyboard type: " << cfg_keyboard << std::endl;
    }

    // get display settings
    {
        bool fullscreen;
        yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_FULLSCREEN, fullscreen );
        if ( fullscreen )
        {
            _p_fullscreen->setSelected( true );
            _p_resolution->disable();
        }
        else
        {
            _p_wndscreen->setSelected( true );
        }

        unsigned int width, height, colorbits;
        std::stringstream resolution;
        yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_SCREENWIDTH, width );
        yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_SCREENHEIGHT, height );
        yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_COLORBITS, colorbits );
        resolution << width << "x" << height << "@" << colorbits;
        _p_resolution->setText( resolution.str().c_str() );
    }
}

// dialog callbacks
//-----------------
bool DialogGameSettings::onClickedOk( const CEGUI::EventArgs& arg )
{
    // set player name
    {
        std::string playername = _p_playerName->getText().c_str();
        yaf3d::Configuration::get()->setSettingValue( VRC_GS_PLAYER_NAME, playername );
    }

    // set key bindings
    {
        std::string cfg_key;
        cfg_key = _p_keyMoveForward->getText().c_str();
        yaf3d::Configuration::get()->setSettingValue( VRC_GS_KEY_MOVE_FORWARD, cfg_key );

        cfg_key = _p_keyMoveBackward->getText().c_str();
        yaf3d::Configuration::get()->setSettingValue( VRC_GS_KEY_MOVE_BACKWARD, cfg_key );

        cfg_key = _p_keyMoveLeft->getText().c_str();
        yaf3d::Configuration::get()->setSettingValue( VRC_GS_KEY_MOVE_LEFT, cfg_key );

        cfg_key = _p_keyMoveRight->getText().c_str();
        yaf3d::Configuration::get()->setSettingValue( VRC_GS_KEY_MOVE_RIGHT, cfg_key );

        cfg_key = _p_keyJump->getText().c_str();
        yaf3d::Configuration::get()->setSettingValue( VRC_GS_KEY_JUMP, cfg_key );

        cfg_key = _p_keyCameraMode->getText().c_str();
        yaf3d::Configuration::get()->setSettingValue( VRC_GS_KEY_CAMERAMODE, cfg_key );

        cfg_key = _p_keyChatMode->getText().c_str();
        yaf3d::Configuration::get()->setSettingValue( VRC_GS_KEY_CHATMODE, cfg_key );
    }

    // set mouse settings
    {
        yaf3d::Configuration::get()->setSettingValue( VRC_GS_MOUSESENS, _mouseSensitivity );    
        bool mouseInvert  = _p_mouseInvert->isSelected();
        yaf3d::Configuration::get()->setSettingValue( VRC_GS_INVERTMOUSE, mouseInvert );
    }

    // set server settings
    {
        std::string servername = _p_serverName->getText().c_str();
        yaf3d::Configuration::get()->setSettingValue( YAF3D_GS_SERVER_NAME, servername );

        std::string serverip   = _p_serverIP->getText().c_str();
        yaf3d::Configuration::get()->setSettingValue( YAF3D_GS_SERVER_IP, serverip );

        std::stringstream portasstring;
        portasstring << _p_serverPort->getText().c_str();
        unsigned int serverport = 0;
        portasstring >> serverport;
        yaf3d::Configuration::get()->setSettingValue( YAF3D_GS_SERVER_PORT, serverport );
    }

    // set keyboard type
    {
        std::string cfg_keyboard;
        if ( _p_keyKeybEnglish->isSelected() )
        {
            yaf3d::KeyMap::get()->setup( yaf3d::KeyMap::English ); // re-init keyboard
            cfg_keyboard = YAF3D_GS_KEYBOARD_ENGLISH;
        }
        else
        {
            yaf3d::KeyMap::get()->setup( yaf3d::KeyMap::German ); // re-init keyboard
            cfg_keyboard = YAF3D_GS_KEYBOARD_GERMAN;
        }
        yaf3d::Configuration::get()->setSettingValue( YAF3D_GS_KEYBOARD, cfg_keyboard );
    }

    // set the display setting
    {
        bool fullscreen = _p_fullscreen->isSelected();
        yaf3d::Configuration::get()->setSettingValue( YAF3D_GS_FULLSCREEN, fullscreen );
        if ( !fullscreen )
        {
            unsigned int width, height, colorbits;
            // get the resolution out of the combobox string
            std::string  resstring( _p_resolution->getText().c_str() );
            resstring.replace( resstring.find( "x", 0 ), 1, " " ); // replace x by space so that the stream operator below can work
            resstring.replace( resstring.find( "@", 0 ), 1, " " ); // replace @ by space so that the stream operator below can work
            std::stringstream resolution( resstring );
            resolution >> width >> height >> colorbits;
            yaf3d::Configuration::get()->setSettingValue( YAF3D_GS_SCREENWIDTH, width );
            yaf3d::Configuration::get()->setSettingValue( YAF3D_GS_SCREENHEIGHT, height );
            yaf3d::Configuration::get()->setSettingValue( YAF3D_GS_COLORBITS, colorbits );
        }
    }

    // store all settings into file
    yaf3d::Configuration::get()->store();

    // play click sound
    if ( _p_clickSound )
        _p_clickSound->startPlaying();

    // let the menu know that we are finish
    _p_menuEntity->onSettingsDialogClose();

    return true;
}

bool DialogGameSettings::onClickedPlayerConfig( const CEGUI::EventArgs& arg )
{
    // store the settings changes ( in particular the player name, as the player dialog also can change the player name )
    std::string playername = _p_playerName->getText().c_str();
    yaf3d::Configuration::get()->setSettingValue( VRC_GS_PLAYER_NAME, playername );
    yaf3d::Configuration::get()->store();

    _p_settingsDialog->hide();
    _playerConfigDialog->show( true );

    return true;
}

void DialogGameSettings::onPlayerConfigDialogClose()
{
    _p_settingsDialog->show();
    _playerConfigDialog->show( false );
    // update player name
    std::string playername;
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_PLAYER_NAME, playername );
    _p_playerName->setText( playername.c_str() );
}

bool DialogGameSettings::onClickedCancel( const CEGUI::EventArgs& arg )
{
    // play click sound
    if ( _p_clickSound )
        _p_clickSound->startPlaying();

    if ( !isDirty() )
        return true;

    _p_settingsDialog->disable();

    // ask user for saving changes using a messagebox
    {
        yaf3d::MessageBoxDialog* p_msg = new yaf3d::MessageBoxDialog( "Attention", "You have changed the settings.\nDo you want to save changes?", yaf3d::MessageBoxDialog::YES_NO, true );
        // set busy flag for our object ( DialogGameSettings ). the busy flag is removed then when the message box is terminated
        //  by clicking any button. the object must not be destroyed before that! this flag helps to catch such programming pittfall.
        _busy = true;
        // create a call back for yes/no buttons of messagebox
        class MsgYesNoClick: public yaf3d::MessageBoxDialog::ClickCallback
        {
        public:

                                    MsgYesNoClick( DialogGameSettings* p_dlg ) : _p_dialogSettings( p_dlg ) {}

            virtual                 ~MsgYesNoClick() {}

            void                    onClicked( unsigned int btnId )
                                    {
                                        // did the user clicked yes? if so then store settings
                                        if ( btnId == yaf3d::MessageBoxDialog::BTN_YES )
                                        {
                                            // store the changes
                                            CEGUI::EventArgs earg;
                                            _p_dialogSettings->onClickedOk( earg ); // get gui control values
                                        }
                                        else
                                        {
                                            // just disappear the dialog
                                            _p_dialogSettings->show( false );
                                            _p_dialogSettings->_p_menuEntity->onSettingsDialogClose();

                                        }

                                        // release the busy lock
                                        _p_dialogSettings->_busy = false;
                                        // enable the dialog again
                                        _p_dialogSettings->_p_settingsDialog->enable();

                                        // play click sound
                                        if ( _p_dialogSettings->_p_clickSound )
                                            _p_dialogSettings->_p_clickSound->startPlaying();

                                    }

            DialogGameSettings*     _p_dialogSettings;
        };    
        p_msg->setClickCallback( new MsgYesNoClick( this ) );    
        p_msg->show();
    }

    return true;
}

bool DialogGameSettings::onTabChanged( const CEGUI::EventArgs& arg )
{
    // play click sound
    if ( _p_clickSound )
        _p_clickSound->startPlaying();

    return true;
}

bool DialogGameSettings::onMouseSensitivityChanged( const CEGUI::EventArgs& arg )
{
    _mouseSensitivity = _p_mouseSensivity->getScrollPosition();
    return true;
}

bool DialogGameSettings::onKeyboardEnglishChanged( const CEGUI::EventArgs& arg )
{    
    if ( _p_keyKeybEnglish->isSelected() )
        _p_keyKeybGerman->setSelected( false );
    else
        _p_keyKeybGerman->setSelected( true );

    return true;
}

bool DialogGameSettings::onKeyboardGermanChanged( const CEGUI::EventArgs& arg )
{    
    if ( _p_keyKeybGerman->isSelected() )
        _p_keyKeybEnglish->setSelected( false );
    else
        _p_keyKeybEnglish->setSelected( true );

    return true;
}

bool DialogGameSettings::onClickedForward( const CEGUI::EventArgs& arg )
{
    // begin key sensing for "move forward"
    senseKeybinding( _p_keyMoveForward );
    return true;
}

bool DialogGameSettings::onClickedBackward( const CEGUI::EventArgs& arg )
{
    // begin key sensing for "move backward"
    senseKeybinding( _p_keyMoveBackward );
    return true;
}

bool DialogGameSettings::onClickedLeft( const CEGUI::EventArgs& arg )
{
    // begin key sensing for "move left"
    senseKeybinding( _p_keyMoveLeft );
    return true;
}

bool DialogGameSettings::onClickedRight( const CEGUI::EventArgs& arg )
{
    // begin key sensing for "move right"
    senseKeybinding( _p_keyMoveRight );
    return true;
}

bool DialogGameSettings::onClickedJump( const CEGUI::EventArgs& arg )
{
    // begin key sensing for "jump"
    senseKeybinding( _p_keyJump );
    return true;
}

bool DialogGameSettings::onClickedCameraMode( const CEGUI::EventArgs& arg )
{
    // begin key sensing for "camera mode"
    senseKeybinding( _p_keyCameraMode );
    return true;
}

bool DialogGameSettings::onClickedChatMode( const CEGUI::EventArgs& arg )
{
    // begin key sensing for "chat mode"
    senseKeybinding( _p_keyChatMode );
    return true;
}

bool DialogGameSettings::onFullscreenChanged( const CEGUI::EventArgs& arg )
{
    if ( _p_fullscreen->isSelected() ) 
    {
        _p_resolution->disable();
    }
    else
    {
        _p_resolution->enable();
    }

    return true;
}

void DialogGameSettings::senseKeybinding( CEGUI::PushButton* p_btn )
{    
    // disable dialog so only the key sensing will be active
    _p_settingsDialog->disable();
    new BtnInputHandler( p_btn, this );
}

void BtnInputHandler::updateBindings( const std::string newkey )
{
    // look for overriding key binding
    //--------------------------------
    DialogGameSettings::tBindingLookup::iterator 
        p_beg = _p_dlg->_keyBindingLookup.begin(), 
        p_end = _p_dlg->_keyBindingLookup.end(),
        p_we;

    for ( ; p_beg != p_end; ++p_beg )
    {
        if ( p_beg->second == _p_userObject )
        {
            p_we = p_beg;
            continue;
        }
        if ( p_beg->first == newkey )
        {
            p_beg->second->setText( "..." );
            p_beg->first = "...";                    
        }
    }
    p_we->first = newkey;
    //-------------------
    getUserObject()->setText( newkey );
}

bool BtnInputHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
    // we take only the first event for key sensing
    if ( _lockInput )
        return false;

    const osgSDL::SDLEventAdapter* p_eventAdapter = dynamic_cast< const osgSDL::SDLEventAdapter* >( &ea );
    assert( p_eventAdapter && "invalid event adapter received" );

    unsigned int     eventType = p_eventAdapter->getEventType();
    Uint16           key       = p_eventAdapter->getSDLKey();
    // dispatch key activity
    if ( eventType == osgGA::GUIEventAdapter::KEYDOWN )
    {
        if ( key != yaf3d::KeyMap::get()->getKeyCode( "Esc" ) )
        {            
            std::string curkey = yaf3d::KeyMap::get()->getKeyName( key );
            updateBindings( curkey ); // update all bindings, handle overriding exsiting binding
            _p_dlg->_p_settingsDialog->enable();
            _lockInput = true;
            destroyHandler();
        }
        return false;
    }

    // check for mouse buttons
    unsigned int buttonMask = ea.getButtonMask();
    // left mouse button
    if ( buttonMask & 
        ( 
        osgGA::GUIEventAdapter::LEFT_MOUSE_BUTTON   |
        osgGA::GUIEventAdapter::MIDDLE_MOUSE_BUTTON |
        osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON 
        ) )

    {
        std::string curkey( yaf3d::KeyMap::get()->getMouseButtonName( buttonMask ) );
        updateBindings( curkey ); // update all bindings, handle overriding exsiting binding
        _p_dlg->_p_settingsDialog->enable();
        _lockInput = true;
        destroyHandler();
    }
    return false; // do not consume any key codes!
}

//-----------------

void DialogGameSettings::update( float deltaTime )
{
}

void DialogGameSettings::show( bool visible )
{
    if ( visible )
    {
        setupControls();
        _p_settingsDialog->enable();
        _p_settingsDialog->show();
    }
    else
    {
        _p_settingsDialog->hide();
    }
}

}
