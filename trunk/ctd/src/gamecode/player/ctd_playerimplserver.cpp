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

BasePlayerImplServer::BasePlayerImplServer( EnPlayer* player ) :
BasePlayerImplementation( player )
{
}

BasePlayerImplServer::~BasePlayerImplServer()
{
    if ( _p_playerNetworking )
        delete _p_playerNetworking;
}

void BasePlayerImplServer::handleNotification( EntityNotification& notify )
{
}

void BasePlayerImplServer::initialize()
{
    _currentPos = getPlayerEntity()->getPosition();
    _currentRot = getPlayerEntity()->getRotation();
}

void BasePlayerImplServer::postInitialize()
{
    log << Log::LogLevel( Log::L_INFO ) << "  setup player implementation Server ..." << endl;

    // attach physics entity
    log << Log::LogLevel( Log::L_DEBUG ) << "   - searching for physics entity '" << _playerAttributes._physicsEntity << "' ..." << endl;
    // find and attach physics component
    _p_playerPhysics = dynamic_cast< EnPlayerPhysics* >( EntityManager::get()->findEntity( ENTITY_NAME_PLPHYS, _playerAttributes._physicsEntity ) );
    assert( _p_playerPhysics && "given instance name does not belong to a EnPlayerPhysics entity type!" );
    _p_playerPhysics->setPlayer( this );
    log << Log::LogLevel( Log::L_DEBUG ) << "   -  physics entity successfully attached" << endl;

    // get configuration settings
    getConfiguration();

    log << Log::LogLevel( Log::L_INFO ) << "  player implementation successfully initialized" << endl;
}

void BasePlayerImplServer::getConfiguration()
{
}

void BasePlayerImplServer::update( float deltaTime )
{
    // update player's actual position and rotation once per frame
    getPlayerEntity()->setPosition( _currentPos ); 
    getPlayerEntity()->setRotation( _currentRot ); 
}

} // namespace CTD
