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
 # entity for fog setup
 #
 #   date of creation:  04/05/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_FOG_H_
#define _CTD_FOG_H_

#include <ctd_main.h>

namespace CTD
{

#define ENTITY_NAME_FOG    "Fog"

//! This entity sets up fog.
class EnFog :  public BaseEntity
{
    public:
                                                    EnFog();

        virtual                                     ~EnFog();

        //! Initializing function, this is called after all engine modules are initialized and a map is loaded.
        void                                        initialize();

        //! As the actual fog object is shared between entities so call this method to set the right 
        //!  attributes for particular entity.
        void                                        activateFog();

        //! Disable fog. This method diables the entire fog ( also for all other fog entities ).
        static void                                 deactivateFog();

    protected:

        float                                       _density;

        float                                       _start;

        float                                       _end;

        //! Fog color
        osg::Vec3f                                  _fogColor;

    protected:

        static osg::Fog*                            _p_fog;
};

//! Entity type definition used for type registry
class FogEntityFactory : public BaseEntityFactory
{
    public:
                                                    FogEntityFactory() :
                                                     BaseEntityFactory( ENTITY_NAME_FOG, BaseEntityFactory::Standalone | BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~FogEntityFactory() {}

        Macro_CreateEntity( EnFog );
};

} // namespace CTD

#endif // _CTD_FOG_H_
