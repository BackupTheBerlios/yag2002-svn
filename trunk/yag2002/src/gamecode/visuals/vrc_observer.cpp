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
 # entity observer
 #  this entity can be used for freely flying through a level
 #
 #   date of creation:  07/20/2005
 #
 #   author:            ali botorabi (boto)
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_observer.h"
#include "vrc_camera.h"

namespace vrc
{
#define OBSERVER_WND      "_observer_"


//! Input handler for observer
class ObserverIH : public vrc::gameutils::GenericInputHandler< EnObserver >
{
    public:

        explicit                            ObserverIH( EnObserver* p_ent ) :
                                             vrc::gameutils::GenericInputHandler< EnObserver >( p_ent )
                                            {
                                                _lockMovement    = false;
                                                _moveRight       = false;
                                                _moveLeft        = false;
                                                _moveForward     = false;
                                                _moveBackward    = false;
                                                _rotationEnabled = false;
                                                _infoEnabled     = false;

                                                _pitch = 0.0f;
                                                _yaw   = 0.0f;

                                                // get the current screen size
                                                unsigned int width, height;
                                                yaf3d::Application::get()->getScreenSize( width, height );
                                                // calculate the middle of app window
                                                _screenMiddleX = static_cast< Uint16 >( width * 0.5f );
                                                _screenMiddleY = static_cast< Uint16 >( height * 0.5f );
                                            }

        virtual                             ~ObserverIH() {}

        bool                                handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

        void                                lockMovement( bool en )
                                            {
                                                _lockMovement = en;
                                            }

        void                                enableInfoWindow( bool en )
                                            {
                                                _infoEnabled = en;
                                                getUserObject()->enableInfoWindow( en );
                                            }

        void                                setCameraPosition( const osg::Vec3f& pos)
                                            {
                                                getUserObject()->_p_cameraEntity->setCameraPosition( pos );
                                            }

        void                                setCameraPitchYaw( float pitch, float yaw )
                                            {
                                                _pitch = -pitch;
                                                _yaw   = -yaw;
                                                getUserObject()->_p_cameraEntity->setLocalPitchYaw( pitch, yaw );
                                            }

    protected:

        // some internal variables
        bool                                _lockMovement;

        bool                                _moveRight;
        bool                                _moveLeft;
        bool                                _moveForward;
        bool                                _moveBackward;

        float                               _pitch;
        float                               _yaw;

        bool                                _rotationEnabled;
        bool                                _infoEnabled;

        Uint16                              _screenMiddleX;
        Uint16                              _screenMiddleY;
};

bool ObserverIH::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*aa*/ )
{
    const osgSDL::SDLEventAdapter* p_eventAdapter = dynamic_cast< const osgSDL::SDLEventAdapter* >( &ea );
    assert( p_eventAdapter && "invalid event adapter received" );

    unsigned int eventType  = p_eventAdapter->getEventType();
    int          key        = p_eventAdapter->getSDLKey();

    if ( eventType == osgGA::GUIEventAdapter::FRAME )
        return false;

    // terminate application on Escape
    if ( key == SDLK_ESCAPE )
        yaf3d::Application::get()->stop();

    // toggle info dialog rendering
    if ( ( key == SDLK_F9 ) && ( eventType == osgGA::GUIEventAdapter::KEYDOWN ) )
    {
        _infoEnabled = !_infoEnabled;
        getUserObject()->enableInfoWindow( _infoEnabled );
    }

    // don't check for movement keys when locked
    if ( _lockMovement )
        return false;

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

    if ( _moveForward )
        pos._v[ 1 ] += speed * dt;
    else if ( _moveBackward )
        pos._v[ 1 ] -= speed * dt;

    if ( _moveLeft )
        pos._v[ 0 ] -= speed * dt;
    else if ( _moveRight )
        pos._v[ 0 ] += speed * dt;

    const SDL_Event& sdlevent = p_eventAdapter->getSDLEvent();

    // enable the camera rotation on dragging right mouse button
    if ( sdlevent.button.button == SDL_BUTTON_RIGHT )
    {
        if ( eventType == osgGA::GUIEventAdapter::PUSH )
            _rotationEnabled = true;
        else if ( eventType == osgGA::GUIEventAdapter::RELEASE )
            _rotationEnabled = false;
    }

    // adjust pitch and yaw
    if ( ( eventType == osgGA::GUIEventAdapter::DRAG ) && _rotationEnabled )
    {
        // skip events which come in when we warp the mouse pointer to middle of app window ( see below )
        if ( (  sdlevent.motion.x == _screenMiddleX ) && ( sdlevent.motion.y == _screenMiddleY ) )
            return false;

        static float lastxrel = 0;
        static float lastyrel = 0;
        float xrel = float( sdlevent.motion.xrel ) * dt;
        float yrel = float( sdlevent.motion.yrel ) * dt;

        // smooth the view change avoiding hart camera rotations
        _yaw   += ( xrel + lastxrel ) * 0.1f;
        _pitch += ( yrel + lastyrel ) * 0.1f;
        lastxrel = xrel;
        lastyrel = yrel;

        // set new camera orientation
        p_camera->setLocalPitchYaw( -_pitch, -_yaw );

        // reset mouse position in order to avoid leaving the app window
        yaf3d::Application::get()->getViewer()->requestWarpPointer( _screenMiddleX, _screenMiddleY );
    }

    // update camera position
    if ( _moveForward || _moveBackward || _moveLeft || _moveRight )
    {
        pos = p_camera->getLocalRotation() * pos;
        p_camera->setCameraPosition( pos + p_camera->getCameraPosition() );
    }

    return false;
}

