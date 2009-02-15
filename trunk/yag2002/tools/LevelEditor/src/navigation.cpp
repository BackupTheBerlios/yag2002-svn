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
#include <osg/ComputeBoundsVisitor>


//! Some navigation constants
#define MOVE_SPEED_STEP     10.0f
#define MOVE_WHEEL_STEP    ( _moveSpeed * 0.5f )


YAF3D_SINGLETON_IMPL( GameNavigator )


GameNavigator::GameNavigator() :
 _enable( true ),
 _mode( ShowPickArrow ),
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
 _iscreenWidth( 0.0f ),
 _iscreenHeight( 0.0f ),
 _pickClickCount( 0 ),
 _lastX( 0.0f ),
 _currX( 0 ),
 _lastY( 0.0f ),
 _currY( 0 ),
 _inputCode( NoCode ),
 _p_cbNotify( NULL ),
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

    if ( mode & ShowPickArrow )
    {
        if ( ( _marker.valid() ) && !yaf3d::Application::get()->getSceneRootNode()->containsNode( _marker.get() ) )
            yaf3d::Application::get()->getSceneRootNode()->addChild( _marker.get() );
    }
    else
    {
        yaf3d::Application::get()->getSceneRootNode()->removeChild( _marker.get() );
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
    _fov = fov;

    {
        ScopedGameUpdateLock lock;
        unsigned int width, height;
        yaf3d::Application::get()->getScreenSize( width, height );
        yaf3d::Application::get()->getSceneView()->setProjectionMatrixAsPerspective( _fov, ( float( width ) / float( height ) ), _nearClip, _farClip );
    }
}

float GameNavigator::getFOV() const
{
    return _fov;
}

void GameNavigator::setBackgroundColor( const osg::Vec3f& color )
{
    _backgroundColor = color;

    {
        ScopedGameUpdateLock lock;
        yaf3d::Application::get()->getSceneView()->setClearColor( osg::Vec4f( _backgroundColor, 1.0f ) );
    }
}

const osg::Vec3f& GameNavigator::getBackgroundColor() const
{
    return _backgroundColor;
}

