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
 # base of all kinds of pickable objects
 #
 #   date of creation:  10/31/2007
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_baseobject.h"
#include "../player/vrc_playerimpl.h"
#include "../visuals/vrc_camera.h"

//! Picking angle ( cos alpha, where alpha is the opening angle / 2 )
#define OBJECT_PICK_ANGLE                   0.965f
//! Picking's check period (in seconds )
#define OBJECT_PICK_CHK_PERIOD              1.0f
//! Picking's distance update period (in seconds )
#define OBJECT_PICK_DIST_UPDATE_PERIOD      1.0f


namespace vrc
{

//! Implementation of distance sorted objects used for internal house-keeping
std::vector< BaseObject* > BaseObject::_objects;


BaseObject::BaseObject() :
 _shadowEnable( false ),
 _maxViewDistance( 10.0f ),
 _maxViewDistance2( 10.0f * 10.0f ),
 _hit( false ),
 _animTime( 0.0f ),
 _enable( true ),
 _checkPickingPeriod( 0.0f ),
 _sortDistancePeriod( 0.0f ),
 _p_player( NULL ),
 _p_playercamera( NULL )
{
    getAttributeManager().addAttribute( "meshFile"        , _meshFile        );
    getAttributeManager().addAttribute( "position"        , _position        );
    getAttributeManager().addAttribute( "rotation"        , _rotation        );
    getAttributeManager().addAttribute( "shadowEnable"    , _shadowEnable    );
    getAttributeManager().addAttribute( "maxViewDistance" , _maxViewDistance );
}

BaseObject::~BaseObject()
{
    // remove shadow from shadow manager
    if ( ( _shadowEnable ) && getTransformationNode() )
    {
        yaf3d::ShadowManager::get()->removeShadowNode( getTransformationNode() );
    }
}

void BaseObject::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle notifications
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_MENU_ENTER:
        {
        }
        break;

        case YAF3D_NOTIFY_MENU_LEAVE:
        {
        }
        break;

        case YAF3D_NOTIFY_ENTITY_TRANSNODE_CHANGED:
        {
        }
        break;

        case YAF3D_NOTIFY_ENTITY_ATTRIBUTE_CHANGED:
        {
        }
        break;

        case YAF3D_NOTIFY_SHUTDOWN:
        {
        }
        break;

        default:
            ;
    }
}

void BaseObject::initialize()
{
    osg::Node* p_node = yaf3d::LevelManager::get()->loadMesh( _meshFile, true );

    if ( !p_node )
    {
        log_error << "*** could not load mesh file: " << _meshFile << ", in '" << getInstanceName() << "'" << std::endl;
        return;
    }

    setPosition( _position );
    osg::Quat   rot( 
                     osg::DegreesToRadians( _rotation.x() ), osg::Vec3f( 1.0f, 0.0f, 0.0f ),
                     osg::DegreesToRadians( _rotation.y() ), osg::Vec3f( 0.0f, 1.0f, 0.0f ),
                     osg::DegreesToRadians( _rotation.z() ), osg::Vec3f( 0.0f, 0.0f, 1.0f )
                    );

    setRotation( rot );

    // get the shadow flag in configuration
    bool shadow;
    yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_SHADOW_ENABLE, shadow );

    yaf3d::EntityManager::get()->removeFromScene( this );

    // enable shadow only if it is enabled in configuration
    if ( shadow && _shadowEnable )
    {
        yaf3d::ShadowManager::get()->addShadowNode( getTransformationNode() );
        yaf3d::ShadowManager::get()->updateShadowArea();
    }
    else
    {
        yaf3d::EntityManager::get()->addToScene( this );
    }

    // append the mesh node to our transformation node
    addToTransformationNode( p_node );

    // setup picking params
    _maxViewDistance2 = _maxViewDistance * _maxViewDistance;

    //! TODO ...do game mode specific things
    switch ( yaf3d::GameState::get()->getMode() )
    {
        case yaf3d::GameState::Standalone:
        {
        }
        break;

        case yaf3d::GameState::Client:
        {
        }
        break;

        case yaf3d::GameState::Server:
        {
        }
        break;

        default:
            assert( NULL && "unsupported game mode" );
    }

    // register entity in order to get updated per simulation step.
    yaf3d::EntityManager::get()->registerUpdate( this, true );
   // register entity in order to get notifications (e.g. from menu entity)
    yaf3d::EntityManager::get()->registerNotification( this, true );
}

