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
 # networking for chat member
 #
 # this class implements the networking functionality for chat member
 #
 #
 #   date of creation:  12/25/2004
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_playernetworking.h"
#include "vrc_playerimpl.h"
#include "vrc_playerimplclient.h"
#include "vrc_playerimplserver.h"
#include "vrc_playerphysics.h"
#include "vrc_player.h"
#include "chat/vrc_chatmgr.h"
#include <RNReplicaNet/Inc/DataBlock_Function.h>

using namespace RNReplicaNet;

PlayerNetworking::PlayerNetworking( vrc::BasePlayerImplementation* p_playerImpl ) :
_positionX( 0 ),
_positionY( 0 ),
_positionZ( 0 ),
_yaw( 0 ),
_cmdAnimFlags( 0 ),
_voiceChat( false ),
_voiceChatEnable( false ),
_remoteClient( false ),
_remoteClientInitialized( false ),
_p_playerImpl( p_playerImpl ),
_loadedPlayerEntity( NULL )
{
    _p_configFile[ 0 ] = 0;
    _cmdAnimFlags      = 0;
    _ip[ 0 ]           = 0;

    // this constructor can be called either by player entity or networking system (in client or server mode)
    //  when called by player entity then it means that we are a local client, otherwise we are a remote client
    if ( !p_playerImpl )
    {
        _remoteClient = true;
        _p_playerName[ 0 ] = 0;
    }
    else
    {
        strcpy( _p_playerName, _p_playerImpl->getPlayerEntity()->getPlayerName().c_str() );

        // set the voice chat flag so it can be set by replicas on initialization
        yaf3d::Configuration::get()->getSettingValue( VRC_GS_VOICECHAT_ENABLE, _voiceChat );
        _p_playerImpl->getPlayerEntity()->setVoiceChatEnabled( _voiceChat );
        _voiceChatEnable = _voiceChat;

        // if a public host is given then set our ip to that host ip
        std::string publichost;
        yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_PUBLIC_HOST, publichost );
        publichost = yaf3d::resolveHostName( publichost );
        if ( publichost.length() )
            strcpy( _ip, publichost.c_str() );
    }
}

PlayerNetworking::~PlayerNetworking()
{
    log_info << "player left: " << _p_playerName << std::endl;

    // remove ghost from simulation ( server and client )

    // remove remote player form voice chat list
    if ( isRemoteClient() )
    {
        // remove voice chat notify registration
        if ( _voiceChat && ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Client ) )
            vrc::gameutils::PlayerUtils::get()->removeRemotePlayerVoiceChat( _p_playerImpl->getPlayerEntity() );
    }

    // we have to delete player associated entities only if we are not unloading the level or quitting/shutdown
    if ( !( yaf3d::GameState::get()->getState() & ( yaf3d::GameState::LeavingLevel | yaf3d::GameState::Quitting | yaf3d::GameState::Shutdown ) ) )
    {
        // filter server and remoteclient objects ( no clients' local objects )
        if ( isRemoteClient() )
        {
            // PlayerNetworking has created the player implementation, so set its networking and other components to NULL in order to avoid deleting it also by player's implementation
            _p_playerImpl->setPlayerNetworking( NULL );
            _p_playerImpl->setPlayerSound( NULL );
            _p_playerImpl->setPlayerAnimation( NULL );
            _p_playerImpl->setPlayerPhysics( NULL );

            // remove all associated entities
            std::vector< yaf3d::BaseEntity* >::iterator p_beg = _loadedEntities.begin(), p_end = _loadedEntities.end();
            for ( ; p_beg != p_end; ++p_beg )
                yaf3d::EntityManager::get()->deleteEntity( *p_beg );
        }
    }
}

