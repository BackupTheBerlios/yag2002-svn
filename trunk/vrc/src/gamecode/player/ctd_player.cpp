/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2002-2005, Ali Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU General Public License 
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public 
 *  License along with this program; if not, write to the Free 
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 * 
 ****************************************************************/

/*###############################################################
 # player
 #
 # this class implements the player
 #
 #
 #   date of creation:  01/14/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_base.h>
#include <ctd_application.h>
#include <ctd_levelmanager.h>
#include <ctd_physics.h>
#include "ctd_player.h"
#include "ctd_playerphysics.h"
#include "ctd_playeranim.h"

using namespace osg;
using namespace std;
using namespace CTD;

//! Implement and register the player entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( PlayerEntityFactory );

namespace CTD
{
    
template
< 
    class PlayerPhysicsT, 
    class PlayerAnimationT 
>
Player< PlayerPhysicsT, PlayerAnimationT >::Player() :
_p_playerPhysics( NULL ),
_p_playerAnimation( NULL ),
_playerName( "noname" ),
_dimensions( Vec3f( 0.5f, 0.5f, 1.8f ) ),
_stepHeight( 0.5f ),
_linearForce( 0.1f ),
_angularForce( 0.05f ),
_rotation( 0 ),
_moveDir( Vec3f( 0, 1, 0 ) ),
_gravity( Physics::get()->getWorldGravity() ),
_linearDamping( 0.2f )
{
    EntityManager::get()->registerUpdate( this );     // register entity in order to get updated per simulation step

    getAttributeManager().addAttribute( "name"          , _playerName    );
    getAttributeManager().addAttribute( "animconfig"    , _animCfgFile   );
    getAttributeManager().addAttribute( "dimensions"    , _dimensions    );
    getAttributeManager().addAttribute( "position"      , _position      );
    getAttributeManager().addAttribute( "stepheight"    , _stepHeight    );
    getAttributeManager().addAttribute( "linearforce"   , _linearForce   );
    getAttributeManager().addAttribute( "lineardamping" , _linearDamping );
    getAttributeManager().addAttribute( "angularforce"  , _angularForce  );
    getAttributeManager().addAttribute( "mass"          , _mass          );
    getAttributeManager().addAttribute( "gravity"       , _gravity       );

    // register us in viewer to get event callbacks
    Application::get()->getViewer()->getEventHandlerList().push_back( new InputHandler( this ) );

    // create physics component, it is important that the physics component is created in constructor
    //  due to own material definitions for player
    _p_playerPhysics = new PlayerPhysicsT( this );

    // create animation component
    _p_playerAnimation = new PlayerAnimationT( this );
}

template
< 
    class PlayerPhysicsT,
    class PlayerAnimationT
>
Player< PlayerPhysicsT, PlayerAnimationT >::~Player()
{
    if ( _p_playerPhysics )
        delete _p_playerPhysics;
}

template
< 
    class PlayerPhysicsT,
    class PlayerAnimationT
>
void Player< PlayerPhysicsT, PlayerAnimationT >::initialize()
{

    // init animation component
    _p_playerAnimation->initialize();

    // now we add the new mesh into our transformable scene group
    addTransformableNode( _p_playerAnimation->getNode() );
    setPosition( _position );

    // now we can initialize the physics component ( after the transform node is set )
    _p_playerPhysics->initialize();
}

template
< 
    class PlayerPhysicsT,
    class PlayerAnimationT
>
void Player< PlayerPhysicsT, PlayerAnimationT >::postInitialize()
{
}

template
< 
    class PlayerPhysicsT,
    class PlayerAnimationT
>
void Player< PlayerPhysicsT, PlayerAnimationT >::updateEntity( float deltaTime )
{
    _p_playerPhysics->update( deltaTime );
    _p_playerAnimation->update( deltaTime );
}

template
< 
    class PlayerPhysicsT,
    class PlayerAnimationT
>
void Player< PlayerPhysicsT, PlayerAnimationT >::applyForce( const Vec3f& force )
{
    _force = force;
}

template
< 
    class PlayerPhysicsT,
    class PlayerAnimationT
>
bool Player< PlayerPhysicsT, PlayerAnimationT >::InputHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
    bool         ret         = false;
    unsigned int eventType   = ea.getEventType();
    int          key         = ea.getKey();

    //! TODO make these member vars
    static bool  rotateRight = false, rotateLeft = false;
    static bool  moveForward = false, moveBackward = false;

    // dispatch key activity
    if ( eventType == osgGA::GUIEventAdapter::KEYDOWN )
    {
        if ( key == osgGA::GUIEventAdapter::KEY_Up )
            moveForward = true;

        if ( key == osgGA::GUIEventAdapter::KEY_Down )
            moveBackward = true;

        if ( key == osgGA::GUIEventAdapter::KEY_Right )
            rotateRight = true;

        if ( key == osgGA::GUIEventAdapter::KEY_Left )
            rotateLeft = true;

        if ( key == 'm' )
        {
            _p_player->_p_playerPhysics->jump();
            _p_player->_p_playerAnimation->actionWalk();
        }

        ret = true;
    }
    else if ( eventType == osgGA::GUIEventAdapter::KEYUP )
    {
        if ( ( key == osgGA::GUIEventAdapter::KEY_Up ) || ( key == osgGA::GUIEventAdapter::KEY_Down ) )
        {
            if ( key == osgGA::GUIEventAdapter::KEY_Up )
                moveForward = false;

            if ( key == osgGA::GUIEventAdapter::KEY_Down )
                moveBackward = false;

            _p_player->_force._v[ 0 ] = 0;
            _p_player->_force._v[ 1 ] = 0;
        }

        if ( key == osgGA::GUIEventAdapter::KEY_Right )
            rotateRight = false;

        if ( key == osgGA::GUIEventAdapter::KEY_Left )
            rotateLeft = false;

        ret = true;
    }

    if ( rotateRight )
    {
        _p_player->_rotation += _p_player->_angularForce;
        if ( _p_player->_rotation > PI * 2.0f )
            _p_player->_rotation -= PI * 2.0f;

        _p_player->_moveDir._v[ 0 ] = sinf( _p_player->_rotation );
        _p_player->_moveDir._v[ 1 ] = cosf( _p_player->_rotation );
    }

    if ( rotateLeft )
    {
        if ( _p_player->_rotation < 0 )
            _p_player->_rotation += PI * 2.0f;
        _p_player->_rotation -= _p_player->_angularForce;

        _p_player->_moveDir._v[ 0 ] = sinf( _p_player->_rotation );
        _p_player->_moveDir._v[ 1 ] = cosf( _p_player->_rotation );
    }

    if ( moveForward )
    {
        _p_player->_force = _p_player->_moveDir * _p_player->_linearForce;
        _p_player->_p_playerAnimation->actionWalk();
    }

    if ( moveBackward )
    {
        _p_player->_force = _p_player->_moveDir * ( -_p_player->_linearForce );
        _p_player->_p_playerAnimation->actionWalk();
    }

    if ( !moveForward && !moveBackward )
        _p_player->_p_playerAnimation->actionIdle();

    return ret;
}

} // namespace CTD
