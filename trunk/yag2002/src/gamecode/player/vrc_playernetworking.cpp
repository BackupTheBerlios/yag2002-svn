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
 # networking for chat member
 #
 # this class implements the networking functionality for chat member
 #
 #
 #   date of creation:  12/25/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_playernetworking.h"
#include "vrc_playerimpl.h"
#include "vrc_playerimplclient.h"
#include "vrc_playerimplserver.h"
#include "vrc_playerphysics.h"
#include "vrc_player.h"
#include "chat/vrc_chatmgr.h"

PlayerNetworking::PlayerNetworking( vrc::BasePlayerImplementation* p_playerImpl ) :
_positionX( 0 ),
_positionY( 0 ),
_positionZ( 0 ),
_yaw( 0 ),
_cmdAnimFlags( 0 ),
_remoteClient( false ),
_p_playerImpl( p_playerImpl ),
_loadedPlayerEntity( NULL )
{   
    // we have to lock creation / deletion of network objects during construction
    yaf3d::NetworkDevice::get()->lockObjects();

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
    }
    _p_configFile[ 0 ] = 0;
    _cmdAnimFlags      = 0;

    yaf3d::NetworkDevice::get()->unlockObjects();
}

PlayerNetworking::~PlayerNetworking()
{
    // we have to lock creation / deletion of network objects during destruction
    yaf3d::NetworkDevice::get()->lockObjects();

    log_info << "player left: " << _p_playerName << std::endl;

    // remove ghost from simulation ( server and client )

    // we have to delete player associated entities only if we are not unloading the level or quitting
    if ( ( yaf3d::GameState::get()->getState() != yaf3d::GameState::Leaving ) && ( yaf3d::GameState::get()->getState() != yaf3d::GameState::Quitting ) )
    {
        if ( isRemoteClient() )
        {
            // PlayerNetworking has created the player implementation, so set its networking and other components to NULL in order to abvoid deleting it also by player's implementation
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

    yaf3d::NetworkDevice::get()->unlockObjects();
}

void PlayerNetworking::PostObjectCreate()
{
    // complete setting up ghost ( remote client ) or server-side player
    if ( isRemoteClient() ) 
    {
        // create the player and its related entities
        createPlayer();

        // setup new connected client
        if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server )
        {
            // send intialization data
            static tInitializationData init;
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

            ALL_REPLICAS_FUNCTION_CALL( RPC_Initialize( init ) );
        }
    }

    log_info << " player created: " << _p_playerName << std::endl;
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
        return;
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

    ContinuityBreak( _positionX, RNReplicaNet::DataBlock::ContinuityBreakTypes::kTeleport );
    ContinuityBreak( _positionY, RNReplicaNet::DataBlock::ContinuityBreakTypes::kTeleport );
    ContinuityBreak( _positionZ, RNReplicaNet::DataBlock::ContinuityBreakTypes::kTeleport );
    ContinuityBreak( _yaw, RNReplicaNet::DataBlock::ContinuityBreakTypes::kTeleport );
}

void PlayerNetworking::RPC_Initialize( tInitializationData initData )
{
    // server directs all messages into a log file!
    assert( ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Client ) && "this RPC must be called only for clients!" );

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

    ContinuityBreak( _positionX, RNReplicaNet::DataBlock::ContinuityBreakTypes::kTeleport );
    ContinuityBreak( _positionY, RNReplicaNet::DataBlock::ContinuityBreakTypes::kTeleport );
    ContinuityBreak( _positionZ, RNReplicaNet::DataBlock::ContinuityBreakTypes::kTeleport );
    ContinuityBreak( _yaw, RNReplicaNet::DataBlock::ContinuityBreakTypes::kTeleport );
}

void PlayerNetworking::update()
{
}

void PlayerNetworking::DataBlockPacketDataReceived( const RNReplicaNet::DataBlock* p_datablock )
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
