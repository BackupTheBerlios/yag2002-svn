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
 # entity spotlight
 #
 #   date of creation:  04/16/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _VRC_SPOTLIGHT_H_
#define _VRC_SPOTLIGHT_H_

#include <vrc_main.h>
#include "vrc_lightmanager.h"

namespace vrc
{

#define ENTITY_NAME_SPOTLIGHT    "SpotLight"

//! Spot light entity
class EnSpotLight :  public yaf3d::BaseEntity, public BaseLight
{
    public:
                                                    EnSpotLight();

        virtual                                     ~EnSpotLight();

        
        //! Initializing function, this is called after all engine modules are initialized and a map is loaded.
        void                                        initialize();

    protected:

        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        //! Setup a light with specified parameters
        osg::ref_ptr< osg::Light >                  setupLight();

        osg::Vec3f                                  _direction;

        float                                       _spotCutOff;

        float                                       _spotExponent;

        //! Max distance used for culling ( deactivating ) light source during rendering
        float                                       _maxDistance;

        //! Set to true if this entity is used in menu system
        bool                                        _usedInMenu;

        //! Enable / disable light
        bool                                        _enable;
};

//! Entity type definition used for type registry
class SpotLightEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    SpotLightEntityFactory() : 
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_SPOTLIGHT, yaf3d::BaseEntityFactory::Standalone | yaf3d::BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~SpotLightEntityFactory() {}

        Macro_CreateEntity( EnSpotLight );
};

}

#endif // _VRC_SPOTLIGHT_H_
