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
 # player's chat box gui
 #
 #   date of creation:  04/19/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_chatguibox.h"

namespace CTD
{
// layout prefix
#define CHATLAYOUT_PREFIX       "chatbox_"
#define FADE_TIME               1.0f

#define CTD_IMAGE_SET           "CTDImageSet"
#define CTD_IMAGE_SET_FILE      "gui/imagesets/CTDImageSet.imageset"

ChatGuiBox::ChannelTabPane::ChannelTabPane( CEGUI::TabControl* p_tabcontrol, ChatGuiBox* p_guibox ) :
_p_tabCtrl( p_tabcontrol ),
_p_guibox( p_guibox ),
_p_tabPane( NULL ),
_p_messagebox( NULL ),
_p_editbox( NULL ),
_p_listbox( NULL )
{
    try
    {
        // every pane must have unique name
        std::string postfix;
        static unsigned int instnum = 0;
        std::stringstream   prefcounter;
        prefcounter << instnum++;
        postfix += prefcounter.str().c_str();

        _p_tabPane = static_cast< CEGUI::Window* >( CEGUI::WindowManager::getSingleton().createWindow( "DefaultWindow", std::string( CHATLAYOUT_PREFIX "tabpane" ) + postfix ) );
        _p_tabCtrl->addTab( _p_tabPane );

        _p_messagebox = static_cast< CEGUI::MultiLineEditbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/MultiLineEditbox", std::string( CHATLAYOUT_PREFIX "tabpane_msgbox" ) + postfix ) );
        _p_messagebox->setReadOnly( true );
        _p_messagebox->setPosition( CEGUI::Point( 0.005f, 0.005f ) );
        _p_messagebox->setSize( CEGUI::Size( 0.90f, 0.7f ) );
        _p_messagebox->setFont( "CTD-8" );
        _p_tabPane->addChildWindow( _p_messagebox );

        _p_editbox = static_cast< CEGUI::Editbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Editbox", std::string( CHATLAYOUT_PREFIX "tabpane_editbox" ) + postfix ) );
        _p_editbox->subscribeEvent( CEGUI::MultiLineEditbox::EventCharacterKey, CEGUI::Event::Subscriber( &CTD::ChatGuiBox::ChannelTabPane::onEditboxTextChanged, this ) );
        _p_editbox->setPosition( CEGUI::Point( 0.005f, 0.75f ) );
        _p_editbox->setSize( CEGUI::Size( 0.90f, 0.2f ) );
        _p_tabPane->addChildWindow( _p_editbox );

        // nickname list
        _p_listbox = static_cast< CEGUI::Listbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Listbox", std::string( CHATLAYOUT_PREFIX "tabpane_nicklist" ) + postfix ) );
        _p_listbox->subscribeEvent( CEGUI::Listbox::EventSelectionChanged, CEGUI::Event::Subscriber( &CTD::ChatGuiBox::ChannelTabPane::onListItemSelChanged, this ) );
        _p_listbox->setSortingEnabled( true );
        _p_listbox->setPosition( CEGUI::Point( 0.91f, 0.005f ) );
        _p_listbox->setSize( CEGUI::Size( 0.085f, 0.93f ) );
        _p_listbox->setAlpha( 0.8f );
        _p_listbox->setFont( "CTD-8" );
        _p_tabPane->addChildWindow( _p_listbox );
    }
    catch ( const CEGUI::Exception& e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "ChannelTabPane: problem creating a new tab pane" << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

ChatGuiBox::ChannelTabPane::~ChannelTabPane()
{
    try
    {
        _p_tabCtrl->removeTab( _p_tabPane->getID() );
        CEGUI::WindowManager::getSingleton().destroyWindow( _p_tabPane );
    }
    catch ( const CEGUI::Exception& e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "~ChannelTabPane: problem cleaning up gui resources" << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

void ChatGuiBox::ChannelTabPane::updateMemberList( std::vector< std::string >& list )
{
    // set selection background color
    CEGUI::ColourRect col( 
                            CEGUI::colour( 255.0f / 255.0f, 214.0f / 255.0f, 9.0f / 255.0f, 0.8f ),
                            CEGUI::colour( 12.0f  / 255.0f, 59.0f  / 255.0f, 0.0f         , 0.8f ),
                            CEGUI::colour( 255.0f / 255.0f, 214.0f / 255.0f, 9.0f / 255.0f, 0.8f ),
                            CEGUI::colour( 12.0f  / 255.0f, 59.0f  / 255.0f, 0.0f         , 0.8f )
                          );    
    // fill up the list
    _p_listbox->resetList();
    std::vector< std::string >::iterator p_beg = list.begin(), p_end = list.end();
    for ( ; p_beg != p_end; p_beg++ )
    {
        CEGUI::ListboxTextItem * p_item = new CEGUI::ListboxTextItem( p_beg->c_str() );
        p_item->setSelectionColours( col );
        p_item->setSelectionBrushImage( "TaharezLook", "ListboxSelectionBrush" );
        _p_listbox->insertItem( p_item, NULL );
    }

    if ( list.size() > 0 )
    {
        _p_listbox->getListboxItemFromIndex( 0 )->setSelected( true );
    }
}

void ChatGuiBox::ChannelTabPane::addMessage( const CEGUI::String& msg, const CEGUI::String& author )
{
    CEGUI::String buffer = _p_messagebox->getText();
    buffer += author + "> " + msg;
    _p_messagebox->setText( buffer );
    // set carat position in order to trigger text scrolling after a new line has been added
    _p_messagebox->setCaratIndex( buffer.length() - 1 );
}

bool ChatGuiBox::ChannelTabPane::onListItemSelChanged( const CEGUI::EventArgs& arg )
{
    //! TODO: in combination with other buttons we can implement the pm, or whois etc.
    //// get selection
    //CEGUI::ListboxItem* p_sel = _p_listbox->getFirstSelectedItem();
    //if ( !p_sel )
    //{
    //    return;
    //} ...

    return true;
}

bool ChatGuiBox::ChannelTabPane::onEditboxTextChanged( const CEGUI::EventArgs& arg )
{
    // check for 'Return' key
    CEGUI::KeyEventArgs& ke = static_cast< CEGUI::KeyEventArgs& >( const_cast< CEGUI::EventArgs& >( arg ) );
    if ( ke.codepoint == SDLK_RETURN )
    {
        assert( _configuration._p_protocolHandler && "invalid protocol handler!" );

        // skip empty lines
        if ( !_p_editbox->getText().length() )
            return true;
        
        // send the msg over net
        _configuration._p_protocolHandler->send( _p_editbox->getText().c_str(), _configuration._channel ); 

        // add the msg to local chat box ( if it was not a command )
        if ( _p_editbox->getText().compare( 0, 1, "/" ) )
        {
            addMessage( _p_editbox->getText(), _configuration._nickname );
        }
        _p_editbox->setText( "" );
    }
    return true;
}

void ChatGuiBox::ChannelTabPane::setSelection()
{
    _p_tabCtrl->setSelectedTab( _p_tabPane->getID() );
}

void ChatGuiBox::ChannelTabPane::setEditBoxFocus( bool en )
{
    if ( en )
        _p_editbox->activate();
    else
        _p_editbox->deactivate();
}

void ChatGuiBox::ChannelTabPane::onReceive( const std::string& channel, const std::string& sender, const std::string& msg )
{
    addMessage( msg, sender );
}

void ChatGuiBox::ChannelTabPane::onNicknameChanged( const std::string& newname, const std::string& oldname )
{
    // did _we_ change our nick name or someone else changed nickname?
    if ( newname == oldname )
    {
        // change nick name in internal list
        size_t numnicks = _nickNames.size();
        for ( size_t cnt = 0; cnt < numnicks; cnt ++ )
        {
            if ( _nickNames[ cnt ] == _configuration._nickname )
            {
                _nickNames[ cnt ] = newname;
                break;
            }
        }

        addMessage( " you changed your nickname to '" + newname  + "'", "* " );
        _configuration._nickname = newname;
    }
    else
    {
        // change nick name in internal list
        size_t numnicks = _nickNames.size();
        for ( size_t cnt = 0; cnt < numnicks; cnt ++ )
        {
            if ( _nickNames[ cnt ] == oldname )
            {
                _nickNames[ cnt ] = newname;
                break;
            }
        }

        addMessage( "'" + oldname + "' changed nickname to '" + newname  + "'", "* " );
    }

    // trigger updating the listbox
    _configuration._p_protocolHandler->requestMemberList( _configuration._channel );
}

void ChatGuiBox::ChannelTabPane::onJoinedChannel( const ChatConnectionConfig& cfg )
{
    if ( _configuration._nickname != cfg._nickname )
        addMessage( cfg._nickname + " entered the chat room", "* " );

    // trigger updating the listbox
    _configuration._p_protocolHandler->requestMemberList( _configuration._channel );
}

void ChatGuiBox::ChannelTabPane::onLeftChannel( const ChatConnectionConfig& cfg )
{
    // have _we_ left the channel ?
    if ( ( cfg._nickname == _configuration._nickname ) && ( cfg._channel == _configuration._channel ) )
    {
        // trigger removal of this tab pane
        _p_guibox->removeTabPane( this );
    }
    else
    {
        addMessage( cfg._nickname + " left the chat room", "* " );
        // trigger updating the listbox
        _configuration._p_protocolHandler->requestMemberList( _configuration._channel );
    }
}

void ChatGuiBox::ChannelTabPane::onReceiveMemberList( const std::string& channel )
{
    // update member list
    _nickNames.clear();
    _configuration._p_protocolHandler->getMemberList( _configuration._channel, _nickNames );
    updateMemberList( _nickNames );
}

//------------------------

ChatGuiBox::ConnectionDialog::ConnectionDialog( ChatGuiBox* p_chatbox ) :
_p_chatbox( p_chatbox )
{
    try
    {
        _p_frame = static_cast< CEGUI::FrameWindow* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/FrameWindow", std::string( CHATLAYOUT_PREFIX "_connect_dialog_" ) ) );
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
        CEGUI::StaticText*  p_stProtocol = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", std::string( CHATLAYOUT_PREFIX "_connect_dialog_st_protocol_" ) ) );
        p_stProtocol->setPosition( CEGUI::Point( 0.05f, 0.15f ) ); 
        p_stProtocol->setSize( CEGUI::Size( 0.3f, 0.05f ) );
        p_stProtocol->setFrameEnabled( false );
        p_stProtocol->setBackgroundEnabled( false );
        p_stProtocol->setText( "protocol" );
        _p_frame->addChildWindow( p_stProtocol );

        _p_editProtocol = static_cast< CEGUI::Editbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Editbox", std::string( CHATLAYOUT_PREFIX "_connect_dialog_st_editprotocol_" ) ) );
        _p_editProtocol->setPosition( CEGUI::Point( 0.5f, 0.15f ) ); 
        _p_editProtocol->setSize( CEGUI::Size( 0.45f, 0.07f ) );
        _p_editProtocol->setText( "?" );
        _p_editProtocol->setFont( "CTD-8" );
        _p_editProtocol->disable(); // protocol field is read-only
        _p_frame->addChildWindow( _p_editProtocol );

        // server URL 
        CEGUI::StaticText*  p_stServerUrl = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", std::string( CHATLAYOUT_PREFIX "_connect_dialog_st_serverurl_" ) ) );
        p_stServerUrl->setPosition( CEGUI::Point( 0.05f, 0.24f ) ); 
        p_stServerUrl->setSize( CEGUI::Size( 0.3f, 0.05f ) );
        p_stServerUrl->setFrameEnabled( false );
        p_stServerUrl->setBackgroundEnabled( false );
        p_stServerUrl->setText( "server" );
        _p_frame->addChildWindow( p_stServerUrl );

        _p_editServerUrl = static_cast< CEGUI::Editbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Editbox", std::string( CHATLAYOUT_PREFIX "_connect_dialog_st_editserverurl_" ) ) );
        _p_editServerUrl->setPosition( CEGUI::Point( 0.5f, 0.24f ) ); 
        _p_editServerUrl->setSize( CEGUI::Size( 0.45f, 0.07f ) );
        _p_editServerUrl->setText( "irc.freenode.net" );
        _p_editServerUrl->setFont( "CTD-8" );
        _p_frame->addChildWindow( _p_editServerUrl );

        // channel
        CEGUI::StaticText*  p_stChannel = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", std::string( CHATLAYOUT_PREFIX "_connect_dialog_st_channel_" ) ) );
        p_stChannel->setPosition( CEGUI::Point( 0.05f, 0.33f ) ); 
        p_stChannel->setSize( CEGUI::Size( 0.3f, 0.05f ) );
        p_stChannel->setFrameEnabled( false );
        p_stChannel->setBackgroundEnabled( false );
        p_stChannel->setText( "channel" );
        _p_frame->addChildWindow( p_stChannel );

        _p_editChannel = static_cast< CEGUI::Editbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Editbox", std::string( CHATLAYOUT_PREFIX "_connect_dialog_st_editchannel_" ) ) );
        _p_editChannel->setPosition( CEGUI::Point( 0.5f, 0.33f ) ); 
        _p_editChannel->setSize( CEGUI::Size( 0.45f, 0.07f ) );
        _p_editChannel->setText( "#" );
        _p_editChannel->setFont( "CTD-8" );
        _p_frame->addChildWindow( _p_editChannel );

        // nickname
        CEGUI::StaticText*  p_stNickName = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", std::string( CHATLAYOUT_PREFIX "_connect_dialog_st_nickname_" ) ) );
        p_stNickName->setPosition( CEGUI::Point( 0.05f, 0.42f ) ); 
        p_stNickName->setSize( CEGUI::Size( 0.3f, 0.05f ) );
        p_stNickName->setFrameEnabled( false );
        p_stNickName->setBackgroundEnabled( false );
        p_stNickName->setText( "nick name" );
        _p_frame->addChildWindow( p_stNickName );

        _p_editNickName = static_cast< CEGUI::Editbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Editbox", std::string( CHATLAYOUT_PREFIX "_connect_dialog_st_editnickname_" ) ) );
        _p_editNickName->setPosition( CEGUI::Point( 0.5f, 0.42f ) ); 
        _p_editNickName->setSize( CEGUI::Size( 0.45f, 0.07f ) );
        _p_editNickName->setText( "" );
        _p_editNickName->setFont( "CTD-8" );
        _p_frame->addChildWindow( _p_editNickName );

        // username
        CEGUI::StaticText*  p_stUserName = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", std::string( CHATLAYOUT_PREFIX "_connect_dialog_st_username_" ) ) );
        p_stUserName->setPosition( CEGUI::Point( 0.05f, 0.51f ) ); 
        p_stUserName->setSize( CEGUI::Size( 0.3f, 0.05f ) );
        p_stUserName->setFrameEnabled( false );
        p_stUserName->setBackgroundEnabled( false );
        p_stUserName->setText( "user name" );
        _p_frame->addChildWindow( p_stUserName );

        _p_editUserName = static_cast< CEGUI::Editbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Editbox", std::string( CHATLAYOUT_PREFIX "_connect_dialog_st_editusername_" ) ) );
        _p_editUserName->setPosition( CEGUI::Point( 0.5f, 0.51f ) ); 
        _p_editUserName->setSize( CEGUI::Size( 0.45f, 0.07f ) );
        _p_editUserName->setText( "" );
        _p_editUserName->setFont( "CTD-8" );
        _p_frame->addChildWindow( _p_editUserName );

         // realname
        CEGUI::StaticText*  p_stRealName = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", std::string( CHATLAYOUT_PREFIX "_connect_dialog_st_realname_" ) ) );
        p_stRealName->setPosition( CEGUI::Point( 0.05f, 0.60f ) ); 
        p_stRealName->setSize( CEGUI::Size( 0.3f, 0.05f ) );
        p_stRealName->setFrameEnabled( false );
        p_stRealName->setBackgroundEnabled( false );
        p_stRealName->setText( "real name" );
        _p_frame->addChildWindow( p_stRealName );

        _p_editRealName = static_cast< CEGUI::Editbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Editbox", std::string( CHATLAYOUT_PREFIX "_connect_dialog_st_editrealname_" ) ) );
        _p_editRealName->setPosition( CEGUI::Point( 0.5f, 0.60f ) ); 
        _p_editRealName->setSize( CEGUI::Size( 0.45f, 0.07f ) );
        _p_editRealName->setText( "" );
        _p_editRealName->setFont( "CTD-8" );
        _p_frame->addChildWindow( _p_editRealName );

         // passwd
        CEGUI::StaticText*  p_stPassword = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", std::string( CHATLAYOUT_PREFIX "_connect_dialog_st_password_" ) ) );
        p_stPassword->setPosition( CEGUI::Point( 0.05f, 0.69f ) ); 
        p_stPassword->setSize( CEGUI::Size( 0.3f, 0.05f ) );
        p_stPassword->setFrameEnabled( false );
        p_stPassword->setBackgroundEnabled( false );
        p_stPassword->setText( "password" );
        _p_frame->addChildWindow( p_stPassword );

        _p_editPassword = static_cast< CEGUI::Editbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Editbox", std::string( CHATLAYOUT_PREFIX "_connect_dialog_st_editpassword_" ) ) );
        _p_editPassword->setPosition( CEGUI::Point( 0.5f, 0.69f ) ); 
        _p_editPassword->setSize( CEGUI::Size( 0.45f, 0.07f ) );
        _p_editPassword->setText( "" );
        _p_editPassword->setFont( "CTD-8" );
        _p_frame->addChildWindow( _p_editPassword );

         // port
        CEGUI::StaticText*  p_stPort = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", std::string( CHATLAYOUT_PREFIX "_connect_dialog_st_port_" ) ) );
        p_stPort->setPosition( CEGUI::Point( 0.05f, 0.78f ) ); 
        p_stPort->setSize( CEGUI::Size( 0.3f, 0.05f ) );
        p_stPort->setFrameEnabled( false );
        p_stPort->setBackgroundEnabled( false );
        p_stPort->setText( "port" );
        _p_frame->addChildWindow( p_stPort );

        _p_editPort = static_cast< CEGUI::Editbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Editbox", std::string( CHATLAYOUT_PREFIX "_connect_dialog_st_editport_" ) ) );
        _p_editPort->setPosition( CEGUI::Point( 0.5f, 0.78f ) ); 
        _p_editPort->setSize( CEGUI::Size( 0.45f, 0.07f ) );
        _p_editPort->setText( "0" );
        _p_editPort->setFont( "CTD-8" );
        _p_frame->addChildWindow( _p_editPort );

        // create connect button
        CEGUI::PushButton*  p_btnConnect = static_cast< CEGUI::PushButton* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Button", std::string( CHATLAYOUT_PREFIX "_connect_dialog_btn_connect_" ) ) );
        p_btnConnect->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &CTD::ChatGuiBox::ConnectionDialog::onClickedConnect, this ) );
        p_btnConnect->setPosition( CEGUI::Point( 0.65f, 0.9f ) ); 
        p_btnConnect->setSize( CEGUI::Size( 0.3f, 0.075f ) ); 
        p_btnConnect->setText( "Connect" );
        p_btnConnect->setFont( "CTD-8" );
        _p_frame->addChildWindow( p_btnConnect );

        // create cancel button
        CEGUI::PushButton*  p_btnCancel = static_cast< CEGUI::PushButton* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Button", std::string( CHATLAYOUT_PREFIX "_connect_dialog_btn_cancel_" ) ) );
        p_btnCancel->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &CTD::ChatGuiBox::ConnectionDialog::onClickedCancel, this ) );
        p_btnCancel->setPosition( CEGUI::Point( 0.3f, 0.9f ) ); 
        p_btnCancel->setSize( CEGUI::Size( 0.3f, 0.075f ) ); 
        p_btnCancel->setText( "Cancel" );
        p_btnCancel->setFont( "CTD-8" );
        _p_frame->addChildWindow( p_btnCancel );

        // append frame to main chat window
        _p_chatbox->_p_wnd->addChildWindow( _p_frame );
    }
    catch ( const CEGUI::Exception& e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "ChatGuiBox::ConnectionDialog: problem creating connection dialog" << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

ChatGuiBox::ConnectionDialog::~ConnectionDialog()
{
}

void ChatGuiBox::ConnectionDialog::show( bool en )
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

        _p_frame->show();       // unhide
        _p_frame->activate();   // gain focus
    }
    else
    {
        _p_frame->hide();
    }
}

bool ChatGuiBox::ConnectionDialog::onClickedConnect( const CEGUI::EventArgs& arg )
{
    // store settings
    _cfg._protocol  = _p_editProtocol->getText().c_str();
    _cfg._serverURL = _p_editServerUrl->getText().c_str();
    _cfg._channel   = _p_editChannel->getText().c_str();
    _cfg._nickname  = _p_editNickName->getText().c_str();
    _cfg._username  = _p_editUserName->getText().c_str();

    std::stringstream port;
    port << _p_editPort->getText().c_str();
    port >> _cfg._port;

    _p_chatbox->onConnectionDialogClickedConnect( _cfg );

    return true;
}

bool ChatGuiBox::ConnectionDialog::onClickedCancel( const CEGUI::EventArgs& arg )
{
    _p_chatbox->onConnectionDialogClickedCancel();

    return true;
}

//------------------------
ChatGuiBox::ChatGuiBox() :
_state( Idle ),
_p_chatMgr( NULL ),
_p_wnd( NULL ),
_p_frame( NULL ),
_p_btnOpen( NULL ),
_p_btnMsgArrived( NULL ),
_p_tabCtrl( NULL ),
_p_connectionDialog( NULL ),
_hidden( true ),
_modeEdit( false ),
_fadeTimer( 0 ),
_frameAlphaValue( 1.0f )
{
}

ChatGuiBox::~ChatGuiBox()
{
    try
    {
        if ( _p_wnd )
        {
            // delete all tab panes
            ChatGuiBox::TabPanePairList::iterator p_beg = _tabpanes.begin(), p_end = _tabpanes.end();
            for ( ; p_beg != p_end; p_beg++ )
                delete p_beg->second;

            CEGUI::WindowManager::getSingleton().destroyWindow( _p_wnd );
            CEGUI::ImagesetManager::getSingleton().destroyImageset( CTD_IMAGE_SET );
        }
    }
    catch ( const CEGUI::Exception& e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "ChatGuiBox: problem cleaning up gui resources" << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

void ChatGuiBox::initialize( ChatManager* p_chatMgr )
{
    _p_chatMgr = p_chatMgr;
    // init nickname with player name
    Configuration::get()->getSettingValue( CTD_GS_PLAYER_NAME, _nickname );

    // as this class can be instatiated several times it is important to generated unique prefix for every instance
    std::string prefix( CHATLAYOUT_PREFIX );
    static unsigned int instnum = 0;
    std::stringstream   prefcounter;
    prefcounter << instnum++;
    prefix += prefcounter.str().c_str();

    //! TODO: setup the chat box on-the-fly instead of reading a layout file
    std::string layoutFile( "gui/chat.xml" );
    try
    {
        _p_wnd = static_cast< CEGUI::Window* >( GuiManager::get()->loadLayout( layoutFile, NULL, prefix ) );
        _p_frame = static_cast< CEGUI::Window* >( _p_wnd->getChild( prefix + "fr_chatbox" ) );
		_p_frame->subscribeEvent( CEGUI::FrameWindow::EventCloseClicked, CEGUI::Event::Subscriber( &CTD::ChatGuiBox::onCloseFrame, this ) );
        _p_frame->setMinimumSize( CEGUI::Size( 0.1f, 0.08f ) );
        _p_frame->hide();

        _boxFrameSize = osg::Vec2f( _p_frame->getSize().d_width, _p_frame->getSize().d_height );
        _frameAlphaValue = _p_frame->getAlpha();
    }
    catch ( const CEGUI::Exception& e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** error loading layout '" << layoutFile << "'" << std::endl;
        log << "   reason: " << e.getMessage().c_str() << std::endl;
        return;
    }

    try
    {
        CEGUI::PushButton* p_btnIRC = static_cast< CEGUI::PushButton* >( _p_frame->getChild( prefix + "btn_irc" ) );
        p_btnIRC->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &CTD::ChatGuiBox::onClickedIRC, this ) );

        CEGUI::PushButton* p_btnDisconnect = static_cast< CEGUI::PushButton* >( _p_frame->getChild( prefix + "btn_disconnect" ) );
        p_btnDisconnect->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &CTD::ChatGuiBox::onClickedDisconnect, this ) );