void PlayerNetworking::PostObjectCreate()
{ // this method is only called for remote clients ( on server and clients )

    // complete setting up ghost ( remote client ) or server-side player
    if ( isRemoteClient() )
    {
        // struct for sending init data
        static tInitializationData init;

        // create the player and its components
        createPlayer();

        // set session if in player entity
        int sid = GetSessionID();
        _loadedPlayerEntity->setNetworkID( sid );
        log_verbose << "  remote player's network ID: " << sid << std::endl;

        // setup new connected client
        if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server )
        {
            // send intialization data
            osg::Vec3f pos = _loadedPlayerEntity->getPlayerImplementation()->getPlayerPosition();
            _positionX = pos.x();
            _positionY = pos.y();
            _positionZ = pos.z();
            osg::Quat  rot = _loadedPlayerEntity->getPlayerImplementation()->getPlayerRotation();
            double angle;
            osg::Vec3d vec( 0.0, 0.0, 1.0 );
            rot.getRotate( angle, vec );
            _yaw = angle;
            init._posX = _positionX;
            init._posY = _positionY;
            init._posZ = _positionZ;
            init._rotZ = _yaw;

            // determine the public client ip and tell it to new connected client
            // consider also a possible "public host" set in client, if so _ip already contains the valid IP address
            _ip[ sizeof( _ip ) - 1 ] = 0;
            std::string ip;

            if ( _ip[ 0 ] == 0 )
                ip = yaf3d::NetworkDevice::get()->getClientIP( sid );
            else
                ip = _ip;

            strcpy( init._ip, ip.c_str() );
            log_debug << "  session id / client ip: " << sid << " / " << ip << std::endl;

            // set ip address in player entity
            _loadedPlayerEntity->setIPAdress( ip );

            // grant new connected client to session with initialization data
            ALL_REPLICAS_FUNCTION_CALL( RPC_ServerGrantsAccess( init ) );
        }
        else // client code
        {
            // check if we have a valid ip
            if ( _ip[ 0 ] )
            {
                // terminate the ip string to be on safe side
                _ip[ sizeof( _ip ) - 1 ] = 0;
                _p_playerImpl->getPlayerEntity()->setIPAdress( _ip );

                // is voice chat enabled?
                _p_playerImpl->getPlayerEntity()->setVoiceChatEnabled( _voiceChat );
                if ( _voiceChat )
                    vrc::gameutils::PlayerUtils::get()->addRemotePlayerVoiceChat( _p_playerImpl->getPlayerEntity() );

                _voiceChatEnable = _voiceChat;
            }

            // request initialization data from server
            MASTER_FUNCTION_CALL( RPC_RequestInitialization() );
        }
    }

    log_info << "  remote player created: " << _p_playerName << std::endl;
}

void PlayerNetworking::RPC_RequestInitialization()
{ // this method is called only on server

    assert( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Client );

    log_info << "  remote player requests for initialization: " << _p_playerName << std::endl;

    static tInitializationData init;
    init._posX = _positionX;
    init._posY = _positionY;
    init._posZ = _positionZ;
    init._rotZ = _yaw;

    if ( _ip[ 0 ] )
        strcpy_s( init._ip, sizeof( init._ip ), _ip );
    else
        memset( init._ip, 0, sizeof( init._ip ) );

    // call the initialize function on the remote players (ghosts)
    ALL_REPLICAS_FUNCTION_CALL( RPC_Initialize( init ) );
}

void PlayerNetworking::RPC_ServerGrantsAccess( tInitializationData initData )
{ // this method is called on new connected client

    log_info << "  player networking: server grants access to network session" << std::endl;

    assert( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Client );

    // init player
    {
        // init player position set by server ( it's the job of server to init the player position and rotation )
        _p_playerImpl->setPlayerPosition( osg::Vec3f( initData._posX, initData._posY, initData._posZ ) );
        _p_playerImpl->setPlayerRotation( osg::Quat( initData._rotZ, osg::Vec3f( 0.0f, 0.0f, 1.0f ) ) );

        // set physics body transformation
        osg::Matrixf mat;
        mat *= mat.rotate( _p_playerImpl->getPlayerRotation() );
        mat.setTrans( _p_playerImpl->getPlayerPosition() );
        _p_playerImpl->getPlayerPhysics()->setTransformation( mat );

        _positionX = initData._posX;
        _positionY = initData._posY;
        _positionZ = initData._posZ;
        _yaw       = initData._rotZ;

        unsigned char breaktype = static_cast< unsigned char >( RNReplicaNet::DataBlock::kTeleport | RNReplicaNet::DataBlock::kSuddenChange );
        ContinuityBreak( _positionX, breaktype );
        ContinuityBreak( _positionY, breaktype );
        ContinuityBreak( _positionZ, breaktype );
        ContinuityBreak( _yaw, breaktype );

        // store our public ip
        initData._ip[ sizeof( _ip ) - 1 ] = 0;
        strcpy( _ip, initData._ip );
        _p_playerImpl->getPlayerEntity()->setIPAdress( _ip );
    }

    // now it's time to set the valid network id in local player
    if ( !isRemoteClient() )
    {
        int sid = GetSessionID();
        _p_playerImpl->getPlayerEntity()->setNetworkID( sid );
    }

    // set the connection status
    vrc::PlayerImplClient* p_playerClient = dynamic_cast< vrc::PlayerImplClient* >( _p_playerImpl );
    assert( p_playerClient && "the player object must be a client implementation if this function is called!" );
    p_playerClient->setNetworkInitialized( true );
}

