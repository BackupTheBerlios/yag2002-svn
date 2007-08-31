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
 # console command 'writeobjfile', use this to write out the current
 #  scene into a given obj file ( Alias Wavefront 3D file format ).
 #
 #   date of creation:  08/19/2007
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_basecmd.h"
#include "vrc_cmdwriteobjfile.h"
#include "vrc_cmdregistry.h"
#include "vrc_console.h"
#include <osgDB/WriteFile>

namespace vrc
{

//! Implement and register the command
VRC_IMPL_CONSOLE_CMD( CmdWriteOBJFile )

//! Visitor for traversing all Geode objects in scenegraph and writing the OBJ file
class GeomVisitor : public osg::NodeVisitor
{
    public:

        explicit                        GeomVisitor( osg::NodeVisitor::TraversalMode tm = osg::NodeVisitor::TRAVERSE_ALL_CHILDREN );

                                        ~GeomVisitor();

        //! Open the output file
        bool                            openOutput( const std::string& filename );

        //! Start exporing
        void                            apply( osg::Geode& node );

        //! Statistics methods, use them after traversal
        unsigned int                    getNumPrimitives() const { return _numPrimitives; }

        //! Statistics methods, use them after traversal
        unsigned int                    getNumVertices()  const { return _numVertices; }

        //! Statistics methods, use them after traversal
        unsigned int                    getNumTrianlges()  const { return _numTriangles; }

        //! Return the string containing the export results
        const std::string&              getResults() const { return _results; }

    protected:

        void                            exportTrianlges( const std::string& name, const osg::PrimitiveSet* p_set, osg::Array* p_verts, osg::Array* p_tcoords, osg::Array* p_normals, const osg::Matrixf& mat, const osg::IndexArray* p_indices = NULL );

        void                            exportTrianlgeStrip( const std::string& name, const osg::PrimitiveSet* p_set, osg::Array* p_verts, osg::Array* p_tcoords, osg::Array* p_normals, const osg::Matrixf& mat, const osg::IndexArray* p_indices = NULL );

        //! Output file
        std::ofstream*                  _p_output;

        //! Total count of exported primitives
        unsigned int                    _numPrimitives;

        //! Total count of exported primitives
        unsigned int                    _numVertices;

        //! Total count of exported triangles
        unsigned int                    _numTriangles;

        //! Results of export
        std::string                     _results;
};


CmdWriteOBJFile::CmdWriteOBJFile() :
 BaseConsoleCommand( CMD_NAME_WRITEOBJFILE )
{
    setUsage( CMD_USAGE_WRITEOBJFILE );
}

CmdWriteOBJFile::~CmdWriteOBJFile()
{
}

const std::string& CmdWriteOBJFile::execute( const std::vector< std::string >& arguments )
{
    bool hasinputfile = false;
    if ( arguments.size() < 1 )
    {
        _cmdResult = getUsage();
        return _cmdResult;
    }
    else if ( arguments.size() > 2 )
    {
        if ( arguments[ 0 ] == "-i" )
        {
            hasinputfile = true;
        }
        else
        {
            _cmdResult = getUsage();
            return _cmdResult;
        }
    }

    EnConsole* p_console = static_cast< EnConsole* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_CONSOLE ) );
    assert( p_console && "CmdWriteOBJFile::execute: console entity could not be found!" );
    std::string cwd = yaf3d::Application::get()->getMediaPath() + p_console->getCWD() + "/";
    
    std::string outfilename;
    if ( hasinputfile )
        outfilename = cwd + arguments[ 2 ];
    else
        outfilename = cwd + arguments[ 0 ];

    GeomVisitor geomvisitor;
    if ( !geomvisitor.openOutput( outfilename ) )
    {
        _cmdResult += "could not create output file!\n";
        return _cmdResult;
    }

    if ( !hasinputfile )
    {
        osg::Group* p_grp = yaf3d::Application::get()->getSceneRootNode();
        if ( p_grp )
        {
            _cmdResult = "writing scene to osg file '" + outfilename + "\n";
            p_grp->accept( geomvisitor );
            _cmdResult += geomvisitor.getResults() + "\n";

            std::stringstream stats;
            stats << "exported " << geomvisitor.getNumPrimitives() << " primitives" << std::endl;
            stats << "exported " << geomvisitor.getNumVertices() << " vertices" << std::endl;
            stats << "exported " << geomvisitor.getNumTrianlges() << " triangles" << std::endl;
            _cmdResult += stats.str();
        }
    }
    else
    {
        _cmdResult = "reading scene file " + arguments[ 1 ] + " ...\n";
        std::string infilename = cwd + arguments[ 1 ];
        osg::ref_ptr< osg::Node > p_node = osgDB::readNodeFile( infilename );
        if ( p_node.get() )
        {            
            _cmdResult = "writing scene to osg file '" + outfilename + "\n";
            p_node->accept( geomvisitor );
            _cmdResult += geomvisitor.getResults() + "\n";

            std::stringstream stats;
            stats << "exported " << geomvisitor.getNumPrimitives() << " primitives" << std::endl;
            stats << "exported " << geomvisitor.getNumVertices() << " vertices" << std::endl;
            stats << "exported " << geomvisitor.getNumTrianlges() << " triangles" << std::endl;
            _cmdResult += stats.str();
        }
        else
        {
            _cmdResult += "cannot read scene file " + infilename + "\n";
        }
    }

    return _cmdResult;
}

