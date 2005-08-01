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
#include "ctd_inspector.h"
#include "../visuals/ctd_camera.h"
#include <osgUtil/IntersectVisitor>

namespace CTD
{
// prefix for gui elements
#define INSPECTOR_WND      "_inspector_"

// maximal length of picking ray
#define MAX_PICKING_DISTANCE 1000.0f

//! Input handler for observer
class InspectorIH : public GenericInputHandler< EnInspector >
{
    public:

        explicit                            InspectorIH( EnInspector* p_ent ) : 
                                             GenericInputHandler< EnInspector >( p_ent )
                                            {
                                                _lockMovement    = false;
                                                _moveRight       = false;
                                                _moveLeft        = false;
                                                _moveForward     = false;
                                                _moveBackward    = false;
                                                _rotationEnabled = false;
                                                _infoEnabled     = false;
                                                _pickClickCount  = 0;
                                                _lastX           = 0.0f;
                                                _lastY           = 0.0f;
                                                _pickingEnabled  = false;

                                                _pitch = 0.0f;
                                                _yaw   = 0.0f;

                                                // get the current screen size
                                                unsigned int width, height;
                                                Application::get()->getScreenSize( width, height );
                                                // calculate the middle of app window
                                                _screenMiddleX = static_cast< Uint16 >( width * 0.5f );
                                                _screenMiddleY = static_cast< Uint16 >( height * 0.5f );

                                                // store the window size, used for picking
                                                _iscreenWidth  = 1.0f / static_cast< float >( width );
                                                _iscreenHeight = 1.0f / static_cast< float >( height );

                                                // create a node for bbox visualization
                                                {
                                                    _bboxGeode = new osg::Geode;
                                                    _bboxGeode->setName( "_bboxCube_" );
                                                    _p_linesGeom = new osg::Geometry;
                                                    _p_linesGeom->setSupportsDisplayList( false ); 
                                                    _p_linesGeom->setUseDisplayList( false );

                                                    osg::StateSet* p_stateSet = new osg::StateSet;
                                                    p_stateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF ); 
                                                    _p_linesGeom->setStateSet( p_stateSet );

                                                    osg::Vec3Array* vertices = new osg::Vec3Array( 8 );

                                                    // pass the created vertex array to the points geometry object.
                                                    _p_linesGeom->setVertexArray( vertices );

                                                    // set the colors as before, plus using the aobve
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
                                                    Application::get()->getSceneRootNode()->addChild( _bboxGeode.get() );

                                                    // create a line segment for intersection tests
                                                    _p_lineSegment = new osg::LineSegment;

                                                    // intersection line for visualizing the line, is just for debugging and can be removed later
                                                    {
                                                        _p_linesIntersect = new osg::Geometry;
                                                        _p_linesIntersect->setSupportsDisplayList( false ); 
                                                        _p_linesIntersect->setUseDisplayList( false );

                                                        _p_linesIntersect->setStateSet( p_stateSet );

                                                        osg::Vec3Array* ivertices = new osg::Vec3Array( 2 );

                                                        // pass the created vertex array to the points geometry object.
                                                        _p_linesIntersect->setVertexArray( ivertices );

                                                        // set the colors as before, plus using the aobve
                                                        osg::Vec4Array* icolors = new osg::Vec4Array;
                                                        icolors->push_back( osg::Vec4( 1.0f, 1.0f, 0.0f, 1.0f ) );
                                                        icolors->push_back( osg::Vec4( 0.0f, 0.0f, 0.0f, 1.0f ) );
                                                        _p_linesIntersect->setColorArray( icolors );
                                                        _p_linesIntersect->setColorBinding( osg::Geometry::BIND_PER_VERTEX );

                                                        GLushort iindices[] = { 0, 1 };
                                                        _p_linesIntersect->addPrimitiveSet( new osg::DrawElementsUShort( osg::PrimitiveSet::LINES, 2, iindices ) );
                                                        _bboxGeode->addDrawable( _p_linesIntersect ); 
                                                    }
                                                }
                                            }

        virtual                             ~InspectorIH() {}

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

        // pick an drawable in level using ray-casting and return its nodepath. x, y is the normalized pointer position [-1...+1]
        typedef std::pair< osg::Drawable*, const osg::NodePath* >  PickResults;
        PickResults                         pick( float x, float y );

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
        float                               _iscreenWidth;
        float                               _iscreenHeight;

