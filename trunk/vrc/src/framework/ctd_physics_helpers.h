/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2004-2006, Ali Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU General Public License 
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public 
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

#include <ctd_base.h>
#include <osg/Transform>
#include <Newton.h>

#ifndef _PHYSICS_HELPERS_H_
#define _PHYSICS_HELPERS_H_

namespace CTD
{

//! Class used for rendering physics debug info 
class PhysicsDebugDrawable : public osg::Drawable
{
    public :

                                        PhysicsDebugDrawable() { _supportsDisplayList = false; _useDisplayList = false; }
                                                                        
        // Copy constructor using CopyOp to manage deep vs shallow copy.
                                        PhysicsDebugDrawable( const PhysicsDebugDrawable&, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY ) {};

        osg::Object*                    cloneType() const { return new PhysicsDebugDrawable(); }

        osg::Object*                    clone( const osg::CopyOp& ) const { return new PhysicsDebugDrawable(); }
        
        bool                            isSameKindAs( const osg::Object* obj ) const { return dynamic_cast<const PhysicsDebugDrawable*>(obj)!=NULL; }
        
        const char*                     className() const { return "PhysicsDebugDrawable"; }
        
        void                            drawImplementation( osg::State& state ) const;
        
    protected:
    
        virtual                         ~PhysicsDebugDrawable() {}        
};

//! Visitor for traversing all Geode objects in scenegraph and build collision geometry
class PhysicsVisitor : public osg::NodeVisitor
{
    public:
                                        PhysicsVisitor( osg::NodeVisitor::TraversalMode tm,  NewtonCollision* p_col ) :
                                            osg::NodeVisitor( tm ),                                            
                                            _attribute( 0 )
                                        {
                                            // we take all nodes
                                            setTraversalMask( 0xffffffff );
                                            assert( p_col );
                                            _p_collision    = p_col;                                                                                        
                                            _numPrimitives  = 0;
                                            _numVertices    = 0;
                                        }

                                        ~PhysicsVisitor() {}

        void                            apply( osg::Geode& node );

        //! Statistics methods, use them after traversal
        static unsigned int             getNumPrimitives() { return _numPrimitives; }

        //! Statistics methods, use them after traversal
        static unsigned int             getNumVertices()   { return _numVertices; }

    protected:

        void                            buildTrianlges( osg::PrimitiveSet* p_set, osg::Array* p_verts, osg::Matrixf& mat, osg::IndexArray* p_indices = NULL );

        void                            buildTrianlgeStrip( osg::PrimitiveSet* p_set, osg::Array* p_verts, osg::Matrixf& mat, osg::IndexArray* p_indices = NULL );

        static NewtonCollision*         _p_collision;

        // collision attribute
        unsigned int                    _attribute;

        // statistics
        static unsigned int             _numPrimitives;

        // statistics
        static unsigned int             _numVertices;

};

}

#endif // _PHYSICS_HELPERS_H_
