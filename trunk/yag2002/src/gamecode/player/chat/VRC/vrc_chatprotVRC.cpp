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
 # VRC built-in chat protocol
 #
 #   date of creation:  10/13/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_chatprotVRC.h"
#include "../vrc_chatmgr.h"
#include "../../vrc_playerimpl.h"
#include "networkingRoles/_RO_PlayerChat.h"
#include <RNReplicaNet/Inc/DataBlock_Function.h>

using namespace RNReplicaNet;

#define VRC_CMD_LIST    "\n"\
                        "/help\n"\
                        "/names\n"\
                        "/nick [nickname]"

ImplChatNetworkingVRC::ImplChatNetworkingVRC( ChatNetworkingVRC* p_nw ) :
_p_protVRC( p_nw )
{
    // store sesstion ids
    _serverSID = GetSessionID();
    _clientSID = yaf3d::NetworkDevice::get()->getSessionID();
    assert( ( _serverSID >= 0 ) && "invalid server session id!" );
    assert( ( _clientSID >= 0 ) && "invalid client session id!" );

    _config._sessionID = _clientSID;
    strcpy( _config._realname, "" );

    // p_nw is NULL then the instance is created by ReplicaNet, i.e. this instance is a ghost on a client
    // the only master instance is created on server
    if ( !_p_protVRC )
    {
        _p_protVRC = new ChatNetworkingVRC;
        _p_protVRC->setNetworkingImpl( this );

        // we have to setup the player chat manager now when the object is replicated
        vrc::ChatManager* p_playerChatMgr = vrc::BasePlayerImplementation::getChatManager();
        _p_protVRC->registerProtocolCallback( p_playerChatMgr, "*" );

        // use player name as default nickname
        std::string playername;
        yaf3d::Configuration::get()->getSettingValue( VRC_GS_PLAYER_NAME, playername );
        strcpy( _config._nickname, playername.c_str() );
    }
    else
    { // this code is executed on server
        strcpy( _config._nickname, "" );
        // register for getting network session joining / leaving notification
        yaf3d::NetworkDevice::get()->registerSessionNotify( this );
    }
}

ImplChatNetworkingVRC::~ImplChatNetworkingVRC()
{
    _p_protVRC->setNetworkingImpl( NULL );
}

void ImplChatNetworkingVRC::PostObjectCreate()
{// this method is called by ReplicaNet only when an object is replicated here
    
    // notify connection
    _p_protVRC->connected();

    // do an RPC on server object
    log_debug << "requesting VRC server to join ..." << std::endl;
    NOMINATED_REPLICAS_FUNCTION_CALL( 1, &_serverSID, RPC_RequestJoin( _config ) );
}

void ImplChatNetworkingVRC::onSessionJoined( int sessionID )
{
    // nothing to do
}

void ImplChatNetworkingVRC::onSessionLeft( int sessionID )
{ // this callback is done only on server ( see registration in constructor )
    // inform all remaining clients about left client
    tChatData chatdata;
    strcpy( chatdata._nickname, _nickNames[ sessionID ].c_str() );
    chatdata._sessionID = sessionID;
    RPC_RequestLeave( chatdata );
}

void ImplChatNetworkingVRC::getMemberList( const std::string& channel, std::vector< std::string >& list )
{
    // currently 'channel' is unused in VRC protocol!
    std::map< int, std::string >::iterator p_beg = _nickNames.begin(), p_end = _nickNames.end();
    for ( ; p_beg != p_end; ++p_beg )
        list.push_back( p_beg->second );
}

std::string ImplChatNetworkingVRC::makeUniqueNickname( const std::string& reqnick )
{
    std::string  newnick( reqnick );
    bool         found   = false;
    unsigned int postfix = 0;
    do
    {
        std::map< int, std::string >::iterator p_beg = _nickNames.begin(), p_end = _nickNames.end();
        for ( ; p_beg != p_end; ++p_beg )
            if ( p_beg->second == newnick )
                break;

        if ( p_beg == p_end )
        {
            found = true;
        }
        else
        {
            std::stringstream postf;
            postf << postfix;
            newnick = reqnick + postf.str();
            ++postfix;
        }
        
    } while ( !found );

    return newnick;
}

void ImplChatNetworkingVRC::RPC_InitializeClient( tChatData chatdata )
{
    chatdata._nickname[ sizeof( chatdata._nickname ) - 1 ] = '\0';
    log_debug << "successfully joined to VRC chat server, our nickname: " << chatdata._nickname << std::endl;
    // store the server given nick name
    std::string channel;
    _p_protVRC->recvNicknameChange( chatdata._nickname, _config._nickname );
    strcpy( _config._nickname, chatdata._nickname );
}

