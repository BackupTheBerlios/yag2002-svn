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
 #   author:            boto (botorabi at users.sourceforge.net)
 #
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
#define REGDLG_PREFIX               "reg_"

#define DLG_REGISTRATION_FILE       "gui/registration.xml"

DialogLogin::DialogLogin( EnMenu* p_menuEntity ) :
_p_loginDialog( NULL ),
_p_registrationDialog( NULL ),
_p_login( NULL ),
_p_passwd( NULL ),
_p_regNickName( NULL ),
_p_regRealName( NULL ),
_p_regPasswd( NULL ),
_p_regEmail( NULL ),
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

        if ( _p_registrationDialog )
            CEGUI::WindowManager::getSingleton().destroyWindow( _p_registrationDialog );

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

        // setup create account button
        CEGUI::PushButton* p_btnreg = static_cast< CEGUI::PushButton* >( _p_loginDialog->getChild( LOGINDLG_PREFIX "btn_create_account" ) );
        p_btnreg->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::DialogLogin::onClickedRegistration, this ) );

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

bool DialogLogin::onClickedRegistration( const CEGUI::EventArgs& /*arg*/ )
{
    // create the registration  dialog
    if ( !_p_registrationDialog )
    {
        try
        {
            _p_registrationDialog = yaf3d::GuiManager::get()->loadLayout( DLG_REGISTRATION_FILE, NULL, REGDLG_PREFIX );
            if ( !_p_registrationDialog )
            {
                log_error << "DialogLogin: error creating registration dialog" << std::endl;
                return true;
            }

            // setup cancel button
            CEGUI::PushButton* p_btncancel = static_cast< CEGUI::PushButton* >( _p_registrationDialog->getChild( REGDLG_PREFIX "btn_cancel" ) );
            p_btncancel->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::DialogLogin::onClickedRegistrationCancel, this ) );

            // setup create registration button
            CEGUI::PushButton* p_btnregister = static_cast< CEGUI::PushButton* >( _p_registrationDialog->getChild( REGDLG_PREFIX "btn_create_account" ) );
            p_btnregister->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::DialogLogin::onClickedRegistrationCreate, this ) );

            // make sure that the edit box displays the first line
            CEGUI::MultiLineEditbox* p_termscond = static_cast< CEGUI::MultiLineEditbox* >( _p_registrationDialog->getChild( REGDLG_PREFIX "st_conditions" ) );
            p_termscond->ensureCaratIsVisible();

            // get registration edit fields
            _p_regNickName = static_cast< CEGUI::Editbox* >( _p_registrationDialog->getChild( REGDLG_PREFIX "text_username" ) );
            _p_regPasswd   = static_cast< CEGUI::Editbox* >( _p_registrationDialog->getChild( REGDLG_PREFIX "text_password" ) );
            _p_regRealName = static_cast< CEGUI::Editbox* >( _p_registrationDialog->getChild( REGDLG_PREFIX "text_real_name" ) );
            _p_regEmail    = static_cast< CEGUI::Editbox* >( _p_registrationDialog->getChild( REGDLG_PREFIX "text_email" ) );
        }
        catch ( const CEGUI::Exception& e )
        {
            log_error << "*** DialogLogin: cannot setup registration dialog layout." << std::endl;
            log_out << "      reason: " << e.getMessage().c_str() << std::endl;
        }

    }

    _p_loginDialog->hide();
    _p_registrationDialog->show();

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

bool DialogLogin::onClickedRegistrationCancel( const CEGUI::EventArgs& /*arg*/ )
{
    _p_loginDialog->show();
    _p_registrationDialog->hide();
    return true;
}

