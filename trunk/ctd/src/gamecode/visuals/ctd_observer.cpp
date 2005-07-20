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
 # entity observer
 #  this entity can be used for freely flying through a level
 #
 #   date of creation:  07/20/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_observer.h"
#include "ctd_camera.h"

namespace CTD
{

//! Input handler for observer
class ObserverIH : public GenericInputHandler< EnObserver >
{
    public:

        explicit                            ObserverIH( EnObserver* p_ent ) : 
                                             GenericInputHandler< EnObserver >( p_ent )
                                            {
                                                _moveRight      = false;
                                                _moveLeft       = false;
                                                _moveForward    = false;
                                                _moveBackward   = false;

                                                // get the current screen size
                                                unsigned int width, height;
                                                Application::get()->getScreenSize( width, height );
                                                // calculate the middle of app window
                                                _screenMiddleX = static_cast< Uint16 >( width * 0.5f );
                                                _screenMiddleY = static_cast< Uint16 >( height * 0.5f );

                                            }

        virtual                             ~ObserverIH() {}

        bool                                handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

    protected:

        // some internal variables
        bool                                _moveRight;
        bool                                _moveLeft;
        bool                                _moveForward;
        bool                                _moveBackward;

        Uint16                              _screenMiddleX;
        Uint16                              _screenMiddleY;
};

bool ObserverIH::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
    const osgSDL::SDLEventAdapter* p_eventAdapter = dynamic_cast< const osgSDL::SDLEventAdapter* >( &ea );
    assert( p_eventAdapter && "invalid event adapter received" );

    unsigned int eventType  = p_eventAdapter->getEventType();
    int          key        = p_eventAdapter->getSDLKey();

    if ( eventType == osgGA::GUIEventAdapter::FRAME )
        return false;

    // terminate application on Escape
    if ( key == SDLK_ESCAPE )
        Application::get()->stop();

    EnCamera*   p_camera = getUserObject()->_p_cameraEntity;
    float&      speed    = getUserObject()->_speed;
    float&      dt       = getUserObject()->_deltaTime;
    osg::Vec3f  pos;

    if ( eventType == osgGA::GUIEventAdapter::KEYDOWN )
    {
        if ( key == SDLK_w )
            _moveForward = true;
        else if ( key == SDLK_s )
            _moveBackward = true;
        else if ( key == SDLK_a )
            _moveLeft = true;
        else if ( key == SDLK_d )
            _moveRight = true;
    }
    else if ( eventType == osgGA::GUIEventAdapter::KEYUP )
    {
        if ( key == SDLK_w )
            _moveForward = false;
        else if ( key == SDLK_s )
            _moveBackward = false;
        else if ( key == SDLK_a )
            _moveLeft = false;
        else if ( key == SDLK_d )
            _moveRight = false;
    }

    bool  noPosUpdateFB = false;
    bool  noPosUpdateRL = false;

    if ( _moveForward )
        pos._v[ 1 ] += speed * dt;
    else if ( _moveBackward )
        pos._v[ 1 ] -= speed * dt;
    else
        noPosUpdateFB = true;

    if ( _moveLeft )
        pos._v[ 0 ] -= speed * dt;
    else if ( _moveRight )
        pos._v[ 0 ] += speed * dt;
    else
        noPosUpdateRL = true;


    static float pitch = 0.0f;
    static float yaw   = 0.0f;

    const SDL_Event& sdlevent = p_eventAdapter->getSDLEvent();
    
    // adjust pitch and yaw
    if ( eventType == osgGA::GUIEventAdapter::DRAG )
    {
        // skip events which come in when we warp the mouse pointer to middle of app window ( see below )
        if ( (  sdlevent.motion.x == _screenMiddleX ) && ( sdlevent.motion.y == _screenMiddleY ) )
            return false;
        float xrel = float( sdlevent.motion.xrel ) * speed * 0.001f;
        float yrel = float( sdlevent.motion.yrel ) * speed * 0.001f;

        yaw   += xrel;
        pitch += yrel;
        p_camera->setLocalPitchYaw( -pitch, -yaw );

        // reset mouse position in order to avoid leaving the app window
        Application::get()->getViewer()->requestWarpPointer( _screenMiddleX, _screenMiddleY );
    }

    // update camera position
    if ( !noPosUpdateRL || !noPosUpdateFB )
    {
        pos = p_camera->getLocalRotation() * pos;
        p_camera->setCameraPosition( pos + p_camera->getCameraPosition() );
    }

    return false;
}

//! Implement and register the observer entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( ObserverEntityFactory );

EnObserver::EnObserver() :
_isPersistent( false ),
_needUpdate ( false ),
_p_cameraEntity( NULL ),
_speed( 10.0f )
{
    // register entity attributes
    getAttributeManager().addAttribute( "position", _position );
    getAttributeManager().addAttribute( "rotation", _rotation );
}

EnObserver::~EnObserver()
{
}

void EnObserver::handleNotification( const EntityNotification& notification )
{
    // handle notifications
    switch( notification.getId() )
    {
        case CTD_NOTIFY_NEW_LEVEL_INITIALIZED:
            break;

        // we have to trigger the deletion ourselves! ( this entity can be peristent )
        case CTD_NOTIFY_SHUTDOWN:

            if ( _isPersistent )
                EntityManager::get()->deleteEntity( this );
            break;

        default:
            ;
    }
}

void EnObserver::initialize()
{
    EntityManager::get()->registerUpdate( this, true );         // register entity in order to get updated per simulation step
    EntityManager::get()->registerNotification( this, true );   // register entity in order to get notifications (e.g. from menu entity)
}

void EnObserver::postInitialize()
{
    _p_cameraEntity = dynamic_cast< EnCamera* >( EntityManager::get()->findEntity( ENTITY_NAME_CAMERA ) );
    if ( _p_cameraEntity )
    {
        log << Log::LogLevel( Log::L_WARNING ) << "observer entity cannot be set up as there is already a camera instance in level!" << std::endl;
        return;
    }

    // create a camera instance for observer
    _p_cameraEntity = dynamic_cast< EnCamera* >( EntityManager::get()->createEntity( ENTITY_NAME_CAMERA, "_observerCamera_" ) );
    assert( _p_cameraEntity && "error creating observer camera" );

    _curRotation = osg::Quat( 
                                osg::DegreesToRadians( _rotation.x() ), osg::Vec3f( 0.0f, 1.0f, 0.0f ), // roll
                                osg::DegreesToRadians( _rotation.y() ), osg::Vec3f( 1.0f, 0.0f, 0.0f ), // pitch
                                osg::DegreesToRadians( _rotation.z() ), osg::Vec3f( 0.0f, 0.0f, 1.0f )  // yaw
                            );

    _curPosition = _position;
    _p_cameraEntity->initialize();
    _p_cameraEntity->postInitialize();
    _p_cameraEntity->setEnable( true );
    _p_cameraEntity->setCameraTranslation( _position, _curRotation );

    _inputHandler = new ObserverIH( this );
}

void EnObserver::updateEntity( float deltaTime )
{
    // store delta time for input handler
    _deltaTime = deltaTime;
}

} // namespace CTD
