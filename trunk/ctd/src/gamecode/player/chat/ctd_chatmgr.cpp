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

#include <ctd_main.h>
#include "ctd_chatmgr.h"
#include "ctd_chatguibox.h"
#include "ctd_chatguictrl.h"
#include "ctd_chatprotocol.h"

namespace CTD
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
    GuiManager::get()->showMousePointer( _activeBox );
}

void ChatManager::show( bool en )
{
    assert( !_serverMode && "don't call this method in server mode!" );

    _p_chatGuiBox->show( en );
    _p_chatGuiCtrl->show( en );
}

void ChatManager::createConnection( const ChatConnectionConfig& config )
{
    // this method has no effect on server
    assert( !_serverMode && "don't call this method in server mode!" );
    assert( ( _availableProtocols.find( config._protocol ) != _availableProtocols.end() ) && "requested chat protocol not availabe!" );

    // first check if there is already a connection to requested server
    ChatConnections::iterator p_beg = _connections.begin(), p_end = _connections.end();
    for ( ; p_beg != p_end; p_beg++ )
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
        MessageBoxDialog* p_msg = new MessageBoxDialog( "Connection error", "Could not connect to server.\n\n" + e.what(), MessageBoxDialog::OK, true );
        p_msg->show();
    }

    //! TODO: 
    // start a timer, after about 30 seconds without connecting a message box should appear notifying that no connection possible
    // in that time no new connection should be possible (disable connection button)
}

void ChatManager::closeConnections()
{
    // freeup all connections
    ChatConnections::iterator p_beg = _connections.begin(), p_end = _connections.end();
    for ( ; p_beg != p_end; p_beg++ )
    {
        p_beg->second->destroyConnection();
    }
    _connections.clear();
}

void ChatManager::onConnection( const ChatConnectionConfig& config )
{
    // store the new created connection for internal housekeeping
    _connections.push_back( std::make_pair( config, config._p_protocolHandler ) );
}

void ChatManager::onDisconnection( const ChatConnectionConfig& config )
{
    // remove connection
    ChatConnections::iterator p_beg = _connections.begin(), p_end = _connections.end();
    for ( ; p_beg != p_end; p_beg++ )
    {
        if ( config._p_protocolHandler == p_beg->second )
            break;
    }
    assert( ( p_beg != p_end ) && "chat protocol handler was previously not stored!" );
    _connections.erase( p_beg );
}

void ChatManager::onJoinedChannel( const ChatConnectionConfig& config )
{
    // create a new chat io for the channel
    if ( !_serverMode )
        _p_chatGuiBox->createChatIO( config );
}

void ChatManager::update( float deltaTime )
{
    if ( !_serverMode )
        _p_chatGuiBox->update( deltaTime );
}

} // namespace CTD
