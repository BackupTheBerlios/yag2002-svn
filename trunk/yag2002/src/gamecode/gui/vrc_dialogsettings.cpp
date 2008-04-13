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
 # settings dialog control
 #
 #   date of creation:  04/05/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_menu.h"
#include "vrc_dialogsettings.h"
#include "vrc_dialogplayercfg.h"
#include "../sound/vrc_2dsound.h"
#include "vrc_microinput.h"

namespace vrc
{

// some defines
#define SDLG_PREFIX                 "sd_"
#define PLAYER_CONFIG_GUI_LAYOUT    "gui/playerconfig.xml"

DialogGameSettings::DialogGameSettings( EnMenu* p_menuEntity ) :
_busy( false ),
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
_p_resolution( NULL ),
_p_enableFullscreen( NULL ),
_p_enableDynShadow( NULL ),
_p_enableMusic( NULL ),
_p_volumeMusic( NULL ),
_volumeMusic( 1.0f ),
_p_enableFX( NULL ),
_p_volumeFX( NULL ),
_volumeFX( 1.0f ),
_p_enableVoiceChat( NULL ),
_inputDevices( NULL ),
_p_voiceInputGain( NULL ),
_voiceInputGain( 1.0f ),
_p_voiceOutputGain( NULL ),
_p_enablePortForwarding( NULL ),
_p_portForwarding( NULL ),
_voiceOutputGain( 1.0f ),
_p_microInput( NULL ),
_p_menuEntity( p_menuEntity ),
_cfgShadows( false )
{
}

DialogGameSettings::~DialogGameSettings()
{
    if ( _p_settingsDialog )
        CEGUI::WindowManager::getSingleton().destroyWindow( _p_settingsDialog );

    // store the dynamic shadow settings on shutdown
    yaf3d::Configuration::get()->setSettingValue( YAF3D_GS_SHADOW_ENABLE, _cfgShadows );

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

    CEGUI::PushButton* p_inputtest = NULL;

    try
    {
        // setup dialog
        _p_settingsDialog->subscribeEvent( CEGUI::FrameWindow::EventCloseClicked, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onClickedCancel, this ) );

        // setup ok button
        CEGUI::PushButton* p_btnok = static_cast< CEGUI::PushButton* >( _p_settingsDialog->getChild( SDLG_PREFIX "btn_ok" ) );
        p_btnok->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onClickedOk, this ) );

        // set a keydown callback for handling Return key ( the action is the same as clicking on Ok button )
        _p_settingsDialog->subscribeEvent( CEGUI::PushButton::EventKeyDown, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onKeyDown, this ) );

        // setup player configuration button
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
        {
            CEGUI::TabPane*    p_paneNetworking = static_cast< CEGUI::TabPane* >( p_tabctrl->getTabContents( SDLG_PREFIX "pane_networking" ) );
            _p_serverName = static_cast< CEGUI::Editbox* >( p_paneNetworking->getChild( SDLG_PREFIX "nw_servername" ) );
            _p_serverIP   = static_cast< CEGUI::Editbox* >( p_paneNetworking->getChild( SDLG_PREFIX "nw_serverip" ) );
            _p_serverPort = static_cast< CEGUI::Editbox* >( p_paneNetworking->getChild( SDLG_PREFIX "nw_serverport" ) );
        }

        // get contents of pane Control
        //#############################
        {
            CEGUI::TabPane* p_paneControl = static_cast< CEGUI::TabPane* >( p_tabctrl->getTabContents( SDLG_PREFIX "pane_control" ) );

            _p_keyMoveForward  = static_cast< CEGUI::PushButton* >( p_paneControl->getChild( SDLG_PREFIX "btn_forward" ) );

            _p_keyMoveBackward = static_cast< CEGUI::PushButton* >( p_paneControl->getChild( SDLG_PREFIX "btn_backward" ) );

            _p_keyMoveLeft     = static_cast< CEGUI::PushButton* >( p_paneControl->getChild( SDLG_PREFIX "btn_left" ) );

            _p_keyMoveRight    = static_cast< CEGUI::PushButton* >( p_paneControl->getChild( SDLG_PREFIX "btn_right" ) );

            _p_keyJump         = static_cast< CEGUI::PushButton* >( p_paneControl->getChild( SDLG_PREFIX "btn_jump" ) );

            _p_keyCameraMode   = static_cast< CEGUI::PushButton* >( p_paneControl->getChild( SDLG_PREFIX "btn_camera" ) );
            _p_keyCameraMode->setVisible( false );
            _p_keyCameraMode->disable();

            _p_keyChatMode     = static_cast< CEGUI::PushButton* >( p_paneControl->getChild( SDLG_PREFIX "btn_chatmode" ) );
            
            _p_mouseSensivity = static_cast< CEGUI::Scrollbar* >( p_paneControl->getChild( SDLG_PREFIX "sb_mousesensivity" ) );
            
            _p_mouseInvert = static_cast< CEGUI::Checkbox* >( p_paneControl->getChild( SDLG_PREFIX "cbx_mouseinvert" ) );
        }

        // get contents of pane Display
        //#############################
        {
            CEGUI::TabPane* p_paneDisplay = static_cast< CEGUI::TabPane* >( p_tabctrl->getTabContents( SDLG_PREFIX "pane_display" ) );

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

            // get fullscreen checkbox
            _p_enableFullscreen = static_cast< CEGUI::Checkbox* >( p_paneDisplay->getChild( SDLG_PREFIX "cb_fullscreen" ) );

            // get dynamic shadow checkbox
            _p_enableDynShadow = static_cast< CEGUI::Checkbox* >( p_paneDisplay->getChild( SDLG_PREFIX "cb_shadows" ) );

            // dynamic shadows need glsl, set the checkbox initial value
            yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_SHADOW_ENABLE, _cfgShadows );
            if ( yaf3d::isGlslAvailable() )
            {
                if ( _cfgShadows )
                    _p_enableDynShadow->setSelected( true );
                else
                    _p_enableDynShadow->setSelected( false );
            }
            else
            {
                _p_enableDynShadow->setSelected( false );
                _p_enableDynShadow->disable();
            }
        }

        // get contents of pane Sound/Voice
        //#############################
        {
            CEGUI::TabPane* p_paneSoundVoice = static_cast< CEGUI::TabPane* >( p_tabctrl->getTabContents( SDLG_PREFIX "pane_soundvoice" ) );

            // get music on/off
            _p_enableMusic = static_cast< CEGUI::Checkbox* >( p_paneSoundVoice->getChild( SDLG_PREFIX "cbx_music" ) );
            // music volume
            _p_volumeMusic = static_cast< CEGUI::Scrollbar* >( p_paneSoundVoice->getChild( SDLG_PREFIX "sb_musicvolume" ) );
            // get fx on/off
            _p_enableFX = static_cast< CEGUI::Checkbox* >( p_paneSoundVoice->getChild( SDLG_PREFIX "cbx_fx" ) );
            // fx volume
            _p_volumeFX = static_cast< CEGUI::Scrollbar* >( p_paneSoundVoice->getChild( SDLG_PREFIX "sb_fxvolume" ) );

            // get voice chat on/off
            _p_enableVoiceChat = static_cast< CEGUI::Checkbox* >( p_paneSoundVoice->getChild( SDLG_PREFIX "cbx_voicechat" ) );

            // input devices
            _inputDevices = static_cast< CEGUI::Combobox* >( p_paneSoundVoice->getChild( SDLG_PREFIX "cbox_voiceinputdevice" ) );

            // input gain
            _p_voiceInputGain = static_cast< CEGUI::Scrollbar* >( p_paneSoundVoice->getChild( SDLG_PREFIX "sb_voiceinputgain" ) );
            _p_voiceInputGain->setDocumentSize( 1.0f ); // the scrollbar works in range 0...1

            // output gain
            _p_voiceOutputGain = static_cast< CEGUI::Scrollbar* >( p_paneSoundVoice->getChild( SDLG_PREFIX "sb_voiceoutputgain" ) );
            _p_voiceOutputGain->setDocumentSize( 1.0f ); // the scrollbar works in range 0...1

            p_inputtest = static_cast< CEGUI::PushButton* >( p_paneSoundVoice->getChild( SDLG_PREFIX "btn_voicetest" ) );
            _p_enablePortForwarding = static_cast< CEGUI::Checkbox* >( p_paneSoundVoice->getChild( SDLG_PREFIX "cbx_portforward" ) );
            _p_portForwarding = static_cast< CEGUI::Editbox* >( p_paneSoundVoice->getChild( SDLG_PREFIX "text_portforwarding" ) );
        }
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "*** DialogGameSettings: cannot setup dialog layout." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }

    // create player config dialog
    _playerConfigDialog = std::auto_ptr< DialogPlayerConfig >( new DialogPlayerConfig( this ) );
    _playerConfigDialog->initialize( PLAYER_CONFIG_GUI_LAYOUT );
    _playerConfigDialog->show( false );

    // setup all control contents
    setupControls();


    // now register the event callbacks; note: we do that after setting up the controls, otherwise all sounds get activated on setup!

    _p_keyMoveForward->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onClickedForward, this ) );
    _p_keyMoveBackward->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onClickedBackward, this ) );
    _p_keyMoveLeft->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onClickedLeft, this ) );
    _p_keyMoveRight->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onClickedRight, this ) );
    _p_keyJump->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onClickedJump, this ) );
    //! NOTE: the camera switch mode is disabled until we have implemented a camera physics in order to avoid going through meshes!
    //_p_keyCameraMode->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onClickedCameraMode, this ) );
    _p_keyChatMode->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onClickedChatMode, this ) );
    _p_mouseSensivity->subscribeEvent( CEGUI::Scrollbar::EventScrollPositionChanged, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onMouseSensitivityChanged, this ) );
    _p_enableFullscreen->subscribeEvent( CEGUI::Checkbox::EventCheckStateChanged, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onFullscreenChanged, this ) );
    _p_enableDynShadow->subscribeEvent( CEGUI::Checkbox::EventCheckStateChanged, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onDynShadowChanged, this ) );

    _p_enableMusic->subscribeEvent( CEGUI::Checkbox::EventCheckStateChanged, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onEnableMusicChanged, this ) );
    _p_volumeMusic->subscribeEvent( CEGUI::Scrollbar::EventScrollPositionChanged, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onMusicVolumeChanged, this ) );
    _p_enableFX->subscribeEvent( CEGUI::Checkbox::EventCheckStateChanged, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onEnableFXChanged, this ) );
    _p_volumeFX->subscribeEvent( CEGUI::Scrollbar::EventScrollPositionChanged, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onFXVolumeChanged, this ) );
    _p_enableVoiceChat->subscribeEvent( CEGUI::Checkbox::EventCheckStateChanged, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onEnableVoiceChatChanged, this ) );
    _inputDevices->subscribeEvent( CEGUI::Combobox::EventListSelectionAccepted, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onVoiceInputDeviceChanged, this ) );
    _p_voiceInputGain->subscribeEvent( CEGUI::Scrollbar::EventScrollPositionChanged, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onVoiceInputGainChanged, this ) );
    _p_voiceOutputGain->subscribeEvent( CEGUI::Scrollbar::EventScrollPositionChanged, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onVoiceOutputGainChanged, this ) );
    p_inputtest->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onVoiceTestClicked, this ) );
    _p_enablePortForwarding->subscribeEvent( CEGUI::Checkbox::EventCheckStateChanged, CEGUI::Event::Subscriber( &vrc::DialogGameSettings::onEnablePortForwardingChanged, this ) );

    return true;
}

