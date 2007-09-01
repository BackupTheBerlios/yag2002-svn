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
 # entity terrain
 #
 #   date of creation:  07/27/2007
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_TERRAIN_H_
#define _VRC_TERRAIN_H_

#include <vrc_main.h>
#include <vrc_gameutils.h>

//! TODO: move this include to vrc_main.h
#include <terrainmanager.h>

namespace vrc
{

#define ENTITY_NAME_TERRAINSECTION    "TerrainSection"

//! Terrain section entity supporting up to 3 LODs, 1 base texture, and 3 detail maps
class EnTerrainSection :  public yaf3d::BaseEntity
{
    public:
                                                    EnTerrainSection();

        virtual                                     ~EnTerrainSection();
        
        //! Initializing function, this is called after all engine modules are initialized and a map is loaded.
        void                                        initialize();

        //! Enable / disable terrain rendering
        void                                        enable( bool en );

    protected:

        //! This entity is persistent so it has to trigger its destruction on shutdown ifself.
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        //! Setup the terrain
        osg::ref_ptr< osg::Group >                  setup();


        // Entity parameters
        //------------------

        //! Section ID
        unsigned int                                _sectionID;

        //! Position of the terrain section
        osg::Vec3f                                  _position;

        //! Rotation of the terrain section
        osg::Vec3f                                  _rotation;

        //! Scaling the height and X/Y
        osg::Vec3f                                  _scale;

        //! Count of tiles in X direction 
        int                                         _tilesX;

        //! Count of tiles in Y direction 
        int                                         _tilesY;

        //! Heightmap
        std::string                                 _fileHeightmap;

        //! Basemap
        std::string                                 _fileBasemap;

        //! Blend factor for mixing base map and detail maps ( only when using shaders )
        float                                       _blendBasemap;

        //! Layer mask file. This file contains the mask of the detail maps (Red Detail 0, Green Detail 1, etc).
        std::string                                 _fileLayerMask;

        //! Repeat factor in X/Y direction for basemap
        osg::Vec2f                                  _basemapRepeat;

        //! Detail map 0 file
        std::string                                 _fileDetailmap0;

        //! Repeat factor in X/Y direction for detailmap 0
        osg::Vec2f                                  _detailmap0Repeat;

        //! Detail map 1 file
        std::string                                 _fileDetailmap1;

        //! Repeat factor in X/Y direction for detailmap 1
        osg::Vec2f                                  _detailmap1Repeat;

        //! Detail map 2 file
        std::string                                 _fileDetailmap2;

        //! Repeat factor in X/Y direction for detailmap 2
        osg::Vec2f                                  _detailmap2Repeat;

        //! LOD 0 resolution in X direction
        int                                         _lod0ResolutionX;

        //! LOD 0 resolution in Y direction
        int                                         _lod0ResolutionY;

        //! LOD 0 minimal range ( distance to camera )
        float                                       _lod0RangeMin;

        //! LOD 0 mamimal range ( distance to camera )
        float                                       _lod0RangeMax;

        //! LOD 1 resolution in X direction
        int                                         _lod1ResolutionX;

        //! LOD 1 resolution in Y direction
        int                                         _lod1ResolutionY;

        //! LOD 1 minimal range ( distance to camera )
        float                                       _lod1RangeMin;

        //! LOD 1 mamimal range ( distance to camera )
        float                                       _lod1RangeMax;

        //! LOD 2 resolution in X direction
        int                                         _lod2ResolutionX;

        //! LOD 2 resolution in Y direction
        int                                         _lod2ResolutionY;

        //! LOD 2 minimal range ( distance to camera )
        float                                       _lod2RangeMin;

        //! LOD 2 mamimal range ( distance to camera )
        float                                       _lod2RangeMax;

        //! Enable/disable the terrain section
        bool                                        _enable;
        
        //------------------
        
        osg::ref_ptr< osg::Group >                  _p_terrainGrp;
};

//! Entity type definition used for type registry
class TerrainEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    TerrainEntityFactory() : 
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_TERRAINSECTION, yaf3d::BaseEntityFactory::Standalone | yaf3d::BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~TerrainEntityFactory() {}

        Macro_CreateEntity( EnTerrainSection );
};

} // namespace vrc

#endif // _VRC_TERRAIN_H_