void GameNavigator::selectEntity( yaf3d::BaseEntity* p_entity )
{
    ScopedGameUpdateLock lock;
    _p_selEntity = p_entity;
    highlightEntity( _p_selEntity );
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

    // setup picking related stuff
    {
        _p_lineSegment = new osg::LineSegment;
        // store the window size, used for picking
        _iscreenWidth  = 1.0f / static_cast< float >( width );
        _iscreenHeight = 1.0f / static_cast< float >( height );

        _bboxGeode = new osg::Geode;
        _bboxGeode->setName( "_editorBBoxCube_" );
        _p_linesGeom = new osg::Geometry;
        _p_linesGeom->setSupportsDisplayList( false );
        _p_linesGeom->setUseDisplayList( false );

        osg::StateSet* p_stateSet = new osg::StateSet;
        p_stateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
        _p_linesGeom->setStateSet( p_stateSet );

        osg::Vec3Array* vertices = new osg::Vec3Array( 8 );

        // pass the created vertex array to the points geometry object.
        _p_linesGeom->setVertexArray( vertices );

        // set the colors as before
        osg::Vec4Array* colors = new osg::Vec4Array;
        colors->push_back( osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
        _p_linesGeom->setColorArray( colors );
        _p_linesGeom->setColorBinding( osg::Geometry::BIND_OVERALL );

        GLushort indices[] =
        {
            0,1,
            0,4,
            1,5,
            0,2,

            1,3,
            3,7,
            3,2,
            2,6,

            6,7,
            4,5,
            4,6,
            5,7
        };

        // create the primitive set for bbox and append it to top root node
        _p_linesGeom->addPrimitiveSet( new osg::DrawElementsUShort( osg::PrimitiveSet::LINES, 24, indices ) );
        _bboxGeode->addDrawable( _p_linesGeom );
        yaf3d::Application::get()->getSceneRootNode()->addChild( _bboxGeode.get() );
    }

    // create hit marker
    {
        _marker = new osg::PositionAttitudeTransform;

        osg::ref_ptr< osg::Geode > geodepart1 = new osg::Geode;
        osg::ref_ptr< osg::Geode > geodepart2 = new osg::Geode;

        _marker->addChild( geodepart1.get() );
        _marker->addChild( geodepart2.get() );

        osg::ref_ptr< osg::TessellationHints > hints = new osg::TessellationHints;
        hints->setDetailRatio( 1.0f );

        osg::ref_ptr< osg::ShapeDrawable > shape;

        shape = new osg::ShapeDrawable( new osg::Cylinder( osg::Vec3( 0.f, 0.0f, 0.25f ), 0.1f, 0.5f ), hints.get() );
        shape->setColor(osg::Vec4( 1.0f, 1.0f, 0.5f, 1.0f ) );
        geodepart1->addDrawable( shape.get() );

        shape = new osg::ShapeDrawable( new osg::Cone(osg::Vec3( 0.0f, 0.0f, 0.5f ), 0.2f, 0.3f ), hints.get() );
        shape->setColor( osg::Vec4( 1.0f, 1.0f, 0.0f, 1.0f ) );
        geodepart2->addDrawable( shape.get() );

        _marker->setScale( osg::Vec3f( 3.0f, 3.0f, 4.0f ) );
    }
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
    if ( _mode & ShowPickArrow )
    {
        if ( eventType == osgGA::GUIEventAdapter::MOVE )
        {
            // this updates the marker transformation
            hit( sdlevent.motion.x, sdlevent.motion.y );
        }

        if ( _p_cbNotify && ( sdlevent.button.button == SDL_BUTTON_LEFT ) && ( eventType == osgGA::GUIEventAdapter::PUSH ) )
        {
            if ( _p_selEntity && _p_selEntity->getTransformationNode() )
            {
                _p_selEntity->getTransformationNode()->setPosition( _hitPosition );
                highlightEntity( _p_selEntity );
            }

            _p_cbNotify->onArrowClick( _hitPosition );
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

                _p_selEntity = pick( sdlevent.motion.x, sdlevent.motion.y, p_currsel );
                if ( _p_selEntity != p_currsel )
                {
                    if ( _p_selEntity && _p_selEntity->getTransformationNode() )
                    {
                        _hitPosition = _p_selEntity->getTransformationNode()->getPosition();
                        if ( _marker.valid() )
                        {
                            _marker->setPosition( _hitPosition );
                            _marker->setAttitude( osg::Quat() );
                        }
                    }
                }
            }
            else if ( eventType == osgGA::GUIEventAdapter::RELEASE )
            {
                btnleftpressed = false;
                if ( p_currsel && entitymoved && _p_selEntity->getTransformationNode() )
                {
                    p_currsel->getTransformationNode()->setPosition( _hitPosition );
                    highlightEntity( p_currsel );

                    // update entity position attribute if one exists
                    osg::Vec3f pos;
                    if ( p_currsel->getAttributeManager().getAttributeValue( "position", pos ) )
                    {
                        p_currsel->getAttributeManager().setAttributeValue( "position", _hitPosition );
                        yaf3d::EntityManager::get()->sendNotification( YAF3D_NOTIFY_ENTITY_ATTRIBUTE_CHANGED, p_currsel );

                        if ( entitymoved && _p_cbNotify )
                            _p_cbNotify->onEntityPicked( p_currsel );

                    }

                    _marker->setAttitude( osg::Quat() );
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

                    bool didhit = hit( sdlevent.motion.x, sdlevent.motion.y );
                    if ( entitymoved && didhit && _p_selEntity && _p_selEntity->getTransformationNode() )
                    {
                        p_currsel->getTransformationNode()->setPosition( _hitPosition );
                        highlightEntity( p_currsel );
                    }

                    entitymoved = true;
                }
            }
        }
    }
    else if ( _mode & EntityPick ) // pick an entity
    {
        if ( ( sdlevent.button.button == SDL_BUTTON_LEFT ) && ( eventType == osgGA::GUIEventAdapter::PUSH ) )
        {
            // try to pick something
            pick( sdlevent.motion.x, sdlevent.motion.y );
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

void GameNavigator::highlightEntity( yaf3d::BaseEntity* p_entity )
{
    // set the current highlighted entity, used for zooming
    _p_selEntity = p_entity;

    if ( !p_entity || !p_entity->isTransformable() )
    {
        // let the box disappear when no entity given
        osg::Vec3Array* p_vertices = static_cast< osg::Vec3Array* >( _p_linesGeom->getVertexArray() );
        ( *p_vertices )[ 0 ] = osg::Vec3();
        ( *p_vertices )[ 1 ] = osg::Vec3();
        ( *p_vertices )[ 2 ] = osg::Vec3();
        ( *p_vertices )[ 3 ] = osg::Vec3();
        ( *p_vertices )[ 4 ] = osg::Vec3();
        ( *p_vertices )[ 5 ] = osg::Vec3();
        ( *p_vertices )[ 6 ] = osg::Vec3();
        ( *p_vertices )[ 7 ] = osg::Vec3();
        _p_linesGeom->setVertexArray( p_vertices );

        return;
    }

    // get the world transformation matrix
    osg::Matrix accumat;
    osg::MatrixList wm = p_entity->getTransformationNode()->getWorldMatrices();
    osg::MatrixList::iterator p_matbeg = wm.begin(), p_matend = wm.end();
    for ( ; p_matbeg != p_matend; ++p_matbeg )
        accumat = accumat * ( *p_matbeg );

    // the entity may be transformable but currently withough transformation node
    if ( !p_entity->getTransformationNode() )
    {
        osg::BoundingBox defaultbb( osg::Vec3f( -1.0f, -1.0f, -1.0f ), osg::Vec3f( 1.0f, 1.0f, 1.0f ) );

        // let the box disappear when no entity given
        osg::Vec3Array* p_vertices = static_cast< osg::Vec3Array* >( _p_linesGeom->getVertexArray() );
        ( *p_vertices )[ 0 ] = defaultbb.corner( 0 ) * accumat;
        ( *p_vertices )[ 1 ] = defaultbb.corner( 1 ) * accumat;
        ( *p_vertices )[ 2 ] = defaultbb.corner( 2 ) * accumat;
        ( *p_vertices )[ 3 ] = defaultbb.corner( 3 ) * accumat;
        ( *p_vertices )[ 4 ] = defaultbb.corner( 4 ) * accumat;
        ( *p_vertices )[ 5 ] = defaultbb.corner( 5 ) * accumat;
        ( *p_vertices )[ 6 ] = defaultbb.corner( 6 ) * accumat;
        ( *p_vertices )[ 7 ] = defaultbb.corner( 7 ) * accumat;
        _p_linesGeom->setVertexArray( p_vertices );

        // set the bbox color
        osg::Vec4Array* colors = new osg::Vec4Array;
        colors->push_back( osg::Vec4( 1.0f, 1.0f, 0.0f, 1.0f ) );
        _p_linesGeom->setColorArray( colors );
        _p_linesGeom->setColorBinding( osg::Geometry::BIND_OVERALL );
        return;
    }

    // set the bbox color
    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back( osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
    _p_linesGeom->setColorArray( colors );
    _p_linesGeom->setColorBinding( osg::Geometry::BIND_OVERALL );

    // get the bounds of selected entity
    osg::Node* p_transnode = p_entity->getTransformationNode();
    osg::ComputeBoundsVisitor cbv;
    cbv.apply( *p_transnode );
    osg::BoundingBox& bb = cbv.getBoundingBox();

    // update the bbox lines
    osg::Vec3Array* p_vertices = static_cast< osg::Vec3Array* >( _p_linesGeom->getVertexArray() );
    ( *p_vertices )[ 0 ] = bb.corner( 0 ) * accumat;
    ( *p_vertices )[ 1 ] = bb.corner( 1 ) * accumat;
    ( *p_vertices )[ 2 ] = bb.corner( 2 ) * accumat;
    ( *p_vertices )[ 3 ] = bb.corner( 3 ) * accumat;
    ( *p_vertices )[ 4 ] = bb.corner( 4 ) * accumat;
    ( *p_vertices )[ 5 ] = bb.corner( 5 ) * accumat;
    ( *p_vertices )[ 6 ] = bb.corner( 6 ) * accumat;
    ( *p_vertices )[ 7 ] = bb.corner( 7 ) * accumat;
    _p_linesGeom->setVertexArray( p_vertices );
}

yaf3d::BaseEntity* GameNavigator::pick( unsigned short xpos, unsigned short ypos, yaf3d::BaseEntity* p_entity )
{
    float x =  xpos;
    x = 2.0f * ( x * _iscreenWidth  ) - 1.0f;
    float y = ypos;
    y = 2.0f * ( y * _iscreenHeight ) - 1.0f;
    y = -y;

    // calculate start and end point of ray
    osgUtil::SceneView* p_sv = yaf3d::Application::get()->getSceneView();
    osg::Matrixd vum;
    vum.set(
            osg::Matrixd( p_sv->getViewMatrix() ) *
            osg::Matrixd( p_sv->getProjectionMatrix() )
            );
    osg::Matrixd inverseMVPW;
    inverseMVPW.invert( vum );
    osg::Vec3 start = osg::Vec3( x, y, -1.0f ) * inverseMVPW;
    osg::Vec3 end   = osg::Vec3( x, y,  1.0f ) * inverseMVPW;

    // update line segment for intersection test
    _p_lineSegment->set( start, end );

    // reset multi-click checking if the mouse pointer moved too far since last picking
    bool resetMultiClick = false;
    if ( ( fabs( _lastX - x ) > 0.2f ) || ( fabs( _lastY - y ) > 0.2f ) )
    {
        resetMultiClick = true;
    }
    _lastX = x;
    _lastY = y;

    // we are going to test the complete scenegraph
    osg::Group* p_grp = yaf3d::Application::get()->getSceneRootNode();
    osgUtil::IntersectVisitor iv;
    iv.addLineSegment( _p_lineSegment.get() );

    // do not pick the marker!
    if ( _marker.valid() )
        _marker->setNodeMask( 0 );

    // do the intesection test
    iv.apply( *p_grp );

    // do not pick the marker!
    if ( _marker.valid() )
        _marker->setNodeMask( 0xffffffff );

    std::vector< EditorSGData* > pickedentities;
    osgUtil::IntersectVisitor::HitList& hlist = iv.getHitList( _p_lineSegment.get() );
    osgUtil::IntersectVisitor::HitList::iterator p_beg = hlist.begin(), p_end = hlist.end();
    // collect all picked nodes
    for( ; p_beg != p_end; ++p_beg )
    {
        osg::NodePath& nodepath = p_beg->getNodePath();
        osg::NodePath::iterator p_ent = nodepath.begin(), p_entend = nodepath.end();
        for( ; p_ent != p_entend; ++p_ent )
        {
            osg::Node* p_node = *p_ent;
            EditorSGData* p_data = dynamic_cast< EditorSGData* >( p_node->getUserData() );
            // take only nodes with user data including editor's scenegraph entity type
            if ( !p_data )
                continue;

            // we want every entity only once in the list; note: the same entity may intersect several times with a ray
            std::vector< EditorSGData* >::const_iterator p_ebeg = pickedentities.begin(), p_eend = pickedentities.end();
            for ( ; p_ebeg != p_eend; ++p_ebeg )
            {
                // is there any preference for picked entities?
                if ( ( *p_ebeg )->getEntity() == p_entity )
                    return p_entity;

                if ( ( *p_ebeg )->getEntity() == p_data->getEntity() )
                    break;
            }

            // we have picked a new entity
            if ( p_ebeg == p_eend )
                pickedentities.push_back( p_data );
        }
    }

    // set the picking click count, it is used for selecting occluded drawables
    if ( !resetMultiClick )
        ++_pickClickCount;
    else
        _pickClickCount = 0;

    yaf3d::BaseEntity* p_selentity = NULL;
    size_t numpickedents = pickedentities.size();
    if ( numpickedents > 0 )
    {
        // if the mouse pointer moved too far from last position then we take the first entity
        if ( resetMultiClick )
        {
            p_selentity = pickedentities[ 0 ]->getEntity();
        }
        else // otherwise take the next entity
        {
            p_selentity = pickedentities[ _pickClickCount % numpickedents ]->getEntity();
        }
    }

    // is there a picking callback?
    if ( _p_cbNotify )
        _p_cbNotify->onEntityPicked( p_selentity );

    return p_selentity;
}

bool GameNavigator::hit( unsigned short xpos, unsigned short ypos )
{
    float x =  xpos;
    x = 2.0f * ( x * _iscreenWidth  ) - 1.0f;
    float y = ypos;
    y = 2.0f * ( y * _iscreenHeight ) - 1.0f;
    y = -y;

    // calculate start and end point of ray
    osgUtil::SceneView* p_sv = yaf3d::Application::get()->getSceneView();
    osg::Matrixd vum;
    vum.set(
            osg::Matrixd( p_sv->getViewMatrix() ) *
            osg::Matrixd( p_sv->getProjectionMatrix() )
            );
    osg::Matrixd inverseMVPW;
    inverseMVPW.invert( vum );
    osg::Vec3 start = osg::Vec3( x, y, -1.0f ) * inverseMVPW;
    osg::Vec3 end   = osg::Vec3( x, y,  1.0f ) * inverseMVPW;

    // update line segment for intersection test
    _p_lineSegment->set( start, end );

    // we are going to test the complete scenegraph
    osg::Group* p_grp = yaf3d::Application::get()->getSceneRootNode();
    osgUtil::IntersectVisitor iv;
    iv.addLineSegment( _p_lineSegment.get() );

    // do not pick the marker!
    if ( _marker.valid() )
        _marker->setNodeMask( 0 );

    // if an entity is placing then exclude it from intersection tests
    unsigned int nodemask = 0;
    osg::Node* p_transnode = NULL;
    if ( _p_selEntity && _p_selEntity->getTransformationNode() )
    {
        p_transnode = _p_selEntity->getTransformationNode();
        nodemask = p_transnode->getNodeMask();
        p_transnode->setNodeMask( 0 );
    }

    // do the intesection test
    iv.apply( *p_grp );

    if ( _marker.valid() )
        _marker->setNodeMask( 0xffffffff );

    // restore entity's nodemask
    if ( p_transnode )
        p_transnode->setNodeMask( nodemask );

    std::vector< EditorSGData* > pickedentities;
    osgUtil::IntersectVisitor::HitList& hlist = iv.getHitList( _p_lineSegment.get() );
    osgUtil::IntersectVisitor::HitList::iterator p_beg = hlist.begin(), p_end = hlist.end();

    bool       didhit = false;
    osg::Vec3f hitpos;
    osg::Vec3f hitnormal;
    float      mindist = 0xfffffff;

    // traverse all hit positions and select the nearest one
    for( ; p_beg != p_end; ++p_beg )
    {
        osg::Vec3f ip   = p_beg->getWorldIntersectPoint();
        osg::Vec3f in   = p_beg->getWorldIntersectNormal();
        osg::Vec3f hdist = _position - ip;

        // ignore back-facing polygons
        if ( ( hdist * in ) < 0.0f )
            continue;

        float currdist = hdist.length();
        if ( currdist < mindist )
        {
            hitpos    = ip;
            hitnormal = in;
            mindist   = currdist;
            didhit    = true;
        }
    }

    if ( didhit )
    {
        // alight the marker with hit normal
        osg::Quat rot;
        rot.makeRotate( osg::Vec3f( 0.0f, 0.0f, 1.0f ), hitnormal );
        _marker->setAttitude( rot );
        // set marker's position
        _marker->setPosition( hitpos );
        _hitPosition = hitpos;
        // enable rendering
        _marker->setNodeMask( 0xffffffff );
    }
    else
    {
        // do not render
        _marker->setNodeMask( 0 );
    }

    return didhit;
}
