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
 # entity water
 #
 # the water shader is basing on RenderMonkey's Reflection/Refraction 
 #  example
 #
 #   date of creation:  03/26/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 #  changes:
 #
 #  07/15/2005          replaced the wave equation on cpu by gpu shaders
 #
 ################################################################*/

#ifndef _VRC_WATER_H_
#define _VRC_WATER_H_

#include <vrc_main.h>

namespace vrc
{

#define ENTITY_NAME_WATER    "Water"

class DeltaWaveUpdateCallback;
class DeltaNoiseUpdateCallback;
class EnCamera;

//! This entity simulates water using glsl shaders
class EnWater :  public yaf3d::BaseEntity
{
    public:
                                                    EnWater();

        virtual                                     ~EnWater();

        
        //! Initializing function, this is called after all engine modules are initialized and a map is loaded.
        void                                        initialize();

        //! If this entity is used in menu system then we want it to be persistent
        const bool                                  isPersistent() const { return _usedInMenu; }

    protected:

        // Entity attribbutes
        //--------------------------------------------------------//

        std::string                                 _meshFile;

        osg::Vec3f                                  _position;

        float                                       _sizeX;

        float                                       _sizeY;

        float                                       _fadeBias;

        float                                       _noiseSpeed;

        float                                       _waveSpeed;

        float                                       _fadeExp;

        osg::Vec3f                                  _scale;

        osg::Vec3f                                  _waterColor;

        float                                       _transparency;

        bool                                        _usedInMenu;

        //! Reflection cube map textures
        std::string                                 _cubeMapTextures[ 6 ];

    protected:

        // internals
        //--------------------------------------------------------//

        //! Notification call-back
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        //! Set the shader params specified by entity attributes
        void                                        setShaderParams( osg::Node* p_node );

        //! Setup water geom, shaders and state sets
        osg::Node*                                  setupWater();
        
        //! Water node
        osg::ref_ptr< osg::Node >                   _water;

        //! Enable / disable water
        bool                                        _enable;

        //! Texture cube for refletion map
        osg::ref_ptr< osg::TextureCubeMap >         _reflectmap;

        //! Shared stateset for water effect
        static osg::ref_ptr< osg::StateSet >        s_stateSet;

        //! Shared shader uniforms
        static osg::Uniform*                        s_fadeBias;
        static osg::Uniform*                        s_waveSpeed;
        static osg::Uniform*                        s_fadeExp;
        static osg::Uniform*                        s_noiseSpeed;
        static osg::Uniform*                        s_waterColor;
        static osg::Uniform*                        s_scale;
        static osg::Uniform*                        s_trans;

    friend class DeltaWaveUpdateCallback;
    friend class DeltaNoiseUpdateCallback;
};

//! Entity type definition used for type registry
class WaterEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    WaterEntityFactory() : 
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_WATER, yaf3d::BaseEntityFactory::Standalone | yaf3d::BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~WaterEntityFactory() {}

        Macro_CreateEntity( EnWater );
};

}

#endif // _VRC_WATER_H_
