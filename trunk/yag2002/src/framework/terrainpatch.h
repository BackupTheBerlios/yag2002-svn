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

//! Max number of texture channels for a patch
#define MAX_TEX_CHANNELS    8

class TerrainManager;
class TerrainSection;

/*! Terrain patch builder class */
class TerrainPatchBuilder
{
    public:

        //! Get patch's scene node
        osg::ref_ptr< osg::PositionAttitudeTransform >  getSceneNode();

    protected:

        //! Create a patch
                                                        TerrainPatchBuilder();

        virtual                                         ~TerrainPatchBuilder();

        //! Build the patch given the height data as Image, return false if the patch could not be built.
        bool                                            build( 
                                                               const ImageTGA&   image,   // heightmap image
                                                               const osg::Vec3f& scale,   // scale
                                                               unsigned short    column,  // column in image
                                                               unsigned short    row,     // row in image
                                                               unsigned short    sizeX,   // patch size X in pixels 
                                                               unsigned short    sizeY,   // patch size Y in pixels 
                                                               unsigned short    subdivX, // count of subdivisions in X direction, e.g. 8
                                                               unsigned short    subdivY  // count of subdivisions in Y direction, e.g. 8
                                                              );

        //! Build texture coordinates for given channel. Let scale be 0,0 in order to automatically span the texture over the whole terrain ( e.g. useful for base map ).
        //! sizeS and SizeT are the texture dimensions in pixels.
        bool                                            buildTexCoords( unsigned int channel, const osg::Vec2f& scale = osg::Vec2f( 0.0f, 0.0f ) );

        //! Reset the patch freeing up the allocated resources
        void                                            reset();

        //! Scenegraph's patch node
        osg::ref_ptr< osg::PositionAttitudeTransform >  _p_node;

        //! Patch geometry object
        osg::ref_ptr< osg::Geometry >                   _p_drawable;

        //! Patch size in x/y direction relative to the total hight map extends
        osg::Vec2f                                      _relativeSize;

        //! Relative patch position in terrain
        osg::Vec2f                                      _relativePosition;

        //! Subdivision in X direction
        unsigned short                                  _subDivX;

        //! Subdivision in Y direction
        unsigned short                                  _subDivY;

        //! Flag indicating that the patch has been successfully built
        bool                                            _built;

    friend class TerrainManager;
    friend class TerrainSection;
};

} // namespace yaf3d

#endif // _TERRAINPATCH_H_