//! Implementation of the geometry visitor
GeomVisitor::GeomVisitor( osg::NodeVisitor::TraversalMode tm ) :
  osg::NodeVisitor( tm )                                            
{
    // we take all nodes
    setTraversalMask( 0xffffffff );
    _numPrimitives  = 0;
    _numVertices    = 0;
    _numTriangles   = 0;
}

GeomVisitor::~GeomVisitor()
{
    if ( _p_output )
        delete _p_output;
}

bool GeomVisitor::openOutput( const std::string& filename )
{
    _p_output = new std::ofstream;
    _p_output->open( filename.c_str() );

    if ( !_p_output )
    {
        delete _p_output;
        return false;
    }

    return true;
}

void GeomVisitor::apply( osg::Geode& node )
{
    assert( _p_output && "no valid output!" );

    // get the accumulated world matrix for this node
    osg::Matrixf  mat = computeLocalToWorld( getNodePath() );
    unsigned int numDrawables = node.getNumDrawables();
    for ( unsigned int cnt = 0; cnt < numDrawables; ++cnt )
    {
        osg::Drawable* p_drawable = node.getDrawable( cnt );
        osg::Geometry* p_geom     = p_drawable->asGeometry();

        // evaluate the geom and generate an appropriate collision geometry
        if ( p_geom )
        {
            // get the node name
            std::string nodename = node.getName();
            if ( !nodename.length() && node.getNumParents() )
            {
                for ( osg::Group* p_parent = node.getParent( 0 ); p_parent; )
                {
                    if ( p_parent->getName().length() )
                    {
                        nodename =  p_parent->getName();
                        break;
                    }
                    if ( !p_parent->getNumParents() )
                        break;

                    p_parent = p_parent->getParent( 0 );
                }
            }

            osg::Array*      p_verts   = p_geom->getVertexArray();
            osg::Array*      p_tcoords = p_geom->getTexCoordArray( 0 );
            osg::Array*      p_normals = p_geom->getNormalArray();
            osg::IndexArray* p_indices = p_geom->getVertexIndices();
            unsigned int     numPrims  = p_geom->getNumPrimitiveSets();
            {
                for ( unsigned int primcnt = 0; primcnt < numPrims; ++primcnt )
                {
                    osg::PrimitiveSet* p_set = p_geom->getPrimitiveSet( primcnt );
                    switch( p_set->getMode() )
                    {
                        case osg::PrimitiveSet::POINTS:
                        case osg::PrimitiveSet::LINES:
                        case osg::PrimitiveSet::LINE_STRIP:
                        case osg::PrimitiveSet::LINE_LOOP:
                            return;

                        case osg::PrimitiveSet::TRIANGLES:
                            exportTrianlges( nodename, p_set, p_verts, p_tcoords, p_normals, mat, p_indices );
                            break;

                        case osg::PrimitiveSet::TRIANGLE_STRIP:
                            exportTrianlgeStrip( nodename, p_set, p_verts, p_tcoords, p_normals, mat, p_indices );
                            break;

                        case osg::PrimitiveSet::TRIANGLE_FAN:
                        case osg::PrimitiveSet::QUADS:
                        case osg::PrimitiveSet::QUAD_STRIP:
                        case osg::PrimitiveSet::POLYGON:

                        default:
                            _results += "skipping geom type as not supported!\n";
                    }
                }
            }
        }
    }
}

