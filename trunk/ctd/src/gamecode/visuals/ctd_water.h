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
 # entity water
 #
 # the rendering code is basing on osg examples osgreflect and 
 #  osgvertexprogram 
 #
 #   date of creation:  03/26/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_WATER_H_
#define _CTD_WATER_H_

#include <ctd_main.h>

namespace CTD
{

#define ENTITY_NAME_WATER    "Water"

//! This entity simulates water using a vertex program
class EnWater :  public BaseEntity
{
    public:
                                                    EnWater();

        virtual                                     ~EnWater();

        
        //! Initializing function, this is called after all engine modules are initialized and a map is loaded.
        void                                        initialize();

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

        //! Walter' origin
        osg::Vec3f                                  _position;

        //! Fluid viscosity
        float                                       _viscosity;

        //! Wave distribution speed
        float                                       _speed;

        //! Stimulation rate ( per sec )
        float                                       _stimulationRate;

        //! Stimulus amplitude
        float                                       _amplitude;

        //! Reflection cube map textures
        std::string                                 _cubeMapTextures[ 6 ];

    protected:


        //! Update entity
        void                                        updateEntity( float deltaTime );

        // internals
        //--------------------------------------------------------//

        //! Notification call-back
        void                                        handleNotification( const EntityNotification& notification );

        //! Given the step width calculate the liquid equation constants
        void                                        calcConstants( float stepWidth );

        //! Setup water geom, vertex shader and state sets
        osg::Node*                                  setupWater();

        //! Create the water surface mesh ( grid )
        osg::Geometry*                              makeMesh();

        //! Read the six sides of reflection / refraction cube map
        osg::TextureCubeMap*                        readCubeMap();
        
        osg::ref_ptr< osg::Node >                   _water;

        bool                                        _primaryPosBuffer;

        // vertex position array ( two buffers for fast switching )
        osg::ref_ptr< osg::Vec3Array >              _posArray1;

        osg::ref_ptr< osg::Vec3Array >              _posArray2;
    
        osg::ref_ptr< osg::Vec3Array >              _normArray;

        // water geometry
        osg::Geometry*                              _p_geom;

        //! Ratio of indicies of refraction
        float                                       _refract;

        //! Fresnel multiplier
        float                                       _fresnel;


        // some internal vaiables used for water simulation
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
                                                    WaterEntityFactory() : 
                                                     BaseEntityFactory( ENTITY_NAME_WATER, BaseEntityFactory::Standalone | BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~WaterEntityFactory() {}

        Macro_CreateEntity( EnWater );
};

}

#endif // _CTD_WATER_H_
