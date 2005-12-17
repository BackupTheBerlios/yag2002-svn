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

#ifndef _VRC_FOG_H_
#define _VRC_FOG_H_

#include <vrc_main.h>

namespace vrc
{

#define ENTITY_NAME_FOG    "Fog"

//! This entity sets up fog.
class EnFog :  public yaf3d::BaseEntity
{
    public:
                                                    EnFog();

        virtual                                     ~EnFog();

        //! Initializing function, this is called after all engine modules are initialized and a map is loaded.
        void                                        initialize();

        //! The actual fog object is shared between entities so call this method to activate fog parameters set in entity.
        void                                        enable();

        //! If this entity is used in menu system then we want it to be persistent
        const bool                                  isPersistent() const { return _usedInMenu; }

    protected:

        //! This entity is persistent so it has to trigger its destruction on shutdown ifself.
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        float                                       _density;

        float                                       _start;

        float                                       _end;

        //! Fog color
        osg::Vec3f                                  _fogColor;

        static osg::Fog*                            _p_fog;

        bool                                        _usedInMenu;

        bool                                        _enable;

        static unsigned int                         _instCount;
};

//! Entity type definition used for type registry
class FogEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    FogEntityFactory() :
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_FOG, yaf3d::BaseEntityFactory::Standalone | yaf3d::BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~FogEntityFactory() {}

        Macro_CreateEntity( EnFog );
};

} // namespace vrc

#endif // _VRC_FOG_H_