//! Implement and register the observer entity factory
YAF3D_IMPL_ENTITYFACTORY( ObserverEntityFactory )

EnObserver::EnObserver() :
_maxSpeed( 10.0f ),
_deltaTime( 0.03f ),
_isPersistent( false ),
_needUpdate ( false ),
_p_cameraEntity( NULL ),
_p_wnd( NULL ),
_p_outputText( NULL ),
_p_speedBar( NULL ),
_p_lockCheckbox( NULL ),
_speed( 10.0f ),
_infoWindowEnable( true ),
_fpsTimer( 0.0f ),
_fpsCounter( 0 ),
_fps( 0 )
{
    // register entity attributes
    getAttributeManager().addAttribute( "position", _position );
    getAttributeManager().addAttribute( "rotation", _rotation );
    getAttributeManager().addAttribute( "maxSpeed", _maxSpeed );
}

EnObserver::~EnObserver()
{
    try
    {
        CEGUI::WindowManager::getSingleton().destroyWindow( _p_wnd );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "EnObserver: problem cleaning up entity." << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

void EnObserver::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle notifications
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_NEW_LEVEL_INITIALIZED:
            break;

        case YAF3D_NOTIFY_ENTITY_ATTRIBUTE_CHANGED:

            // set initial pitch yaw in input handler
            _inputHandler->setCameraPosition( _position );
            _inputHandler->setCameraPitchYaw( osg::DegreesToRadians( _rotation.y() ), osg::DegreesToRadians( _rotation.z() ) );
            break;

        // we have to trigger the deletion ourselves! ( this entity can be peristent )
        case YAF3D_NOTIFY_SHUTDOWN:

            if ( _isPersistent )
                yaf3d::EntityManager::get()->deleteEntity( this );
            break;

        default:
            ;
    }
}

void EnObserver::initialize()
{
    try
    {
        _p_wnd = static_cast< CEGUI::FrameWindow* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/FrameWindow", OBSERVER_WND "mainFrame" ) );
        _p_wnd->subscribeEvent( CEGUI::FrameWindow::EventCloseClicked, CEGUI::Event::Subscriber( &vrc::EnObserver::onClickedClose, this ) );
        _p_wnd->setSize( CEGUI::Size( 0.35f, 0.25f ) );
        _p_wnd->setText( "tools" );
        _p_wnd->setPosition( CEGUI::Point( 0, 0 ) );
        _p_wnd->setAlpha( 0.7f );
        _p_wnd->setAlwaysOnTop( true );
        _p_wnd->setSizingEnabled( false );

        _p_lockCheckbox = static_cast< CEGUI::Checkbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Checkbox", OBSERVER_WND "lock" ) );
        _p_lockCheckbox->subscribeEvent( CEGUI::Checkbox::EventCheckStateChanged, CEGUI::Event::Subscriber( &vrc::EnObserver::onLockChanged, this ) );
        _p_lockCheckbox->setSize( CEGUI::Size( 0.9f, 0.22f ) );
        _p_lockCheckbox->setPosition( CEGUI::Point( 0.05f, 0.1f ) );
        _p_lockCheckbox->setSelected( false );
        _p_lockCheckbox->setText( "lock movement" );
        _p_wnd->addChildWindow( _p_lockCheckbox );

        CEGUI::StaticText* p_stext = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", OBSERVER_WND "st01" ) );
        p_stext->setSize( CEGUI::Size( 0.15f, 0.08f ) );
        p_stext->setPosition( CEGUI::Point( 0.05f, 0.3f ) );
        p_stext->setText( "speed" );
        p_stext->setFrameEnabled( false );
        p_stext->setBackgroundEnabled( false );
        _p_wnd->addChildWindow( p_stext );

        _p_speedBar = static_cast< CEGUI::Scrollbar* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/HorizontalScrollbar", OBSERVER_WND "speed" ) );
        _p_speedBar->subscribeEvent( CEGUI::Scrollbar::EventScrollPositionChanged, CEGUI::Event::Subscriber( &vrc::EnObserver::onSpeedChanged, this ) );
        _p_speedBar->setSize( CEGUI::Size( 0.75f, 0.075f ) );
        _p_speedBar->setPosition( CEGUI::Point( 0.2f, 0.3f ) );
        _p_speedBar->setScrollPosition( 1.0f );
        _p_wnd->addChildWindow( _p_speedBar );

        _p_outputText = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", OBSERVER_WND "output" ) );
        _p_outputText->setSize( CEGUI::Size( 0.9f, 0.4f ) );
        _p_outputText->setPosition( CEGUI::Point( 0.05f, 0.45f ) );
        _p_outputText->setFont( YAF3D_GUI_CONSOLE );
        _p_wnd->addChildWindow( _p_outputText );

        yaf3d::GuiManager::get()->getRootWindow()->addChildWindow( _p_wnd );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "EnPlayerInfoDisplay: problem creating gui" << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
    }


    yaf3d::EntityManager::get()->registerUpdate( this, true );         // register entity in order to get updated per simulation step
    yaf3d::EntityManager::get()->registerNotification( this, true );   // register entity in order to get notifications (e.g. from menu entity)
}

