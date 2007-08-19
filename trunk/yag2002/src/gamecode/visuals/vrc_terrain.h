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

namespace vrc
{

#define ENTITY_NAME_TERRAINSECTION    "TerrainSection"

//! Terrain section entity
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

        //! Heightmap
        std::string                                 _fileHeightmap;

        //! Basemap
        std::string                                 _fileBasemap;

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

        //------------------
           
        osg::ref_ptr< osg::Group >                  _p_terrainGrp;

        bool                                        _enable;
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