        bool                                _pickingEnabled;
        unsigned int                        _pickClickCount;
        float                               _lastX;
        float                               _lastY;
        osg::ref_ptr< osg::Geode >          _bboxGeode;
        osg::ref_ptr< osg::LineSegment >    _p_lineSegment;
        osg::Geometry*                      _p_linesGeom;
        osg::Geometry*                      _p_linesIntersect;
};

InspectorIH::PickResults InspectorIH::pick( float x, float y )
{
    // calculate start and end point of ray
    osgUtil::SceneView* p_sv = Application::get()->getSceneView();
    osg::Matrixd vum;
    vum.set( 
            osg::Matrixd( p_sv->getViewMatrix() ) *
            osg::Matrixd( p_sv->getProjectionMatrix() ) 
            );
    osg::Matrixd inverseMVPW;
    inverseMVPW.invert( vum );
    osg::Vec3 start = osg::Vec3( x, y, -1.0f ) * inverseMVPW;
    osg::Vec3 end   = osg::Vec3( x, y, 1.0f ) * inverseMVPW;
    // update line segment for intersection test
    _p_lineSegment->set( start, end );

    // reset mulit-click checking if the mouse pointer moved to far since last picking
    bool resetMultiClick = false;
    if ( ( fabs( _lastX - x ) > 0.2 ) || ( fabs( _lastY - y ) > 0.2 ) )
    {
        resetMultiClick = true;
    }
    _lastX = x;
    _lastY = y;

    // we are going to test the complete scenegraph
    osg::Group* p_grp = Application::get()->getSceneRootNode();
    osgUtil::IntersectVisitor iv;
    iv.addLineSegment( _p_lineSegment.get() );
    // do the intesection test
    iv.apply( *p_grp );
    
    // evaluate the results of intersection test
    osgUtil::IntersectVisitor::HitList&  hlist = iv.getHitList( _p_lineSegment.get() );
    osgUtil::IntersectVisitor::HitList::iterator p_beg = hlist.begin(), p_end = hlist.end();

    std::vector< std::pair< osg::Drawable*, osgUtil::Hit* > > pickeddrawables;
    osgUtil::Hit* p_pickedHit = NULL;

    // collect all picked drawables
    for( ; p_beg != p_end; p_beg++ )
    {
        // check for intersection with drawables
        if ( p_beg->_drawable.valid() )
        {
            // we want every geom only once in the list; note: geoms can intersect several times with a ray
            std::vector< std::pair< osg::Drawable*, osgUtil::Hit* > >::iterator p_dbeg = pickeddrawables.begin(), p_dend = pickeddrawables.end();
            for ( ; p_dbeg != p_dend; p_dbeg++ )
            {
                if ( p_dbeg->first == p_beg->_drawable.get() )
                    break;
            }
            if ( p_dbeg == p_dend )
                pickeddrawables.push_back( std::make_pair( p_beg->_drawable.get(), &*p_beg ) );          
        }
    }

    // set the picking click count, it is used for selecting occluded drawables
    if ( !resetMultiClick )
        _pickClickCount++;
    else
        _pickClickCount = 0;

    // select an drawable from a list of all ray-intersected drawables considering multi-clicks
    //  this allows to select also occluded drawabled by multi-clicking
    osg::Drawable* p_seldrawable = NULL;
    size_t         numdrawables = pickeddrawables.size();
    if ( numdrawables > 0 )
    {
        // if the mouse pointer moved too far from last position then we take the first drawable
        if ( resetMultiClick )
        {
            p_seldrawable = pickeddrawables[ 0 ].first;
            p_pickedHit   = pickeddrawables[ 0 ].second;
        }
        else // ... otherwise take the next drawable
        {
            p_seldrawable = pickeddrawables[ _pickClickCount % numdrawables ].first;
            p_pickedHit   = pickeddrawables[ _pickClickCount % numdrawables ].second;
        }
    }

    // if we found and intersection then update the bbox for visualization
    if ( p_seldrawable )
    {
        const osg::BoundingBox& bb = p_seldrawable->getBound();

        // update the bbox lines
        osg::Vec3Array* p_vertices = static_cast< osg::Vec3Array* >( _p_linesGeom->getVertexArray() );
        ( *p_vertices )[ 0 ] = bb.corner( 0 );
        ( *p_vertices )[ 1 ] = bb.corner( 1 );
        ( *p_vertices )[ 2 ] = bb.corner( 2 );
        ( *p_vertices )[ 3 ] = bb.corner( 3 );
        ( *p_vertices )[ 4 ] = bb.corner( 4 );
        ( *p_vertices )[ 5 ] = bb.corner( 5 );
        ( *p_vertices )[ 6 ] = bb.corner( 6 );
        ( *p_vertices )[ 7 ] = bb.corner( 7 );
        _p_linesGeom->setVertexArray( p_vertices );
    }
    else
    {
        // let the box disappear when no intersection detected
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
    }

    // update the intesection line visualization
    osg::Vec3Array* p_ivertices = static_cast< osg::Vec3Array* >( _p_linesIntersect->getVertexArray() );
    ( *p_ivertices )[ 0 ] = start;
    ( *p_ivertices )[ 1 ] = end;
    _p_linesIntersect->setVertexArray( p_ivertices );

    // return the node path of picked drawable
    return p_pickedHit ? std::make_pair( p_seldrawable, &p_pickedHit->getNodePath() ) : std::make_pair( static_cast< osg::Drawable* >( NULL ), static_cast< osg::NodePath* >( NULL ) );
}