        // setup chat box hide button with ctd specific image set
        _p_btnOpen = static_cast< CEGUI::PushButton* >( _p_wnd->getChild( prefix + "btn_openbox" ) );
        _p_btnOpen->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &CTD::ChatGuiBox::onClickedOpen, this ) );
        _p_btnOpen->setStandardImageryEnabled( false );

        // load our custom imageset
        CEGUI::Imageset* p_imageSet = NULL;
        if ( CEGUI::ImagesetManager::getSingleton().isImagesetPresent( CTD_IMAGE_SET ) )
        {
            p_imageSet = CEGUI::ImagesetManager::getSingleton().getImageset( CTD_IMAGE_SET );
        }
        else
        {
            p_imageSet = CEGUI::ImagesetManager::getSingleton().createImageset( CTD_IMAGE_SET_FILE );
        }

        // set editbox open button images
        const CEGUI::Image* p_image = &p_imageSet->getImage( "HandNormal" );
        CEGUI::RenderableImage* p_rendImage = new CEGUI::RenderableImage;
        p_rendImage->setImage( p_image );
        _p_btnOpen->setPushedImage( p_rendImage );
        _p_btnOpen->setNormalImage( p_rendImage );
        delete p_rendImage;

        p_image = &p_imageSet->getImage( "HandHoover" );
        p_rendImage = new CEGUI::RenderableImage;
        p_rendImage->setImage( p_image );
        _p_btnOpen->setHoverImage( p_rendImage );
        delete p_rendImage;

        _p_btnMsgArrived = static_cast< CEGUI::StaticImage* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticImage", prefix + "btn_post" ) );
        _p_btnMsgArrived->setPosition( _p_btnOpen->getPosition() );
        _p_btnMsgArrived->setAlpha( 0.9f );
        _p_btnMsgArrived->setAlwaysOnTop( true );
        _p_btnMsgArrived->setPosition( CEGUI::Relative, CEGUI::Point( 0.01f, 0.7f ) );
        _p_btnMsgArrived->setSize( CEGUI::Size( 0.03f, 0.03f ) );
        _p_btnMsgArrived->setBackgroundEnabled( false );
        _p_btnMsgArrived->setFrameEnabled( false );
        _p_wnd->addChildWindow( _p_btnMsgArrived );
        p_image = &p_imageSet->getImage( "Post" );
        _p_btnMsgArrived->setImage( p_image );
        _p_btnMsgArrived->hide();

        // create tab control
        _p_tabCtrl = static_cast< CEGUI::TabControl* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/TabControl", "_chatbox_channeltab_" ) );
        _p_tabCtrl->setPosition( CEGUI::Point( 0.02f, 0.15f ) );
        _p_tabCtrl->setSize( CEGUI::Size( 0.96f, 0.7f ) );
        _p_tabCtrl->setRelativeTabHeight( 0.15f );
        _p_frame->addChildWindow( _p_tabCtrl );
    }
    catch ( const CEGUI::Exception& e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** error setting up layout '" << layoutFile << "'" << std::endl;
        log << "   reason: " << e.getMessage().c_str() << std::endl;
        return;
    }
}

