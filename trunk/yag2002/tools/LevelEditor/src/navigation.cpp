/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2009, A. Botorabi
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
 # game navigation related classes
 #
 #   date of creation:  02/01/2009
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/


#include <vrc_main.h>
#include "navigation.h"
#include "gameinterface.h"
#include "editor.h"
#include "editorutils.h"
#include "scenetools.h"


//! Some navigation constants
#define MOVE_SPEED_STEP     10.0f
#define MOVE_WHEEL_STEP    ( _moveSpeed * 0.5f )


YAF3D_SINGLETON_IMPL( GameNavigator )


GameNavigator::GameNavigator() :
 _enable( true ),
 _mode( EntitySelect ),
 _moveSpeed( 100.0f ),
 _rotationSpeed( 5.0f ),
 _yaw( 0.0f ),
 _pitch( 0.0f ),
 _deltaTime( 0.0f ),
 _nearClip( 0.5f ),
 _farClip( 1000.0f ),
 _fov( 45.0f ),
 _screenMiddleX( 0 ),
 _screenMiddleY( 0 ),
 _fpsTimer( 0.0f ),
 _fpsCnt( 0 ),
 _fps( 0 ),
 _currX( 0 ),
 _currY( 0 ),
 _inputCode( NoCode ),
 _p_cbNotify( NULL ),
 _p_sceneTools( NULL ),
 _p_selEntity( NULL )
{
}

GameNavigator::~GameNavigator()
{
}

void GameNavigator::enable( bool en )
{
    ScopedGameUpdateLock lock;
    _enable = en;
}

void GameNavigator::setMode( unsigned int mode )
{
    // lock the game loop first
    ScopedGameUpdateLock lock;

    _mode = mode;

    _p_sceneTools->showHitMarker( false );
    _p_sceneTools->showAxisMarker( false );

    switch ( mode )
    {
        case EntitySelect:
        {
             // activate the axis marker
             _p_sceneTools->showAxisMarker( false );
             _p_sceneTools->showHitMarker( false );
        }
        break;

        case EntityMove:
        {
             // activate the axis marker
             _p_sceneTools->showAxisMarker( true );
             if ( _p_selEntity && _p_selEntity->getTransformationNode() )
                 _p_sceneTools->setMarkerPosition( _p_selEntity->getTransformationNode()->getPosition() );
        }
        break;

        case EntityRotate:
        {
             // activate the axis marker
             _p_sceneTools->showAxisMarker( true );
             if ( _p_selEntity && _p_selEntity->getTransformationNode() )
                 _p_sceneTools->setMarkerPosition( _p_selEntity->getTransformationNode()->getPosition() );
        }
        break;

        case EntityPlace:
        {
            // exclude the selected entity from hit tests, the selected entity can be also empty.
             _p_sceneTools->excludeFromPicking( _p_selEntity );
             // activate the axis marker
             _p_sceneTools->showAxisMarker( false );
             if ( _p_selEntity && _p_selEntity->getTransformationNode() )
                 _p_sceneTools->setMarkerPosition( _p_selEntity->getTransformationNode()->getPosition() );
        }
        break;

        case Inspect:
        {
            // remove any entity exclusion from picking
            _p_sceneTools->excludeFromPicking( NULL );
            _p_sceneTools->showHitMarker( true );
        }
        break;

        default:
            assert( NULL && "invalid navigator mode!" );
    }
}

unsigned int GameNavigator::getMode() const
{
    return _mode;
}

void GameNavigator::setSpeed( float speed )
{
    ScopedGameUpdateLock lock;
    _moveSpeed = speed;
}

unsigned int GameNavigator::getFPS() const
{
    return _fps;
}

float GameNavigator::getSpeed() const
{
    return _moveSpeed;
}

