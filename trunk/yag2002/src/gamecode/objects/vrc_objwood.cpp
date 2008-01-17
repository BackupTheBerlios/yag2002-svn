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
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include <gamelogic/vrc_storysystem.h>
#include "vrc_objwood.h"
#include "vrc_objectnetworking.h"
#include "../storage/vrc_storageclient.h"
#include "../storage/vrc_userinventory.h"

namespace vrc
{

//! Implement and register the wood entity factory
YAF3D_IMPL_ENTITYFACTORY( ObjectWoodEntityFactory )

//! Register the actor type, the registry is needed for networking
VRC_REGISTER_ACTOR( VRC_ACTOR_TYPE_WOOD, ENTITY_NAME_OBJ_WOOD );


EnObjectWood::EnObjectWood() :
 BaseObject( VRC_ACTOR_TYPE_WOOD )
{
}

EnObjectWood::~EnObjectWood()
{
}

void EnObjectWood::onObjectUse()
{
    // TODO: make something useful
    StoryEvent event( StoryEvent::eTypePickItem, getActorType(), getActorID(), 0, 0, StoryEvent::eFilterStoryPublic, 10, 20, 10.0f, 20.0f, "picking wood" );
    StorySystem::get()->sendEvent( event );
}

void EnObjectWood::onEventReceived( const StoryEvent& event )
{
    UserInventory* p_inv = gameutils::PlayerUtils::get()->getPlayerInventory();
    if ( !p_inv->increaseItem( ITEM_NAME_OBJ_WOOD, 1 ) )
    {
        //! TODO ...assemble the item string out of entity attributes
        p_inv->addItem( ITEM_NAME_OBJ_WOOD, getActorID(), "test=10,test2=21" );
    }

    //! TODO: get the respawn time from object attributes
    disappear( 5.0f );
}

} // namespace vrc