bool DialogGameSettings::isDirty()
{
    // for easiness the settings are always dirty, a real check may be implemented later
    return true;
}

void DialogGameSettings::updateInputDeviceList()
{
    _inputDevices->setEnabled( true );
    _inputDevices->resetList();
    _inputDevices->setSortingEnabled( false );

    CEGUI::ListboxTextItem* p_item = NULL;

    // determine all available input devices
    MicrophoneInput* p_micinput = new MicrophoneInput;
    MicrophoneInput::InputDeviceMap inputs;
    if ( p_micinput->getInputDevices( inputs ) )
    {
        for ( int numdevs = inputs.size() - 1; numdevs >= 0 ; --numdevs )
        {
            p_item = new CEGUI::ListboxTextItem( inputs[ numdevs ].c_str() );
            _inputDevices->insertItem( p_item, NULL );
        }
    }
    else
    {
        log_error << "cannot initialize microphone input detection" << std::endl;
    }
    // release the micro input object
    delete p_micinput;

    // set the right device in drop-down list
    unsigned int inputdevice;
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_VOICECHAT_INPUT_DEVICE, inputdevice );
    if ( inputdevice < inputs.size() )
    {
        _inputDevices->setItemSelectState( inputdevice, true );
        _inputDevices->setText( _inputDevices->getListboxItemFromIndex( inputdevice )->getText() );
    }
    else
    {
        _inputDevices->setSelection( 0, 0 );
    }
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

    // get display settings
    {
        bool fullscreen;
        yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_FULLSCREEN, fullscreen );
        if ( fullscreen )
            _p_enableFullscreen->setSelected( true );
        else
            _p_enableFullscreen->setSelected( false );

        unsigned int width, height, colorbits;
        std::stringstream resolution;
        yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_SCREENWIDTH, width );
        yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_SCREENHEIGHT, height );
        yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_COLORBITS, colorbits );
        resolution << width << "x" << height << "@" << colorbits;
        _p_resolution->setText( resolution.str().c_str() );
    }

    // get sound/voice settings
    {
        // Music
        bool musicenable;
        yaf3d::Configuration::get()->getSettingValue( VRC_GS_MUSIC_ENABLE, musicenable );
        _p_enableMusic->setSelected( musicenable );

        yaf3d::Configuration::get()->getSettingValue( VRC_GS_MUSIC_VOLUME, _volumeMusic );
        _p_volumeMusic->setScrollPosition( _volumeMusic );

        if ( musicenable )
            yaf3d::SoundManager::get()->setGroupVolume( yaf3d::SoundManager::SoundGroupMusic, _volumeMusic );
        else
            yaf3d::SoundManager::get()->setGroupVolume( yaf3d::SoundManager::SoundGroupMusic, 0.0f );

        if ( !musicenable )
            _p_volumeMusic->setEnabled( false );
        else
            _p_volumeMusic->setEnabled( true );

        // FX
        bool fxenable;
        yaf3d::Configuration::get()->getSettingValue( VRC_GS_FX_ENABLE, fxenable );
        _p_enableFX->setSelected( fxenable );

        yaf3d::Configuration::get()->getSettingValue( VRC_GS_FX_VOLUME, _volumeFX );
        _p_volumeFX->setScrollPosition( _volumeFX );

        if ( fxenable )
            yaf3d::SoundManager::get()->setGroupVolume( yaf3d::SoundManager::SoundGroupFX, _volumeFX );
        else
            yaf3d::SoundManager::get()->setGroupVolume( yaf3d::SoundManager::SoundGroupFX, 0.0f );

        _p_volumeFX->setEnabled( fxenable );


        // voice chat

        bool voiceenable;
        yaf3d::Configuration::get()->getSettingValue( VRC_GS_VOICECHAT_ENABLE, voiceenable );
        _p_enableVoiceChat->setSelected( voiceenable );

        // setup input device combo box
        if ( voiceenable )
        {
            updateInputDeviceList();
        }
        else
        {
            _inputDevices->setEnabled( false );
        }

        yaf3d::Configuration::get()->getSettingValue( VRC_GS_VOICE_INPUT_GAIN, _voiceInputGain );
        _p_voiceInputGain->setScrollPosition( _voiceInputGain );
        yaf3d::Configuration::get()->getSettingValue( VRC_GS_VOICE_OUTPUT_GAIN, _voiceOutputGain );
        _p_voiceOutputGain->setScrollPosition( _voiceOutputGain );

        if ( voiceenable )
        {
            _p_voiceInputGain->setEnabled( true );
            _p_voiceOutputGain->setEnabled( true );
        }
        else
        {
            _p_voiceInputGain->setEnabled( false );
            _p_voiceOutputGain->setEnabled( false );
        }

        // setup the port forwarding controls
        std::stringstream port;
        unsigned int      portnum;
        bool              portfw = false;
        yaf3d::Configuration::get()->getSettingValue( VRC_GS_VOICE_IP_FWD_PORT, portnum );
        port << portnum;
        _p_portForwarding->setText( ( std::string( "      " ) + port.str() ) );

        yaf3d::Configuration::get()->getSettingValue( VRC_GS_VOICE_IP_FWD, portfw );
        if ( portfw )
        {
            _p_enablePortForwarding->setSelected( true );
            _p_portForwarding->enable();
        }
        else
        {
            _p_enablePortForwarding->setSelected( false );
            _p_portForwarding->disable();
        }
    }
}