void GameNavigator::setFOV( float fov )
{
    ScopedGameUpdateLock lock;

    _fov = fov;

    unsigned int width, height;
    yaf3d::Application::get()->getScreenSize( width, height );
    yaf3d::Application::get()->getSceneView()->setProjectionMatrixAsPerspective( _fov, ( float( width ) / float( height ) ), _nearClip, _farClip );
}

float GameNavigator::getFOV() const
{
    return _fov;
}

void GameNavigator::setBackgroundColor( const osg::Vec3f& color )
{
    ScopedGameUpdateLock lock;

    _backgroundColor = color;

     yaf3d::Application::get()->getSceneView()->setClearColor( osg::Vec4f( _backgroundColor, 1.0f ) );
}

const osg::Vec3f& GameNavigator::getBackgroundColor() const
{
    return _backgroundColor;
}

void GameNavigator::selectEntity( yaf3d::BaseEntity* p_entity )
{
    ScopedGameUpdateLock lock;

    _p_selEntity = p_entity;

    if ( _p_sceneTools )
        _p_sceneTools->highlightEntity( _p_selEntity );
}

void GameNavigator::setNotifyCallback( CallbackNavigatorNotify* p_cb )
{
    _p_cbNotify = p_cb;
}

void GameNavigator::initialize()
{
    // register in viewer to get event callbacks
    yaf3d::Application::get()->getViewer()->addEventHandler( this );

    unsigned int width, height;
    yaf3d::Application::get()->getScreenSize( width, height );
    _screenMiddleX = static_cast< Uint16 >( width * 0.5f );
    _screenMiddleY = static_cast< Uint16 >( height * 0.5f );

    yaf3d::Application::get()->getSceneView()->setProjectionMatrixAsPerspective( _fov, ( float( width ) / float( height ) ), _nearClip, _farClip );
    yaf3d::Application::get()->getSceneView()->setClearColor( osg::Vec4f( _backgroundColor, 1.0f ) );
    // avoid overriding our near and far plane setting by scene viewer
    yaf3d::Application::get()->getSceneView()->setComputeNearFarMode( osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR );

    // get the initial rotation and position from settings
    yaf3d::SettingsPtr settings = yaf3d::SettingsManager::get()->getProfile( EDITOR_SETTINGS_PROFILE );
    assert( settings.valid() && "invalid editor configuration!" );
    osg::Vec3f navPos;
    osg::Vec2f navRot;
    settings->getValue( ES_NAV_POSITION, navPos );
    settings->getValue( ES_NAV_ROTATION, navRot );
    setCameraPosition( navPos );
    setCameraPitchYaw( -navRot._v[ 0 ], -navRot._v[ 1 ] );

    // setup the scene tools
    _p_sceneTools = new SceneTools;
    if ( !_p_sceneTools->initialize( width, height ) )
    {
        log_error << "[Editor] could not initialize scene tools" << std::endl;
        assert( NULL && "cannot initialize scene tools!" );
    }
    // set the top node as scene node in tools
    _p_sceneTools->setSceneNode( yaf3d::Application::get()->getSceneRootNode() );
}

void GameNavigator::shutdown()
{
    // deregister in viewer from getting event callbacks
    yaf3d::Application::get()->getViewer()->removeEventHandler( this );
}

void GameNavigator::setCameraPosition( osg::Vec3f pos )
{
    _position = pos;
}

const osg::Vec3f& GameNavigator::getCameraPosition() const
{
    return _position;
}

void GameNavigator::setCameraPitchYaw( float pitch, float yaw )
{
    _rotation = osg::Quat(
                              0.0f,                           osg::Vec3f( 0.0f, 1.0f, 0.0f ), // roll
                              osg::DegreesToRadians( pitch ), osg::Vec3f( 1.0f, 0.0f, 0.0f ), // pitch
                              osg::DegreesToRadians( yaw   ), osg::Vec3f( 0.0f, 0.0f, 1.0f )  // yaw
                          );

    _pitch = -pitch;
    _yaw   = -yaw;
}

