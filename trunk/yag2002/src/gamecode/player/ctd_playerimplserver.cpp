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
 # player implementation for server mode
 #
 #   date of creation:  05/31/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_playerimplserver.h"
#include "ctd_player.h"
#include "ctd_playerphysics.h"
#include "ctd_playernetworking.h"

using namespace std;

namespace CTD
{

PlayerImplServer::PlayerImplServer( EnPlayer* player ) :
BasePlayerImplementation( player )
{
}

PlayerImplServer::~PlayerImplServer()
{
}

void PlayerImplServer::handleNotification( const EntityNotification& notification )
{
}

void PlayerImplServer::initialize()
{
}

void PlayerImplServer::postInitialize()
{
    _currentPos = getPlayerEntity()->getPosition();
    _currentRot = getPlayerEntity()->getRotation();

    log << Log::LogLevel( Log::L_INFO ) << "  setup player implementation Server ..." << endl;

    //! TODO: check if we need physics on server

    log << Log::LogLevel( Log::L_INFO ) << "  player implementation successfully initialized" << endl;
}

void PlayerImplServer::update( float deltaTime )
{
    // update player's actual position and rotation
    getPlayerNetworking()->getPosition( _currentPos._v[ 0 ], _currentPos._v[ 1 ], _currentPos._v[ 2 ] );
    getPlayerNetworking()->getRotation( _rotZ );
    _currentRot.makeRotate( _rotZ, osg::Vec3f( 0.0f, 0.0f, 1.0f ) );
    getPlayerEntity()->setPosition( _currentPos );
    getPlayerEntity()->setRotation( _currentRot );
}

} // namespace CTD