// dialog callbacks
//-----------------
bool DialogGameSettings::onClickedOk( const CEGUI::EventArgs& /*arg*/ )
{
    // set player name
    {
        std::string playername( _p_playerName->getText().c_str() );
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

    // set the display setting
    {
        bool fullscreen = _p_enableFullscreen->isSelected();
        yaf3d::Configuration::get()->setSettingValue( YAF3D_GS_FULLSCREEN, fullscreen );

        // we store this flag on application exit, so it takes effect next time when the app starts
        _cfgShadows = _p_enableDynShadow->isSelected();

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

    // set sound/voice settings
    {
        bool musicenable = _p_enableMusic->isSelected();
        yaf3d::Configuration::get()->setSettingValue( VRC_GS_MUSIC_ENABLE, musicenable );
        yaf3d::Configuration::get()->setSettingValue( VRC_GS_MUSIC_VOLUME, _volumeMusic );

        bool fxenable = _p_enableFX->isSelected();
        yaf3d::Configuration::get()->setSettingValue( VRC_GS_FX_ENABLE, fxenable );
        yaf3d::Configuration::get()->setSettingValue( VRC_GS_FX_VOLUME, _volumeFX );

        bool voiceenable = _p_enableVoiceChat->isSelected();
        yaf3d::Configuration::get()->setSettingValue( VRC_GS_VOICECHAT_ENABLE, voiceenable );

        CEGUI::ListboxItem* p_item = _inputDevices->getSelectedItem();
        if ( p_item )
        {
            unsigned int inputdevice = _inputDevices->getItemIndex( p_item );
            yaf3d::Configuration::get()->setSettingValue( VRC_GS_VOICECHAT_INPUT_DEVICE, inputdevice );
        }

        yaf3d::Configuration::get()->setSettingValue( VRC_GS_VOICE_INPUT_GAIN, _voiceInputGain );
        yaf3d::Configuration::get()->setSettingValue( VRC_GS_VOICE_OUTPUT_GAIN, _voiceOutputGain );

        // release micro input object
        if ( _p_microInput )
        {
            delete _p_microInput;
            _p_microInput = NULL;
        }

        // store the port forwarding controls
        std::string port;
        if ( _p_portForwarding->getText().length() )
            port = _p_portForwarding->getText().c_str();
        else
            port = "      32210";

        std::stringstream p;
        unsigned int portnum;
        p << port;
        p >> portnum;
        yaf3d::Configuration::get()->setSettingValue( VRC_GS_VOICE_IP_FWD_PORT, portnum );

        bool enabled = _p_enablePortForwarding->isSelected();
        yaf3d::Configuration::get()->setSettingValue( VRC_GS_VOICE_IP_FWD, enabled );
    }

    // store all settings into file
    yaf3d::Configuration::get()->store();

    // play mouse click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

    // let the menu know that we are finish
    _p_menuEntity->onSettingsDialogClose();

    return true;
}

bool DialogGameSettings::onKeyDown( const CEGUI::EventArgs& arg )
{
    // check for 'Return' key
    CEGUI::KeyEventArgs& ke = static_cast< CEGUI::KeyEventArgs& >( const_cast< CEGUI::EventArgs& >( arg ) );
    if ( ( ke.codepoint == SDLK_RETURN ) || ( ke.scancode == CEGUI::Key::Return ) )
    {
        onClickedOk( arg );
    }

    return true;
}

bool DialogGameSettings::onClickedPlayerConfig( const CEGUI::EventArgs& /*arg*/ )
{
    // store the settings changes ( in particular the player name, as the player dialog also can change the player name )
    std::string playername( _p_playerName->getText().c_str() );
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

bool DialogGameSettings::onClickedCancel( const CEGUI::EventArgs& /*arg*/ )
{
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

                                        // play mouse click sound
                                        vrc::gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

                                    }

            DialogGameSettings*     _p_dialogSettings;
        };
        p_msg->setClickCallback( new MsgYesNoClick( this ) );
        p_msg->show();

        // play attention sound
        vrc::gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_ATTENTION );
    }

    // release micro input object
    if ( _p_microInput )
    {
        delete _p_microInput;
        _p_microInput = NULL;
    }

    return true;
}

