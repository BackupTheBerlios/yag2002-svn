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
 # physics system
 #
 #   date of creation:  02/20/2005
 #
 #   author:            ali botorabi (boto)
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>
#include <osg/Transform>
#include <Newton.h>

#ifndef _PHYSICS_HELPERS_H_
#define _PHYSICS_HELPERS_H_

namespace yaf3d
{

//! Class used for rendering physics debug info
class PhysicsDebugDrawable : public osg::Geometry
{
    public :

                                        PhysicsDebugDrawable() { _supportsDisplayList = false; _useDisplayList = false; }

        // Copy constructor using CopyOp to manage deep vs shallow copy.
                                        PhysicsDebugDrawable( const PhysicsDebugDrawable& drawable, const osg::CopyOp& copyop = osg::CopyOp::SHALLOW_COPY ) :
                                         Geometry( drawable, copyop )
                                        {};

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
        unsigned int                    getNumPrimitives() { return _numPrimitives; }

        //! Statistics methods, use them after traversal
        unsigned int                    getNumVertices()   { return _numVertices; }

    protected:

        void                            buildTrianlges( const osg::PrimitiveSet* p_set, osg::Array* p_verts, const osg::Matrixf& mat, const osg::IndexArray* p_indices = NULL );

        void                            buildTrianlgeStrip( const osg::PrimitiveSet* p_set, osg::Array* p_verts, const osg::Matrixf& mat, const osg::IndexArray* p_indices = NULL );

        static NewtonCollision*         _p_collision;

        // collision attribute
        unsigned int                    _attribute;

        // statistics
        unsigned int                    _numPrimitives;

        // statistics
        unsigned int                    _numVertices;

};

}

#endif // _PHYSICS_HELPERS_H_