void ImplChatNetworkingVRC::RPC_RequestJoin( tChatData chatdata )
{
    log_debug << "VRC chat server got joining request" << std::endl;
    log_debug << "nickname: " << chatdata._nickname << ", sid: " << chatdata._sessionID << std::endl;

    chatdata._nickname[ sizeof( chatdata._nickname ) - 1 ] = '\0';
    // create a unique nickname
    std::string  reqnick( chatdata._nickname );
    std::string  newnick = makeUniqueNickname( reqnick );

    // store nickname
    _nickNames[ chatdata._sessionID ] = newnick;
    strcpy( chatdata._nickname, newnick.c_str() );

    // init new client
    NOMINATED_REPLICAS_FUNCTION_CALL( 1, &chatdata._sessionID, RPC_InitializeClient( chatdata ) );

    // notify all clients about new chat memeber
    ALL_REPLICAS_FUNCTION_CALL( RPC_ClientJoined( chatdata ) );

    _p_protVRC->joined( "", newnick );
}

void ImplChatNetworkingVRC::RPC_ClientJoined( tChatData chatdata )
{// this is called whenever a new chat memeber enters the network
    // try to get the old nickname first
    chatdata._nickname[ sizeof( chatdata._nickname ) - 1 ] = '\0';
    std::string newname( chatdata._nickname );
    // store new or changed nickname
    _nickNames[ chatdata._sessionID ] = newname;
    // notify protocol handler
    std::string channel;
    _p_protVRC->joined( channel, newname );

    // get a fresh member list
    NOMINATED_REPLICAS_FUNCTION_CALL( 1, &_serverSID, RPC_RequestMemberList( chatdata ) );

    // notify for updating member list
    _p_protVRC->recvMemberList( "" );
}

void ImplChatNetworkingVRC::RPC_RequestLeave( tChatData chatdata )
{ // this is called only on server

    log_debug << "request leaving: " << chatdata._nickname << " " << chatdata._sessionID << std::endl;

    // remove client form nickname lookup
    std::map< int, std::string >::iterator p_it = _nickNames.find( chatdata._sessionID );
    if ( p_it != _nickNames.end() )
        _nickNames.erase( p_it );

    ALL_REPLICAS_FUNCTION_CALL( RPC_ClientLeft( chatdata ) );

    _p_protVRC->left( "", chatdata._nickname );
}

void ImplChatNetworkingVRC::RPC_ClientLeft( tChatData chatdata )
{ // this is called only on clients

    // avoid echos
    if ( chatdata._sessionID == _clientSID )
        return;

    chatdata._nickname[ sizeof( chatdata._nickname ) - 1 ] = '\0';
    std::string name( chatdata._nickname );

    // remove client form nickname lookup
    std::map< int, std::string >::iterator p_it = _nickNames.find( chatdata._sessionID );
    if ( p_it != _nickNames.end() )
        _nickNames.erase( p_it );

    _p_protVRC->left( "", name );

    // notify for updating member list
    _p_protVRC->recvMemberList( "" );
}

void ImplChatNetworkingVRC::RPC_RequestChangeNickname( tChatData chatdata )
{ // this is called only on server
    chatdata._nickname[ sizeof( chatdata._nickname ) - 1 ] = '\0';
    // create a unique nickname
    std::string  oldname = _nickNames[ chatdata._sessionID ];
    std::string  reqnick( chatdata._nickname );
    std::string  newnick = makeUniqueNickname( reqnick );

    // store nickname
    _nickNames[ chatdata._sessionID ] = newnick;
    strcpy( chatdata._nickname, newnick.c_str() );

    // notify all clients about changed nickname
    ALL_REPLICAS_FUNCTION_CALL( RPC_ChangedNickname( chatdata ) );

    _p_protVRC->recvNicknameChange( newnick, oldname );
}

void ImplChatNetworkingVRC::RPC_ChangedNickname( tChatData chatdata )
{ // this is called only on clients
    chatdata._nickname[ sizeof( chatdata._nickname ) - 1 ] = '\0';
    std::string newname( chatdata._nickname );
    std::string oldname;
    if ( _nickNames.size() )
        oldname = _nickNames[ chatdata._sessionID ];

    // store new or changed nickname
    _nickNames[ chatdata._sessionID ] = newname;

    if ( chatdata._sessionID == _clientSID ) // this means _we_ have changed our nick name
    {
        _p_protVRC->recvNicknameChange( newname, newname );

        // if _we_ changed our nick name then change also the player name ( only on client )
        vrc::EnPlayer* p_player = dynamic_cast< vrc::EnPlayer* >( vrc::gameutils::PlayerUtils::get()->getLocalPlayer() );
        p_player->setPlayerName( newname );
    }
    else // another one has changed nickname
    {
        _p_protVRC->recvNicknameChange( newname, oldname );
    }

    // notify for updating member list
    _p_protVRC->recvMemberList( "" );
}

