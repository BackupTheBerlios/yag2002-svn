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
 # dialog constrol for selecting a level
 #
 #   date of creation:  09/29/2007
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_dialoglogin.h"
#include "vrc_menu.h"
#include <storage/vrc_storageclient.h>

namespace vrc
{

// some defines
#define LOGINDLG_PREFIX             "login_"

DialogLogin::DialogLogin( EnMenu* p_menuEntity ) :
_p_loginDialog( NULL ),
_p_login( NULL ),
_p_passwd( NULL ),
_p_menuEntity( p_menuEntity ),
_enable( false )
{
}

DialogLogin::~DialogLogin()
{
    // free up allocated gui elements
    try
    {
        if ( _p_loginDialog )
            CEGUI::WindowManager::getSingleton().destroyWindow( _p_loginDialog );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "DialogLogin: problem destroying level select dialog." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

bool DialogLogin::initialize( const std::string& layoutfile )
{
    _p_loginDialog = yaf3d::GuiManager::get()->loadLayout( layoutfile, NULL, LOGINDLG_PREFIX );
    if ( !_p_loginDialog )
    {
        log_error << "*** DialogLogin: cannot find layout: " << layoutfile << std::endl;
        return false;
    }

    _p_loginDialog->hide();

    try
    {
        // setup login button
        CEGUI::PushButton* p_btnlogin = static_cast< CEGUI::PushButton* >( _p_loginDialog->getChild( LOGINDLG_PREFIX "btn_login" ) );
        p_btnlogin->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::DialogLogin::onClickedLogin, this ) );

        // setup cancel button
        CEGUI::PushButton* p_btncancel = static_cast< CEGUI::PushButton* >( _p_loginDialog->getChild( LOGINDLG_PREFIX "btn_cancel" ) );
        p_btncancel->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::DialogLogin::onClickedCancel, this ) );


        // get the login and passwd edit boxes
        _p_login = static_cast< CEGUI::Editbox* >( _p_loginDialog->getChild( LOGINDLG_PREFIX "text_username" ) );
        _p_passwd = static_cast< CEGUI::Editbox* >( _p_loginDialog->getChild( LOGINDLG_PREFIX "text_password" ) );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "*** DialogLogin: cannot setup dialog layout." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }

    return true;
}

// dialog callbacks
//-----------------
bool DialogLogin::onClickedLogin( const CEGUI::EventArgs& /*arg*/ )
{
    // play mouse click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

    // store back the login name
    std::string cfg_playerrname = _p_login->getText().c_str();
    yaf3d::Configuration::get()->setSettingValue( VRC_GS_PLAYER_NAME, cfg_playerrname );

    _p_menuEntity->onLoginDialogClose( true );

    return true;
}

bool DialogLogin::onClickedCancel( const CEGUI::EventArgs& /*arg*/ )
{
    // play mouse click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

    // let the menu know that the dialog closes
    _p_menuEntity->onLoginDialogClose( false );

    return true;
}

void DialogLogin::enable( bool en )
{
    // just be safe
    if ( en == _enable )
        return;

    if ( en )
    {
        // set the login name to player name
        std::string cfg_playerrname;
        yaf3d::Configuration::get()->getSettingValue( VRC_GS_PLAYER_NAME, cfg_playerrname );
        _p_login->setText( cfg_playerrname );

        _p_loginDialog->activate();
        _p_loginDialog->show();
    }
    else
    {
        _p_loginDialog->hide();
    }

    _enable = en;
}

void DialogLogin::getAndErazeDetails( std::string& login, std::string& passwd )
{
    CEGUI::String::size_type loginlen  = _p_login->getText().length();
    CEGUI::String::size_type passwdlen = _p_passwd->getText().length();

    if ( loginlen )
        login = _p_login->getText().c_str();

    if ( passwdlen )
        passwd = _p_passwd->getText().c_str();

    _p_passwd->setText( ".................." );
    _p_passwd->setText( "" );
    _p_login->setText( ".................." );
    _p_login->setText( "" );
}

} // namespace vrc
