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
_p_settingsDialog( NULL ),
_p_playername( NULL )
{
}

DialogGameSettings::~DialogGameSettings()
{
    if ( _p_settingsDialog )
        CEGUI::WindowManager::getSingleton().destroyWindow( _p_settingsDialog );
}

bool DialogGameSettings::initialize( const string& layoutfile )
{    

    //MessageBoxDialog* p_msg = new MessageBoxDialog( "hello", "this is a text\nand another text", MessageBoxDialog::YES_NO );
    //p_msg->show();

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

    // set player name
    string plyname;
    Configuration::get()->getSettingValue( CTD_GS_PLAYERNAME, plyname );
    _p_playername->setText( plyname );

    //!TODO rest of settings

    return true;
}

// dialog callbacks
//-----------------
bool DialogGameSettings::onClickedOk( const CEGUI::EventArgs& arg )
{
    string playername = _p_playername->getText().c_str();
    Configuration::get()->setSettingValue( CTD_GS_PLAYERNAME, playername );
    
    return true;
}

bool DialogGameSettings::onClickedCancel( const CEGUI::EventArgs& arg )
{

    return true;
}
//-----------------

void DialogGameSettings::update( float deltaTime )
{
}

void DialogGameSettings::show( bool visible )
{
    if ( visible )
        _p_settingsDialog->show();
    else
        _p_settingsDialog->hide();
}

}