void GameNavigator::getCameraPitchYaw( float& pitch, float& yaw ) const
{
    pitch = _pitch;
    yaw   = _yaw;
}

void GameNavigator::setNearFarClip( float nearclip, float farclip )
{
    _nearClip = nearclip;
    _farClip  = farclip;

    {
        ScopedGameUpdateLock lock;
        unsigned int width, height;
        yaf3d::Application::get()->getScreenSize( width, height );
        yaf3d::Application::get()->getSceneView()->setProjectionMatrixAsPerspective( _fov, ( float( width ) / float( height ) ), _nearClip, _farClip );
    }
}

void GameNavigator::getNearFarClip( float& nearclip, float& farclip ) const
{
    nearclip = _nearClip;
    farclip  = _farClip;
}

void GameNavigator::update( float deltatime )
{
    // mesure the fps
    _fpsTimer += deltatime;
    _fpsCnt++;
    if ( _fpsTimer > 1.0f )
    {
        _fpsTimer -= 1.0f;
        _fps       = _fpsCnt;
        _fpsCnt    = 0;
    }

    if ( !_enable )
        return;

    _deltaTime = deltatime;

    osg::Vec3f  pos;

    if ( _inputCode & Forward )
        pos._v[ 1 ] += _moveSpeed * _deltaTime;
    else if ( _inputCode & Backward )
        pos._v[ 1 ] -= _moveSpeed * _deltaTime;

    if ( _inputCode & Left )
        pos._v[ 0 ] -= _moveSpeed * _deltaTime;
    else if ( _inputCode & Right )
        pos._v[ 0 ] += _moveSpeed * _deltaTime;

    // handle the mouse wheel
    if ( _inputCode & Ctrl )
    {
        // change move speed
        if ( _inputCode & WheelUp )
        {
            _moveSpeed += MOVE_SPEED_STEP;
        }
        else if ( _inputCode & WheelDown )
        {
            if ( ( _moveSpeed - MOVE_SPEED_STEP ) > 0 )
                _moveSpeed -= MOVE_SPEED_STEP;
        }
    }
    else
    {
        // fast move with wheels
        if ( _inputCode & WheelUp )
            _position += _rotation * osg::Vec3f( 0, MOVE_WHEEL_STEP,  0 );
        else if ( _inputCode & WheelDown )
            _position += _rotation * osg::Vec3f( 0, -MOVE_WHEEL_STEP,  0 );
    }

    // reset the wheel flags
    _inputCode &= ~( WheelUp | WheelDown );

    // handle selection focus
    if ( _inputCode & ObjFocus )
    {
        if ( _p_selEntity && _p_selEntity->getTransformationNode() )
        {
            const osg::BoundingSphere& bs = _p_selEntity->getTransformationNode()->getBound();
            if ( bs.valid() )
            {
                float radius   = bs.radius();
                float distance = ( _position - _p_selEntity->getPosition() ).length();
                distance -= radius * 2.0f;
                if ( distance < 0.0f )
                    distance = -radius * 2.0f;

                osg::Vec3f dir = _p_selEntity->getPosition() - _position;
                dir.normalize();
                _position += dir * distance;
            }
        }

        // remove the focus flag
        _inputCode &= ~ObjFocus;
    }

    // update camera position
    if ( _inputCode & ( Forward|Backward|Left|Right) )
    {
        pos = _rotation * pos;
        _position += pos;
    }

    // setup the view matrix basing on position and transformation
    osg::Matrixf trans;
    trans.makeTranslate( _position.x(), _position.y(), _position.z() );
    osg::Matrix rot;
    rot.makeRotate( _rotation );

    osg::Matrixf mat;
    mat = rot * trans;

    //  inverse the matrix
    osg::Matrixf inv = osg::Matrixf::inverse( mat );

    // adjust Z-UP
    static osg::Matrixf adjustZ_Up ( osg::Matrixf::rotate( -osg::PI / 2.0f, 1.0f, 0.0f, 0.0f ) );

    // set view matrix
    yaf3d::Application::get()->getSceneView()->setViewMatrix( osg::Matrixf( inv.ptr() ) * adjustZ_Up  );
}