ChatGuiBox::ChannelTabPane* ChatGuiBox::getTabPane( const ChatConnectionConfig& cfg )
{
    ChatGuiBox::TabPanePairList::iterator p_beg = _tabpanes.begin(), p_end = _tabpanes.end();
    for ( ; p_beg != p_end; p_beg++ )
        if ( p_beg->first == cfg )
            return p_beg->second;

    return NULL;
}

void ChatGuiBox::destroyChannelPane( const ChatConnectionConfig& cfg )
{
    // close all tab panes
    ChatGuiBox::TabPanePairList::iterator p_beg = _tabpanes.begin(), p_end = _tabpanes.end();
    for ( ; p_beg != p_end; p_beg++ )
        if ( cfg._channel == p_beg->first._channel )
            break;

    // tollerate when a pane cannot be found
    if ( p_beg != p_end )
    {
        // deregister callback first
        p_beg->second->getConfiguration()._p_protocolHandler->deregisterProtocolCallback( p_beg->second );
        delete p_beg->second;
        _tabpanes.erase( p_beg );
    }
}

ChatGuiBox::ChannelTabPane* ChatGuiBox::getOrCreateChannelPane( const ChatConnectionConfig& cfg )
{
    // check if already have this pane
    ChannelTabPane* p_pane = getTabPane( cfg );
    if ( p_pane )
        return p_pane;

    // create new tab pane
    p_pane = new ChannelTabPane( _p_tabCtrl, this );
    p_pane->setTitle( cfg._protocol + " " + cfg._channel );

    // append the new channel to list
    _tabpanes.push_back( std::make_pair( cfg, p_pane ) );

    // register callback
    p_pane->setConfiguration( cfg );
    cfg._p_protocolHandler->registerProtocolCallback( p_pane, cfg._channel );

    return p_pane;
}

