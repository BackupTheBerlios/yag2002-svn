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
 #  GLOD is used for actual geometry reduction / expansion.
 #  http://www.cs.jhu.edu/~graphics/GLOD
 #
 #   date of creation:  02/15/2006
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_LOD_H_
#define _VRC_LOD_H_

#include <vrc_main.h>

namespace vrc
{

class GLODMesh;

//! Class for holding all LOD related settings
class LODSettings : public osg::Referenced
{
    public:

        //! LOD mode
        enum
        {
            LOD_DISCRETE    = 0x1,
            LOD_CONTINUOUS  = 0x2
        };

        //! Create LOD settings for LOD_DISCRETE or LOD_CONTINUOUS mode
        explicit                                    LODSettings( GLenum mode );

        //! Copy constructor
                                                    LODSettings( const LODSettings& lodsettings );

        virtual                                     ~LODSettings();

        //! Use one of the following methods for setting up the LOD adaptation and error mode

        //! Set 'threshold', adaption mode is set to error threshold and error mode to screen space.
        void                                        setupScreenSpaceThreshold( GLfloat threshold );

        //! Set 'threshold', adaption mode is set to error threshold and error mode to object space.
        void                                        setupObjectSpaceThreshold( GLfloat threshold );

        //! Set 'max triangles', adaption mode is set to triangle budget and error mode to screen space.
        void                                        setupScreenSpaceMaxTriangles( GLuint maxtris );

        //! Set 'max triangles', adaption mode is set to triangle budget and error mode to object space.
        void                                        setupObjectSpaceMaxTriangles( GLuint maxtris );

        //! Get LOD mode, it can be GLOD_DISCRETE or GLOD_CONTINUOUS.
        inline GLenum                               getMode() const;

        //! Get adaptation mode, it can be GLOD_ERROR_THRESHOLD or GLOD_TRIANGLE_BUDGET.
        inline GLenum                               getAdaptationMode() const;

        //! Get error mode, it can be GLOD_OBJECT_SPACE_ERROR or GLOD_SCREEN_SPACE_ERROR.
        inline GLenum                               getErrorMode() const;

        //! Get error threshold, use only in screen or object space adaptation modes.
        inline GLfloat                              getErrorThreshold() const;

        //! Get maximal triangles, use only in GLOD_TRIANGLE_BUDGET mode.
        inline GLuint                               getMaxTriangles() const;

    protected:

        //! Apply settings to given LOD mesh, used internally by GLODMesh.
        void                                        apply( GLODMesh* p_lodmesh );

        //! LOD mode, it can be GLOD_DISCRETE or GLOD_CONTINUOUS.
        GLenum                                      _mode;

        //! Adapt mode, it can be GLOD_ERROR_THRESHOLD or GLOD_TRIANGLE_BUDGET.
        GLenum                                      _adaptMode;

        //! Error mode, it can be GLOD_OBJECT_SPACE_ERROR or GLOD_SCREEN_SPACE_ERROR.
        GLenum                                      _errorMode;

        //! Error threshold used when in mode GLOD_ERROR_THRESHOLD
        GLfloat                                     _errorThreshold;

        //! Maximal triangles used when in mode GLOD_TRIANGLE_BUDGET
        GLuint                                      _maxTriangles;

    friend class GLODMesh;
};

//! Class for interfacing GLOD and OSG geometry
class GLODMesh : public osg::Referenced
{
    public:

        explicit                                    GLODMesh( osg::Geometry* p_geom );

        virtual                                     ~GLODMesh();

        //! Create a new GLOD object given LOD settings. Returns the GLOD object name.
        GLuint                                      createObject( const LODSettings& lodsettings );

        //! Get LOD settings
        inline const LODSettings*                   getLODSettings() const;

        //! Get GLOD group name
        inline const GLuint                         getGroupName() const;

        //! Get GLOD object name
        inline const GLuint                         getObjectName() const;

        //! Get number of object patches
        inline const GLuint                         getNumPatches() const;

        //! Set LOD adaptation flag, this is called during scene updating.
        inline void                                 setAdaptLOD( bool adapt );

        //! Returns true if the LOD must be adapted, this is used by renderer.
        inline const bool                           adaptLOD() const;

        //! Get total num of GLOD objects
        static GLuint                               getTotalNumObjects();

    protected:

        //! Create a new GLOD group name
        GLuint                                      createGroupName();

        //! Build GLOD object out of type TRIANGLE
        void                                        buildObjectTrianlge( const osg::PrimitiveSet* p_set, const osg::IndexArray* p_indices );

        //! LOD group name of drawable
        GLuint                                      _lodGroupName;

        //! LOD object name of drawable
        GLuint                                      _lodObjectName;

        //! LOD patch name belonging to an object
        GLuint                                      _lodPatchName;

        //! Flag indicating need for LOD adaptation
        bool                                        _adaptLOD;

        //! LOD settings
        osg::ref_ptr< LODSettings >                 _p_lodSettings;

        //! Geometry data
        osg::Geometry*                              _p_geometry;

        //! Total number of LOD objects
        static GLuint                               s_glodNumObjects;

        //! Used for creating unique group names
        static GLuint                               s_lodGroup;

        //! Used for creating unique object names
        static GLuint                               s_lodObject;
};

//! Visitor for traversing all Geode objects and creating LOD
class LODVisitor : public osg::NodeVisitor
{
    public:

        explicit                                    LODVisitor( osg::NodeVisitor::TraversalMode tm = osg::NodeVisitor::TRAVERSE_ALL_CHILDREN );
 
        virtual                                     ~LODVisitor() {}

        void                                        apply( osg::Geode& node );

        //! Set LOD settings which will be used for creating LOD meshes
        inline void                                 setLODSettings( LODSettings* p_lodsettings );

        //! Get LOD settings
        inline const LODSettings*                   getLODSettings() const;

    protected:

        osg::ref_ptr< LODSettings >                 _p_lodSettings;
};

//! Draw callback for Geometries using LOD
class LODDrawCallback: public osg::Drawable::DrawCallback
{
    public:

        explicit                                    LODDrawCallback( GLODMesh* p_lodmesh ) :
                                                     _p_lodMesh( p_lodmesh )
                                                    {}

                                                    LODDrawCallback( const LODDrawCallback&, const osg::CopyOp& ) {}

        void                                        drawImplementation( osg::State& state, const osg::Drawable* p_drawable ) const;

    protected:

        GLODMesh*                                   _p_lodMesh;
};

//! Update callback for Geometries using LOD
class LODUpdateCallback : public osg::Drawable::UpdateCallback
{
    public:

        //! 'adaptperiod' describes the LOD adaptation period in seconds
        explicit                                    LODUpdateCallback( GLODMesh* p_lodmesh, float adaptperiod ) :
                                                     _p_lodMesh( p_lodmesh ),
                                                     _adaptationPeriod( adaptperiod ),
                                                     _lastRefTime( 0.0 ),
                                                     _updateTimer( 0.0 )
                                                    {}

                                                    LODUpdateCallback( const LODUpdateCallback&, const osg::CopyOp& ) {}

        void                                        update( osg::NodeVisitor* p_nv, osg::Drawable* p_drawable );

    protected:

        GLODMesh*                                   _p_lodMesh;

        double                                      _adaptationPeriod;

        double                                      _lastRefTime;

        double                                      _updateTimer;
};

#include "vrc_lod.inl"

} // namespace vrc

#endif // _VRC_LOD_H_
