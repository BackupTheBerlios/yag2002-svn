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
 # all things needed for a terrain patch
 #
 #   date of creation:  07/30/2007
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _TERRAINPATCH_H_
#define _TERRAINPATCH_H_

#include "base.h"
#include "utils.h"

namespace yaf3d
{

class TerrainManager;
class BasePatchTesselator;

/*! Terrain patch */
class TerrainPatch
{
    public:

        //! Get patch's scene node
        osg::ref_ptr< osg::PositionAttitudeTransform >  getSceneNode();

    protected:

        //! Create a patch with given tesselator type, one of PatchTesselatorType enums
                                                        TerrainPatch( unsigned int tesselatortype );

        virtual                                         ~TerrainPatch();

        //! Build the patch given the height data as Image, return false if the patch could not be built.
        bool                                            build( ImageTGA& image, const osg::Vec3f& scale, unsigned int column , unsigned int row, unsigned int sizeS, unsigned int sizeT );

        //! Reset the patch freeing up the allocated resources
        void                                            reset();

        //! Update the patch tesselation and visibility
        void                                            update( osg::CameraNode* p_cam );

        //! Is the patch visible to camera?
        bool                                            isVisible();

        //! Render the patch
        void                                            render();

        //! Tesselator types
        enum PatchTesselatorType
        {
            eTesselatorScreenSpace = 0x10,

            eTesselatorWorldSpace
        };

        //! Scenegraph's patch node
        osg::ref_ptr< osg::PositionAttitudeTransform >  _p_node;

        //! Patch tesselator
        BasePatchTesselator*                            _p_tesselator;

    friend class TerrainManager;
};

} // namespace yaf3d

#endif // _TERRAINPATCH_H_
