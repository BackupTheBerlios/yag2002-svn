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
 # chat manager
 #
 #   date of creation:  10/16/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_chatmgr.h"
#include "vrc_chatguibox.h"
#include "vrc_chatguictrl.h"
#include "vrc_chatprotocol.h"

namespace vrc
{

// application window notification stuff
#define APP_WINDOW_NOTIFY_INTERVAL  0.7f
#define APP_WINDOW_NOTIFY_COUNT     6

ChatManager::ChatManager() :
_p_chatNetworkingVRC( NULL ),
_p_chatGuiCtrl( NULL ),
_p_chatGuiBox( NULL ),
_activeBox( false ),
_serverMode( false ),
_built( false ),
_enableAppWindowNotification( false ),
_appWindowNotifyTimer( 0.0f ),
_appWindowNotifyCounter( 0 )
{
    // store original window title
    _appWindowTitle = yaf3d::Application::get()->getWindowTitle();

    // register for getting application window state changes
    yaf3d::GameState::get()->registerCallbackAppWindowStateChange( this );
}

ChatManager::~ChatManager()
{
    closeConnections();

    if ( _p_chatGuiCtrl )
        delete _p_chatGuiCtrl;

    if ( _p_chatGuiBox )
        delete _p_chatGuiBox;

    // deregister for getting application window state changes
    yaf3d::GameState::get()->registerCallbackAppWindowStateChange( this, false );
}

void ChatManager::buildClient() throw ( ChatExpection )
{
    if ( !_availableProtocols.size() )
        throw ChatExpection( "No chat protocols available!" );

    if ( _built )
        throw ChatExpection( "Obviousely the chat manager has been already built!" );

    _p_chatGuiCtrl  = new ChatGuiCtrl;
    _p_chatGuiCtrl->initialize( this );

    _p_chatGuiBox   = new ChatGuiBox;
    _p_chatGuiBox->initialize( this );

    _serverMode     = false;
    _built          = true;
}

void ChatManager::buildServer() throw ( ChatExpection )
{
    if ( !_availableProtocols.size() )
        throw ChatExpection( "No chat protocols available!" );

    if ( _built )
        throw ChatExpection( "Obviousely the chat manager has been already built!" );

    _serverMode     = true;
    _built          = true;
}

bool ChatManager::registerChatProtocol( const std::string& prot, BaseChatProtocol* p_prot )
{
    // check if the protocol is already registered
    if ( _availableProtocols.find( prot ) != _availableProtocols.end() )
        return false;

    _availableProtocols[ prot ] = p_prot;

    return true;
}

ChatManager::ProtocolMap& ChatManager::getRegisteredProtocols()
{
    return _availableProtocols;
}

BaseChatProtocol* ChatManager::getRegisteredProtocol( const std::string& prot )
{
    // check if the protocol is already registered
    if ( _availableProtocols.find( prot ) == _availableProtocols.end() )
        return NULL;

    return _availableProtocols[ prot ];
}

void ChatManager::activateBox( bool en )
{
    assert( !_serverMode && "don't call this method in server mode!" );

    if ( _activeBox == en )
        return;

    _activeBox = en;
    _p_chatGuiCtrl->setEditMode( _activeBox );
    _p_chatGuiBox->setEditBoxFocus( _activeBox );
    gameutils::GuiUtils::get()->showMousePointer( _activeBox );
}

void ChatManager::show( bool en )
{
    assert( !_serverMode && "don't call this method in server mode!" );

    _p_chatGuiCtrl->show( en );
    _p_chatGuiBox->show( en );
}

void ChatManager::createConnection( const ChatConnectionConfig& config ) throw ( ChatExpection )
{
    // this method has no effect on server
    assert( !_serverMode && "don't call this method in server mode!" );
    assert( ( _availableProtocols.find( config._protocol ) != _availableProtocols.end() ) && "requested chat protocol not availabe!" );

    // first check if there is already a connection to requested server
    ChatConnections::iterator p_beg = _connections.begin(), p_end = _connections.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        if ( p_beg->first._serverURL == config._serverURL )
        {
            throw ChatExpection( "Connection to server already exists." );
        }
    }

