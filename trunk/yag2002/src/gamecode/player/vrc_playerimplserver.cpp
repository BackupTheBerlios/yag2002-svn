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
 # entity VRC chat server
 #
 #   date of creation:  10/26/2005
 #
 #   author:            ali botorabi (boto)
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_playerimplserver.h"
#include "vrc_player.h"
#include "vrc_playerphysics.h"
#include "vrc_playernetworking.h"
#include "chat/vrc_chatmgr.h"
#include "chat/VRC/vrc_chatprotVRC.h"

namespace vrc
{

PlayerImplServer::PlayerImplServer( EnPlayer* player ) :
BasePlayerImplementation( player )
{
}

PlayerImplServer::~PlayerImplServer()
{
}

void PlayerImplServer::handleNotification( const yaf3d::EntityNotification& /*notification*/ )
{
}

void PlayerImplServer::initialize()
{
}

void PlayerImplServer::postInitialize()
{
    _currentPos = getPlayerEntity()->getPosition();
    _currentRot = getPlayerEntity()->getRotation();

    log_info << "  setup player implementation Server ..." << std::endl;
    log_info << "  player implementation successfully initialized" << std::endl;
}

void PlayerImplServer::update( float /*deltaTime*/ )
{
    // update player's actual position and rotation
    getPlayerNetworking()->getPosition( _currentPos._v[ 0 ], _currentPos._v[ 1 ], _currentPos._v[ 2 ] );
    getPlayerNetworking()->getRotation( _rotZ );
    _currentRot.makeRotate( _rotZ, osg::Vec3f( 0.0f, 0.0f, 1.0f ) );
    getPlayerEntity()->setPosition( _currentPos );
    getPlayerEntity()->setRotation( _currentRot );
}

} // namespace vrc
