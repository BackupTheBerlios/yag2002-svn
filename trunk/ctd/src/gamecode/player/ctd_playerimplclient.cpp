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
 # player implementation for client mode
 #
 #   date of creation:  05/31/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_player.h"
#include "ctd_playersound.h"
#include "ctd_inputhandler.h"
#include "ctd_playerphysics.h"
#include "ctd_playerimplclient.h"
#include "ctd_playernetworking.h"
#include "../visuals/ctd_camera.h"

using namespace std;

namespace CTD
{

BasePlayerImplClient::BasePlayerImplClient( EnPlayer* player ) :
BasePlayerImplementation( player )
{
}

BasePlayerImplClient::~BasePlayerImplClient()
{
    if ( _p_playerNetworking )
        delete _p_playerNetworking;
}

void BasePlayerImplClient::handleNotification( EntityNotification& notify )
{
}

void BasePlayerImplClient::initialize()
{
    _currentPos = getPlayerEntity()->getPosition();
    _currentRot = getPlayerEntity()->getRotation();

    // if the player networking component is created already then this implementation is created
    //  for a remote client on local machine, otherwise it's a local client.
    if ( !_p_playerNetworking )
    {
        _p_playerNetworking = new PlayerNetworking( this );
        // create a new input handler for this player
        _p_inputHandler = new PlayerIHCharacterCameraCtrl< BasePlayerImplClient >( this, _p_player );
    }
}

void BasePlayerImplClient::postInitialize()
{
    log << Log::LogLevel( Log::L_INFO ) << "  setup player implementation Server ..." << endl;

    // attach camera entity
    log << Log::LogLevel( Log::L_DEBUG ) << "   - searching for camera entity '" << PLAYER_CAMERA_ENTITIY_NAME << "'..." << endl;
    // get camera entity
    _p_camera = dynamic_cast< EnCamera* >( EntityManager::get()->findEntity( ENTITY_NAME_CAMERA, PLAYER_CAMERA_ENTITIY_NAME ) );
    assert( _p_camera && "could not find the camera entity!" );
    log << Log::LogLevel( Log::L_DEBUG ) << "   -  camera entity successfully attached" << endl;

    // attach physics entity
    log << Log::LogLevel( Log::L_DEBUG ) << "   - searching for physics entity '" << _playerAttributes._physicsEntity << "' ..." << endl;
    // find and attach physics component
    _p_playerPhysics = dynamic_cast< EnPlayerPhysics* >( EntityManager::get()->findEntity( ENTITY_NAME_PLPHYS, _playerAttributes._physicsEntity ) );
    assert( _p_playerPhysics && "given instance name does not belong to a EnPlayerPhysics entity type!" );
    _p_playerPhysics->setPlayer( this );
    log << Log::LogLevel( Log::L_DEBUG ) << "   -  physics entity successfully attached" << endl;

    // attach sound entity
    log << Log::LogLevel( Log::L_DEBUG ) << "   - searching for sound entity '" << _playerAttributes._soundEntity << "' ..." << endl;
    // find and attach sound component, tollerate missing sound for now
    _p_playerSound = dynamic_cast< EnPlayerSound* >( EntityManager::get()->findEntity( ENTITY_NAME_PLSOUND, _playerAttributes._soundEntity ) );
    if ( !_p_playerSound )
        log << Log::LogLevel( Log::L_ERROR ) << "  *** could not find sound entity '" << _playerAttributes._soundEntity << "' of type PlayerSound. player sound deactivated" << endl;
    else
    {
        _p_playerSound->setPlayer( this );
        log << Log::LogLevel( Log::L_DEBUG ) << "   -  sound entity successfully attached" << endl;
    }

    // get configuration settings
    getConfiguration();

    log << Log::LogLevel( Log::L_INFO ) << "  player implementation successfully initialized" << endl;
}

void BasePlayerImplClient::getConfiguration()
{
}

void BasePlayerImplClient::update( float deltaTime )
{
    // update player's actual position and rotation once per frame
    getPlayerEntity()->setPosition( _currentPos ); 
    getPlayerEntity()->setRotation( _currentRot ); 
}

} // namespace CTD