bool DialogGameSettings::onTabChanged( const CEGUI::EventArgs& /*arg*/ )
{
    // play mouse click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );
    return true;
}

bool DialogGameSettings::onMouseSensitivityChanged( const CEGUI::EventArgs& /*arg*/ )
{
    // play scroll sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_SCROLLBAR );

    _mouseSensitivity = _p_mouseSensivity->getScrollPosition();
    return true;
}

bool DialogGameSettings::onClickedForward( const CEGUI::EventArgs& /*arg*/ )
{
    // begin key sensing for "move forward"
    senseKeybinding( _p_keyMoveForward );
    return true;
}

bool DialogGameSettings::onClickedBackward( const CEGUI::EventArgs& /*arg*/ )
{
    // begin key sensing for "move backward"
    senseKeybinding( _p_keyMoveBackward );
    return true;
}

bool DialogGameSettings::onClickedLeft( const CEGUI::EventArgs& /*arg*/ )
{
    // begin key sensing for "move left"
    senseKeybinding( _p_keyMoveLeft );
    return true;
}

bool DialogGameSettings::onClickedRight( const CEGUI::EventArgs& /*arg*/ )
{
    // begin key sensing for "move right"
    senseKeybinding( _p_keyMoveRight );
    return true;
}

bool DialogGameSettings::onClickedJump( const CEGUI::EventArgs& /*arg*/ )
{
    // begin key sensing for "jump"
    senseKeybinding( _p_keyJump );
    return true;
}

