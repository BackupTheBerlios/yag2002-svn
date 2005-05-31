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
 # base clas for player implementations
 #
 #  the actual player behaviour is described in one of its implemnets
 #  depending on game mode ( server, client, or standalone )
 #
 #
 #   date of creation:  05/28/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_playerimpl.h"
#include "ctd_playerphysics.h"
#include "ctd_playeranim.h"
#include "ctd_playersound.h"
#include "ctd_inputhandler.h"
#include "../visuals/ctd_camera.h"

using namespace osg;
using namespace std;

namespace CTD
{

BasePlayerImplementation::BasePlayerImplementation( EnPlayer* player ) :
_p_player( player ),
_p_playerSound( NULL ),
_p_playerPhysics( NULL ),
_p_playerAnimation( NULL ),
_p_camera( NULL ),
_cameraMode( Spheric ),
_moveDir( Vec3f( 0, 1, 0 ) )
{
    // copy player's attributes
    _playerAttributes = _p_player->getPlayerAttributes();
}

BasePlayerImplementation::~BasePlayerImplementation()
{
    // release resources
    if ( _p_playerPhysics )
    {
        EntityManager::get()->deleteEntity( _p_playerPhysics );
    }

    if ( _p_playerAnimation )
    {
        EntityManager::get()->deleteEntity( _p_playerAnimation );
    }

    if ( _p_playerSound )
    {
        EntityManager::get()->deleteEntity( _p_playerSound );
    }
}

void BasePlayerImplementation::setNextCameraMode()
{
    if ( _cameraMode == Spheric )
        setCameraMode( Ego );
    else
        setCameraMode( Spheric );
}

void BasePlayerImplementation::setCameraPitchYaw( float pitch, float yaw )
{
    if ( _cameraMode == Spheric )
    {
        float angleY = yaw * 360.0f;
        float angleX = ( pitch * LIMIT_PITCH_ANGLE ) + LIMIT_PITCH_OFFSET;
        _p_camera->setLocalPitchYaw( angleX, angleY );
    }
    // in ego mode we turn the character instead of yawing the camera!
    else
    {
        float angleX = pitch * LIMIT_PITCH_ANGLE;
        _p_camera->setLocalPitch( angleX );
    }
}

void BasePlayerImplementation::setPlayerPosition( const osg::Vec3f& pos ) 
{ 
    _p_player->setPosition( pos ); 
}

void BasePlayerImplementation::setPlayerRotation( const osg::Quat& rot )
{ 
    _p_player->setRotation( rot ); 
}

const osg::Vec3d& BasePlayerImplementation::getPlayerPosition() 
{
    return _p_player->getPosition(); 
}

const osg::Quat& BasePlayerImplementation::getPlayerRotation()
{ 
    return _p_player->getRotation(); 
}

const osg::Vec3f& BasePlayerImplementation::getPlayerMoveDirection() 
{ 
    return _moveDir; 
}

} // namespace CTD