bool InspectorIH::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
    const osgSDL::SDLEventAdapter* p_eventAdapter = dynamic_cast< const osgSDL::SDLEventAdapter* >( &ea );
    assert( p_eventAdapter && "invalid event adapter received" );

    unsigned int     eventType    = p_eventAdapter->getEventType();
    int              key          = p_eventAdapter->getSDLKey();
    const SDL_Event& sdlevent     = p_eventAdapter->getSDLEvent();

    if ( eventType == osgGA::GUIEventAdapter::FRAME )
        return false;

    // terminate application on Escape
    if ( key == SDLK_ESCAPE )
        Application::get()->stop();

    // toggle info dialog rendering
    if ( ( key == SDLK_SPACE ) && ( eventType == osgGA::GUIEventAdapter::KEYDOWN ) )
    {
        _infoEnabled = !_infoEnabled;
        getUserObject()->enableInfoWindow( _infoEnabled );
    }

    // enable / disable picking mode
    if ( ( key == SDLK_RCTRL ) || ( key == SDLK_LCTRL ) )
    {
        if ( eventType == osgGA::GUIEventAdapter::KEYDOWN )
            _pickingEnabled = true;
        else if ( eventType == osgGA::GUIEventAdapter::KEYUP )
            _pickingEnabled = false;
    }


    // pick an object
    //! TODO: implement evaluation of results of picking
    if ( _pickingEnabled )
    {
        if ( sdlevent.button.button == SDL_BUTTON_LEFT && eventType == osgGA::GUIEventAdapter::PUSH )
        {
            float x = sdlevent.motion.x, y = sdlevent.motion.y;
            x = 2.0f * ( x * _iscreenWidth ) - 1.0f; 
            y = 2.0f * ( y * _iscreenHeight ) - 1.0f;
            InspectorIH::PickResults res = pick( x, -y );

            const osg::NodePath* p_nodepath = res.second;
            osg::Drawable*       p_drawable = res.first;
            if ( p_drawable )
            {
                osg::Node* p_node = p_drawable->getParent( 0 );
                while( p_node && ( p_node->getName() == "" ) )
                    p_node = p_node->getParent( 0 );

                if ( p_node )
                {
                    std::ostringstream str;
                    str << "name      " << p_node->getName() << std::endl;
                    const osg::Vec3& pos = p_node->getBound().center();
                    str << "position  " << pos.x() << " " << pos.y() << " " << pos.z() << std::endl;
 
                    CEGUI::String text( str.str() );
                    getUserObject()->setPickerOutputText( text );
                }
            }
            else
            {
                //! TODO: here we could evalute the path node
                if ( p_nodepath )
                {
                    size_t numnodes = p_nodepath->size();
                    if ( numnodes > 0 )
                    {
                        CEGUI::String text( ( *p_nodepath )[ 0 ]->getName() );
                        getUserObject()->setPickerOutputText( text );
                    }
                }
            }
        }
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
        Application::get()->getViewer()->requestWarpPointer( _screenMiddleX, _screenMiddleY );
    }

    // update camera position
    if ( _moveForward || _moveBackward || _moveLeft || _moveRight )
    {
        pos = p_camera->getLocalRotation() * pos;
        p_camera->setCameraPosition( pos + p_camera->getCameraPosition() );
    }

    return false;
}