void EnObserver::postInitialize()
{
    _p_cameraEntity = dynamic_cast< EnCamera* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_CAMERA ) );
    if ( _p_cameraEntity )
    {
        log_warning << "observer entity cannot be set up as there is already a camera instance in level!" << std::endl;
        return;
    }

    // create a camera instance for observer
    _p_cameraEntity = dynamic_cast< EnCamera* >( yaf3d::EntityManager::get()->createEntity( ENTITY_NAME_CAMERA, "_observerCamera_" ) );
    assert( _p_cameraEntity && "error creating observer camera" );

    _p_cameraEntity->initialize();
    _p_cameraEntity->postInitialize();
    _p_cameraEntity->setEnable( true );
    _inputHandler = new ObserverIH( this );

    // set initial pitch yaw in input handler
    _inputHandler->setCameraPosition( _position );
    _inputHandler->setCameraPitchYaw( osg::DegreesToRadians( _rotation.y() ), osg::DegreesToRadians( _rotation.z() ) );

    // the default is info window is disabled ( press F1 to activate it )
    enableInfoWindow( false );
}

bool EnObserver::onClickedClose( const CEGUI::EventArgs& /*arg*/ )
{
    // hide the info window via input handler, so it has the change to update its internal state
    _inputHandler->enableInfoWindow( false );
    return true;
}

bool EnObserver::onSpeedChanged( const CEGUI::EventArgs& /*arg*/ )
{
    _speed = _p_speedBar->getScrollPosition();
    _speed *= _maxSpeed;
    return true;
}

bool EnObserver::onLockChanged( const CEGUI::EventArgs& /*arg*/ )
{
    _inputHandler->lockMovement( _p_lockCheckbox->isSelected() );
    return true;
}

void EnObserver::updateEntity( float deltaTime )
{
    // store delta time for input handler
    _deltaTime = deltaTime;

    // update info window content if enabled
    if ( _infoWindowEnable )
    {
        _fpsTimer += deltaTime;
        if ( _fpsTimer > 1.0f )
        {
            _fpsTimer -= 1.0f;
            _fps = _fpsCounter;
            _fpsCounter = 0;
        }
        else
        {
            _fpsCounter++;
        }

        osg::Vec3f pos( _p_cameraEntity->getCameraPosition() );
        osg::Quat  rot( _p_cameraEntity->getLocalRotation() );

        // update output window
        static char text[ 1024 ];
        sprintf( text,
            " fps       %d\n"
            " position  %0.2f %0.2f %0.2f\n"
            " rotation  %0.2f %0.2f %0.2f %0.2f\n",
            _fps,
            pos.x(), pos.y(), pos.z(),
            rot.x(), rot.y(), rot.z(), rot.w()
            );

        CEGUI::String info( text );
        _p_outputText->setText( info );
    }
}

void EnObserver::enableInfoWindow( bool en )
{
    _infoWindowEnable = en;
    if ( en )
        _p_wnd->show();
    else
        _p_wnd->hide();
}

} // namespace vrc
