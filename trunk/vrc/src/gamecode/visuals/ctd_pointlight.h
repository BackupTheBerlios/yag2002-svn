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
#include "ctd_lightmanager.h"

namespace CTD
{

#define ENTITY_NAME_POINTLIGHT    "PointLight"

class LightManager;

//! Point light entity
class EnPointLight :  public BaseEntity, public BaseLight
{
    public:
                                                    EnPointLight();

        virtual                                     ~EnPointLight();

        /**
        * Initializing function, this is called after all engine modules are initialized and a map is loaded.
        */
        void                                        initialize();

    protected:

        //! Max radius for culling ( deactivating ) light source during rendering
        float                                       _lightRadius;
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