void GeomVisitor::exportTrianlges( const std::string& name, const osg::PrimitiveSet* p_set, osg::Array* p_verts, osg::Array* p_tcoords, osg::Array* p_normals, const osg::Matrixf& mat, const osg::IndexArray* p_indices )
{
    if( p_verts->getType() != osg::Array::Vec3ArrayType )
    {
        _results += "cannot export geom, wrong vertex array type!\n";
        return;
    }

    // write the OBJ group tag
    *_p_output << std::endl;
    *_p_output << "g" << std::endl;

    osg::Vec3Array*   p_vertVecs  = dynamic_cast< osg::Vec3Array* >( p_verts );
    osg::Vec2Array*   p_texCoords = dynamic_cast< osg::Vec2Array* >( p_tcoords );
    osg::Vec3Array*   p_norms     = dynamic_cast< osg::Vec3Array* >( p_normals );
    unsigned int      numVerts    = p_vertVecs->size();
    unsigned int      numIndices  = p_set->getNumIndices();
    unsigned int      numTexCoords = p_texCoords ? p_texCoords->size() : 0;
    unsigned int      numNormals   = p_norms ? p_norms->size() : 0;
    osg::Vec3f        triVert;

    // write out the vertices
    for( unsigned int vindex = 0; vindex < numVerts; vindex++ )
    {         
        triVert = ( *p_vertVecs )[ vindex ] * mat;
        *_p_output << "v " << triVert.x() << " " << triVert.y() << " " << triVert.z() << std::endl;
    }

    // output the tex coords
    if ( p_texCoords )
    {
        *_p_output << std::endl;
        *_p_output << "g" << std::endl;

        osg::Vec2f coord;
        for( unsigned int cnt = 0; cnt < numTexCoords; cnt++ )
        {
            coord = ( *p_texCoords )[ cnt ];
            *_p_output << "vt " << coord.x() << " " << coord.y() << std::endl;
        }
    }

    // output the normals
    if ( p_norms )
    {
        *_p_output << std::endl;
        *_p_output << "g" << std::endl;

        osg::Vec3f normal;
        for( unsigned int cnt = 0; cnt < numNormals; cnt++ )
        {
            normal = ( *p_norms )[ cnt ];
            *_p_output << "vn " << normal.x() << " " << normal.y() << " " << normal.z() << std::endl;
        }
    }

    // export faces
    *_p_output << std::endl;
    *_p_output << "g " << name << _numPrimitives << std::endl;

    unsigned int index1, index2, index3;
    for( unsigned int vindex = 0; vindex < numIndices; vindex += 3 )
    {         
        index1 = _numVertices + ( p_indices ? p_indices->index( vindex + 0 ) : p_set->index( vindex + 0 ) );
        index2 = _numVertices + ( p_indices ? p_indices->index( vindex + 1 ) : p_set->index( vindex + 1 ) );
        index3 = _numVertices + ( p_indices ? p_indices->index( vindex + 2 ) : p_set->index( vindex + 2 ) );

        *_p_output << "f ";
        if ( p_texCoords && p_norms )
        {
            *_p_output << index1 + 1 << "/" << index1 + 1 << "/" << index1 + 1 << " ";
            *_p_output << index2 + 1 << "/" << index2 + 1 << "/" << index2 + 1 << " ";
            *_p_output << index3 + 1 << "/" << index3 + 1 << "/" << index3 + 1;
        }
        else if ( p_texCoords || p_norms )
        {
            *_p_output << index1 + 1 << "/" << index1 + 1 << " ";
            *_p_output << index2 + 1 << "/" << index2 + 1 << " ";
            *_p_output << index3 + 1 << "/" << index3 + 1;
        }
        else
        {
            *_p_output << index1 + 1 << " ";
            *_p_output << index2 + 1 << " ";
            *_p_output << index3 + 1;
        }
        *_p_output << std::endl;

        ++_numTriangles;
    }

    ++_numPrimitives;
    _numVertices += numVerts;
}

