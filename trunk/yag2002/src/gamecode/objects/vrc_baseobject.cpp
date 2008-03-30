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
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_baseobject.h"
#include "vrc_objectnetworking.h"
#include "../player/vrc_playerimpl.h"
#include "../visuals/vrc_camera.h"

//! Picking angle ( cos alpha, where alpha is the opening angle / 2 )
#define OBJECT_PICK_ANGLE                   0.965f
//! Picking's check period (in seconds )
#define OBJECT_PICK_CHK_PERIOD              0.5f
//! Picking's distance update period (in seconds )
#define OBJECT_PICK_DIST_UPDATE_PERIOD      1.0f


namespace vrc
{

//! Implementation of distance sorted objects used for internal house-keeping
std::vector< BaseObject* >              BaseObject::_objects;

//! Implementation of object input handler
BaseObject::ObjectInputHandler*         BaseObject::_p_inputHandler;

//! Implementation of object registry
void ActorRegistry::registerEntityType( unsigned int ID, const std::string& entitytype )
{
    std::map< unsigned int, std::string >::iterator p_end = ActorRegistry::_p_actorTypes->end(), p_type;
    p_type = ActorRegistry::_p_actorTypes->find( ID );

    if ( p_type != p_end )
    {
        log_error << "ActorRegistry: type with ID " << ID << " is already registered!" << std::endl;
        return;
    }

    // register type
    ( *_p_actorTypes )[ ID ] = entitytype;
}

std::string ActorRegistry::getEntityType( unsigned int ID )
{
    std::map< unsigned int, std::string >::iterator p_end = ActorRegistry::_p_actorTypes->end(), p_type;
    p_type = ActorRegistry::_p_actorTypes->find( ID );
    if ( p_type == p_end )
    {
        log_error << "ActorRegistry: invalid object ID: " << ID << std::endl;
        return std::string( "" );
    }

    return p_type->second;
}

//! Implementation of input handler
BaseObject::ObjectInputHandler::ObjectInputHandler() :
_p_highlightedObject( NULL ),
_enable( false ),
_keyCodePick( 1 )
{
    std::string keyname;
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_KEY_OBJECTPICK, keyname );
    _keyCodePick = yaf3d::KeyMap::get()->getCode( keyname );
}

bool BaseObject::ObjectInputHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*aa*/ )
{
    // first check if picking is locked
    unsigned int ctrlmodes = gameutils::PlayerUtils::get()->getPlayerControlModes();
    if ( ctrlmodes & gameutils::PlayerUtils::eLockPicking )
        return false;

    if ( !_p_highlightedObject || !_enable )
        return false;

    const osgSDL::SDLEventAdapter* p_eventAdapter = dynamic_cast< const osgSDL::SDLEventAdapter* >( &ea );
    assert( p_eventAdapter && "invalid event adapter received" );

    unsigned int eventType  = p_eventAdapter->getEventType();
    unsigned int mouseBtn   = p_eventAdapter->getButton();
    bool mouseButtonRelease = ( eventType == osgGA::GUIEventAdapter::RELEASE );

    if ( mouseButtonRelease )
    {
        if ( ( mouseBtn == _keyCodePick ) && ( _p_highlightedObject->_enable ) )
            _p_highlightedObject->onObjectUse();
    }

    // let the event further process by other handlers
    return false;
}


