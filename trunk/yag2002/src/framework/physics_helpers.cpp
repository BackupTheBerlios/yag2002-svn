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
 # physics system 
 #
 #   date of creation:  02/20/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>
#include "log.h"
#include "physics_helpers.h"
#include "physics.h"
#include <osg/Transform>

namespace yaf3d
{

NewtonCollision* PhysicsVisitor::_p_collision   = NULL;

// debug display related
//----------------------------//
void DebugShowGeometryCollision( const NewtonBody* p_body, int vertexCount, const float* p_faceVertec, int id )
{
	int i = vertexCount - 1;
	osg::Vec3f p0( p_faceVertec[ i * 3 + 0 ], p_faceVertec[ i * 3 + 1 ], p_faceVertec[ i * 3 + 2 ] );
	for ( i = 0; i < vertexCount; ++i ) 
    {
		osg::Vec3f p1( p_faceVertec[ i * 3 + 0 ], p_faceVertec[ i * 3 + 1 ], p_faceVertec[ i * 3 + 2 ] );
		glVertex3f( p0._v[ 0 ], p0._v[ 1 ], p0._v[ 2 ] );
		glVertex3f( p1._v[ 0 ], p1._v[ 1 ], p1._v[ 2 ] );
		p0 = p1;
	}
}

void DebugShowBodyCollision( const NewtonBody* p_body )
{
	NewtonBodyForEachPolygonDo( p_body, DebugShowGeometryCollision );
}

void PhysicsDebugDrawable::drawImplementation( osg::State& state ) const
{
    glPushClientAttrib( GL_CLIENT_ALL_ATTRIB_BITS );
    glPushAttrib( GL_ALL_ATTRIB_BITS );

	glColor3f( 1.0f, 1.0f, 1.0f );
	glDisable( GL_LIGHTING );
	glDisable( GL_TEXTURE_2D );

    osg::Matrixf mat;
    mat.identity();
    glPushMatrix();
    glMultMatrixf( ( float* )mat.ptr() );
	glBegin( GL_LINES );
    NewtonWorldForEachBodyDo( Physics::get()->getWorld(), DebugShowBodyCollision );
	glEnd();
    glPopMatrix();

    glPopClientAttrib();
    glPopAttrib();
}
//----------------------------//

void PhysicsVisitor::apply( osg::Geode& node )
{ 
    // retrieve the node mask which is used for physics material and later for other properies
    // only the first byte is relevant for pyhsics material description
    _attribute = node.getNodeMask() & 0xFF;
    
    // this means no need for building static collision geom
    //  this is not the same as MAT_NOCOL, as MAT_NOCOL collisions are detected
    if ( _attribute == Physics::NO_BUILD )
        return;

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
            osg::Array*  p_verts   = p_geom->getVertexArray();                        
            osg::IndexArray* p_indices  = p_geom->getVertexIndices();
            unsigned int numPrims = p_geom->getNumPrimitiveSets();
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
                            buildTrianlges( p_set, p_verts, mat, p_indices );
                            break;

                        case osg::PrimitiveSet::TRIANGLE_STRIP:
                            buildTrianlgeStrip( p_set, p_verts, mat, p_indices );
                            break;

                        case osg::PrimitiveSet::TRIANGLE_FAN:
                        case osg::PrimitiveSet::QUADS: 
                        case osg::PrimitiveSet::QUAD_STRIP:
                        case osg::PrimitiveSet::POLYGON:

                        default:
                            assert( NULL && "*** unsupported primitive set for physics! currently only TRIANGLE and TRIANGLE_STRIP types are supported." );
                    }
                }
            }
        }
    }
}

void PhysicsVisitor::buildTrianlges( const osg::PrimitiveSet* p_set, osg::Array* p_verts, const osg::Matrixf& mat, const osg::IndexArray* p_indices )
{
    if( p_verts->getType() != osg::Array::Vec3ArrayType ) 
    {
        log_error << "Physics: cannot build trimesh collision data as the vertexarray has not a Vec3 format!" << std::endl;
        return;
    }

    osg::Vec3Array*   p_vertVecs  = static_cast< osg::Vec3Array* >( p_verts );
    unsigned int      numIndices  = p_set->getNumIndices();
    osg::Vec3f        triVert[ 3 ];
    // copy the vertices into triVerts buffer
    for( unsigned int vindex = 0; vindex < numIndices; vindex += 3 )
    {
        triVert[ 0 ] = ( *p_vertVecs )[ p_indices ? p_indices->index( vindex + 0 ) : p_set->index( vindex + 0 ) ];
        triVert[ 0 ] = triVert[ 0 ] * mat;

        triVert[ 1 ] = ( *p_vertVecs )[ p_indices ? p_indices->index( vindex + 1 ) : p_set->index( vindex + 1 ) ];
        triVert[ 1 ] = triVert[ 1 ] * mat;

        triVert[ 2 ] = ( *p_vertVecs )[ p_indices ? p_indices->index( vindex + 2 ) : p_set->index( vindex + 2 ) ];
        triVert[ 2 ] = triVert[ 2 ] * mat;

        // create collision face
        NewtonTreeCollisionAddFace( _p_collision, 3, ( float* )triVert, 12, _attribute );
    }

    ++_numPrimitives;
    _numVertices += 3 * numIndices;
}

void PhysicsVisitor::buildTrianlgeStrip( const osg::PrimitiveSet* p_set, osg::Array* p_verts, const osg::Matrixf& mat, const osg::IndexArray* p_indices )
{
    if( p_verts->getType() != osg::Array::Vec3ArrayType ) 
    {
        log_error << "Physics: cannot build trimesh collision data as the vertexarray has not a Vec3 format!" << std::endl;
        return;
    }

    osg::Vec3Array*   p_vertVecs  = static_cast< osg::Vec3Array* >( p_verts );
    unsigned int      numIndices = p_set->getNumIndices();  
    osg::Vec3f        triVerts[ 3 ];
    unsigned int odd, even;
    // destripify the mesh and build triangle faces
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

        triVerts[ 2 ] = ( *p_vertVecs )[ p_indices ? p_indices->index( vindex + ( 1 * odd ) + ( 0 * even ) ) : p_set->index( vindex + ( 1 * odd ) + ( 0 * even ) ) ];
        triVerts[ 2 ] = triVerts[ 2 ] * mat;

        triVerts[ 1 ] = ( *p_vertVecs )[ p_indices ? p_indices->index( vindex + ( 0 * odd ) + ( 2 * even ) ) : p_set->index( vindex + ( 0 * odd ) + ( 2 * even ) ) ];
        triVerts[ 1 ] = triVerts[ 1 ] * mat;
 
        triVerts[ 0 ] = ( *p_vertVecs )[ p_indices ? p_indices->index( vindex + ( 2 * odd ) + ( 1 * even ) ) : p_set->index( vindex + ( 2 * odd ) + ( 1 * even ) ) ];
        triVerts[ 0 ] = triVerts[ 0 ] * mat;

        // create collision face
        NewtonTreeCollisionAddFace( _p_collision, 3, ( float* )triVerts, 12, _attribute );
    }

    ++_numPrimitives;
    _numVertices += numIndices;
}

} // namespace yaf3d