void ChatGuiBox::setEditBoxFocus( bool en )
{
    if ( !_p_tabCtrl )
        return;

    if ( _p_tabCtrl->getTabCount() > 0 )
    {
        unsigned int tabindex = _p_tabCtrl->getSelectedTabIndex();
        if( _tabpanes.size() )
        {
            ChannelTabPane* p_sel = _tabpanes[ tabindex ].second;
            p_sel->setEditBoxFocus( en );
        }
    }
}

void ChatGuiBox::update( float deltaTime )
{
    switch( _state )
    {
        case Idle:
            break;

        case BoxFadeIn:
        {
            _p_frame->show();
            if ( _fadeTimer > FADE_TIME )
            {
                _fadeTimer = 0;
                // restore the initial size
                CEGUI::Size size( _boxFrameSize.x(), _boxFrameSize.y() ); 
                _p_frame->setAlpha( _frameAlphaValue );
                _p_frame->setSize( size );
                _p_btnOpen->hide();
                setEditBoxFocus( true );
                _state = Idle;
                break;
            }
            _fadeTimer += deltaTime;
            // fade in the box
            float fadefac = _fadeTimer / FADE_TIME;
            CEGUI::Size size( _boxFrameSize.x() * fadefac, _boxFrameSize.y() * fadefac ); 
            _p_frame->setSize( size );
            _p_frame->setAlpha( fadefac * _frameAlphaValue );
            _p_btnOpen->setAlpha( std::max( 0.0f, ( 1.0f - ( _fadeTimer / FADE_TIME ) ) * _frameAlphaValue ) );
        }
        break;

        case BoxFadeOut:
        {
            _p_btnOpen->show();
            if ( _fadeTimer > FADE_TIME )
            {
                _fadeTimer = 0;
                // set size to zero
                CEGUI::Size size( 0, 0 ); 
                _p_frame->setSize( size );
                _p_frame->hide();
                setEditBoxFocus( false );
                _state = Idle;
                break;
            }
            _fadeTimer += deltaTime;
            // fade in the box
            float fadefac = std::max( 0.0f, 1.0f - ( _fadeTimer / FADE_TIME ) );
            CEGUI::Size size( _boxFrameSize.x() * fadefac, _boxFrameSize.y() * fadefac ); 
            _p_frame->setSize( size );
            _p_frame->setAlpha( fadefac * _frameAlphaValue );
            _p_btnOpen->setAlpha( ( _fadeTimer / FADE_TIME ) * _frameAlphaValue );
        }
        break;

        default:
            assert( NULL && "invalid chat gui state!" );

    }

    // check the tab pane removal queue
    while ( _queueRemoveTabPane.size() )
    { 
        ChannelTabPane* p_pane = _queueRemoveTabPane.front();
        destroyChannelPane( p_pane->getConfiguration() );
        _queueRemoveTabPane.pop();
    }
}