bool DialogGameSettings::onClickedCameraMode( const CEGUI::EventArgs& /*arg*/ )
{
    // begin key sensing for "camera mode"
    senseKeybinding( _p_keyCameraMode );
    return true;
}

bool DialogGameSettings::onClickedChatMode( const CEGUI::EventArgs& /*arg*/ )
{
    // begin key sensing for "chat mode"
    senseKeybinding( _p_keyChatMode );
    return true;
}

bool DialogGameSettings::onFullscreenChanged( const CEGUI::EventArgs& /*arg*/ )
{
    // play sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );
    return true;
}

bool DialogGameSettings::onDynShadowChanged( const CEGUI::EventArgs& /*arg*/ )
{
    // play sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );
    return true;
}

bool DialogGameSettings::onEnableMusicChanged( const CEGUI::EventArgs& /*arg*/ )
{
    // play mouse click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

    if ( _p_enableMusic->isSelected() )
    {
        _volumeMusic = _p_volumeMusic->getScrollPosition();
        _p_volumeMusic->setEnabled( true );
        yaf3d::SoundManager::get()->setGroupVolume( yaf3d::SoundManager::SoundGroupMusic, _volumeMusic );
        _p_menuEntity->setBkgMusicEnable( true );
        _p_menuEntity->setBkgMusicVolume( _volumeMusic );
    }
    else
    {
        _p_volumeMusic->setEnabled( false );
        yaf3d::SoundManager::get()->setGroupVolume( yaf3d::SoundManager::SoundGroupMusic, 0.0f );
        _p_menuEntity->setBkgMusicEnable( false );
        _p_menuEntity->setBkgMusicVolume( 0.0f );
    }
    return true;
}