    // create a new chat handler from registered prototype
    BaseChatProtocol* p_prot = _availableProtocols[ config._protocol ];
    BaseChatProtocol* p_prothandler = p_prot->createInstance();
    p_prothandler->registerProtocolCallback( this, "*" ); // set an unfiltered protocol callback for receiving connection notification
    try
    {
        // create a pane for server connection
        ChatConnectionConfig conectiontabconf;
        // for server connection panes we set channel and server url both to server url
        conectiontabconf._channel   = config._serverURL;
        conectiontabconf._serverURL = config._serverURL;
        conectiontabconf._protocol  = config._protocol;
        conectiontabconf._p_protocolHandler = p_prothandler;
        _p_chatGuiBox->setupChatIO( conectiontabconf, true );

        // start creating the connection
        p_prothandler->createConnection( config );
    }
    catch( const ChatExpection& e )
    {
        // could not setup connection       
        yaf3d::MessageBoxDialog* p_msg = new yaf3d::MessageBoxDialog( "Connection Problem", std::string( "Reason: " ) + e.what(), yaf3d::MessageBoxDialog::OK, true );
        p_msg->show();
    }
}

void ChatManager::closeConnections()
{
    // freeup all connections
    ChatConnections::iterator p_beg = _connections.begin(), p_end = _connections.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        _p_chatGuiBox->outputText( "*", "*** we have been disconnected from server: " + p_beg->first._serverURL );
        p_beg->second->destroyConnection();
        delete p_beg->second;
    }
    _connections.clear();
}

void ChatManager::setVRCNickName( const std::string& nick )
{
    std::string cmd( "/nick " );
    cmd += nick;
    _p_chatNetworkingVRC->send( cmd, "", "" );
}

void ChatManager::onConnection( const ChatConnectionConfig& config )
{
    // VRC connection is handled special
    if ( config._protocol != VRC_PROTOCOL_NAME )
    {
        // store the new created connection for internal housekeeping
        _connections.push_back( std::make_pair( config, config._p_protocolHandler ) );
    }
    else
    {
        _p_chatNetworkingVRC = config._p_protocolHandler;
    }

    // check the app window state and animate it if minimized
    if ( !_serverMode )
        notifyAppWindow( "connected" );
}

void ChatManager::onDisconnection( const ChatConnectionConfig& config )
{
    // remove connection
    ChatConnections::iterator p_beg = _connections.begin(), p_end = _connections.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        if ( config._p_protocolHandler == p_beg->second )
            break;
    }
    if ( p_beg != p_end )
    {
        delete config._p_protocolHandler;
        _connections.erase( p_beg );
    }

    if ( !_serverMode )
        notifyAppWindow( "disconnected" );
}

void ChatManager::onJoinedChannel( const ChatConnectionConfig& config )
{
    // create a new chat io for the channel
    if ( !_serverMode )
    {
        _p_chatGuiBox->setupChatIO( config );

        notifyAppWindow( config._nickname + " joined " + config._channel );
    }
}

void ChatManager::onReceiveSystemMessage( const std::string& msg )
{
    if ( !_serverMode )
        _p_chatGuiBox->outputText( "*", msg );
}

void ChatManager::onReceive( const std::string& channel, const std::string& sender, const CEGUI::String& msg )
{
    std::string smsg( reinterpret_cast< const char* >( msg.c_str() ) );
    if ( !_serverMode )
        notifyAppWindow( channel + " [" + sender + "] " + smsg );
}

void ChatManager::onAppWindowStateChange( unsigned int state )
{
    if ( ( state == yaf3d::GameState::Restored ) || ( state == yaf3d::GameState::GainedFocus ) )
    {
        // restore the original app window title
        yaf3d::Application::get()->setWindowTitle( _appWindowTitle );
        _appWindowNotifyCounter      = 0;
        _enableAppWindowNotification = false;
    }
    else
    {
        _enableAppWindowNotification = true;
    }
}

void ChatManager::notifyAppWindow( const std::string& text )
{
    if ( !_enableAppWindowNotification )
        return;

    _appWindowNotifyCounter = APP_WINDOW_NOTIFY_COUNT;
    _appWindowNotifyTimer   = 0.0f;
    _appWindowNotifyText    = text;

    // begin with title animation
    yaf3d::Application::get()->setWindowTitle( _appWindowNotifyText );
}

void ChatManager::update( float deltaTime )
{
    if ( !_serverMode )
    {
        // update gui box
        _p_chatGuiBox->update( deltaTime );

        // app icon animation whenever needed
        if ( _appWindowNotifyCounter > 0 )
        {
            _appWindowNotifyTimer += deltaTime;

            if ( _appWindowNotifyTimer > APP_WINDOW_NOTIFY_INTERVAL )
            {
                _appWindowNotifyTimer = 0.0f;
                --_appWindowNotifyCounter;

                // toggle the app title
                if ( _appWindowNotifyCounter % 2 )
                {
                    yaf3d::Application::get()->setWindowTitle( _appWindowTitle );
                }
                else
                {
                    yaf3d::Application::get()->setWindowTitle( _appWindowNotifyText );
                }
            }
        }
    }
}

} // namespace vrc