void ImplChatNetworkingVRC::RPC_RequestMemberList( tChatData chatdata )
{ // this is called only on server
    std::map< int, std::string >::iterator p_beg = _nickNames.begin(), p_end = _nickNames.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        tChatData data;
        strcpy( data._nickname, p_beg->second.c_str() );
        data._sessionID = p_beg->first;
        NOMINATED_REPLICAS_FUNCTION_CALL( 1, &chatdata._sessionID, RPC_RecvMemberList( data ) );  
    }
}

void ImplChatNetworkingVRC::RPC_RecvMemberList( tChatData chatdata )
{ // this is called only on clients in order to get current members
    chatdata._nickname[ sizeof( chatdata._nickname ) - 1 ] = '\0';
    std::string newnick( chatdata._nickname );
    _nickNames[ chatdata._sessionID ] = newnick;
    _p_protVRC->recvMemberList( "" );
}

void ImplChatNetworkingVRC::RPC_RecvChatText( tChatMsg chatMsg )
{
    // cancel echos
    if ( chatMsg._sessionID == _clientSID )
        return;

    chatMsg._text[ 255 ] = 0; // limit text length    
    _p_protVRC->recvMessage( "VRC", _nickNames[ chatMsg._sessionID ], reinterpret_cast< char* >( chatMsg._text ) );
}

void ImplChatNetworkingVRC::RPC_PostChatText( tChatMsg chatMsg )
{ // this method is called only on server

    chatMsg._text[ 250 ] = 0; // limit text length
    // here some text filtering can be done before distributing new post to all clients
    ALL_REPLICAS_FUNCTION_CALL( RPC_RecvChatText( chatMsg ) );

    _p_protVRC->recvMessage( "VRC", _nickNames[ chatMsg._sessionID ], reinterpret_cast< char* >( chatMsg._text ) );
}

void ImplChatNetworkingVRC::leave()
{
    tChatData chatdata;
    strcpy( chatdata._nickname, _nickNames[ _clientSID ].c_str() );
    chatdata._sessionID = _clientSID;
    // send out notification to server
    NOMINATED_REPLICAS_FUNCTION_CALL( 1, &_serverSID, RPC_RequestLeave( chatdata ) );
}

void ImplChatNetworkingVRC::postChatText( const CEGUI::String& text )
{
    // check for commands
    if ( !text.compare( 0, 1, "/" ) )
    {
        std::vector< std::string > args;
        yaf3d::explode( text.c_str(), " ", &args );

        // all commands without arguments go here
        if ( args.size() == 1 )
        {
            if ( ( args[ 0 ] == "/names" ) || ( args[ 0 ] == "/NAMES" ) )
            {
                tChatData chatdata;
                chatdata._sessionID = _clientSID;
                NOMINATED_REPLICAS_FUNCTION_CALL( 1, &_serverSID, RPC_RequestMemberList( chatdata ) );
                return;
            } 
            else
            {
                _p_protVRC->recvMessage( "", "", VRC_CMD_LIST );
                return;
            }

        }
        // all commands with one single argument go here
        else if ( args.size() == 2 )        
        {
            if ( ( args[ 0 ] == "/nick" ) || ( args[ 0 ] == "/NICK" ) )
            {
                tChatData chatdata;
                chatdata._sessionID = _clientSID;
                strcpy( chatdata._nickname, args[ 1 ].c_str() );
                NOMINATED_REPLICAS_FUNCTION_CALL( 1, &_serverSID, RPC_RequestChangeNickname( chatdata ) );  
                return;
            }
            else
            {
                _p_protVRC->recvMessage( "", "", VRC_CMD_LIST );
                return;
            }
        }
        else
        {
            _p_protVRC->recvMessage( "", "", VRC_CMD_LIST );
            return;
        }
    }
    else // if no command given then send the raw text
    {
        // prepare the telegram
        tChatMsg textdata;
        memset( textdata._text, 0, sizeof( tChatMsg ) ); // zero out the text buffer
        text.copy( textdata._text );
        textdata._sessionID = _clientSID;
        NOMINATED_REPLICAS_FUNCTION_CALL( 1, &_serverSID, RPC_PostChatText( textdata ) );
    }    
}

//------------

ChatNetworkingVRC::ChatNetworkingVRC() :
_p_nwImpl( NULL ),
_p_config( NULL )
{
    _p_config  = new vrc::ChatConnectionConfig;
    _p_config->_nickname = "NOT-SET";
    _p_config->_protocol = "VRC";
    _p_config->_p_protocolHandler = this;
}

