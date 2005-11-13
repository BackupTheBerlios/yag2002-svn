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

#include <ctd_main.h>
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

        void                                        handleNotification( const EntityNotification& notification );

        //! Max radius for culling ( deactivating ) light source during rendering
        float                                       _lightRadius;
};

//! Entity type definition used for type registry
class PointLightEntityFactory : public BaseEntityFactory
{
    public:
                                                    PointLightEntityFactory() : 
                                                     BaseEntityFactory( ENTITY_NAME_POINTLIGHT, BaseEntityFactory::Standalone | BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~PointLightEntityFactory() {}

        Macro_CreateEntity( EnPointLight );
};

}

#endif // _CTD_POINTLIGHT_H_