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
 # entity pointlight
 #
 #   date of creation:  04/14/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_POINTLIGHT_H_
#define _CTD_POINTLIGHT_H_

#include <ctd_base.h>
#include <ctd_baseentity.h>
#include <ctd_entitymanager.h>
#include <ctd_utils.h>

namespace CTD
{

#define ENTITY_NAME_POINTLIGHT    "PointLight"

class EnPointLight :  public BaseEntity
{
    public:
                                                    EnPointLight();

        virtual                                     ~EnPointLight();

        /**
        * Initializing function, this is called after all engine modules are initialized and a map is loaded.
        */
        void                                        initialize();

    protected:

        // Entity parameters
        osg::Vec3f                                  _position;

        std::string                                 _meshFile;

        osg::Vec3f                                  _ambientColor;

        osg::Vec3f                                  _diffuseColor;
        
        osg::Vec3f                                  _specularColor;

        float                                       _constAttenuation;

        float                                       _linearAttenuation;

        float                                       _quadraticAttenuation;

        //! Max radius for culling ( deactivating ) light source during rendering
        float                                       _lightRadius;

    protected:

        osg::ref_ptr< osg::LightSource >            _lightSource;

        GLint                                       _lightId;

        static GLint                                _totalLights;

        osg::BoundingSphere                         _bSphere;

        //! Culling callback class for auto-activating light per frame
        class LightCallback : public osg::NodeCallback
        {

            public:

                                                    LightCallback( EnPointLight* p_lightentity ) : 
                                                    _lightEntity( p_lightentity )  
                                                    {}

                void                                operator()( osg::Node* node, osg::NodeVisitor* nv );

                EnPointLight*                       _lightEntity;

        };

};

//! Entity type definition used for type registry
class PointLightEntityFactory : public BaseEntityFactory
{
    public:
                                                    PointLightEntityFactory() : BaseEntityFactory( ENTITY_NAME_POINTLIGHT ) {}

        virtual                                     ~PointLightEntityFactory() {}

        Macro_CreateEntity( EnPointLight );
};

}

#endif // _CTD_POINTLIGHT_H_
