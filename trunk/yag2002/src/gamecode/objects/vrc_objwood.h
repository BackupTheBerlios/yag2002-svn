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
 # object wood
 #
 #   date of creation:  10/31/2007
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_OBJWOOD_H_
#define _VRC_OBJWOOD_H_

#include <vrc_main.h>
#include "vrc_baseobject.h"

//! Entity name
#define ENTITY_NAME_OBJWOOD     "ObjectWood"

namespace vrc
{

//! Entity object wood
class EnObjectWood : public BaseObject
{
    public:

                                                    EnObjectWood();

        virtual                                     ~EnObjectWood();

    protected:

        //! Called when the object is used by user, e.g. by picking
        virtual void                                onObjectUse();
};

//! Entity type definition used for type registry
class ObjectWoodEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    ObjectWoodEntityFactory() : 
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_OBJWOOD, yaf3d::BaseEntityFactory::Standalone | yaf3d::BaseEntityFactory::Client | yaf3d::BaseEntityFactory::Server )
                                                    {}

        virtual                                     ~ObjectWoodEntityFactory() {}

        Macro_CreateEntity( EnObjectWood );
};

} // namespace vrc

#endif // _VRC_OBJWOOD_H_