void ChatGuiBox::show( bool visible )
{
    if ( visible )
        _p_wnd->show();
    else
        _p_wnd->hide();
}

bool ChatGuiBox::onCloseFrame( const CEGUI::EventArgs& arg )
{
    onClickedOpen( arg );
    return true;
}

bool ChatGuiBox::onClickedOpen( const CEGUI::EventArgs& arg )
{
    // are we already in fading action?
    if ( _state != Idle )
        return true;

    _hidden = !_hidden;

    if ( _hidden )
    {
        // store the current size for later fade-out
        _boxFrameSize = osg::Vec2f( _p_frame->getSize().d_width, _p_frame->getSize().d_height );
        _state = BoxFadeOut;
    }
    else
    {
        _state = BoxFadeIn;
        _p_btnMsgArrived->hide();
    }

    return true;
}

void ChatGuiBox::onConnectionDialogClickedConnect( const ChatConnectionConfig& conf )
{
    _p_connectionDialog->show( false );

    // try to connect
    try
    {
        _p_chatMgr->createConnection( conf );
    }
    catch( const ChatExpection& e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "exception occured trying to connect to a chat server" << std::endl;
        log << "   reason: " << e.what() << std::endl;
    }
}

void ChatGuiBox::onConnectionDialogClickedCancel()
{
    _p_connectionDialog->show( false );
}