void BaseObject::postInitialize()
{
}

void BaseObject::updateEntity( float deltaTime )
{
    // animate the mesh if it is pickable
    if ( _hit )
    {
        _animTime += deltaTime;
        float z = 0.1f + 0.1f * sinf( _animTime );
        osg::Vec3f pos = _position;
        pos._v[ 2 ] += z;
        setPosition( pos );
    }

    _sortDistancePeriod += deltaTime;
    if ( _sortDistancePeriod > OBJECT_PICK_DIST_UPDATE_PERIOD )
    {
        bool inrange = checkObjectDistance();
        _sortDistancePeriod = 0.0f;
        // do not continue if distance messurement fails
        if ( !inrange )
            return;
    }

    _checkPickingPeriod += deltaTime;
    if ( _checkPickingPeriod > OBJECT_PICK_CHK_PERIOD )
    {
        checkCameraFocus();
        _checkPickingPeriod = 0.0f;
    }
}

bool BaseObject::checkObjectDistance()
{
    // in networked mode the player may be created later!
    if ( !_p_player )
    {
        _p_player = dynamic_cast< EnPlayer* >( vrc::gameutils::PlayerUtils::get()->getLocalPlayer() );
        if ( !_p_player )
            return false;

        _p_playercamera = _p_player->getPlayerImplementation()->getPlayerCamera();
        if ( !_p_playercamera )
        {
            log_error << "cannot find player's camera entity!" << std::endl;
            
            // deregister entity from getting updated
            yaf3d::EntityManager::get()->registerUpdate( this, false );

            return false;
        }

        // randomly offset the initial distance period for distributing the workload when many objects exist in the scene
        _sortDistancePeriod -= float( rand() % int( OBJECT_PICK_DIST_UPDATE_PERIOD * 1000 ) ) / 1000.0f;
        _checkPickingPeriod -= float( rand() % int( OBJECT_PICK_CHK_PERIOD * 1000 ) ) / 1000.0f;

        _objects.push_back( this );
    }

    // store the current camera postition
    _currCamPosition = _p_playercamera->getCameraPosition() + _p_playercamera->getCameraOffsetPosition();
    _ray = getPosition() - _currCamPosition;

    return true;
}

void BaseObject::checkCameraFocus()
{
    // reset the hit flag
    _hit = false;

    // check if the player entity is already available
    if ( !_p_player )
        return;

    // picking works only in Ego camera mode
    if ( _p_player->getPlayerImplementation()->getCameraMode() != BasePlayerImplementation::Ego )
        return;

    float dist2 = _ray.length2();

    // first do a distance check
    if ( dist2 > _maxViewDistance2 )
        return;

    // calculate the player look direction
    osg::Vec3f lookdir = _p_playercamera->getLocalRotation() * _p_playercamera->getCameraRotation() * osg::Vec3f( 0.0f, 1.0f, 0.0f );
    osg::Vec3f ray( _ray );
    ray.normalize();

    // check if the player is in our view
    if ( ( ray * lookdir ) > OBJECT_PICK_ANGLE )
    {
        //! TODO: we should maintain a temporal sorted list in order to speed up the nearest-object-test
        std::vector< BaseObject* >::const_iterator p_beg = _objects.begin(), p_end = _objects.end();
        for ( ; p_beg != p_end; ++p_beg )
        {
            // skip outself
            if ( *p_beg == this )
                continue;

            // is there another object more near to camera and in focus?
            if ( dist2 > ( *p_beg )->_ray.length2() && ( *p_beg )->_hit )
                return;
        }

        _hit = true;
    }

    if ( !_hit )
        return;

    // we can pick the object, it is the nearest one to camera and it lies in camera focus
    onHitObject();
}

} // namespace vrc
