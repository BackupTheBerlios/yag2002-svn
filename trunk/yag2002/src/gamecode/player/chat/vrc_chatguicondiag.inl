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
 # player's chat box gui
 #
 #   date of creation:  04/19/2005
 #
 #   author:            ali botorabi (boto)
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_CHATGUICONDIAG_H_
#error  "do not include this file directly!"
#endif

#define CONDIALOGLAYOUT_PREFIX  "_connection_dialog_"

// Implementation of connection dialog
template< class TypeT >
ConnectionDialog< TypeT >::ConnectionDialog( TypeT* p_parent ) :
_p_parent( p_parent )
{
    try
    {
        _p_frame = static_cast< CEGUI::FrameWindow* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/FrameWindow", std::string( CONDIALOGLAYOUT_PREFIX "_connect_dialog_" ) ) );
        _p_frame->subscribeEvent( CEGUI::PushButton::EventKeyDown, CEGUI::Event::Subscriber( &vrc::ConnectionDialog< TypeT >::onKeyDown, this ) );
        _p_frame->setPosition( CEGUI::Point( 0.35f, 0.1f ) );
        _p_frame->setSize( CEGUI::Size( 0.3f, 0.4f ) );
        _p_frame->setAlwaysOnTop( true );
        _p_frame->setDragMovingEnabled( false );
        _p_frame->setCloseButtonEnabled( false );
        _p_frame->setSizingEnabled( false );
        _p_frame->setText( "Connection settings" );
        _p_frame->setAlpha( 0.75f );
        _p_frame->hide();

        // protocol
        CEGUI::StaticText*  p_stProtocol = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", std::string( CONDIALOGLAYOUT_PREFIX "_connect_dialog_st_protocol_" ) ) );
        p_stProtocol->setPosition( CEGUI::Point( 0.05f, 0.15f ) );
        p_stProtocol->setSize( CEGUI::Size( 0.3f, 0.05f ) );
        p_stProtocol->setFrameEnabled( false );
        p_stProtocol->setBackgroundEnabled( false );
        p_stProtocol->setText( "protocol" );
        _p_frame->addChildWindow( p_stProtocol );

        _p_editProtocol = static_cast< CEGUI::Editbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Editbox", std::string( CONDIALOGLAYOUT_PREFIX "_connect_dialog_st_editprotocol_" ) ) );
        _p_editProtocol->setPosition( CEGUI::Point( 0.5f, 0.15f ) );
        _p_editProtocol->setSize( CEGUI::Size( 0.45f, 0.07f ) );
        _p_editProtocol->setText( "?" );
        _p_editProtocol->setFont( YAF3D_GUI_FONT8 );
        _p_editProtocol->disable(); // protocol field is read-only
        _p_frame->addChildWindow( _p_editProtocol );

        // server URL
        CEGUI::StaticText*  p_stServerUrl = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", std::string( CONDIALOGLAYOUT_PREFIX "_connect_dialog_st_serverurl_" ) ) );
        p_stServerUrl->setPosition( CEGUI::Point( 0.05f, 0.24f ) );
        p_stServerUrl->setSize( CEGUI::Size( 0.3f, 0.05f ) );
        p_stServerUrl->setFrameEnabled( false );
        p_stServerUrl->setBackgroundEnabled( false );
        p_stServerUrl->setText( "server" );
        _p_frame->addChildWindow( p_stServerUrl );

        _p_editServerUrl = static_cast< CEGUI::Editbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Editbox", std::string( CONDIALOGLAYOUT_PREFIX "_connect_dialog_st_editserverurl_" ) ) );
        _p_editServerUrl->subscribeEvent( CEGUI::PushButton::EventKeyDown, CEGUI::Event::Subscriber( &vrc::ConnectionDialog< TypeT >::onKeyDown, this ) );
        _p_editServerUrl->setPosition( CEGUI::Point( 0.5f, 0.24f ) );
        _p_editServerUrl->setSize( CEGUI::Size( 0.45f, 0.07f ) );
        _p_editServerUrl->setText( "irc.freenode.net" );
        _p_editServerUrl->setFont( YAF3D_GUI_FONT8 );
        _p_frame->addChildWindow( _p_editServerUrl );

        // channel
        CEGUI::StaticText*  p_stChannel = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", std::string( CONDIALOGLAYOUT_PREFIX "_connect_dialog_st_channel_" ) ) );
        p_stChannel->setPosition( CEGUI::Point( 0.05f, 0.33f ) );
        p_stChannel->setSize( CEGUI::Size( 0.3f, 0.05f ) );
        p_stChannel->setFrameEnabled( false );
        p_stChannel->setBackgroundEnabled( false );
        p_stChannel->setText( "channel" );
        _p_frame->addChildWindow( p_stChannel );

        _p_editChannel = static_cast< CEGUI::Editbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Editbox", std::string( CONDIALOGLAYOUT_PREFIX "_connect_dialog_st_editchannel_" ) ) );
        _p_editChannel->subscribeEvent( CEGUI::PushButton::EventKeyDown, CEGUI::Event::Subscriber( &vrc::ConnectionDialog< TypeT >::onKeyDown, this ) );
        _p_editChannel->setPosition( CEGUI::Point( 0.5f, 0.33f ) );
        _p_editChannel->setSize( CEGUI::Size( 0.45f, 0.07f ) );
        _p_editChannel->setText( "#" );
        _p_editChannel->setFont( YAF3D_GUI_FONT8 );
        _p_frame->addChildWindow( _p_editChannel );

        // nickname
        CEGUI::StaticText*  p_stNickName = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", std::string( CONDIALOGLAYOUT_PREFIX "_connect_dialog_st_nickname_" ) ) );
        p_stNickName->setPosition( CEGUI::Point( 0.05f, 0.42f ) );
        p_stNickName->setSize( CEGUI::Size( 0.3f, 0.05f ) );
        p_stNickName->setFrameEnabled( false );
        p_stNickName->setBackgroundEnabled( false );
        p_stNickName->setText( "nick name" );
        _p_frame->addChildWindow( p_stNickName );

        _p_editNickName = static_cast< CEGUI::Editbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Editbox", std::string( CONDIALOGLAYOUT_PREFIX "_connect_dialog_st_editnickname_" ) ) );
        _p_editNickName->subscribeEvent( CEGUI::PushButton::EventKeyDown, CEGUI::Event::Subscriber( &vrc::ConnectionDialog< TypeT >::onKeyDown, this ) );
        _p_editNickName->setPosition( CEGUI::Point( 0.5f, 0.42f ) );
        _p_editNickName->setSize( CEGUI::Size( 0.45f, 0.07f ) );
        _p_editNickName->setText( "" );
        _p_editNickName->setFont( YAF3D_GUI_FONT8 );
        _p_frame->addChildWindow( _p_editNickName );

        // username
        CEGUI::StaticText*  p_stUserName = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", std::string( CONDIALOGLAYOUT_PREFIX "_connect_dialog_st_username_" ) ) );
        p_stUserName->setPosition( CEGUI::Point( 0.05f, 0.51f ) );
        p_stUserName->setSize( CEGUI::Size( 0.3f, 0.05f ) );
        p_stUserName->setFrameEnabled( false );
        p_stUserName->setBackgroundEnabled( false );
        p_stUserName->setText( "user name" );
        _p_frame->addChildWindow( p_stUserName );

        _p_editUserName = static_cast< CEGUI::Editbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Editbox", std::string( CONDIALOGLAYOUT_PREFIX "_connect_dialog_st_editusername_" ) ) );
        _p_editUserName->subscribeEvent( CEGUI::PushButton::EventKeyDown, CEGUI::Event::Subscriber( &vrc::ConnectionDialog< TypeT >::onKeyDown, this ) );
        _p_editUserName->setPosition( CEGUI::Point( 0.5f, 0.51f ) );
        _p_editUserName->setSize( CEGUI::Size( 0.45f, 0.07f ) );
        _p_editUserName->setText( "" );
        _p_editUserName->setFont( YAF3D_GUI_FONT8 );
        _p_frame->addChildWindow( _p_editUserName );

         // realname
        CEGUI::StaticText*  p_stRealName = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", std::string( CONDIALOGLAYOUT_PREFIX "_connect_dialog_st_realname_" ) ) );
        p_stRealName->setPosition( CEGUI::Point( 0.05f, 0.60f ) );
        p_stRealName->setSize( CEGUI::Size( 0.3f, 0.05f ) );
        p_stRealName->setFrameEnabled( false );
        p_stRealName->setBackgroundEnabled( false );
        p_stRealName->setText( "real name" );
        _p_frame->addChildWindow( p_stRealName );

        _p_editRealName = static_cast< CEGUI::Editbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Editbox", std::string( CONDIALOGLAYOUT_PREFIX "_connect_dialog_st_editrealname_" ) ) );
        _p_editRealName->subscribeEvent( CEGUI::PushButton::EventKeyDown, CEGUI::Event::Subscriber( &vrc::ConnectionDialog< TypeT >::onKeyDown, this ) );
        _p_editRealName->setPosition( CEGUI::Point( 0.5f, 0.60f ) );
        _p_editRealName->setSize( CEGUI::Size( 0.45f, 0.07f ) );
        _p_editRealName->setText( "" );
        _p_editRealName->setFont( YAF3D_GUI_FONT8 );
        _p_frame->addChildWindow( _p_editRealName );

         // passwd
        CEGUI::StaticText*  p_stPassword = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", std::string( CONDIALOGLAYOUT_PREFIX "_connect_dialog_st_password_" ) ) );
        p_stPassword->setPosition( CEGUI::Point( 0.05f, 0.69f ) );
        p_stPassword->setSize( CEGUI::Size( 0.3f, 0.05f ) );
        p_stPassword->setFrameEnabled( false );
        p_stPassword->setBackgroundEnabled( false );
        p_stPassword->setText( "password" );
        _p_frame->addChildWindow( p_stPassword );

        _p_editPassword = static_cast< CEGUI::Editbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Editbox", std::string( CONDIALOGLAYOUT_PREFIX "_connect_dialog_st_editpassword_" ) ) );
        _p_editPassword->subscribeEvent( CEGUI::PushButton::EventKeyDown, CEGUI::Event::Subscriber( &vrc::ConnectionDialog< TypeT >::onKeyDown, this ) );
        _p_editPassword->setPosition( CEGUI::Point( 0.5f, 0.69f ) );
        _p_editPassword->setSize( CEGUI::Size( 0.45f, 0.07f ) );
        _p_editPassword->setText( "" );
        _p_editPassword->setFont( YAF3D_GUI_FONT8 );
        _p_editPassword->setMaskCodePoint( CEGUI::utf32( '*' ) );
        _p_editPassword->setTextMasked( true );
        _p_frame->addChildWindow( _p_editPassword );

         // port
        CEGUI::StaticText*  p_stPort = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", std::string( CONDIALOGLAYOUT_PREFIX "_connect_dialog_st_port_" ) ) );
        p_stPort->setPosition( CEGUI::Point( 0.05f, 0.78f ) );
        p_stPort->setSize( CEGUI::Size( 0.3f, 0.05f ) );
        p_stPort->setFrameEnabled( false );
        p_stPort->setBackgroundEnabled( false );
        p_stPort->setText( "port" );
        _p_frame->addChildWindow( p_stPort );

        _p_editPort = static_cast< CEGUI::Editbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Editbox", std::string( CONDIALOGLAYOUT_PREFIX "_connect_dialog_st_editport_" ) ) );
        _p_editPort->subscribeEvent( CEGUI::PushButton::EventKeyDown, CEGUI::Event::Subscriber( &vrc::ConnectionDialog< TypeT >::onKeyDown, this ) );
        _p_editPort->setPosition( CEGUI::Point( 0.5f, 0.78f ) );
        _p_editPort->setSize( CEGUI::Size( 0.45f, 0.07f ) );
        _p_editPort->setText( "0" );
        _p_editPort->setFont( YAF3D_GUI_FONT8 );
        _p_frame->addChildWindow( _p_editPort );

        // create connect button
        CEGUI::PushButton*  p_btnConnect = static_cast< CEGUI::PushButton* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Button", std::string( CONDIALOGLAYOUT_PREFIX "_connect_dialog_btn_connect_" ) ) );
        p_btnConnect->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::ConnectionDialog< TypeT >::onClickedConnect, this ) );
        p_btnConnect->setPosition( CEGUI::Point( 0.65f, 0.9f ) );
        p_btnConnect->setSize( CEGUI::Size( 0.3f, 0.075f ) );
        p_btnConnect->setText( "Connect" );
        p_btnConnect->setFont( YAF3D_GUI_FONT8 );
        _p_frame->addChildWindow( p_btnConnect );

        // create cancel button
        CEGUI::PushButton*  p_btnCancel = static_cast< CEGUI::PushButton* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Button", std::string( CONDIALOGLAYOUT_PREFIX "_connect_dialog_btn_cancel_" ) ) );
        p_btnCancel->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::ConnectionDialog< TypeT >::onClickedCancel, this ) );
        p_btnCancel->setPosition( CEGUI::Point( 0.3f, 0.9f ) );
        p_btnCancel->setSize( CEGUI::Size( 0.3f, 0.075f ) );
        p_btnCancel->setText( "Cancel" );
        p_btnCancel->setFont( YAF3D_GUI_FONT8 );
        _p_frame->addChildWindow( p_btnCancel );

        // append frame to main gui window
        gameutils::GuiUtils::get()->getMainGuiWindow()->addChildWindow( _p_frame );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "ChatGuiCtrl::ConnectionDialog: problem creating connection dialog" << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