bool ChatGuiBox::onClickedIRC( const CEGUI::EventArgs& arg )
{
    if ( !_p_connectionDialog )
        _p_connectionDialog = new ChatGuiBox::ConnectionDialog( this );

    // set IRC as protocol
    ChatConnectionConfig& conf = _p_connectionDialog->getConfiguration();
    conf._protocol = "IRC";
    conf._port     = 6667;

    //! TODO: remove this test stuff
    conf._serverURL = "localhost";
    conf._channel  = "#vrc";
    conf._nickname = "boto";

    _p_connectionDialog->setConfiguration( conf );

    // show up the dialog
    _p_connectionDialog->setTitle( "IRC Connection Settings" );
    _p_connectionDialog->show( true );

    return true;
}

bool ChatGuiBox::onClickedDisconnect( const CEGUI::EventArgs& arg )
{
    // close all connections
    _p_chatMgr->closeConnections();

    // close all tab panes
    ChatGuiBox::TabPanePairList::iterator p_beg = _tabpanes.begin(), p_end = _tabpanes.end();
    for ( ; p_beg != p_end; p_beg++ )
        destroyChannelPane( p_beg->first );

    return true;
}

void ChatGuiBox::setupChatIO( const ChatConnectionConfig& config )
{
    ChannelTabPane* p_pane = getOrCreateChannelPane( config );
    // set tab selection to new created or exsiting pane
    p_pane->setSelection();
    // set focus
    p_pane->setEditBoxFocus( true );
}

} // namespace CTD
