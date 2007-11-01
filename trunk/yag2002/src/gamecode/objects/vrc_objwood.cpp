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

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_objwood.h"

namespace vrc
{

//! Implement and register the wood entity factory
YAF3D_IMPL_ENTITYFACTORY( ObjectWoodEntityFactory )

//! Register the object type, the registry is needed for networking
VRC_REGISTER_OBJECT( VRC_OBJECT_ID_WOOD, ENTITY_NAME_OBJWOOD );


EnObjectWood::EnObjectWood() :
 BaseObject( VRC_OBJECT_ID_WOOD, ENTITY_NAME_OBJWOOD )
{
}

EnObjectWood::~EnObjectWood()
{

}

void EnObjectWood::onObjectPicked()
{
}

} // namespace vrc