//! Implement and register the inspector entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( InspectorEntityFactory );

EnInspector::EnInspector() :
_isPersistent( false ),
_needUpdate ( false ),
_p_cameraEntity( NULL ),
_maxSpeed( 10.0f ),
_speed( 10.0f ),
_p_outputTextMain( NULL ),
_p_speedBarMain( NULL ),
_p_lockCheckboxMain( NULL ),
_p_wndMain( NULL ),
_fpsTimer( 0.0f ),
_fpsCounter( 0 ),
_fps( 0 )
{
    // register entity attributes
    getAttributeManager().addAttribute( "position", _position );
    getAttributeManager().addAttribute( "rotation", _rotation );
    getAttributeManager().addAttribute( "maxSpeed", _maxSpeed );
}

EnInspector::~EnInspector()
{
    try
    {
        CEGUI::WindowManager::getSingleton().destroyWindow( _p_wndMain );
    }
    catch ( const CEGUI::Exception& e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "EnInspector: problem cleaning up entity." << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

void EnInspector::handleNotification( const EntityNotification& notification )
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

void EnInspector::initialize()
{
    try
    {        
        // setup main gui
        {
            _p_wndMain = static_cast< CEGUI::FrameWindow* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/FrameWindow", INSPECTOR_WND "mainFrame" ) );
            _p_wndMain->subscribeEvent( CEGUI::FrameWindow::EventCloseClicked, CEGUI::Event::Subscriber( &CTD::EnInspector::onClickedCloseMain, this ) );
            _p_wndMain->setSize( CEGUI::Size( 0.35f, 0.25f ) );
            _p_wndMain->setText( "tools" );
            _p_wndMain->setPosition( CEGUI::Point( 0, 0 ) );
            _p_wndMain->setAlpha( 0.7f );
            _p_wndMain->setAlwaysOnTop( true );
            _p_wndMain->setSizingEnabled( false );

            _p_lockCheckboxMain = static_cast< CEGUI::Checkbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Checkbox", INSPECTOR_WND "lockMain" ) );
            _p_lockCheckboxMain->subscribeEvent( CEGUI::Checkbox::EventCheckStateChanged, CEGUI::Event::Subscriber( &CTD::EnInspector::onLockChanged, this ) );
            _p_lockCheckboxMain->setSize( CEGUI::Size( 0.9f, 0.22f ) );
            _p_lockCheckboxMain->setPosition( CEGUI::Point( 0.05f, 0.1f ) );
            _p_lockCheckboxMain->setSelected( false );
            _p_lockCheckboxMain->setText( "lock movement" );
            _p_wndMain->addChildWindow( _p_lockCheckboxMain );

            CEGUI::StaticText* p_stext = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", INSPECTOR_WND "st01" ) );
            p_stext->setSize( CEGUI::Size( 0.15f, 0.08f ) );
            p_stext->setPosition( CEGUI::Point( 0.05f, 0.3f ) );
            p_stext->setText( "speed" );
            p_stext->setFrameEnabled( false );
            p_stext->setBackgroundEnabled( false );
            _p_wndMain->addChildWindow( p_stext );

            _p_speedBarMain = static_cast< CEGUI::Scrollbar* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/HorizontalScrollbar", INSPECTOR_WND "speedMain" ) );
            _p_speedBarMain->subscribeEvent( CEGUI::Scrollbar::EventScrollPositionChanged, CEGUI::Event::Subscriber( &CTD::EnInspector::onSpeedChanged, this ) );
            _p_speedBarMain->setSize( CEGUI::Size( 0.75f, 0.075f ) );
            _p_speedBarMain->setPosition( CEGUI::Point( 0.2f, 0.3f ) );
            _p_speedBarMain->setScrollPosition( 1.0f );
            _p_wndMain->addChildWindow( _p_speedBarMain );

            _p_outputTextMain = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", INSPECTOR_WND "outputMain" ) );
            _p_outputTextMain->setSize( CEGUI::Size( 0.9f, 0.4f ) );
            _p_outputTextMain->setPosition( CEGUI::Point( 0.05f, 0.45f ) );
            _p_outputTextMain->setFont( CTD_GUI_CONSOLE );
            _p_wndMain->addChildWindow( _p_outputTextMain );

            GuiManager::get()->getRootWindow()->addChildWindow( _p_wndMain );
        }
        // setup picker gui
        {
            _p_wndPicker = static_cast< CEGUI::FrameWindow* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/FrameWindow", INSPECTOR_WND "pickerFrame" ) );
            _p_wndPicker->subscribeEvent( CEGUI::FrameWindow::EventCloseClicked, CEGUI::Event::Subscriber( &CTD::EnInspector::onClickedClosePicker, this ) );
            _p_wndPicker->setSize( CEGUI::Size( 0.35f, 0.15f ) );
            _p_wndPicker->setText( "picker" );
            _p_wndPicker->setPosition( CEGUI::Point( 0.65f, 0.0f ) );
            _p_wndPicker->setAlpha( 0.7f );
            _p_wndPicker->setAlwaysOnTop( true );
            _p_wndPicker->setSizingEnabled( false );

            _p_outputTextPicker = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", INSPECTOR_WND "outputPicker" ) );
            _p_outputTextPicker->setSize( CEGUI::Size( 0.9f, 0.5f ) );
            _p_outputTextPicker->setPosition( CEGUI::Point( 0.05f, 0.26f ) );
            _p_outputTextPicker->setFont( CTD_GUI_CONSOLE );
            _p_wndPicker->addChildWindow( _p_outputTextPicker );

            GuiManager::get()->getRootWindow()->addChildWindow( _p_wndPicker );
            _p_wndPicker->hide();
        }
    }
    catch ( const CEGUI::Exception& e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "EnPlayerInfoDisplay: problem creating gui" << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
    }


    EntityManager::get()->registerUpdate( this, true );         // register entity in order to get updated per simulation step
    EntityManager::get()->registerNotification( this, true );   // register entity in order to get notifications (e.g. from menu entity)
}

void EnInspector::postInitialize()
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

    // set any camera attributes before initializing it
    _p_cameraEntity->getAttributeManager().setAttributeValue( "backgroundColor", osg::Vec3f( 0, 0, 0 ) );
    // setup camera entity
    _p_cameraEntity->initialize();
    _p_cameraEntity->postInitialize();
    _p_cameraEntity->setEnable( true );
    _inputHandler = new InspectorIH( this );

    // set initial pitch yaw in input handler
    _inputHandler->setCameraPosition( _position );
    _inputHandler->setCameraPitchYaw( osg::DegreesToRadians( _rotation.y() ), osg::DegreesToRadians( _rotation.z() ) );

    // the default is info window is disabled ( press F1 to activate it )
    enableInfoWindow( false );
}

// main gui callbacks
bool EnInspector::onClickedCloseMain( const CEGUI::EventArgs& arg )
{
    // hide the info window via input handler, so it has the change to update its internal state
    _inputHandler->enableInfoWindow( false );
    return true;
}

bool EnInspector::onSpeedChanged( const CEGUI::EventArgs& arg )
{
    _speed = _p_speedBarMain->getScrollPosition();
    _speed *= _maxSpeed;
    return true;
}

bool EnInspector::onLockChanged( const CEGUI::EventArgs& arg )
{
    _inputHandler->lockMovement( _p_lockCheckboxMain->isSelected() );
    return true;
}

// picker gui callbacks
bool EnInspector::onClickedClosePicker( const CEGUI::EventArgs& arg )
{
    _p_wndPicker->hide();
    return true;
}

void EnInspector::setPickerOutputText( const CEGUI::String& text )
{
    _p_outputTextPicker->setText( text );
    _p_wndPicker->show();
}

void EnInspector::updateEntity( float deltaTime )
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
        _p_outputTextMain->setText( info );
    }
}
       
void EnInspector::enableInfoWindow( bool en )
{
    _infoWindowEnable = en;
    if ( en )
    {
        _p_wndMain->show();
    }
    else
    {
        _p_wndMain->hide();
    }
}

} // namespace CTD
