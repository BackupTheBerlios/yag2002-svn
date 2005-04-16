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
 # entity spotlight
 #
 #   date of creation:  04/16/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_SPOTLIGHT_H_
#define _CTD_SPOTLIGHT_H_

#include <ctd_base.h>
#include <ctd_baseentity.h>
#include <ctd_entitymanager.h>
#include "ctd_lightmanager.h"

namespace CTD
{

#define ENTITY_NAME_SPOTLIGHT    "SpotLight"

//! Spot light entity
class EnSpotLight :  public BaseEntity, public BaseLight
{
    public:
                                                    EnSpotLight();

        virtual                                     ~EnSpotLight();

        /**
        * Initializing function, this is called after all engine modules are initialized and a map is loaded.
        */
        void                                        initialize();

    protected:

        osg::Vec3f                                  _direction;

        float                                       _spotCutOff;

        float                                       _spotExponent;

        //! Max distance used for culling ( deactivating ) light source during rendering
        float                                       _maxDistance;
};

//! Entity type definition used for type registry
class SpotLightEntityFactory : public BaseEntityFactory
{
    public:
                                                    SpotLightEntityFactory() : BaseEntityFactory( ENTITY_NAME_SPOTLIGHT ) {}

        virtual                                     ~SpotLightEntityFactory() {}

        Macro_CreateEntity( EnSpotLight );
};

}

#endif // _CTD_SPOTLIGHT_H_
