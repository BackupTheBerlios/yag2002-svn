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
 # lod related classes
 #
 #   date of creation:  02/15/2006
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_lod.h"
#include <glod.h>


namespace vrc
{

GLuint GLODMesh::s_glodNumObjects = 0;
GLuint GLODMesh::s_lodGroup       = 1;
GLuint GLODMesh::s_lodObject      = 1;


LODSettings::LODSettings( GLenum mode ) :
_mode( ( mode == LOD_DISCRETE ) ? GLOD_DISCRETE : GLOD_CONTINUOUS ),
_adaptMode( GLOD_ERROR_THRESHOLD ),
_errorMode( GLOD_SCREEN_SPACE_ERROR ),
_errorThreshold( 0.05f ),
_maxTriangles( 1 )
{
}

LODSettings::LODSettings( const LODSettings& lodsettings ) :
_mode( lodsettings._mode ),
_adaptMode( lodsettings._adaptMode ),
_errorMode( lodsettings._errorMode ),
_errorThreshold( lodsettings._errorThreshold ),
_maxTriangles( lodsettings._maxTriangles )
{
}

LODSettings::~LODSettings() 
{
}

void LODSettings::setupScreenSpaceThreshold( GLfloat threshold )
{
    _adaptMode = GLOD_ERROR_THRESHOLD;
    _errorMode = GLOD_SCREEN_SPACE_ERROR;
    _errorThreshold = threshold;
}

void LODSettings::setupObjectSpaceThreshold( GLfloat threshold )
{
    _adaptMode = GLOD_ERROR_THRESHOLD;
    _errorMode = GLOD_OBJECT_SPACE_ERROR;
    _errorThreshold = threshold;
}
 
void LODSettings::setupScreenSpaceMaxTriangles( GLuint maxtris )
{
    _adaptMode = GLOD_TRIANGLE_BUDGET;
    _errorMode = GLOD_SCREEN_SPACE_ERROR;
    _maxTriangles = maxtris;
}

void LODSettings::setupObjectSpaceMaxTriangles( GLuint maxtris )
{
    _adaptMode = GLOD_TRIANGLE_BUDGET;
    _errorMode = GLOD_OBJECT_SPACE_ERROR;
    _maxTriangles = maxtris;
}

void LODSettings::apply( GLODMesh* p_lodmesh )
{
    const LODSettings* p_ls = p_lodmesh->getLODSettings();
    assert( p_ls && "invalid lod object" );

    GLuint objectname = p_lodmesh->getObjectName();

    //! TODO: should we replace the hardcoded params here by user-defined ones?
    glodObjectParameteri( objectname, GLOD_BUILD_OPERATOR, GLOD_OPERATOR_EDGE_COLLAPSE );
    glodObjectParameteri( objectname, GLOD_BUILD_ERROR_METRIC, GLOD_METRIC_SPHERES );
    glodObjectParameteri( objectname, GLOD_BUILD_BORDER_MODE, GLOD_BORDER_LOCK );
    glodObjectParameteri( objectname, GLOD_BUILD_QUEUE_MODE, GLOD_QUEUE_GREEDY );

    glodGroupParameteri( objectname, GLOD_ADAPT_MODE, p_ls->getAdaptationMode() );
    glodGroupParameteri( objectname, GLOD_ERROR_MODE, p_ls->getErrorMode() );

    if ( p_ls->getAdaptationMode() == GLOD_ERROR_THRESHOLD )
        glodGroupParameterf( objectname, p_ls->getErrorMode(), p_ls->getErrorThreshold() );
    else // GLOD_TRIANGLE_BUDGET
        glodGroupParameterf( objectname, p_ls->getErrorMode(), p_ls->getMaxTriangles() );
}

GLODMesh::GLODMesh( osg::Geometry* p_geom ) :
_lodObjectName( 0 ),
_lodGroupName( 0 ),
_lodPatchName( 0 ),
_adaptLOD( true ),
_p_geometry( p_geom )
{
    // setup GLOD
    if( !s_glodNumObjects )
    {
        if ( !glodInit() )
        {
            log_error << "GLODMesh: could not initialize GLOD" << std::endl;
            return;
        }
    }
    ++s_glodNumObjects;
}

GLODMesh::~GLODMesh()
{
    if ( _lodObjectName )
        glodDeleteObject( _lodObjectName );

    if ( _lodGroupName )
        glodDeleteGroup( _lodGroupName );

    --s_glodNumObjects;
    if ( s_glodNumObjects == 0 )
    {
        // shutdown GLOD
        glodShutdown();
        // reset group and object id counters
        s_lodGroup  = 1;
        s_lodObject = 1;
    }
}

GLuint GLODMesh::createGroupName()
{
    _lodGroupName  = s_lodGroup;
    glodNewGroup( _lodGroupName );
    ++s_lodGroup;

    return _lodGroupName;
}

GLuint GLODMesh::createObject( const LODSettings& lodsettings )
{
    // check if object is already created
    assert( ( _lodGroupName == 0 ) && "LOD object has already been created!" );

    // make a local copy of lod settings object
    _p_lodSettings = new LODSettings( lodsettings );

    // first create a group
    createGroupName();

    _lodObjectName = s_lodObject;
    ++s_lodObject;
    glodNewObject( _lodGroupName, _lodObjectName, _p_lodSettings->getMode() );

    // create one single vertex array out of exsiting triangles and triangle strips
    osg::Vec3Array*  p_vertarray  = new osg::Vec3Array();
    osg::IndexArray* p_indexarray = new osg::UShortArray();

    osg::IndexArray* p_indices  = _p_geometry->getVertexIndices();
    unsigned int     numPrims   = _p_geometry->getNumPrimitiveSets();
    {
        for ( unsigned int primcnt = 0; primcnt < numPrims; ++primcnt )
        {
            osg::PrimitiveSet* p_set = _p_geometry->getPrimitiveSet( primcnt );
            switch( p_set->getMode() )
            {

                case osg::PrimitiveSet::TRIANGLES: 
                    buildObjectTrianlge( p_set, p_indices );
                    break;

                default:
                {
                    log_warning << "unsupported primitive for building LOD, only TRIANGLES supported" << std::endl; 
                    continue;
                }
            }
        }
    }

    return _lodObjectName;
}

void GLODMesh::buildObjectTrianlge( const osg::PrimitiveSet* p_set, const osg::IndexArray* p_indices )
{
    GLvoid* p_indexdata = p_indices ? const_cast< GLvoid* >( p_indices->getDataPointer() ) : const_cast< GLvoid* >( p_set->getDataPointer() );
    if ( !p_indexdata )
    {
        log_warning << "cannot create LOD, no index array available" << std::endl;
        return;
    }

    //! FIXME: it seems that there is no simple way to determine which array data type is used in PrimitiveSet, currently we assume short
    GLenum  indextype   = p_indices ? p_indices->getType() : GL_SHORT;

    // apply the LOD settings to mesh
    _p_lodSettings->apply( this );

    unsigned int numindices = p_set->getNumIndices();

    // enable all arrays and feed them to GLOD
    glEnableClientState( GL_VERTEX_ARRAY );
    osg::Array* p_verts = _p_geometry->getVertexArray();
    glVertexPointer( p_verts->getDataSize(), p_verts[ 0 ].getDataType(), 0, p_verts->getDataPointer() );
  
    osg::Array* p_normals = _p_geometry->getNormalArray();
    if ( p_normals && p_normals->getNumElements() )
    {
        glEnableClientState( GL_NORMAL_ARRAY );
        glNormalPointer( p_normals->getDataType(), 0, p_normals->getDataPointer() );
    }

    //! TODO: GLOD cannot handle 4 component color arrays :-(
    //osg::Array* p_colors = _p_geometry->getColorArray();
    //if ( p_colors && p_colors->getNumElements() )
    //{
    //    glEnableClientState( GL_COLOR_ARRAY );
    //    glColorPointer( p_colors->getDataSize(), p_colors->getDataType(), 0, p_colors->getDataPointer() );
    //}

    osg::Array* p_texcoords0 = _p_geometry->getTexCoordArray( 0 );
    if ( p_texcoords0 && p_texcoords0->getNumElements() )
    {
        glEnableClientState( GL_TEXTURE_COORD_ARRAY );
        glTexCoordPointer( p_texcoords0->getDataSize(), p_texcoords0->getDataType(), 0, p_texcoords0->getDataPointer() );
    }

    glodInsertElements( _lodObjectName, _lodPatchName, GL_TRIANGLES, numindices, indextype, p_indexdata, 0, 0.0f );

    // remove arrays from geometry ( they are not longer needed now ), also disable gl arrays
    _p_geometry->setVertexArray( NULL );
    glDisableClientState( GL_VERTEX_ARRAY );
    if ( p_normals && p_normals->getNumElements() )
    {
        glDisableClientState( GL_NORMAL_ARRAY );
        _p_geometry->setNormalArray( NULL );
    }
    //if ( p_colors && p_colors->getNumElements() )
    //{
    //    glDisableClientState( GL_COLOR_ARRAY );
    //    _p_geometry->setColorArray( NULL );
    //}
    if ( p_texcoords0 && p_texcoords0->getNumElements() )
    {
        glDisableClientState( GL_TEXTURE_COORD_ARRAY );
        _p_geometry->setTexCoordArray( 0, NULL );
    }

    // build lod hierarchy
    glodBuildObject( _lodObjectName );

    if ( glodGetError() )
        log_warning << "error occured during LOD creation: " << std::endl; 

    ++_lodPatchName;
}

void LODDrawCallback::drawImplementation( osg::State& state, const osg::Drawable* p_drawable ) const
{
    assert( _p_lodMesh && "lod mesh data not available!" );

    const GLuint groupname  = _p_lodMesh->getGroupName();
    const GLuint objectname = _p_lodMesh->getObjectName();
    const GLuint numpatches = _p_lodMesh->getNumPatches();

    // need for LOD adaptation?
    if ( _p_lodMesh->adaptLOD() )
    {
        glodBindObjectXform( objectname, GL_PROJECTION_MATRIX | GL_MODELVIEW_MATRIX );
        glodAdaptGroup( groupname );
        // reset the adaptation flag after every adaptation
        // the adaptation is triggered by update callback
        _p_lodMesh->setAdaptLOD( false );
    }

    // apply drawable's state
    state.apply( p_drawable->getStateSet() );
    
    // draw the object patches
    for( GLuint patch = 0; patch < numpatches; ++patch )
        glodDrawPatch( objectname, patch );
}

void LODUpdateCallback::update( osg::NodeVisitor* p_nv, osg::Drawable* p_drawable )
{
    double currtime  = p_nv->getFrameStamp()->getReferenceTime();
    double deltatime = currtime - _lastRefTime;
    _lastRefTime     = currtime;
    _updateTimer += deltatime;
    if ( _updateTimer > _adaptationPeriod )
    {
        _updateTimer = 0.0;
        _p_lodMesh->setAdaptLOD( true );
    }
}

LODVisitor::LODVisitor( osg::NodeVisitor::TraversalMode tm ) :
 osg::NodeVisitor( tm )
{
    // we take all nodes
    setTraversalMask( 0xffffffff );
}

void LODVisitor::apply( osg::Geode& node )
{
    unsigned int numDrawables = node.getNumDrawables();
    for ( unsigned int cnt = 0; cnt < numDrawables; ++cnt )
    {
        osg::Drawable* p_drawable = node.getDrawable( cnt );
        osg::Geometry* p_geom     = p_drawable->asGeometry();
        // evaluate the geom and generate an appropriate collision geometry
        if ( p_geom )
        {
            p_drawable->setUseDisplayList( false );
            p_drawable->setSupportsDisplayList( false );

            osg::ref_ptr< GLODMesh > p_lodmesh = new GLODMesh( p_geom );
            p_lodmesh->createObject( *( _p_lodSettings.get() ) );
            // the actual lod mesh object is stored in geometry object, the following update and draw callbacks hold references to it
            p_geom->setUserData( p_lodmesh.get() );

            LODDrawCallback* p_cbdraw = new LODDrawCallback( p_lodmesh.get() );
            p_geom->setDrawCallback( p_cbdraw );

            //! TODO get the adaptation period from lod settings!
            LODUpdateCallback* p_cbupdate = new LODUpdateCallback( p_lodmesh.get(), 1.0 );
            p_geom->setUpdateCallback( p_cbupdate );
        }
    }
}

} // namespace vrc