void PlayerNetworking::RPC_Initialize( tInitializationData initData )
{ // this method is called on replicas of new connected client, on server and clients

    // init remote or local client player, ignore this call on server!
    if ( yaf3d::GameState::get()->getMode() != yaf3d::GameState::Client )
        return;

    if ( _remoteClientInitialized )
        return;

    // init new client

    // init player position set by server ( it's the job of server to init the player position and rotation )
    _p_playerImpl->setPlayerPosition( osg::Vec3f( initData._posX, initData._posY, initData._posZ ) );
    _p_playerImpl->setPlayerRotation( osg::Quat( initData._rotZ, osg::Vec3f( 0.0f, 0.0f, 1.0f ) ) );

    // reset physics body transformation
    osg::Matrixf mat;
    mat *= mat.rotate( _p_playerImpl->getPlayerRotation() );
    mat.setTrans( _p_playerImpl->getPlayerPosition() );
    _p_playerImpl->getPlayerPhysics()->setTransformation( mat );

    _positionX = initData._posX;
    _positionY = initData._posY;
    _positionZ = initData._posZ;
    _yaw       = initData._rotZ;

    unsigned char breaktype = static_cast< unsigned char >( RNReplicaNet::DataBlock::kTeleport | RNReplicaNet::DataBlock::kSuddenChange );
    ContinuityBreak( _positionX, breaktype );
    ContinuityBreak( _positionY, breaktype );
    ContinuityBreak( _positionZ, breaktype );
    ContinuityBreak( _yaw, breaktype );

    // is voice chat enabled?
    _p_playerImpl->getPlayerEntity()->setIPAdress( _ip );
    _p_playerImpl->getPlayerEntity()->setVoiceChatEnabled( _voiceChat );
    if ( _voiceChat )
        vrc::gameutils::PlayerUtils::get()->addRemotePlayerVoiceChat( _p_playerImpl->getPlayerEntity() );

    _voiceChatEnable = _voiceChat;

    // set the connection status
    vrc::PlayerImplClient* p_playerClient = dynamic_cast< vrc::PlayerImplClient* >( _p_playerImpl );
    assert( p_playerClient && "the player object must be a client implementation if this function is called!" );
    p_playerClient->setNetworkInitialized( true );

    // set the init flag, this function may be called several times as the server will respond to every remote client which has been joined!
    _remoteClientInitialized = true;
}

void PlayerNetworking::RPC_EnableVoiceChat( bool en )
{
    _p_playerImpl->getPlayerEntity()->setVoiceChatEnabled( en );

    if ( _voiceChatEnable == en )
        return;

    _voiceChatEnable = en;
    _voiceChat       = en;

    // only relevant for remote clients
    if ( ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Client ) && isRemoteClient() )
    {
        if ( _voiceChat )
            vrc::gameutils::PlayerUtils::get()->addRemotePlayerVoiceChat( _p_playerImpl->getPlayerEntity() );
        else
            vrc::gameutils::PlayerUtils::get()->removeRemotePlayerVoiceChat( _p_playerImpl->getPlayerEntity() );
    }
}

