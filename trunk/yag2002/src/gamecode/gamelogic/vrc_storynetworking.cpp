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
 # networking for story system; this is a server side object.
 #
 #   date of creation:  01/14/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_storynetworking.h"
#include "../gamelogic/vrc_storysystem.h"
#include "../storage/vrc_storageserver.h"
#include "../storage/vrc_storageclient.h"
#include <RNReplicaNet/Inc/DataBlock_Function.h>

using namespace RNReplicaNet;

namespace vrc
{

StoryNetworking::StoryNetworking()
{
}

StoryNetworking::~StoryNetworking()
{
    // reset the networking object in story system, note that a server connection loss also causes the destruction of this object (via network device)
    if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Client )
        StorySystem::get()->setNetworking( NULL );
}

void StoryNetworking::PostObjectCreate()
{ // a new client has joined, this is called only on client

    // this function is called only on clients
    assert( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Client );

    // set the networking object of story system
    StorySystem::get()->setNetworking( this );
}

bool StoryNetworking::sendEvent( const StoryEvent& event )
{
    // copy the event data into networking paket
    tEventData eventdata;
    memset( &eventdata, 0, sizeof( eventdata ) );
    eventdata._eventType      = event.getType();
    eventdata._filter         = event.getFilter();
    eventdata._sourceType     = event.getSourceType();
    eventdata._sourceID       = event.getSourceID();
    eventdata._targetType     = event.getTargetType();
    eventdata._targetID       = event.getTargetID();
    eventdata._uiParam[ 0 ]   = event.getUIParam1();
    eventdata._uiParam[ 1 ]   = event.getUIParam2();
    eventdata._fParam[ 0 ]    = event.getFParam1();
    eventdata._fParam[ 1 ]    = event.getFParam2();
    strncpy( eventdata._sParam, event.getSParam().c_str(), sizeof( eventdata._sParam ) - 2 );
    // terminate the string by two zeros at the end
    eventdata._sParam[ sizeof( eventdata._sParam ) - 1 ] = 0;
    eventdata._sParam[ sizeof( eventdata._sParam ) - 2 ] = 0;

    if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Client )
    {
        eventdata._sessionCookie = ( unsigned int )( yaf3d::NetworkDevice::get()->getSessionID() );
        eventdata._userID        = StorageClient::get()->getUserID();
    }

    // call the receive event on clients or server
    MASTER_FUNCTION_CALL( RPC_ReceiveEvent( eventdata ) );

    return true;
}

void StoryNetworking::RPC_ReceiveEvent( tEventData eventdata )
{ // this is called on server or clients

    // let the storage server validate the client first!
    if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server )
    {
        if ( !StorageServer::get()->validateClient( eventdata._userID, eventdata._sessionCookie ) )
        {
            log_info << "*** StoryNetworking: receiving event from unauthorized client" << std::endl;
            return;
        }
    }

    // terminate the string by two zeros at the end
    eventdata._sParam[ sizeof( eventdata._sParam ) - 1 ] = 0;
    eventdata._sParam[ sizeof( eventdata._sParam ) - 2 ] = 0;
    // create the event
    StoryEvent event(
                        eventdata._eventType,
                        eventdata._sourceType,
                        eventdata._sourceID,
                        eventdata._targetType,
                        eventdata._targetID,
                        eventdata._filter,
                        eventdata._uiParam[ 0 ],
                        eventdata._uiParam[ 1 ],
                        eventdata._fParam[ 0 ],
                        eventdata._fParam[ 1 ],
                        eventdata._sParam
                     );

    // feed in the event into story system
    if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server )
        StorySystem::get()->sendEvent( event );
    else
        StorySystem::get()->receiveEvent( event );

#if 0

    std::vector< float > args;
    std::vector< float > result;
    result.push_back( 0.0f ); // return value: success / fail
    result.push_back( 0.0f ); // respawn time

    if ( !GameLogic::get()->requestAction( action._actionType, action._paramUint[ 0 ], action._paramUint[ 1 ], args, result ) )
    {
        log_error << "StoryNetworking: problem executing required action: " << action._actionType << std::endl;
        action._actionResult = -1;
    }
    else
    {
        // push the result of action into first uint parameter
        action._actionResult = int( result[ 0 ] );
        // push the respawn time to first float parameter
        action._paramFloat[ 0 ] = result[ 1 ];
    }

    ALL_REPLICAS_FUNCTION_CALL( RPC_ActionResult( action ) );
#endif
}

} // namespace vrc