//! Implementation of base object
BaseObject::BaseObject( unsigned int actortype ) :
 BaseStoryActor( actortype ),
 _p_node( NULL ),
 _shadowEnable( false ),
 _maxHeighlightDistance( 10.0f ),
 _maxPickDistance( 1.5f ),
 _enable( true ),
 _checkPickingPeriod( 0.0f ),
 _sortDistancePeriod( 0.0f ),
 _maxHeighlightDistance2( 10.0f * 10.0f ),
 _maxPickDistance2( 1.5f * 1.5f ),
 _highlight( false ),
 _animTime( 0.0f ),
 _disappearTime( -1.0f ),
 _destroyTime( -1.0f ),
 _p_player( NULL ),
 _p_playercamera( NULL ),
 _p_networking( NULL )
{
    assert( actortype && "invalid actor type!" );

    getAttributeManager().addAttribute( "meshFile"              , _meshFile              );
    getAttributeManager().addAttribute( "position"              , _position              );
    getAttributeManager().addAttribute( "rotation"              , _rotation              );
    getAttributeManager().addAttribute( "shadowEnable"          , _shadowEnable          );
    getAttributeManager().addAttribute( "maxHeighlightDistance" , _maxHeighlightDistance );
    getAttributeManager().addAttribute( "maxPickDistance"       , _maxPickDistance       );
}

BaseObject::~BaseObject()
{
    // remove shadow from shadow manager
    if ( ( _shadowEnable ) && getTransformationNode() )
    {
        yaf3d::ShadowManager::get()->removeShadowNode( getTransformationNode() );
    }

    // remove us from events listeners
    StorySystem::get()->removeActor( getActorID(), this );

    // remove this object from the object list
    std::vector< BaseObject* >::iterator p_beg = _objects.begin(), p_end = _objects.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        if ( *p_beg == this )
        {
            _objects.erase( p_beg );
            break;
        }
    }

    // delete the input handler when no objects left in level
    if ( _p_inputHandler && !_objects.size() )
    {
        _p_inputHandler->destroyHandler();
        _p_inputHandler = NULL;
    }

    // on server we delete the networking on destruction, so all replicas get deleted
    if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server )
    {
        delete _p_networking;
    }
}

void BaseObject::setNetworking( ObjectNetworking* p_networking )
{
    assert( ( _p_networking == NULL ) && "networking object already exists!" );
    _p_networking = p_networking;
}

void BaseObject::disappear( float period )
{
    if ( period > 0.0f )
    {
        _disappearTime = period;
        enable( false );
    }
}

void BaseObject::destroy( float period )
{
    _destroyTime = period;
}

bool BaseObject::isActive() const
{
    return _enable;
}

void BaseObject::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle notifications
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_MENU_ENTER:
        {
            if ( _p_inputHandler )
                _p_inputHandler->enable( false );
        }
        break;

        case YAF3D_NOTIFY_MENU_LEAVE:
        {
            if ( _p_inputHandler )
                _p_inputHandler->enable( true );
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

        case YAF3D_NOTIFY_UNLOAD_LEVEL:
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
    // setup picking params
    _maxHeighlightDistance2 = _maxHeighlightDistance * _maxHeighlightDistance;

    // add us to events listeners
    StorySystem::get()->addActor( getActorID(), this );

    //! do game mode specific things
    switch ( yaf3d::GameState::get()->getMode() )
    {
        case yaf3d::GameState::Standalone:
        case yaf3d::GameState::Client:
        {
            if ( !_p_inputHandler )
                _p_inputHandler = new BaseObject::ObjectInputHandler;

            setupMesh();

            // register entity in order to get updated per simulation step.
            yaf3d::EntityManager::get()->registerUpdate( this, true );
           // register entity in order to get notifications (e.g. from menu entity)
            yaf3d::EntityManager::get()->registerNotification( this, true );
        }
        break;

        case yaf3d::GameState::Server:
        {
            // create the networking object, this object is replicated on clients
            _p_networking = new ObjectNetworking( this );
            _p_networking->Publish();

            // register entity in order to get updated per simulation step.
            yaf3d::EntityManager::get()->registerUpdate( this, true );

            // add the object to list on server at this point of initialization
            _objects.push_back( this );
        }
        break;

        default:
            assert( NULL && "unsupported game mode" );
    }
}

void BaseObject::setupMesh()
{
    _p_node = yaf3d::LevelManager::get()->loadMesh( _meshFile, true );

    if ( !_p_node )
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

    // enable the scene node
    if ( _enable )
    {
        _enable = false;
        enable( true );
    }
}