bool GameNavigator::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
    if ( !_enable )
        return false;

    const osgSDL::SDLEventAdapter* p_eventAdapter = dynamic_cast< const osgSDL::SDLEventAdapter* >( &ea );
    assert( p_eventAdapter && "invalid event adapter received" );

    unsigned int     eventType = p_eventAdapter->getEventType();
    int              key       = p_eventAdapter->getSDLKey();
    const SDL_Event& sdlevent  = p_eventAdapter->getSDLEvent();

    if ( eventType == osgGA::GUIEventAdapter::FRAME )
        return false;

    if ( eventType == osgGA::GUIEventAdapter::KEYDOWN )
    {
        if ( key == SDLK_w )
            _inputCode |= Forward;
        else if ( key == SDLK_s )
            _inputCode |= Backward;

        if ( key == SDLK_a )
            _inputCode |= Left;
        else if ( key == SDLK_d )
            _inputCode |= Right;
    }
    else if ( eventType == osgGA::GUIEventAdapter::KEYUP )
    {
        if ( key == SDLK_w )
            _inputCode &= ~Forward;
        else if ( key == SDLK_s )
            _inputCode &= ~Backward;

        if ( key == SDLK_a )
            _inputCode &= ~Left;
        else if ( key == SDLK_d )
            _inputCode &= ~Right;
    }

    // enable the camera rotation on dragging right mouse button
    if ( sdlevent.button.button == SDL_BUTTON_RIGHT )
    {
        if ( eventType == osgGA::GUIEventAdapter::PUSH )
            _inputCode |= Rotate;
        else if ( eventType == osgGA::GUIEventAdapter::RELEASE )
            _inputCode &= ~Rotate;
    }

    // adjust pitch and yaw
    if ( ( eventType == osgGA::GUIEventAdapter::DRAG ) && ( _inputCode & Rotate ) )
    {
        // skip events which come in when we warp the mouse pointer to middle of app window ( see below )
        if ( ( sdlevent.motion.x == _screenMiddleX ) && ( sdlevent.motion.y == _screenMiddleY ) )
            return false;

        _inputCode |= Drag;

        static float lastxrel = 0;
        static float lastyrel = 0;
        float xrel = float( sdlevent.motion.xrel ) * _deltaTime * _rotationSpeed;
        float yrel = float( sdlevent.motion.yrel ) * _deltaTime * _rotationSpeed;

        // smooth the view change avoiding hart camera rotations
        _yaw   += ( xrel + lastxrel ) * 0.5f;
        _pitch += ( yrel + lastyrel ) * 0.5f;
        lastxrel = xrel;
        lastyrel = yrel;

        // update the camera rotation
        _rotation = osg::Quat(
                                  0.0f, osg::Vec3f( 0.0f, 1.0f, 0.0f ),                             // roll
                                  osg::DegreesToRadians( -_pitch ), osg::Vec3f( 1.0f, 0.0f, 0.0f ), // pitch
                                  osg::DegreesToRadians( -_yaw ), osg::Vec3f( 0.0f, 0.0f, 1.0f )    // yaw
                              );

        // reset mouse position in order to avoid leaving the app window
        yaf3d::Application::get()->getViewer()->requestWarpPointer( _screenMiddleX, _screenMiddleY );
    }

    // is there a picking callback?
    if ( _mode & Inspect )
    {
        if ( eventType == osgGA::GUIEventAdapter::MOVE )
        {
            // this updates also the marker transformation
            _p_sceneTools->hitScene( sdlevent.motion.x, sdlevent.motion.y, _position );
        }

        if ( ( sdlevent.button.button == SDL_BUTTON_LEFT ) && ( eventType == osgGA::GUIEventAdapter::PUSH ) )
        {
            if ( _p_selEntity && _p_selEntity->getTransformationNode() )
            {
                _p_selEntity->getTransformationNode()->setPosition( _p_sceneTools->getHitPosition() );
                _p_sceneTools->highlightEntity( _p_selEntity );
            }

            if ( _p_cbNotify )
                _p_cbNotify->onArrowClick( _p_sceneTools->getHitPosition() );
        }
    }

    // placing an entity?
    if ( _mode & EntityPlace )
    {
        static bool btnleftpressed = false;
        static bool entitymoved    = false;
        if ( sdlevent.button.button == SDL_BUTTON_LEFT )
        {
            // save the current entity selection, pick method can modify it
            yaf3d::BaseEntity* p_currsel = _p_selEntity;

            // pick another entity?
            if ( eventType == osgGA::GUIEventAdapter::PUSH )
            {
                btnleftpressed = true;
                entitymoved    = false;
                _currX         = sdlevent.motion.x;
                _currY         = sdlevent.motion.y;

                _p_selEntity = _p_sceneTools->pickEntity( sdlevent.motion.x, sdlevent.motion.y, p_currsel );

                // exclude the selected entity from hit tests
                _p_sceneTools->excludeFromPicking( _p_selEntity );
                 if ( _p_selEntity && _p_selEntity->getTransformationNode() )
                     _p_sceneTools->setMarkerPosition( _p_selEntity->getTransformationNode()->getPosition() );

                if ( _p_selEntity != p_currsel )
                {
                    // is there a picking callback?
                    if ( _p_cbNotify )
                        _p_cbNotify->onEntityPicked( _p_selEntity );

                    if ( _p_selEntity && _p_selEntity->getTransformationNode() )
                    {
                        _p_sceneTools->setMarkerPosition( _p_selEntity->getTransformationNode()->getPosition() );
                        _p_sceneTools->setMarkerOrientation( osg::Quat() );
                    }
                }
            }
            else if ( eventType == osgGA::GUIEventAdapter::RELEASE )
            {
                btnleftpressed = false;
                if ( p_currsel && entitymoved && _p_selEntity->getTransformationNode() )
                {
                    p_currsel->getTransformationNode()->setPosition( _p_sceneTools->getHitPosition() );
                    _p_sceneTools->highlightEntity( p_currsel );

                    // update entity position attribute if one exists
                    osg::Vec3f pos;
                    if ( p_currsel->getAttributeManager().getAttributeValue( "position", pos ) )
                    {
                        p_currsel->getAttributeManager().setAttributeValue( "position", _p_sceneTools->getHitPosition() );
                        yaf3d::EntityManager::get()->sendNotification( YAF3D_NOTIFY_ENTITY_ATTRIBUTE_CHANGED, p_currsel );

                        if ( entitymoved && _p_cbNotify )
                            _p_cbNotify->onEntityPicked( p_currsel );

                    }

                    // reset the marker orientation
                    _p_sceneTools->setMarkerOrientation( osg::Quat() );
                }

                // reset the moved flag
                entitymoved = false;
                _currX = sdlevent.motion.x;
                _currY = sdlevent.motion.y;
            }
            else if ( btnleftpressed && ( eventType == osgGA::GUIEventAdapter::DRAG ) )
            {
                if ( ( _currX != sdlevent.motion.x ) || ( _currY != sdlevent.motion.y ) )
                {
                    // now store the current x/y screen coords of pointer
                    _currX = sdlevent.motion.x;
                    _currY = sdlevent.motion.y;

                     if ( _p_selEntity && _p_selEntity->getTransformationNode() )
                         _p_sceneTools->setMarkerPosition( _p_selEntity->getTransformationNode()->getPosition() );

                    bool didhit = _p_sceneTools->hitScene( sdlevent.motion.x, sdlevent.motion.y, _position );
                    if ( entitymoved && didhit && _p_selEntity && _p_selEntity->getTransformationNode() )
                    {
                        p_currsel->getTransformationNode()->setPosition( _p_sceneTools->getHitPosition() );
                        _p_sceneTools->highlightEntity( p_currsel );
                    }

                    entitymoved = true;
                }
            }
        }
    }
    else if ( _mode & EntitySelect ) // pick an entity
    {
        if ( ( sdlevent.button.button == SDL_BUTTON_LEFT ) && ( eventType == osgGA::GUIEventAdapter::PUSH ) )
        {
            // try to pick something
            yaf3d::BaseEntity* p_sel = _p_sceneTools->pickEntity( sdlevent.motion.x, sdlevent.motion.y );

             if ( p_sel && p_sel->getTransformationNode() )
                 _p_sceneTools->setMarkerPosition( p_sel->getTransformationNode()->getPosition() );

            // is there a picking callback?
            if ( _p_cbNotify )
                _p_cbNotify->onEntityPicked( p_sel );
        }
    }
    else if ( _mode & EntityMove ) // move an entity
    {
        static unsigned int moveaxis = SceneTools::AxisNone;
        static osg::Vec3f   dir;

        if ( sdlevent.button.button == SDL_BUTTON_LEFT )
        {
            if ( eventType == osgGA::GUIEventAdapter::PUSH )
            {
                _currX = sdlevent.motion.x;
                _currY = sdlevent.motion.y;

                // was an axis handle hit?
                _p_sceneTools->hitScene( sdlevent.motion.x, sdlevent.motion.y, _position );
                if ( _p_sceneTools->getAxisHits() != SceneTools::AxisNone )
                {
                    moveaxis = _p_sceneTools->getAxisHits();
                    // update the direction to entity
                    if ( _p_selEntity && _p_selEntity->getTransformationNode() )
                    {
                        dir = _position - _p_selEntity->getTransformationNode()->getPosition();
                        dir.normalize();
                    }
                }
                else
                {
                    moveaxis = SceneTools::AxisNone;

                    // try to pick something
                    yaf3d::BaseEntity* p_sel = _p_sceneTools->pickEntity( sdlevent.motion.x, sdlevent.motion.y );

                    if ( p_sel && p_sel->getTransformationNode() )
                    {
                        // update the direction to entity
                        dir = _position - p_sel->getTransformationNode()->getPosition();
                        dir.normalize();
                        // update the marker position
                        _p_sceneTools->setMarkerPosition( p_sel->getTransformationNode()->getPosition() );
                    }

                    // is there a picking callback?
                    if ( _p_cbNotify )
                        _p_cbNotify->onEntityPicked( p_sel );
                }
            }
            else if ( eventType == osgGA::GUIEventAdapter::RELEASE )
            {
                    moveaxis = SceneTools::AxisNone;
                    if ( _p_selEntity )
                        _p_sceneTools->highlightEntity( _p_selEntity );
            }
            else if ( ( moveaxis != SceneTools::AxisNone ) && _p_selEntity && ( eventType == osgGA::GUIEventAdapter::DRAG ) )
            {
                if ( ( _currX != sdlevent.motion.x ) || ( _currY != sdlevent.motion.y ) )
                {
                    // now store the current x/y screen coords of pointer
                    short xdiff = _currX - sdlevent.motion.x;
                    short ydiff = _currY - sdlevent.motion.y;
                    _currX = sdlevent.motion.x;
                    _currY = sdlevent.motion.y;

                    if ( _p_selEntity && _p_selEntity->getTransformationNode() )
                    {
                        //! TODO: implement a better way to move; track the projected mouse coords in 3d space
                        osg::Vec3f currpos = _p_selEntity->getTransformationNode()->getPosition();

                        // we have to track in which quadrant the camera is in order to move in right direction
                        float& X      = dir._v[ 0 ];
                        float& Y      = dir._v[ 1 ];
                        float sign   = 1.0f;

                        log_debug << "x " << dir._v[ 0 ] << " y " << dir._v[ 1 ] << " z " << dir._v[ 2 ] << std::endl;

                        float dist = 0.0f;
                        if ( moveaxis == SceneTools::AxisZ )
                        {
                            dist = ( ydiff > 0 ) ? dist : -dist;
                            dist = 0.01f * float( ydiff );
                        }
                        else // we have to track in which quadrant the camera is in order to move in right direction
                        {
                            dist = ( xdiff < 0 ) ? dist : -dist;
                            dist = 0.01f * float( xdiff );

                            if ( moveaxis == SceneTools::AxisX )
                            {
                                if ( ( X < 0 ) && ( Y < 0 ) )
                                    sign = -sign;
                                else if ( ( X > 0 ) && ( Y < 0 ) )
                                    sign = -sign;
                            }
                            else
                            {
                                if ( ( X > 0 ) && ( Y > 0 ) )
                                    sign = -sign;
                                else if ( ( X > 0 ) && ( Y < 0 ) )
                                    sign = -sign;
                            }

                            dist *= sign;
                        }

                        osg::Vec3f movedist(
                                            ( moveaxis == SceneTools::AxisX ) ? dist : 0.0f,
                                            ( moveaxis == SceneTools::AxisY ) ? dist : 0.0f,
                                            ( moveaxis == SceneTools::AxisZ ) ? dist : 0.0f
                                            );

                        _p_selEntity->getTransformationNode()->setPosition( currpos + movedist );
                        _p_sceneTools->setMarkerPosition( _p_selEntity->getTransformationNode()->getPosition() );
                    }
                }
            }
        }
    }

    // check for control keys
    if ( ( key == SDLK_RCTRL ) || ( key == SDLK_LCTRL ) )
    {
        if ( eventType == osgGA::GUIEventAdapter::KEYDOWN )
             _inputCode |= Ctrl;
        else if ( eventType == osgGA::GUIEventAdapter::KEYUP )
             _inputCode &= ~Ctrl;
    }

    // handle wheel up/down
    if ( sdlevent.button.type == SDL_MOUSEBUTTONDOWN )
    {
        if ( sdlevent.button.button == SDL_BUTTON_WHEELUP )
            _inputCode |= WheelUp;
        else if ( sdlevent.button.button == SDL_BUTTON_WHEELDOWN )
            _inputCode |= WheelDown;
    }

    // check for focus
    if ( ( ( key == SDLK_z ) || ( key == SDLK_y ) ) && ( eventType == osgGA::GUIEventAdapter::KEYDOWN ) )
    {
        _inputCode |= ObjFocus;
    }

    // if the game window changes focus then stop placing entity! the entity may be destroyed by entity view.
    if ( sdlevent.active.state & SDL_APPINPUTFOCUS )
    {
        static unsigned int state = 0;
        if ( sdlevent.active.state == ( SDL_APPINPUTFOCUS | SDL_APPACTIVE ) )
        {
            if ( sdlevent.active.gain == 0 )
                state = yaf3d::GameState::Minimized;
            else
            {
                if ( state == yaf3d::GameState::LostFocus )
                    state = yaf3d::GameState::GainedFocus;
                else
                    state = yaf3d::GameState::Restored;
            }
        }
        else if ( sdlevent.active.state & SDL_APPINPUTFOCUS )
        {
            if ( sdlevent.active.gain == 0 )
                state = yaf3d::GameState::LostFocus;
            else
                state = yaf3d::GameState::GainedFocus;
        }

        // need for any handling? not atm.
        //if ( ( state == yaf3d::GameState::LostFocus ) || ( state == yaf3d::GameState::Minimized ) )
        //{
        //}
    }

    return false;
}
