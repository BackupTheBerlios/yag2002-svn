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

#ifndef _SCENETOOLS_H_
#define _SCENETOOLS_H_

#include <vrc_main.h>
#include <osgUtil/IntersectVisitor>


//! Class implementing scene tools
class SceneTools
{
    public:

        //! Used for detecting axis handle hits.
        enum AxisHit
        {
            AxisNone = 0x0,
            AxisX    = 0x1,
            AxisY    = 0x2,
            AxisZ    = 0x4
        };

                                                        SceneTools();

        virtual                                         ~SceneTools();

        //! Initialize the picker for given screen size. Use this method before using any operation below.
        bool                                            initialize( unsigned int screenwidth, unsigned int screenheight );

        //! Set the scene node which should be used for picking and hitting operations.
        //! Without a valid scene node no operation will work!
        void                                            setSceneNode( osg::Group* p_node );

        //! Show/hide hit marker.
        void                                            showHitMarker( bool en );

        //! Show/hide axis marker.
        void                                            showAxisMarker( bool en );

        //! Set markers position to given one.
        //! Note: if hit marker is shown then its position gets updated when moving the mouse.
        void                                            setMarkerPosition( const osg::Vec3f& position );

        //! Set markers orientation to given one.
        //! Note: if hit marker is shown then its orientation gets updated when moving the mouse.
        void                                            setMarkerOrientation( const osg::Quat& orientation );

        //! Exclude the given entity from picking, can be used when placing an entity.
        void                                            excludeFromPicking( yaf3d::BaseEntity* p_entity );

        //! Highlight given entity.
        void                                            highlightEntity( yaf3d::BaseEntity* p_entity );

        //! Picking entities for given x/y coordinates in screen space. If p_entity is given then it is prefered by searching.
        //! Returns NULL if nothing could be picked.
        yaf3d::BaseEntity*                              pickEntity( unsigned short xpos, unsigned short ypos, yaf3d::BaseEntity* p_entity = NULL );

        //! Hit polygons in scene ( see method setSceneNode above ) for given x/y coordinates in screen space.
        //! 'viewerposition' is usually the camera position. It is used for back-face culling.
        bool                                            hitScene( unsigned short xpos, unsigned short ypos, const osg::Vec3f& viewerposition );

        //! Get the hits against axis (a combination of AxisHit enums) , valid only when axis marker is shown.
        unsigned int                                    getAxisHits() const;

        //! Get the last hit position.
        const osg::Vec3f&                               getHitPosition() const;

        //! Get the normal at last hit position.
        const osg::Vec3f&                               getHitNormal() const;

        //! Get the object name of last hit.
        const std::string&                              getHitObject() const;

        //! Update the marker scale depending on current distance from camera.
        void                                            updateMarkerScale( const osg::Vec3f& viewerposition );

        //! Get the current marker scale which is updated in 'updateMarkerScale'.
        float                                           getMarkerScale() const;

        //! Set marker scale.
        void                                            setMarkerScale( float scale );

    protected:

        //! Used for getting node name which is hit.
        std::string                                     extractNodeName( osg::NodePath& nodepath );

        //! Scene node which is used for all operations
        osg::ref_ptr< osg::Group >                      _sceneNode;

        //! Current tool marker
        osg::ref_ptr< osg::PositionAttitudeTransform >  _marker;

        osg::ref_ptr< osg::PositionAttitudeTransform >  _hitMarker;

        osg::ref_ptr< osg::PositionAttitudeTransform >  _axisMarker;

        unsigned int                                    _hitAxis;

        float                                           _screenWidth;

        float                                           _screenHeight;

        unsigned int                                    _pickClickCount;

        float                                           _lastX;

        float                                           _lastY;

        float                                           _markerScale;

        osg::ref_ptr< osg::Geode >                      _bboxGeode;

        osg::ref_ptr< osg::Geometry >                   _linesGeom;

        osg::ref_ptr< osg::LineSegment >                _lineSegment;

        osg::Vec3f                                      _hitPosition;

        osg::Vec3f                                      _hitNormal;

        std::string                                     _hitObject;

        yaf3d::BaseEntity*                              _p_entityNoPick;
};

#endif // _SCENETOOLS_H_