bool DialogLogin::onClickedRegistrationCreate( const CEGUI::EventArgs& /*arg*/ )
{
    std::string nickname;
    std::string realname;
    std::string passwd;
    std::string email;

    if ( _p_regNickName->getText().length() )
        nickname = _p_regNickName->getText().c_str();

    if ( _p_regRealName->getText().length() )
        realname = _p_regRealName->getText().c_str();

    if ( _p_regPasswd->getText().length() )
        passwd = _p_regPasswd->getText().c_str();

    if ( _p_regEmail->getText().length() )
        email = _p_regEmail->getText().c_str();

    if ( !nickname.length() || !passwd.length() )
    {
        yaf3d::MessageBoxDialog* p_msg = new yaf3d::MessageBoxDialog( "Attention", "You must provide at least your User name and Password!", yaf3d::MessageBoxDialog::OK, true );
        // create a call back for yes/no buttons of messagebox
        class MsgOkClick: public yaf3d::MessageBoxDialog::ClickCallback
        {
        public:

                                    MsgOkClick(  CEGUI::Window* p_dlg ) : _p_registrationDialog( p_dlg ) {}

            virtual                 ~MsgOkClick() {}

            void                    onClicked( unsigned int /*btnId*/ )
                                    {
                                        _p_registrationDialog->show();
                                    }

            CEGUI::Window*          _p_registrationDialog;
        };
        p_msg->setClickCallback( new MsgOkClick( _p_registrationDialog ) );
        p_msg->show();

        _p_registrationDialog->hide();
        _p_regPasswd->setText( "" );

        return true;
    }

    // try to connect the server and create an account
    std::string url;
    yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_SERVER_IP, url );
    yaf3d::NodeInfo nodeinfo( "", nickname );
    unsigned int channel = 0;
    yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_SERVER_PORT, channel );

    bool registrationDenied = true;

    // try to connect the server for a registration
    try
    {
        yaf3d::NetworkDevice::get()->setupClient( url, channel, nodeinfo, nickname, passwd, true, realname, email );

        if ( !nodeinfo.getAccessGranted() )
        {
            throw yaf3d::NetworkException( "Registration failed with given user name.\nIt already exists. Try another User name." );
        }
        registrationDenied = false;
    }
    catch ( const yaf3d::NetworkException& e )
    {
        log_info << "registration failed.\n reason: " << e.what() << std::endl;

        // create a call back for Ok button of messagebox
        yaf3d::MessageBoxDialog* p_msg = new yaf3d::MessageBoxDialog( "Attention", e.what(), yaf3d::MessageBoxDialog::OK, true );

        class MsgOkClick: public yaf3d::MessageBoxDialog::ClickCallback
        {
        public:

                                    MsgOkClick(  CEGUI::Window* p_dlg ) : _p_registrationDialog( p_dlg ) {}

            virtual                 ~MsgOkClick() {}

            void                    onClicked( unsigned int /*btnId*/ )
                                    {
                                        _p_registrationDialog->show();
                                    }

            CEGUI::Window*          _p_registrationDialog;
        };
        p_msg->setClickCallback( new MsgOkClick( _p_registrationDialog ) );
        p_msg->show();

        _p_registrationDialog->hide();
        _p_regPasswd->setText( "" );

        // play attention sound
        vrc::gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_ATTENTION );

        if ( registrationDenied )
            return true;
    }

    // registration was successful

    _p_regRealName->setText( "" );
    _p_regPasswd->setText( "" );
    _p_regEmail->setText( "" );
    _p_registrationDialog->hide();

    {
        // pop up the welcome message
        yaf3d::MessageBoxDialog* p_msgwelcome = new yaf3d::MessageBoxDialog
           (
            "Server",
            "Congratulation, your account has been created.\nNow, you can login with your User name and Password.",
            yaf3d::MessageBoxDialog::OK,
            true
           );
        class MsgOkClick: public yaf3d::MessageBoxDialog::ClickCallback
        {
        public:

                                    MsgOkClick(  DialogLogin* p_dlg ) : _p_dialog( p_dlg ) {}

            virtual                 ~MsgOkClick() {}

            void                    onClicked( unsigned int /*btnId*/ )
                                    {
                                        _p_dialog->_p_loginDialog->show();
                                        // take the fresh registered nick name as login name in login dialog
                                        _p_dialog->_p_login->setText( _p_dialog->_p_regNickName->getText() );
                                    }

            DialogLogin*            _p_dialog;
        };
        p_msgwelcome->setClickCallback( new MsgOkClick( this ) );
        p_msgwelcome->show();
    }

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