ChatNetworkingVRC::~ChatNetworkingVRC()
{
    if ( _p_nwImpl )
        delete _p_nwImpl;

    if ( _p_config )
        delete _p_config;
}

vrc::BaseChatProtocol* ChatNetworkingVRC::createInstance()
{
    // create and setup VRC protocol handler, this is used on server
    // on clients the instance is created by ReplicaNet
    _p_nwImpl = new ImplChatNetworkingVRC( this );
    setNetworkingImpl( _p_nwImpl );
    // publish the chat networking agent in net
    _p_nwImpl->Publish();

    return this;
}

void ChatNetworkingVRC::createConnection( const vrc::ChatConnectionConfig& conf ) throw ( vrc::ChatExpection )
{
}

void ChatNetworkingVRC::destroyConnection()
{
    if ( _p_nwImpl )
    {
        // send out leaving notification if not running in server mode
        _p_nwImpl->leave();
    }
}

void ChatNetworkingVRC::send( const std::string& msg, const std::string& channel )
{
    if ( _p_nwImpl )
        _p_nwImpl->postChatText( msg );
}

void ChatNetworkingVRC::connected()
{
    // send this notification unfiltered
    ProtocolCallbackList::iterator p_beg = _protocolCallbacks.begin(), p_end = _protocolCallbacks.end();
    for ( ; p_beg != p_end; ++p_beg )
        p_beg->second->onConnection( *_p_config );
}

void ChatNetworkingVRC::disconnected()
{
    // send this notification unfiltered
    ProtocolCallbackList::iterator p_beg = _protocolCallbacks.begin(), p_end = _protocolCallbacks.end();
    for ( ; p_beg != p_end; ++p_beg )
        p_beg->second->onDisconnection( *_p_config );
}

void ChatNetworkingVRC::getMemberList( const std::string& channel, std::vector< std::string >& list )
{
    if ( _p_nwImpl )
        _p_nwImpl->getMemberList( channel, list );
}

void ChatNetworkingVRC::joined( const std::string& channel, const std::string& name )
{
    vrc::ChatConnectionConfig cfg( *_p_config );
    cfg._nickname = name;
    ProtocolCallbackList::iterator p_beg = _protocolCallbacks.begin(), p_end = _protocolCallbacks.end();
    for ( ; p_beg != p_end; ++p_beg )
        if ( ( p_beg->first == channel ) || ( p_beg->first == "*" ) )
            p_beg->second->onJoinedChannel( cfg );
}

void ChatNetworkingVRC::left( const std::string& channel, const std::string& name )
{
    vrc::ChatConnectionConfig cfg( *_p_config );
    cfg._nickname = name;
    ProtocolCallbackList::iterator p_beg = _protocolCallbacks.begin(), p_end = _protocolCallbacks.end();
    for ( ; p_beg != p_end; ++p_beg )
        if ( ( p_beg->first == channel ) || ( p_beg->first == "*" ) )
            p_beg->second->onLeftChannel( cfg );
}

void ChatNetworkingVRC::recvNicknameChange( const std::string& newname, const std::string& oldname )
{
    ProtocolCallbackList::iterator p_beg = _protocolCallbacks.begin(), p_end = _protocolCallbacks.end();
    for ( ; p_beg != p_end; ++p_beg )
        if ( ( p_beg->first == _p_config->_channel ) || ( p_beg->first == "*" ) )
            p_beg->second->onNicknameChanged( newname, oldname );
}

void ChatNetworkingVRC::recvMessage( const std::string& channel, const std::string& sender, const std::string& msg )
{
    ProtocolCallbackList::iterator p_beg = _protocolCallbacks.begin(), p_end = _protocolCallbacks.end();
    for ( ; p_beg != p_end; ++p_beg )
        // check also for unfiltered callbacks ( '*' )
        if ( ( channel == p_beg->first ) || ( p_beg->first == "*" ) )
            p_beg->second->onReceive( channel, sender, msg );
}

void ChatNetworkingVRC::requestMemberList( const std::string& channel )
{
    // this protocol has always an up-to-date member list
    // just ready to get, so notify the world about it.
    recvMemberList( channel );
}

void ChatNetworkingVRC::recvMemberList( const std::string& channel )
{
    ProtocolCallbackList::iterator p_beg = _protocolCallbacks.begin(), p_end = _protocolCallbacks.end();
    for ( ; p_beg != p_end; ++p_beg )
        // check also for unfiltered callbacks ( '*' )
        if ( ( channel == p_beg->first ) || ( p_beg->first == "*" ) )
            p_beg->second->onReceiveMemberList( channel );
}
