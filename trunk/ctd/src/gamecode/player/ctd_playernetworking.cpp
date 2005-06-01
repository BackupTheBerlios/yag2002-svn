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
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  12/25/2004 boto       creation of PlayerNetworking
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_playernetworking.h"
#include "ctd_playerimpl.h"
#include "ctd_playerimplClient.h"
#include "ctd_playerimplServer.h"
#include "ctd_player.h"

using namespace std;
using namespace osg;


PlayerNetworking::PlayerNetworking( CTD::BasePlayerImplementation* p_playerImpl )
{    
    // this constructor can be called either by player entity or networking system (in client or server mode)
    //  when called by player entity then it means that we are a local client, otherwise we are a remote client
    _remoteClient = false;
    if ( !p_playerImpl ) 
    {
        _remoteClient = true;

        // create new entity for remote client
        CTD::EnPlayer* p_entity = static_cast< CTD::EnPlayer* > ( CTD::EntityManager::get()->createEntity( ENTITY_NAME_PLAYER, "_newRemoteClient_" ) );
        assert( p_entity && "player entity cannot be created" );

        if ( CTD::GameState::get()->getMode() == CTD::GameState::Server )
        {
            p_playerImpl = new CTD::BasePlayerImplServer( p_entity );
        }
        else if ( CTD::GameState::get()->getMode() == CTD::GameState::Client )
        {
            p_playerImpl = new CTD::BasePlayerImplClient( p_entity );
        }
        else
            assert( NULL && "invalid game state" );

        p_playerImpl->setPlayerNetworking( this );
        p_playerImpl->initialize();

        _p_playerName[ 0 ]   = 0;
        _p_animFileName[ 0 ] = 0;
    }
    _cmdAnimFlags     = 0;
    _p_playerImpl     = p_playerImpl;
}

PlayerNetworking::~PlayerNetworking()
{
    // remove ghost from simulation ( server and client )
    if ( isRemoteClient() ) 
    {    
        _p_playerImpl->setPlayerNetworking( NULL );
        CTD::EntityManager::get()->deleteEntity( _p_playerImpl->getPlayerEntity() );
    }
}

void PlayerNetworking::PostObjectCreate()
{

//    if ( isRemoteClient() && NetworkDevice::get()->getMode() == NetworkDevice::NetworkingMode::CLIENT ) {
//
////        m_pkMember->SetPlayerName( m_pcPlayerName );
////        m_pkMember->SetAnimConfig( m_pcAnimFileName );
//
//        // integrate the entity into scene     
////        assert ( Framework::Get()->AddEntiy( GetLevelSet(), ( CTD::BaseEntity* )m_pkMember ) == true );
//        _p_member->postInitialize();
//    
//    }
}

void PlayerNetworking::initialize( const Vec3f& pos, const string& playerName, const string& meshFileName )
{
    //_positionX = pos._v[ 0 ]; 
    //_positionY = pos._v[ 1 ];
    //_positionZ = pos._v[ 2 ];
    //strcpy( _p_playerName, playerName.c_str() );
    //strcpy( _p_animFileName, meshFileName.c_str() );
}

void PlayerNetworking::putChatText( const string& text )
{
    //static tChatMsg s_textBuffer;
    //strcpy( s_textBuffer.m_pcText, text.c_str() );
    //ALL_REPLICAS_FUNCTION_CALL( RPC_AddChatText( s_textBuffer ) );
}

void PlayerNetworking::RPC_AddChatText( tChatMsg chatMsg )
{
//    // a server could direct all messages into a log file!
//    if ( NetworkDevice::get()->getMode() == NetworkDevice::NetworkingMode::CLIENT ) {
//
//        chatMsg.m_pcText[ 255 ] = 0;
////        _p_player->postChatText( pcText.m_pcText );
//
//    }
}

void PlayerNetworking::update()
{
}

void PlayerNetworking::updateAnimationFlags( unsigned int cmdFlag )
{
    //_cmdAnimFlags = cmdFlag;
}

void PlayerNetworking::getAnimationFlags( unsigned int& cmdFlag )
{
    //cmdFlag = _cmdAnimFlags;
}

void PlayerNetworking::updatePosition( float x, float y, float z )
{
   //_positionX = x;
   //_positionY = y;
   //_positionZ = z;
}

void PlayerNetworking::getPosition( float& x, float& y, float& z )
{
    //x = _positionX;
    //y = _positionY;
    //z = _positionZ;
}

void PlayerNetworking::updateRotation( float yaw )
{
    //_yaw = yaw;
}

void PlayerNetworking::getRotation( float& yaw )
{
    //yaw =_yaw;
}