bool DialogGameSettings::onMusicVolumeChanged( const CEGUI::EventArgs& /*arg*/ )
{
    // play scroll sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_SCROLLBAR );

    _volumeMusic = _p_volumeMusic->getScrollPosition();
    yaf3d::SoundManager::get()->setGroupVolume( yaf3d::SoundManager::SoundGroupMusic, _volumeMusic );

    if ( _p_enableMusic->isSelected() )
        _p_menuEntity->setBkgMusicVolume( _volumeMusic );

    return true;
}

bool DialogGameSettings::onEnableFXChanged( const CEGUI::EventArgs& /*arg*/ )
{
    // play mouse click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

    if ( _p_enableFX->isSelected() )
    {
        _p_volumeFX->setEnabled( true );
        yaf3d::SoundManager::get()->setGroupVolume( yaf3d::SoundManager::SoundGroupFX, _volumeFX );
    }
    else
    {
        _p_volumeFX->setEnabled( false );
        yaf3d::SoundManager::get()->setGroupVolume( yaf3d::SoundManager::SoundGroupFX, 0.0f );
    }
    return true;
}

bool DialogGameSettings::onFXVolumeChanged( const CEGUI::EventArgs& /*arg*/ )
{
    // play scroll sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_SCROLLBAR );

    _volumeFX = _p_volumeFX->getScrollPosition();
    yaf3d::SoundManager::get()->setGroupVolume( yaf3d::SoundManager::SoundGroupFX, _volumeFX );

    return true;
}

