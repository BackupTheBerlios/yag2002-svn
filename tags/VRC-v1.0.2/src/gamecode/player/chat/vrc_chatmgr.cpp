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
 # chat manager
 #
 #   date of creation:  10/16/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
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

ChatManager::ChatManager() :
_p_chatGuiCtrl( NULL ),
_p_chatGuiBox( NULL ),
_activeBox( false ),
_serverMode( false ),
_built( false )
{
}

ChatManager::~ChatManager()
{
    closeConnections();

    if ( _p_chatGuiCtrl )
        delete _p_chatGuiCtrl;

    if ( _p_chatGuiBox )
        delete _p_chatGuiBox;
}

bool ChatManager::buildClient()
{
    assert( _availableProtocols.size() && "no chat protocols available!" );
    assert( !_built && "obviousely the chat manager has been already built!" );

    _p_chatGuiCtrl  = new ChatGuiCtrl;
    _p_chatGuiCtrl->initialize( this );

    _p_chatGuiBox   = new ChatGuiBox;
    _p_chatGuiBox->initialize( this );

    _serverMode     = false;
    _built          = true;

    return true;
}

bool ChatManager::buildServer()
{
    assert( _availableProtocols.size() && "no chat protocols available!" );
    assert( !_built && "obviousely the chat manager has been already built!" );

    _serverMode     = true;
    _built          = true;

    return true;
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

void ChatManager::onConnection( const ChatConnectionConfig& config )
{
    // VRC connection is handled special
    if ( config._protocol != VRC_PROTOCOL_NAME )
    {
        // store the new created connection for internal housekeeping
        _connections.push_back( std::make_pair( config, config._p_protocolHandler ) );
    }
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
}

void ChatManager::onJoinedChannel( const ChatConnectionConfig& config )
{
    // create a new chat io for the channel
    if ( !_serverMode )
        _p_chatGuiBox->setupChatIO( config );
}

void ChatManager::onReceiveSystemMessage( const std::string& msg )
{
    if ( !_serverMode )
        _p_chatGuiBox->outputText( "*", msg );
}

void ChatManager::update( float deltaTime )
{
    if ( !_serverMode )
        _p_chatGuiBox->update( deltaTime );
}

} // namespace vrc