template< class TypeT >
ConnectionDialog< TypeT >::~ConnectionDialog()
{
    try
    {
        gameutils::GuiUtils::get()->getMainGuiWindow()->removeChildWindow( _p_frame );
        CEGUI::WindowManager::getSingleton().destroyWindow( _p_frame );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "~ConnectionDialog: problem cleaning up gui resources" << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

template< class TypeT >
void ConnectionDialog< TypeT >::show( bool en )
{
    if ( en )
    {
        // update settings
        _p_editProtocol->setText( _cfg._protocol );
        _p_editServerUrl->setText( _cfg._serverURL );
        _p_editChannel->setText( _cfg._channel );
        _p_editNickName->setText( _cfg._nickname );
        _p_editUserName->setText( _cfg._username );

        std::stringstream port;
        port << _cfg._port;
        _p_editPort->setText( port.str() );

        gameutils::GuiUtils::get()->getMainGuiWindow()->addChildWindow( _p_frame );
        _p_frame->show();
        _p_frame->activate();   // gain focus
    }
    else
    {
        _p_frame->hide();
        gameutils::GuiUtils::get()->getMainGuiWindow()->removeChildWindow( _p_frame );
    }
}

template< class TypeT >
bool ConnectionDialog< TypeT >::onClickedConnect( const CEGUI::EventArgs& /*arg*/ )
{
    // store settings
    _cfg._protocol  = _p_editProtocol->getText().c_str();
    _cfg._serverURL = _p_editServerUrl->getText().c_str();
    _cfg._channel   = _p_editChannel->getText().c_str();
    _cfg._nickname  = _p_editNickName->getText().c_str();
    _cfg._password  = _p_editPassword->getText().c_str();
    _cfg._username  = _p_editUserName->getText().c_str();
    _cfg._realname  = _p_editRealName->getText().c_str();

    std::stringstream port;
    port << _p_editPort->getText().c_str();
    port >> _cfg._port;

    _p_parent->onConnectionDialogClickedConnect( _cfg );

    return true;
}

template< class TypeT >
bool ConnectionDialog< TypeT >::onKeyDown( const CEGUI::EventArgs& arg )
{
    // check for 'Return' key
    CEGUI::KeyEventArgs& ke = static_cast< CEGUI::KeyEventArgs& >( const_cast< CEGUI::EventArgs& >( arg ) );
    if ( ( ke.codepoint == SDLK_RETURN ) || ( ke.scancode == CEGUI::Key::Return ) )
    {
        onClickedConnect( arg );
    }

    return true;
}

template< class TypeT >
bool ConnectionDialog< TypeT >::onClickedCancel( const CEGUI::EventArgs& /*arg*/ )
{
    _p_parent->onConnectionDialogClickedCancel();

    return true;
}