bool DialogGameSettings::onEnableVoiceChatChanged( const CEGUI::EventArgs& /*arg*/ )
{
    // play mouse click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

    if ( _p_enableVoiceChat->isSelected() )
    {
        _p_voiceInputGain->setEnabled( true );
        _p_voiceOutputGain->setEnabled( true );
        _inputDevices->setEnabled( true );
        updateInputDeviceList();
    }
    else
    {
        _p_voiceInputGain->setEnabled( false );
        _p_voiceOutputGain->setEnabled( false );
        _inputDevices->setEnabled( false );
    }
    return true;
}

bool DialogGameSettings::onVoiceInputDeviceChanged( const CEGUI::EventArgs& /*arg*/ )
{
    // play mouse click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

    if ( _p_microInput )
    {
        CEGUI::ListboxItem* p_item = _inputDevices->getSelectedItem();
        if ( p_item )
        {
            unsigned int inputdevice = _inputDevices->getItemIndex( p_item );
            _p_microInput->setInputDevice( inputdevice );
        }
    }

    return true;
}

bool DialogGameSettings::onVoiceInputGainChanged( const CEGUI::EventArgs& /*arg*/ )
{
    // play scroll sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_SCROLLBAR );

    _voiceInputGain = _p_voiceInputGain->getScrollPosition();
    if ( _p_microInput )
    {
        _p_microInput->setInputGain( _voiceInputGain );
    }

    return true;
}

bool DialogGameSettings::onVoiceTestClicked( const CEGUI::EventArgs& /*arg*/ )
{
    if ( !_p_microInput )
    {
        _p_microInput = new MicrophoneInput;
        unsigned int inputdevice;
        yaf3d::Configuration::get()->getSettingValue( VRC_GS_VOICECHAT_INPUT_DEVICE, inputdevice );
        if ( _p_microInput->setInputDevice( inputdevice ) )
        {
            _p_microInput->beginMicroTest();
        }
        else
        {
            log_error << "gui: cannot set input device" << std::endl;
            delete _p_microInput;
            _p_microInput = NULL;
        }
    }
    else
    {
        delete _p_microInput;
        _p_microInput = NULL;
    }

    return true;
}

bool DialogGameSettings::onVoiceOutputGainChanged( const CEGUI::EventArgs& /*arg*/ )
{
    // play scroll sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_SCROLLBAR );

    _voiceOutputGain = _p_voiceOutputGain->getScrollPosition();

    if ( _p_microInput )
    {
        _p_microInput->setOutputGain( _voiceOutputGain );
    }

    return true;
}

bool DialogGameSettings::onEnablePortForwardingChanged( const CEGUI::EventArgs& /*arg*/ )
{
    // play scroll sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );
    if ( _p_enablePortForwarding->isSelected() )
    {
        _p_portForwarding->enable();
    }
    else
    {
        _p_portForwarding->disable();
    }

    return true;
}

void DialogGameSettings::senseKeybinding( CEGUI::PushButton* p_btn )
{
    // play mouse click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

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

bool BtnInputHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*aa*/ )
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
    unsigned int buttonMask = p_eventAdapter->getButtonMask();
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

void DialogGameSettings::update( float /*deltaTime*/ )
{
}

void DialogGameSettings::show( bool visible )
{
    if ( visible )
    {
        setupControls();
        _p_settingsDialog->activate();
        _p_settingsDialog->show();
    }
    else
    {
        _p_settingsDialog->hide();
        _playerConfigDialog->show( false );
    }
}

}
