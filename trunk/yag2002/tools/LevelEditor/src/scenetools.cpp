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
 # tools for picking, placing etc.
 #
 #   date of creation:  16/01/2009
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/


#include <vrc_main.h>
#include "scenetools.h"
#include "editorutils.h"

  #include "navigation.h"

#include <osg/ComputeBoundsVisitor>

//! Scene graph node data used by editor for picking axis handles
class AxisSGData: public osg::Referenced
{
    public:

                                                    AxisSGData( unsigned int axis ) :
                                                     _axis( axis )
                                                    {
                                                    }

         virtual                                    ~AxisSGData() {}

         unsigned int                               getAxis()
                                                    {
                                                        return _axis;
                                                    }

    protected:

        unsigned int                                _axis;
};


SceneTools::SceneTools() :
 _hitAxis( 0 ),
 _screenWidth( 0.0f ),
 _screenHeight( 0.0f ),
 _pickClickCount( 0 ),
 _lastX( 0.0f ),
 _lastY( 0.0f ),
 _markerScale( 1.0f ),
 _p_entityNoPick( NULL )
{
}

SceneTools::~SceneTools()
{
}

bool SceneTools::initialize( unsigned int screenwidth, unsigned int screenheight )
{
    // setup picking related stuff
    {
        _lineSegment = new osg::LineSegment;

        // store the window size, used for picking
        _screenWidth  = 1.0f / static_cast< float >( screenwidth );
        _screenHeight = 1.0f / static_cast< float >( screenheight );

        _bboxGeode = new osg::Geode;
        _bboxGeode->setName( "_editorBBoxCube_" );
        _linesGeom = new osg::Geometry;
        _linesGeom->setSupportsDisplayList( false );
        _linesGeom->setUseDisplayList( false );

        osg::StateSet* p_stateSet = new osg::StateSet;
        p_stateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
        _linesGeom->setStateSet( p_stateSet );

        osg::Vec3Array* vertices = new osg::Vec3Array( 8 );

        // pass the created vertex array to the points geometry object.
        _linesGeom->setVertexArray( vertices );

        // set the colors as before
        osg::Vec4Array* colors = new osg::Vec4Array;
        colors->push_back( osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
        _linesGeom->setColorArray( colors );
        _linesGeom->setColorBinding( osg::Geometry::BIND_OVERALL );

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
        _linesGeom->addPrimitiveSet( new osg::DrawElementsUShort( osg::PrimitiveSet::LINES, 24, indices ) );
        _bboxGeode->addDrawable( _linesGeom.get() );
    }

    // create hit marker
    {
        _hitMarker = new osg::PositionAttitudeTransform;

        osg::ref_ptr< osg::Geode > geodepart1 = new osg::Geode;
        osg::ref_ptr< osg::Geode > geodepart2 = new osg::Geode;

        osg::ref_ptr< osg::TessellationHints > hints = new osg::TessellationHints;
        hints->setDetailRatio( 0.5f );

        osg::ref_ptr< osg::ShapeDrawable > shape;

        shape = new osg::ShapeDrawable( new osg::Cone(osg::Vec3( 0.0f, 0.0f, 0.25f ), 0.2f, -0.3f ), hints.get() );
        shape->setColor( osg::Vec4( 0.0f, 0.0f, 1.0f, 1.0f ) );
        geodepart1->addDrawable( shape.get() );

        shape = new osg::ShapeDrawable( new osg::Cylinder( osg::Vec3( 0.f, 0.0f, 0.65f ), 0.15f, 0.5f ), hints.get() );
        shape->setColor(osg::Vec4( 1.0f, 1.0f, 0.5f, 1.0f ) );
        geodepart2->addDrawable( shape.get() );

        _hitMarker->addChild( geodepart1.get() );
        _hitMarker->addChild( geodepart2.get() );

        // setup the stateset
        osg::StateSet* p_stateset = _hitMarker->getOrCreateStateSet();
        p_stateset->setMode( GL_LIGHTING, osg::StateAttribute::ON );
        p_stateset->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
        p_stateset->setBinNumber( 100 );

        _hitMarker->setName( "_editor_hitmarker_" );
        _hitMarker->setScale( osg::Vec3f( 1.0f, 1.0f, 1.0f ) );
    }

    // create axis
    {
        _axisMarker = new osg::PositionAttitudeTransform;

        osg::ref_ptr< osg::Geode > geodepartX1 = new osg::Geode;
        osg::ref_ptr< osg::Geode > geodepartX2 = new osg::Geode;
        osg::ref_ptr< osg::Geode > geodepartY1 = new osg::Geode;
        osg::ref_ptr< osg::Geode > geodepartY2 = new osg::Geode;
        osg::ref_ptr< osg::Geode > geodepartZ1 = new osg::Geode;
        osg::ref_ptr< osg::Geode > geodepartZ2 = new osg::Geode;

        osg::ref_ptr< osg::TessellationHints > hints = new osg::TessellationHints;
        hints->setDetailRatio( 0.5f );

        osg::ref_ptr< osg::ShapeDrawable > shapecylinder, shapecone, shapecylinderclone, shapeconecopy;
        osg::ref_ptr< osg::PositionAttitudeTransform > node;
        osg::Quat rot;

        // create the shape prototypes
        shapecylinder = new osg::ShapeDrawable( new osg::Cylinder( osg::Vec3( 0.0f, 0.0f, 0.5f ), 0.05f, 1.0f ), hints.get() );
        shapecylinder->setColor(osg::Vec4( 1.0f, 1.0f, 0.5f, 1.0f ) );
        shapecone = new osg::ShapeDrawable( new osg::Cone( osg::Vec3( 0.0f, 0.0f, 1.2f ), 0.1f, 0.4f ), hints.get() );

        // create X-axis
        shapecylinderclone = new osg::ShapeDrawable( *shapecylinder.get() );
        geodepartX1->addDrawable( shapecylinderclone.get() );
        geodepartX1->setUserData( new AxisSGData( SceneTools::AxisX ) );
        shapeconecopy = new osg::ShapeDrawable( *shapecone.get() );
        shapeconecopy->setColor( osg::Vec4( 1.0f, 0.0f, 0.0f, 1.0f ) );
        geodepartX2->addDrawable( shapeconecopy.get() );
        geodepartX2->setUserData( new AxisSGData( SceneTools::AxisX ) );

        node = new osg::PositionAttitudeTransform;
        node->addChild( geodepartX1.get() );
        node->addChild( geodepartX2.get() );
        rot.makeRotate( osg::Vec3f( 0.0f, 0.0f, 1.0f ), osg::Vec3f( osg::PI / 2.0f, 0.0f, 0.0f ) );
        node->setPosition( osg::Vec3f( 0.2f, 0.0f, 0.0f ) );
        node->setAttitude( rot );
        _axisMarker->addChild( node.get() );

         // create Y-axis
        shapecylinderclone = new osg::ShapeDrawable( *shapecylinder.get() );
        geodepartY1->addDrawable( shapecylinderclone.get() );
        geodepartY1->setUserData( new AxisSGData( SceneTools::AxisY ) );
        shapeconecopy = new osg::ShapeDrawable( *shapecone.get() );
        shapeconecopy->setColor( osg::Vec4( 0.0f, 1.0f, 0.0f, 1.0f ) );
        geodepartY2->addDrawable( shapeconecopy.get() );
        geodepartY2->setUserData( new AxisSGData( SceneTools::AxisY ) );

        node = new osg::PositionAttitudeTransform;
        node->addChild( geodepartY1.get() );
        node->addChild( geodepartY2.get() );
        rot.makeRotate( osg::Vec3f( 0.0f, 0.0f, 1.0f ), osg::Vec3f( 0.0f, osg::PI / 2.0f, 0.0f ) );
        node->setPosition( osg::Vec3f( 0.0f, 0.2f, 0.0f ) );
        node->setAttitude( rot );
        _axisMarker->addChild( node.get() );

       // create Z-axis
        shapecylinderclone = new osg::ShapeDrawable( *shapecylinder.get() );
        geodepartZ1->addDrawable( shapecylinderclone.get() );
        geodepartZ1->setUserData( new AxisSGData( SceneTools::AxisZ ) );
        shapeconecopy = new osg::ShapeDrawable( *shapecone.get() );
        shapeconecopy->setColor( osg::Vec4( 0.0f, 0.0f, 1.0f, 1.0f ) );
        geodepartZ2->addDrawable( shapeconecopy.get() );
        geodepartZ2->setUserData( new AxisSGData( SceneTools::AxisZ ) );

        node = new osg::PositionAttitudeTransform;
        node->addChild( geodepartZ1.get() );
        node->addChild( geodepartZ2.get() );
        node->setPosition( osg::Vec3f( 0.0f, 0.0f, 0.2f ) );
        _axisMarker->addChild( node.get() );

        // setup the stateset
        osg::StateSet* p_stateset = _axisMarker->getOrCreateStateSet();
        p_stateset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
        p_stateset->setMode( GL_DEPTH_TEST, osg::StateAttribute::OFF );
        p_stateset->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
        p_stateset->setBinNumber( 100 );

        _axisMarker->setName( "_editor_axismarker_" );
        _axisMarker->setScale( osg::Vec3f( 1.0f, 1.0f, 1.0f ) );
    }

    // we begin with hit marker
    _marker = _hitMarker;

    return true;
}

void SceneTools::setSceneNode( osg::Group* p_node )
{
    // put bbox geode into right scene node
    if ( _sceneNode.valid() )
    {
        if ( _sceneNode->containsNode( _bboxGeode.get() ) )
            _sceneNode->removeChild( _bboxGeode.get() );
    }

    if ( p_node )
        p_node->addChild( _bboxGeode.get() );

    _sceneNode = p_node;
}

void SceneTools::showHitMarker( bool en )
{
    if ( !_hitMarker.valid() || !_sceneNode.valid() )
        return;

    if ( en )
    {
        if ( !_sceneNode->containsNode( _hitMarker.get() ) )
            _sceneNode->addChild( _hitMarker.get() );
    }
    else
    {
        if ( _sceneNode->containsNode( _hitMarker.get() ) )
            _sceneNode->removeChild( _hitMarker.get() );
    }

    _marker = _hitMarker;
}

void SceneTools::showAxisMarker( bool en )
{
    if ( !_axisMarker.valid() || !_sceneNode.valid() )
        return;

    if ( en )
    {
        if ( !_sceneNode->containsNode( _axisMarker.get() ) )
            _sceneNode->addChild( _axisMarker.get() );
    }
    else
    {
        if ( _sceneNode->containsNode( _axisMarker.get() ) )
            _sceneNode->removeChild( _axisMarker.get() );
    }

    _marker = _axisMarker;
}

void SceneTools::setMarkerPosition( const osg::Vec3f& position )
{
    if ( _marker.valid() )
        _marker->setPosition( position );
}

void SceneTools::setMarkerOrientation( const osg::Quat& orientation )
{
    if ( _marker.valid() )
        _marker->setAttitude( orientation );
}

void SceneTools::excludeFromPicking( yaf3d::BaseEntity* p_entity )
{
    _p_entityNoPick = p_entity;
}

void SceneTools::highlightEntity( yaf3d::BaseEntity* p_entity )
{
    if ( !p_entity || !p_entity->isTransformable() )
    {
        // let the box disappear when no entity given
        osg::Vec3Array* p_vertices = static_cast< osg::Vec3Array* >( _linesGeom->getVertexArray() );
        ( *p_vertices )[ 0 ] = osg::Vec3();
        ( *p_vertices )[ 1 ] = osg::Vec3();
        ( *p_vertices )[ 2 ] = osg::Vec3();
        ( *p_vertices )[ 3 ] = osg::Vec3();
        ( *p_vertices )[ 4 ] = osg::Vec3();
        ( *p_vertices )[ 5 ] = osg::Vec3();
        ( *p_vertices )[ 6 ] = osg::Vec3();
        ( *p_vertices )[ 7 ] = osg::Vec3();
        _linesGeom->setVertexArray( p_vertices );

        return;
    }

    // get the world transformation matrix
    osg::Matrix accumat;
    osg::MatrixList wm = p_entity->getTransformationNode()->getWorldMatrices( p_entity->getTransformationNode() );
    osg::MatrixList::iterator p_matbeg = wm.begin(), p_matend = wm.end();
    for ( ; p_matbeg != p_matend; ++p_matbeg )
        accumat = accumat * ( *p_matbeg );

    // the entity may be transformable but currently withough transformation node
    if ( !p_entity->getTransformationNode() )
    {
        osg::BoundingBox defaultbb( osg::Vec3f( -1.0f, -1.0f, -1.0f ), osg::Vec3f( 1.0f, 1.0f, 1.0f ) );

        // let the box disappear when no entity given
        osg::Vec3Array* p_vertices = static_cast< osg::Vec3Array* >( _linesGeom->getVertexArray() );
        ( *p_vertices )[ 0 ] = defaultbb.corner( 0 ) * accumat;
        ( *p_vertices )[ 1 ] = defaultbb.corner( 1 ) * accumat;
        ( *p_vertices )[ 2 ] = defaultbb.corner( 2 ) * accumat;
        ( *p_vertices )[ 3 ] = defaultbb.corner( 3 ) * accumat;
        ( *p_vertices )[ 4 ] = defaultbb.corner( 4 ) * accumat;
        ( *p_vertices )[ 5 ] = defaultbb.corner( 5 ) * accumat;
        ( *p_vertices )[ 6 ] = defaultbb.corner( 6 ) * accumat;
        ( *p_vertices )[ 7 ] = defaultbb.corner( 7 ) * accumat;
        _linesGeom->setVertexArray( p_vertices );

        // set the bbox color
        osg::Vec4Array* colors = new osg::Vec4Array;
        colors->push_back( osg::Vec4( 1.0f, 1.0f, 0.0f, 1.0f ) );
        _linesGeom->setColorArray( colors );
        _linesGeom->setColorBinding( osg::Geometry::BIND_OVERALL );
        return;
    }

    // set the bbox color
    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back( osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
    _linesGeom->setColorArray( colors );
    _linesGeom->setColorBinding( osg::Geometry::BIND_OVERALL );

    // get the bounds of selected entity
    osg::Node* p_transnode = p_entity->getTransformationNode();
    osg::ComputeBoundsVisitor cbv;
    cbv.apply( *p_transnode );
    osg::BoundingBox& bb = cbv.getBoundingBox();

    // update the bbox lines
    osg::Vec3Array* p_vertices = static_cast< osg::Vec3Array* >( _linesGeom->getVertexArray() );
    ( *p_vertices )[ 0 ] = bb.corner( 0 ) * accumat;
    ( *p_vertices )[ 1 ] = bb.corner( 1 ) * accumat;
    ( *p_vertices )[ 2 ] = bb.corner( 2 ) * accumat;
    ( *p_vertices )[ 3 ] = bb.corner( 3 ) * accumat;
    ( *p_vertices )[ 4 ] = bb.corner( 4 ) * accumat;
    ( *p_vertices )[ 5 ] = bb.corner( 5 ) * accumat;
    ( *p_vertices )[ 6 ] = bb.corner( 6 ) * accumat;
    ( *p_vertices )[ 7 ] = bb.corner( 7 ) * accumat;
    _linesGeom->setVertexArray( p_vertices );
}

yaf3d::BaseEntity* SceneTools::pickEntity( unsigned short xpos, unsigned short ypos, yaf3d::BaseEntity* p_entity )
{
    if ( !_sceneNode.valid() )
        return NULL;

    float x =  xpos;
    x = 2.0f * ( x * _screenWidth  ) - 1.0f;
    float y = ypos;
    y = 2.0f * ( y * _screenHeight ) - 1.0f;
    y = -y;

    // calculate start and end point of ray
    osgUtil::SceneView* p_sv = yaf3d::Application::get()->getSceneView();
    osg::Matrixd vum;
    vum.set( p_sv->getViewMatrix() * p_sv->getProjectionMatrix() );

    osg::Matrixd inverseMVPW;
    inverseMVPW.invert( vum );
    osg::Vec3 start = osg::Vec3( x, y, -1.0f ) * inverseMVPW;
    osg::Vec3 end   = osg::Vec3( x, y,  1.0f ) * inverseMVPW;

    // update line segment for intersection test
    _lineSegment->set( start, end );

    // reset multi-click checking if the mouse pointer moved too far since last picking
    bool resetMultiClick = false;
    if ( ( fabs( _lastX - x ) > 0.2f ) || ( fabs( _lastY - y ) > 0.2f ) )
    {
        resetMultiClick = true;
    }
    _lastX = x;
    _lastY = y;

    // we are going to test the complete scenegraph
    osgUtil::IntersectVisitor iv;
    iv.addLineSegment( _lineSegment.get() );

    // do not pick the marker!
    if ( _marker.valid() )
        _marker->setNodeMask( 0 );

    // do the intesection test
    iv.apply( *( _sceneNode.get() ) );

    // do not pick the marker!
    if ( _marker.valid() )
        _marker->setNodeMask( 0xffffffff );

    std::vector< EditorSGData* > pickedentities;
    osgUtil::IntersectVisitor::HitList& hlist = iv.getHitList( _lineSegment.get() );
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

    return p_selentity;
}

bool SceneTools::hitScene( unsigned short xpos, unsigned short ypos, const osg::Vec3f& viewerposition )
{
    if ( !_sceneNode.valid() )
        return false;

    float x =  xpos;
    x = 2.0f * ( x * _screenWidth  ) - 1.0f;
    float y = ypos;
    y = 2.0f * ( y * _screenHeight ) - 1.0f;
    y = -y;

    // reset the axis hits
    _hitAxis = 0;

    // calculate start and end point of ray
    osgUtil::SceneView* p_sv = yaf3d::Application::get()->getSceneView();
    osg::Matrixd vum;
    vum.set( osg::Matrixd( p_sv->getViewMatrix() ) * osg::Matrixd( p_sv->getProjectionMatrix() ) );

    osg::Matrixd inverseMVPW;
    inverseMVPW.invert( vum );
    osg::Vec3 start = osg::Vec3( x, y, -1.0f ) * inverseMVPW;
    osg::Vec3 end   = osg::Vec3( x, y,  1.0f ) * inverseMVPW;

    // update line segment for intersection test
    _lineSegment->set( start, end );

    // we are going to test the complete scenegraph
    osgUtil::IntersectVisitor iv;
    iv.addLineSegment( _lineSegment.get() );

    // do not check the hit marker itself for hits!
    if ( _marker.valid() && ( _marker.get() == _hitMarker.get() ) )
        _marker->setNodeMask( 0 );

    // if an entity given excluded from intersection tests
    unsigned int nodemask  = 0;
    osg::Node* p_transnode = NULL;
    if ( _p_entityNoPick && _p_entityNoPick->getTransformationNode() )
    {
        p_transnode = _p_entityNoPick->getTransformationNode();
        nodemask = p_transnode->getNodeMask();
        p_transnode->setNodeMask( 0 );
    }

    // do the intesection test
    iv.apply( *( _sceneNode.get() ) );

    if ( _marker.valid() )
        _marker->setNodeMask( 0xffffffff );

    // restore entity's nodemask
    if ( p_transnode )
        p_transnode->setNodeMask( nodemask );

    std::vector< EditorSGData* > pickedentities;
    osgUtil::IntersectVisitor::HitList& hlist = iv.getHitList( _lineSegment.get() );
    osgUtil::IntersectVisitor::HitList::iterator p_beg = hlist.begin(), p_end = hlist.end();

    bool        didhit = false;
    osg::Vec3f  hitpos;
    osg::Vec3f  hitnormal;
    std::string hitobject;
    float       mindist = 0xfffffff;

    // traverse all hit positions and select the nearest one
    for( ; p_beg != p_end; ++p_beg )
    {
        osg::Vec3f ip   = p_beg->getWorldIntersectPoint();
        osg::Vec3f in   = p_beg->getWorldIntersectNormal();
        osg::Vec3f hdist = viewerposition - ip;

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
            hitobject = extractNodeName( p_beg->getNodePath() );
        }

        // if axis marker is active then determine which axis was hit
        if ( _marker.get() == _axisMarker.get() )
        {
            AxisSGData* p_data = dynamic_cast< AxisSGData*>( p_beg->getGeode()->getUserData() );
            if ( p_data )
            {
                _hitAxis |= p_data->getAxis();
            }
        }
    }

    if ( didhit )
    {
        _hitPosition = hitpos;
        _hitNormal   = hitnormal;
        _hitObject   = hitobject;

        // do not rotate and position the axis marker
        if ( _marker.get() == _hitMarker.get() )
        {
            // align the marker with hit normal
            osg::Quat rot;
            rot.makeRotate( osg::Vec3f( 0.0f, 0.0f, 1.0f ), hitnormal );
            _marker->setAttitude( rot );
            // set marker's position
            _marker->setPosition( hitpos );
        }
        // enable rendering
        _marker->setNodeMask( 0xffffffff );
    }
    else
    {
        // do not render hit marker when nothing hit
        if ( _marker.get() == _hitMarker.get() )
            _marker->setNodeMask( 0 );
    }

    return didhit;
}

std::string SceneTools::extractNodeName( osg::NodePath& nodepath )
{
    std::string nodename;
    if ( nodepath.size() )
    {
        osg::MatrixTransform* p_mtNode = NULL;
        osg::NodePath::iterator p_nbeg = nodepath.end(), p_nend = nodepath.begin();
        p_nbeg--;
        for ( ; p_nbeg != p_nend; --p_nbeg )
        {
            osg::MatrixTransform* p_mt  = dynamic_cast< osg::MatrixTransform* >( *p_nbeg );
            osg::Group*           p_grp = dynamic_cast< osg::Group* >( *p_nbeg );

            if ( !nodename.length() )
            {
                if ( p_mt )
                {
                    nodename = p_mt->getName();
                    if ( !p_mtNode ) // we need only the first embedding transform node
                        p_mtNode = p_mt;
                }
                else if ( p_grp )
                {
                    nodename = p_grp->getName();
                }
            }
            else
                break;
        }
    }

    return nodename;
}

unsigned int SceneTools::getAxisHits() const
{
    return _hitAxis;
}

const osg::Vec3f& SceneTools::getHitPosition() const
{
    return _hitPosition;
}

const osg::Vec3f& SceneTools::getHitNormal() const
{
    return _hitNormal;
}

const std::string& SceneTools::getHitObject() const
{
    return _hitObject;
}

void SceneTools::updateMarkerScale( const osg::Vec3f& viewerposition )
{
    if ( !_marker.valid() )
        return;

    // we scale linear to viewer distance
    osg::Vec3f dist = _marker->getPosition() - viewerposition;
    float scale = dist.length();
    // avoid too small scaling when viewer is too close to axis
    if ( scale < 1.0f )
        scale = 1.0f;

    scale *= 0.15f; // factor of 0.15 means about 1/3 of screen space
    _marker->setScale( osg::Vec3f( scale, scale, scale ) );

    _markerScale = scale;
}

float SceneTools::getMarkerScale() const
{
    return _markerScale;
}

void SceneTools::setMarkerScale( float scale )
{
    _markerScale = scale;

    if ( _marker.valid() )
        _marker->setScale( osg::Vec3f( scale, scale, scale ) );
}

