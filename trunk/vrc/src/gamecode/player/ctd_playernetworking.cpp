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
#include "ctd_player.h"

using namespace std;
using namespace CTD;
using namespace osg;


PlayerNetworking::PlayerNetworking( BaseEntity* p_member )
{
    EnPlayer* p_player = static_cast< EnPlayer* >( p_member );
    
//    //m_bIsRemoteClient = IsMaster();
//    _remoteClient = false;
//    if ( !p_member ) 
//    {
//        _remoteClient = true;
//
//        // create new entity for remote client
//        p_member = new EnPlayer;
////        pkMember->SetPlayerNetworking( this );
//        p_member->initialize();
//
//        _p_playerName[ 0 ]   = 0;
//        _p_animFileName[ 0 ] = 0;
//    }
//    _cmdAnimFlags = 0;
//    _p_member        = p_member;
}

PlayerNetworking::~PlayerNetworking()
{
    //// remove ghost from simulation ( server and client )
    //if ( isRemoteClient() ) 
    //{    
    //    EntityManager::get()->deregisterUpdate( ( CTD::BaseEntity* )_p_member );
    //    delete _p_member;
    //}
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
