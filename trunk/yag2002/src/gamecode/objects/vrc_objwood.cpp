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
#include "vrc_objwood.h"
#include "vrc_objectnetworking.h"
#include "../storage/vrc_storageclient.h"
#include "../gamelogic/vrc_gamelogic.h"
#include "../scripting/vrc_scriptconsole.h"

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

void EnObjectWood::onObjectUse()
{
    GameLogic::get()->getScriptConsole()->scAddOutput( ObjectRegistry::getEntityType( getObjectID() ) + ": request for picking action", true );

    // this function is called on client and standalone modes. in client mode the networking object is a valid one.
    if ( _p_networking )
    {
        // request the server for using the object
        tActionData action;
        memset( &action, 0, sizeof( action ) );
        action._sessionCookie   = unsigned int( yaf3d::NetworkDevice::get()->getSessionID() );
        action._userID          = StorageClient::get()->getUserID();
        action._actionType      = GameLogic::eActionPick;
        action._paramUint[ 0 ]  = getObjectID();

        if ( !_p_networking->RequestAction( action, this ) )
            log_debug << "EnObjectWood: cannot send request, a request is already in progress!" << std::endl;
    }
    else
    {
        //! TODO: what to do in standalone? ...
        std::vector< float > args;
        std::vector< float > result;

        result.push_back( 0.0f );

        if ( !GameLogic::get()->requestAction( GameLogic::eActionPick, getObjectID(), args, result ) )
            log_error << getInstanceName() << ": problem executing requested action Pick " << std::endl;
    }
}

void EnObjectWood::actionResult( tActionData& result )
{
    std::string res( result._actionResult > 0 ? "SUCCESS" : "FAIL" );
    GameLogic::get()->getScriptConsole()->scAddOutput( ObjectRegistry::getEntityType( getObjectID() ) + ": got action request result: " + res, true );

    // TODO: handle the action request result
}

} // namespace vrc