void PlayerNetworking::createPlayer()
{
    std::string playerconfig = _p_configFile;
    log_info << "player '" <<  _p_playerName << "' connected" << std::endl;

    log_debug << "loading player configuration file: " << playerconfig << std::endl;
    std::stringstream postfix;
    static unsigned int postcnt = 0;
    postfix << "_" << postcnt;
    ++postcnt;

    // load player related entities
    if ( !yaf3d::LevelManager::get()->loadEntities( playerconfig, &_loadedEntities, postfix.str() ) )
    {
        log_error << "cannot find player configuration file: " << playerconfig << std::endl;

        // get the default player config
        vrc::gameutils::PlayerUtils::get()->getPlayerConfig( yaf3d::GameState::get()->getMode(), true, playerconfig, VRC_GS_DEFAULT_PLAYER_CONFIG );
        log_error << "loading the default player configuration: " << playerconfig << std::endl;
        if ( !yaf3d::LevelManager::get()->loadEntities( playerconfig, &_loadedEntities, postfix.str() ) )
        {
            log_error << "cannot load default player configuration! shutting down the application" << std::endl;
            yaf3d::Application::get()->stop();
            return;
        }
    }

    // search for player entity and set it up
    std::vector< yaf3d::BaseEntity* >::iterator p_beg = _loadedEntities.begin(), p_end = _loadedEntities.end();
    {
        for ( ; p_beg != p_end; ++p_beg )
        {
            if ( ( *p_beg )->getTypeName() == ENTITY_NAME_PLAYER )
                break;
        }
        if ( p_beg == p_end )
        {
            log_error << "cannot find player entity in file: " << playerconfig << std::endl;
            return;
        }
        _loadedPlayerEntity = static_cast< vrc::EnPlayer* >( *p_beg );

        // for a remote client we must setup the player implementation before initializing
        if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server )
            _p_playerImpl = new vrc::PlayerImplServer( _loadedPlayerEntity );
        else
            _p_playerImpl = new vrc::PlayerImplClient( _loadedPlayerEntity );

        _p_playerImpl->setPlayerNetworking( this );
        _p_playerImpl->initialize();
        _loadedPlayerEntity->setPlayerImplementation( _p_playerImpl );
        // set loading prefix
        _p_playerImpl->setLoadingPostfix( postfix.str() );
    }

    // begin initialization of player and its components
    log_debug << "initializing new player instance ... " << std::endl;
    {
        p_beg = _loadedEntities.begin(), p_end = _loadedEntities.end();
        for ( ; p_beg != p_end; ++p_beg )
        {
            ( *p_beg )->initialize();
        }
    }

    // now begin post-initialization of player and its components
    log_debug << "post-initializing new player ..." << std::endl;
    {
        p_beg = _loadedEntities.begin(), p_end = _loadedEntities.end();
        for ( ; p_beg != p_end; ++p_beg )
        {
            ( *p_beg )->postInitialize();
        }
    }
    _loadedPlayerEntity->setPlayerName( _p_playerName );
}

void PlayerNetworking::initialize( const osg::Vec3f& pos, const std::string& playerName, const std::string& cfgFile )
{
    _positionX = pos._v[ 0 ];
    _positionY = pos._v[ 1 ];
    _positionZ = pos._v[ 2 ];
    strcpy( _p_playerName, playerName.c_str() );
    strcpy( _p_configFile, cfgFile.c_str() );

    unsigned char breaktype = static_cast< unsigned char >
                              ( RNReplicaNet::DataBlock::kTeleport |
                                RNReplicaNet::DataBlock::kSuddenChange
                              );
    ContinuityBreak( _positionX, breaktype );
    ContinuityBreak( _positionY, breaktype );
    ContinuityBreak( _positionZ, breaktype );
    ContinuityBreak( _yaw, breaktype );
}

void PlayerNetworking::enableVoiceChat( bool en )
{ // this method must be called only on local client

    _voiceChatEnable = en;
    _p_playerImpl->getPlayerEntity()->setVoiceChatEnabled( en );
    // call the rpc on all replicated clients
    ALL_REPLICAS_FUNCTION_CALL( RPC_EnableVoiceChat( en ) );
}

bool PlayerNetworking::isEnabledVoiceChat()
{
    return _voiceChatEnable;
}

void PlayerNetworking::DataBlockPacketDataReceived( const RNReplicaNet::DataBlock* /*p_datablock*/ )
{
    if ( !_p_playerImpl )
        return;

    // check for changed player name
    RNReplicaNet::DataBlock* p_plyernameDataBlock = FindDataBlock( _p_playerName );
    if ( p_plyernameDataBlock )
        _p_playerImpl->changePlayerName( _p_playerName );
}

void PlayerNetworking::updateAnimationFlags( unsigned char cmdFlag )
{
    _cmdAnimFlags = cmdFlag;
}

unsigned char PlayerNetworking::getAnimationFlags()
{
    return _cmdAnimFlags;
}

void PlayerNetworking::updatePlayerName( const std::string& name )
{
    // limit the name length to be on safe side
    std::string cleanedname = name;
    if ( name.length() > 31 )
        cleanedname[ 31 ] = '\0';
    strcpy( _p_playerName, cleanedname.c_str() );
}

void PlayerNetworking::updatePosition( float x, float y, float z )
{
   _positionX = x;
   _positionY = y;
   _positionZ = z;
}

void PlayerNetworking::getPosition( float& x, float& y, float& z )
{
    x = _positionX;
    y = _positionY;
    z = _positionZ;
}

void PlayerNetworking::updateRotation( float yaw )
{
    _yaw = yaw;
}

void PlayerNetworking::getRotation( float& yaw )
{
    yaw =_yaw;
}