void GeomVisitor::exportTrianlgeStrip( const std::string& name, const osg::PrimitiveSet* p_set, osg::Array* p_verts, osg::Array* p_tcoords, osg::Array* p_normals, const osg::Matrixf& mat, const osg::IndexArray* p_indices )
{
    if( p_verts->getType() != osg::Array::Vec3ArrayType )
    {
        _results += "cannot export geom, wrong vertex array type!\n";
        return;
    }

    // write the OBJ group tag
    *_p_output << std::endl;
    *_p_output << "g" << std::endl;

    osg::Vec3Array*   p_vertVecs  = dynamic_cast< osg::Vec3Array* >( p_verts );
    osg::Vec2Array*   p_texCoords = dynamic_cast< osg::Vec2Array* >( p_tcoords );
    osg::Vec3Array*   p_norms     = dynamic_cast< osg::Vec3Array* >( p_normals );
    unsigned int      numVerts     = p_vertVecs->size();
    unsigned int      numIndices   = p_set->getNumIndices();
    unsigned int      numTexCoords = p_texCoords ? p_texCoords->size() : 0;
    unsigned int      numNormals   = p_norms ? p_norms->size() : 0;
    osg::Vec3f        triVert;

    // output the vertices
    for( unsigned int vindex = 0; vindex < numVerts; vindex++ )
    {
        triVert = ( *p_vertVecs )[ vindex ] * mat;
        *_p_output << "v " << triVert.x() << " " << triVert.y() << " " << triVert.z() << std::endl;
    }

    // output the tex coords
    if ( p_texCoords )
    {
        *_p_output << std::endl;
        *_p_output << "g" << std::endl;

        osg::Vec2f coord;
        for( unsigned int cnt = 0; cnt < numTexCoords; cnt++ )
        {
            coord = ( *p_texCoords )[ cnt ];
            *_p_output << "vt " << coord.x() << " " << coord.y() << std::endl;
        }
    }

    // output the normals
    if ( p_norms )
    {
        *_p_output << std::endl;
        *_p_output << "g" << std::endl;

        osg::Vec3f normal;
        for( unsigned int cnt = 0; cnt < numNormals; cnt++ )
        {
            normal = ( *p_norms )[ cnt ];
            *_p_output << "vn " << normal.x() << " " << normal.y() << " " << normal.z() << std::endl;
        }
    }

    // export faces
    *_p_output << std::endl;
    *_p_output << "g " << name << _numPrimitives << std::endl;

    unsigned int index1, index2, index3;
    unsigned int odd, even;
    for( unsigned int vindex = 0; vindex < numIndices - 2; ++vindex )
    {
        if ( vindex % 2 )
        {
            odd  = 1; even = 0;
        }
        else
        {
            odd  = 0; even = 1;
        }

        // build the triangle indices
        index1 = _numVertices + ( p_indices ? p_indices->index( vindex + ( 1 * odd ) + ( 0 * even ) ) : p_set->index( vindex + ( 1 * odd ) + ( 0 * even ) ) );
        index2 = _numVertices + ( p_indices ? p_indices->index( vindex + ( 0 * odd ) + ( 2 * even ) ) : p_set->index( vindex + ( 0 * odd ) + ( 2 * even ) ) );
        index3 = _numVertices + ( p_indices ? p_indices->index( vindex + ( 2 * odd ) + ( 1 * even ) ) : p_set->index( vindex + ( 2 * odd ) + ( 1 * even ) ) );

        // check for degerated indices
        if ( ( index1 == index2 ) || ( index2 == index3 ) || ( index1 == index3 ) )
        {
            continue;
        }

        // flip triangle winding for even triangles
        if ( even )
        {
            unsigned int tmp = index2;
            index2 = index1;
            index1 = tmp;
        }

        *_p_output << "f ";
        if ( p_texCoords && p_norms )
        {
            *_p_output << index1 + 1 << "/" << index1 + 1 << "/" << index1 + 1 << " ";
            *_p_output << index2 + 1 << "/" << index2 + 1 << "/" << index2 + 1 << " ";
            *_p_output << index3 + 1 << "/" << index3 + 1 << "/" << index3 + 1;
        }
        else if ( p_texCoords || p_norms )
        {
            *_p_output << index1 + 1 << "/" << index1 + 1 << " ";
            *_p_output << index2 + 1 << "/" << index2 + 1 << " ";
            *_p_output << index3 + 1 << "/" << index3 + 1;
        }
        else
        {
            *_p_output << index1 + 1 << " ";
            *_p_output << index2 + 1 << " ";
            *_p_output << index3 + 1;
        }
        *_p_output << std::endl;

        ++_numTriangles;
    }

    ++_numPrimitives;
    _numVertices += numVerts;
}

} // namespace vrc