void BaseObject::enable( bool en )
{
    if ( _enable == en )
        return;

    _enable = en;

    if ( _enable )
    {
        // get the shadow flag in configuration
        bool shadow;
        yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_SHADOW_ENABLE, shadow );

        yaf3d::EntityManager::get()->removeFromScene( this );

        // enable shadow only if it is enabled in configuration
        if ( shadow && _shadowEnable )
        {
            yaf3d::ShadowManager::get()->addShadowNode( getTransformationNode(), yaf3d::ShadowManager::eThrowShadow );
            yaf3d::ShadowManager::get()->updateShadowArea();
        }
        else
        {
            yaf3d::EntityManager::get()->addToScene( this );
        }

        // append the mesh node to our transformation node
        addToTransformationNode( _p_node );
    }
    else
    {
        yaf3d::EntityManager::get()->removeFromScene( this );
    }
}

void BaseObject::postInitialize()
{
}

void BaseObject::updateEntity( float deltaTime )
{
    // check for respawn
    if ( _disappearTime > 0.0f )
    {
        _disappearTime -= deltaTime;
        if ( _disappearTime < 0.0f )
        {
            _disappearTime = 0.0f;
            enable( true );
        }
    }

    if ( _destroyTime > 0.0f )
    {
        _destroyTime -= deltaTime;
        if ( _destroyTime < 0.0f )
        {
            yaf3d::EntityManager::get()->deleteEntity( this );
        }
    }

    // check for picking only if the object is enabled
    if ( _enable && ( yaf3d::GameState::get()->getMode() != yaf3d::GameState::Server ) )
    {
        // animate the mesh if it is pickable
        if ( _highlight )
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
}

void BaseObject::onReceiveEvent( const StoryEvent& event )
{
    // propagate the event to derived object
    onEventReceived( event );
}

bool BaseObject::checkObjectDistance()
{
    // ´further serup of the object
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

    if ( _ray.length2() > _maxHeighlightDistance2 )
    {
        // reset the highlighted object in input handler
        if ( _p_inputHandler->getHighlightedObject() == this )
            _p_inputHandler->setHighlightedObject( NULL );

        return false;
    }

    return true;
}

void BaseObject::checkCameraFocus()
{
    // reset the hit flag
    _highlight = false;

    // check if the player entity is already available
    if ( !_p_player )
        return;

    // picking works only in Ego camera mode
    if ( _p_player->getPlayerImplementation()->getCameraMode() != BasePlayerImplementation::Ego )
        return;

    float dist2 = _ray.length2();

    // first do a distance check
    if ( dist2 > _maxHeighlightDistance2 )
        return;

    // calculate the player look direction
    osg::Vec3f lookdir = _p_playercamera->getLocalRotation() * _p_playercamera->getCameraRotation() * osg::Vec3f( 0.0f, 1.0f, 0.0f );
    osg::Vec3f ray( _ray );
    ray.normalize();

    // reset the highlighted object in input handler
    if ( _p_inputHandler->getHighlightedObject() == this )
        _p_inputHandler->setHighlightedObject( NULL );

    // check if the player is in our view
    if ( ( ray * lookdir ) > OBJECT_PICK_ANGLE )
    {
        //! TODO: we may maintain a temporal sorted list in order to speed up the nearest-object-test
        std::vector< BaseObject* >::const_iterator p_beg = _objects.begin(), p_end = _objects.end();
        for ( ; p_beg != p_end; ++p_beg )
        {
            // skip ourself in search
            if ( *p_beg == this )
                continue;

            // is there another object more near to camera and in focus?
            if ( dist2 > ( *p_beg )->_ray.length2() && ( *p_beg )->_highlight )
                return;
        }

        // set the highlight flag
        _highlight = true;
        // update the highlighted object in input handler
        _p_inputHandler->setHighlightedObject( this );
    }
}

} // namespace vrc
