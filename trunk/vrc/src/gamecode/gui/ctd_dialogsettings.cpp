/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2004-2006, Ali Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU General Public License 
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public 
 *  License along with this program; if not, write to the Free 
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 * 
 ****************************************************************/

/*###############################################################
 # settings dialog control
 #
 #   date of creation:  06/05/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_base.h>
#include <ctd_application.h>
#include <ctd_guimsgbox.h>
#include <ctd_guimanager.h>
#include <ctd_configuration.h>
#include <ctd_log.h>
#include "ctd_dialogsettings.h"

using namespace std;

namespace CTD
{

// some defines
#define SDLG_PREFIX     "sd_"

DialogGameSettings::DialogGameSettings() :
_busy( false ),
_p_settingsDialog( NULL ),
_p_playername( NULL ),
_p_mouseSensivity( NULL ),
_mouseSensitivity( 1.0f ),
_mouseInverted( false )
{
}

DialogGameSettings::~DialogGameSettings()
{
    if ( _p_settingsDialog )
        CEGUI::WindowManager::getSingleton().destroyWindow( _p_settingsDialog );

    assert( !_busy && "this object must not be destroyed before the message box has been closed! see method onClickedOk" );
}

bool DialogGameSettings::initialize( const string& layoutfile )
{    
    _p_settingsDialog = GuiManager::get()->loadLayout( layoutfile, NULL, SDLG_PREFIX );
    if ( !_p_settingsDialog )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** DialogGameSettings: cannot find layout: " << layoutfile << endl;
        return false;
    }

    _p_settingsDialog->hide();

    // setup ok button
    CEGUI::PushButton* p_btnok = static_cast< CEGUI::PushButton* >( _p_settingsDialog->getChild( SDLG_PREFIX "btn_ok" ) );
    assert( p_btnok && "DialogGameSettings: cannot find 'btn_ok'" );
    p_btnok->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( DialogGameSettings::onClickedOk, this ) );
   
    // setup cancel button
    CEGUI::PushButton* p_btncancel = static_cast< CEGUI::PushButton* >( _p_settingsDialog->getChild( SDLG_PREFIX "btn_cancel" ) );
    assert( p_btncancel && "DialogGameSettings: cannot find 'btn_cancel'" );
    p_btncancel->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( DialogGameSettings::onClickedCancel, this ) );

    // get player name text box
    _p_playername = static_cast< CEGUI::Editbox* >( _p_settingsDialog->getChild( SDLG_PREFIX "text_playername" ) );
    assert( _p_playername && "DialogGameSettings: cannot find 'text_playername'" );

    // get tab control contents
    //-------------------------
    CEGUI::TabControl* p_tabctrl = static_cast< CEGUI::TabControl* >( _p_settingsDialog->getChild( SDLG_PREFIX "tab_ctrl" ) );
    assert( p_tabctrl && "DialogGameSettings: cannot find tab control 'tab_ctrl'" );

    // get contents of pane Network
    //#############################
    CEGUI::TabPane*    p_paneNetworking = static_cast< CEGUI::TabPane* >( p_tabctrl->getTabContents( SDLG_PREFIX "pane_networking" ) );
    assert( p_paneNetworking && "DialogGameSettings: cannot find tab pane 'pane_networking'" );

    _p_serverName = static_cast< CEGUI::Editbox* >( p_paneNetworking->getChild( SDLG_PREFIX "nw_servername" ) );
    assert( _p_serverName && "DialogGameSettings: cannot find 'nw_servername'" );

    _p_serverIP   = static_cast< CEGUI::Editbox* >( p_paneNetworking->getChild( SDLG_PREFIX "nw_serverip" ) );
    assert( _p_serverIP && "DialogGameSettings: cannot find 'nw_serverip'" );

    _p_serverPort = static_cast< CEGUI::Editbox* >( p_paneNetworking->getChild( SDLG_PREFIX "nw_serverport" ) );
    assert( _p_serverPort && "DialogGameSettings: cannot find 'nw_serverport'" );

    // get contents of pane Control
    //#############################
    CEGUI::TabPane*    p_paneControl = static_cast< CEGUI::TabPane* >( p_tabctrl->getTabContents( SDLG_PREFIX "pane_control" ) );
    assert( p_paneControl && "DialogGameSettings: cannot find tab pane 'pane_control'" );

    // set callback for mouse sensivity scrollbar
    _p_mouseSensivity = static_cast< CEGUI::Scrollbar* >( p_paneControl->getChild( SDLG_PREFIX "sb_mousesensivity" ) );
    assert( _p_mouseSensivity && "DialogGameSettings: cannot find 'sb_mousesensivity'" );
    _p_mouseSensivity->subscribeEvent( CEGUI::Scrollbar::EventMoved, CEGUI::Event::Subscriber( DialogGameSettings::onMouseSensitivityChanged, this ) );

    // setup invert mouse callback
    _p_mouseInvert = static_cast< CEGUI::Checkbox* >( p_paneControl->getChild( SDLG_PREFIX "cbx_mouseinvert" ) );
    assert( _p_mouseInvert && "DialogGameSettings: cannot find 'cbx_mouseinvert'" );

     //!TODO rest of Control settings ( key input map )

    // setup all control contents
    setupControls();

    return true;
}

bool DialogGameSettings::isDirty()
{
    //! TODO: compare all dialog settings with those is game configuration
    // alternatively we could set change callbacks for all dialog controls and set a dirty flag, but that is much more work ;-)

    return true;
}

void DialogGameSettings::setupControls()
{
    // get current configuration settings
    //-----------------------------------
    string cfg_playername;
    Configuration::get()->getSettingValue( CTD_GS_PLAYERNAME, cfg_playername );
    float  cfg_mousesensitivity;
    Configuration::get()->getSettingValue( CTD_GS_MOUSESENS, cfg_mousesensitivity );
    bool   cfg_mouseInverted;
    Configuration::get()->getSettingValue( CTD_GS_INVERTMOUSE, cfg_mouseInverted );
    string cfg_servername;
    Configuration::get()->getSettingValue( CTD_GS_SERVER_NAME, cfg_servername );
    string cfg_serverip;
    Configuration::get()->getSettingValue( CTD_GS_SERVER_IP, cfg_serverip );
    unsigned int cfg_serverport;
    Configuration::get()->getSettingValue( CTD_GS_SERVER_PORT, cfg_serverport );
    //-----------------------------------

    // set player name
    _p_playername->setText( cfg_playername );

    _p_serverName->setText( cfg_servername );
    _p_serverIP->setText( cfg_serverip );
    stringstream portasstring;
    portasstring << cfg_serverport;
    _p_serverPort->setText( portasstring.str() );

    // setup scrollbar position
    _p_mouseSensivity->setDocumentSize( CTD_GS_MAX_MOUSESENS );
    _p_mouseSensivity->setScrollPosition( cfg_mousesensitivity );
    // setup chekbox
    _p_mouseInvert->setSelected( cfg_mouseInverted );
}

// dialog callbacks
//-----------------
bool DialogGameSettings::onClickedOk( const CEGUI::EventArgs& arg )
{
    string playername = _p_playername->getText().c_str();
    Configuration::get()->setSettingValue( CTD_GS_PLAYERNAME, playername );

    Configuration::get()->setSettingValue( CTD_GS_MOUSESENS, _mouseSensitivity );
    
    bool mouseInvert  = _p_mouseInvert->isSelected();
    Configuration::get()->setSettingValue( CTD_GS_INVERTMOUSE, mouseInvert );

    // get server settings
    string servername = _p_serverName->getText().c_str();
    Configuration::get()->setSettingValue( CTD_GS_SERVER_NAME, servername );
    
    string serverip   = _p_serverIP->getText().c_str();
    Configuration::get()->setSettingValue( CTD_GS_SERVER_IP, serverip );

    stringstream portasstring;
    portasstring << _p_serverPort->getText().c_str();
    unsigned int serverport = 0;
    portasstring >> serverport;
    Configuration::get()->setSettingValue( CTD_GS_SERVER_PORT, serverport );

    // store all settings into file
    Configuration::get()->store();
    // disappear the dialog
    show( false );

    return true;
}

bool DialogGameSettings::onClickedCancel( const CEGUI::EventArgs& arg )
{
    if ( !isDirty() )
        return true;

    // ask user for saving changes using a messagebox
    {
        MessageBoxDialog* p_msg = new MessageBoxDialog( "Attention", "Want to save changes?", MessageBoxDialog::YES_NO, true );
        // set busy flag for our object ( DialogGameSettings ). the busy flag is removed then when the message box is terminated
        //  by clicking any button. the object must not be destroyed before that! this flag helps to catch such programming pittfall.
        _busy = true;
        // create a call back for yes/no buttons of messagebox
        class MsgYesNoClick: public MessageBoxDialog::ClickCallback
        {
        public:

                                    MsgYesNoClick( DialogGameSettings* p_dlg ) : _p_dialogSettings( p_dlg ) {}

            virtual                 ~MsgYesNoClick() {}

            void                    onClicked( unsigned int btnId )
                                    {
                                        // did the user clicked yes? if so then store settings
                                        if ( btnId == MessageBoxDialog::BTN_YES )
                                        {
                                            // store the changes
                                            CEGUI::EventArgs arg;
                                            _p_dialogSettings->onClickedOk( arg ); // get gui control values
                                            Configuration::get()->store();         // store configuration to file
                                        }

                                        // release the busy lock
                                        _p_dialogSettings->_busy = false;
                                        // disappear the dialog
                                        _p_dialogSettings->show( false );
                                    }

            DialogGameSettings*     _p_dialogSettings;
        };    
        p_msg->setClickCallback( new MsgYesNoClick( this ) );    
        p_msg->show();
    }

    return true;
}

bool DialogGameSettings::onMouseSensitivityChanged( const CEGUI::EventArgs& arg )
{
    _mouseSensitivity = _p_mouseSensivity->getScrollPosition();
    return true;
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
        _p_settingsDialog->show();
    }
    else
        _p_settingsDialog->hide();
}

}
