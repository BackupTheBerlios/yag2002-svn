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
 # entity water
 #
 #   date of creation:  03/26/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_WATER_H_
#define _CTD_WATER_H_

#include <ctd_base.h>
#include <ctd_baseentity.h>
#include <ctd_entitymanager.h>
#include <ctd_utils.h>

namespace CTD
{

#define ENTITY_NAME_WATER    "Water"

//! This entity simulates water using a vertex program
class EnWater :  public BaseEntity
{
    public:
                                                    EnWater();

        virtual                                     ~EnWater();

        /**
        * Initializing function, this is called after all engine modules are initialized and a map is loaded.
        */
        void                                        initialize();

        /**
        * Update entity
        * \param deltaTime                          Time passed since last update
        */
        void                                        updateEntity( float deltaTime );

    protected:

        // Entity attribbutes
        //--------------------------------------------------------//

        //! Size in X direction
        float                                       _sizeX;

        //! Size in Y direction
        float                                       _sizeY;

        //! Subdivisions along X direction
        int                                         _subDevisionsX;

        //! Subdivisions along Y direction
        int                                         _subDevisionsY;

        //! Walter height (z value)
        osg::Vec3f                                  _position;

        //! Fluid viscosity
        float                                       _viscosity;

        //! Wave distribution speed
        float                                       _speed;

        //! Stimulation rate ( per sec )
        float                                       _stimulationRate;

        //! Stimulus amplitude
        float                                       _amplitude;

        //! Water texture
        std::string                                 _texFile;

        //! Reflection cube map textures
        std::string                                 _cubeMapTextures[ 6 ];

        //--------------------------------------------------------//

        //! Given the step width calculate the liquid equation constants
        void                                        calcConstants( float stepWidth );

        //! Create the water surface mesh
        osg::Node*                                  makeMesh();

        //! Read the six sides of reflection / refraction cube map
        osg::TextureCubeMap*                        readCubeMap();

        //! Set reflection / refraction stuff for given node
        osg::Node*                                  addRefractStateSet( osg::Node* node );

        EyeTransform*                               _p_transformEyePoint;
           
        osg::ref_ptr< osg::Group >                  _node;
           
        osg::ref_ptr< osg::Geode >                  _geode;

        // vertex position array ( two buffers for fast switching )
        osg::ref_ptr< osg::Vec3Array >              _posArray1;

        osg::ref_ptr< osg::Vec3Array >              _posArray2;
    
        bool                                        _primaryPosBuffer;

        // water geometry
        osg::Geometry*                              _p_geom;

        osg::ref_ptr< osg::Vec3Array >              _normArray;

        // some internal vaiables
        float                                       _k1;
        float                                       _k2;
        float                                       _k3;

        float                                       _stimulationPeriod;
        float                                       _pastTime;

};

//! Entity type definition used for type registry
class WaterEntityFactory : public BaseEntityFactory
{
    public:
                                                    WaterEntityFactory() : BaseEntityFactory( ENTITY_NAME_WATER ) {}

        virtual                                     ~WaterEntityFactory() {}

        Macro_CreateEntity( EnWater );
};

}

#endif // _CTD_WATER_H_
