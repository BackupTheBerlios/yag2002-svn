/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2002-2004, Ali Botorabi
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
 # neoengine, networking core
 #
 # this core uses RakNet and provides network server services
 #
 #
 #   date of creation:  04/23/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_frbase.h>
#include "ctd_network_server.h"
#include <ctd_network.h>
#include <ctd_framework_impl.h>
#include <ctd_descriptor.h>

#include "raknet_NetworkObject.h"

#include <string>
#include <time.h>

using namespace std;
using namespace NeoEngine;

namespace CTD
{


void NetworkServer::Update()
{

    // dispatch received packets
    ProcessPackets( this ); 

}

void NetworkServer::Shutdown()
{ 

    Disconnect();

}

void NetworkServer::ReceiveNewIncomingConnection(Packet *pkPacket,RakServerInterface *pkServerInterface)
{


    // print a time stamp
    time_t kTime;
    time( &kTime );
    neolog << LogLevel( INFO ) << "time: " << ctime( &kTime ) << endl;

    neolog << LogLevel( INFO ) << "nw-server: client ( " 
        <<  pkPacket->playerId.binaryAddress 
        << "; port "
        << pkPacket->playerId.port
        << " ) requests for connection."
        << endl
        << "------------------------------------------------------------------------------" 
        << endl;

    // if another connection request is pending, then handle that first,
    //  the client will try to connect again
    if ( m_kPendingClient.m_bPending == true ) {

        neolog << LogLevel( INFO ) << "nw-server: ( ReceiveNewIncomingConnection ) got a conncetion request from client during integration a previous request " << endl; 
        // kicking is really not nice :-(, this may be replaced by a better handling later
        Kick( pkPacket->playerId );

    }

}

void NetworkServer::ReceiveRemoteNewIncomingConnection(Packet *pkPacket,RakServerInterface *pkServerInterface)
{

    neolog << LogLevel( INFO ) << "nw-server: remote client ( " 
        <<  pkPacket->playerId.binaryAddress 
        << "; port "
        << pkPacket->playerId.port
        << " ) requests for connection." << endl;

    // if another connection request is pending, the client will try that again 
    if ( m_kPendingClient.m_bPending == true ) {

        neolog << LogLevel( INFO ) << "nw-server: ( ReceiveRemoteNewIncomingConnection ) got a conncetion request from client during integration a previous request " << endl; 

    }

}

void NetworkServer::ReceiveRemoteDisconnectionNotification(Packet *pkPacket,RakServerInterface *pkServerInterface)
{

    RemoveClient( pkPacket->playerId );

}

void NetworkServer::ReceiveRemoteConnectionLost(Packet *pkPacket,RakServerInterface *pkServerInterface)
{

    RemoveClient( pkPacket->playerId );

}

void NetworkServer::ReceiveConnectionResumption(Packet *pkPacket,RakServerInterface *pkServerInterface)
{

}

void NetworkServer::ReceiveNoFreeIncomingConnections(Packet *pkPacket,RakServerInterface *pkServerInterface)
{

}

void NetworkServer::ReceiveDisconnectionNotification(Packet *pkPacket,RakServerInterface *pkServerInterface)
{

    // print a time stamp
    time_t kTime;
    time( &kTime );
    neolog << LogLevel( INFO ) << "time: " << ctime( &kTime ) << endl;

    neolog << LogLevel( INFO ) << "nw-server: client ( " 
        <<  pkPacket->playerId.binaryAddress 
        << "; port "
        << pkPacket->playerId.port
        << " ) left the session." << endl;  

    RemoveClient( pkPacket->playerId );

}

void NetworkServer::ReceiveConnectionLost(Packet *pkPacket,RakServerInterface *pkServerInterface)
{

    // print a time stamp
    time_t kTime;
    time( &kTime );
    neolog << LogLevel( INFO ) << "time: " << ctime( &kTime ) << endl;

    neolog << LogLevel( INFO ) << "nw-server: lost connection to client ( " 
        <<  pkPacket->playerId.binaryAddress 
        << "; port "
        << pkPacket->playerId.port
        << " ) " << endl;

    RemoveClient( pkPacket->playerId );

}

void NetworkServer::ReceivedStaticData(Packet *pkPacket,RakServerInterface *pkServerInterface)
{

    tStaticDataClient   *pkData = ( tStaticDataClient* )( ( char* )( pkPacket->data + sizeof(unsigned char) ) );

    neolog << LogLevel( INFO ) << "nw-server: initialization information received " << "from client: " << pkData->m_pcNetworkNodeName << endl;  

    // if another connection request is pending, the new client will try that again
    //  the only thing the server has to do is ignoring the new client's request.
    if ( m_kPendingClient.m_bPending == true ) {

        neolog << LogLevel( INFO ) << "nw-server: ( ReceivedStaticData ) got a conncetion request from client during integration a previous request, ignore it! " << endl; 
        return;

    }

    m_kPendingClient.m_bPending             = true;
    m_kPendingClient.m_pkPendingTimer.Reset();  // reset the pending timer
    // terminate string avoiding a possible stacke overflow ( crash attacks )!
    pkData->m_pcNetworkNodeName[ 255 ]      = '\0';
    m_kPendingClient.m_strNetworkNodeName   = pkData->m_pcNetworkNodeName;
    m_kPendingClient.m_kNodeID              = pkPacket->playerId;

}

void NetworkServer::ReceiveInvalidPassword(Packet *pkPacket,RakServerInterface *pkServerInterface)
{

}

void NetworkServer::ReceiveModifiedPacket(Packet *pkPacket,RakServerInterface *pkServerInterface)
{

    // print a time stamp
    time_t kTime;
    time( &kTime );
    neolog << LogLevel( INFO ) << "time: " << ctime( &kTime ) << endl;

    neolog << LogLevel( INFO ) << "nw-server: got modified packet ( " 
        <<  pkPacket->playerId.binaryAddress 
        << "; port "
        << pkPacket->playerId.port
        << " ) disconnecting client to security issues." << endl;

    Kick( pkPacket->playerId );

    // remove from client list
    RemoveClient( pkPacket->playerId );

}

void NetworkServer::ReceiveRemotePortRefused(Packet *pkPacket,RakServerInterface *pkServerInterface)
{

}

void NetworkServer::ReceiveVoicePacket(Packet *pkPacket,RakServerInterface *pkServerInterface)
{

}

void NetworkServer::ProcessUnhandledPacket( char *pcPacketData, unsigned char ucPacketIdentifier, const PlayerID &rkNodeID, RakServerInterface *pkServerInterface )
{

    if ( m_bServerRunning == false ) {

        return;

    }

    switch( ucPacketIdentifier ) {

        //-------------------------//
        case CTD_NM_NETWORK_ENTITY:
        {

            tCTDEntityPacketHeader  *pkPacketHeader  = ( tCTDEntityPacketHeader* )pcPacketData;

            unsigned short  usNetworkID = pkPacketHeader->m_usNetworkID;

            NetworkObject   *pkNetworkObject = GET_OBJECT_FROM_ID( usNetworkID );

            // check for valid ids!
            if ( pkNetworkObject == NULL ) {

                neolog << LogLevel( WARNING ) << "  (NetworkServer::ProcessUnhandledPacket) server received an undefined object id: " << usNetworkID << endl;
                return;
            }

            // send the message to entity
            pkNetworkObject->GetEntity()->NetworkMessage( CTD_NM_NETWORK_ENTITY, pkPacketHeader );

            break;
        }

        //-------------------------//
        case CTD_NM_NETWORK_SYSTEM:
        {

            tCTDNetworkMgrPacketHeader  *pkPacketHeader  = ( tCTDNetworkMgrPacketHeader* )pcPacketData;
            unsigned char  ucDataType   = pkPacketHeader->m_ucDataType;

            switch ( ucDataType ) {

                //--------------------------------------------------------------//
                case CTD_NM_NETWORK_CLIENT_REQ_ADD_CLIENTOBJECT:
                {

                    tCTD_NM_CLIENT_ADD_CLIENTOBJECT_REQEUST *pkAddClientObjectData = ( tCTD_NM_CLIENT_ADD_CLIENTOBJECT_REQEUST* )pcPacketData;
                    pkAddClientObjectData->m_pcEntityName[ 127 ] = '\0';
                    pkAddClientObjectData->m_pcNetworkNodeName[ 255 ] = '\0';

                    // add a new client to server and send a notification to all clients for creating ghosts
                    if ( AddClientObject( pkAddClientObjectData->m_pcEntityName, rkNodeID, pkAddClientObjectData->m_pcNetworkNodeName ) == false ) {

                        // something went wrong!
                        neolog << LogLevel( WARNING ) << "  (NetworkServer::ProcessUnhandledPacket) server could not add a new client!" << endl;

                    } 

                }
                break;

                    // request for sending a list of exiting clients on server
                case CTD_NM_NETWORK_CLIENT_REQ_CLIENTLIST:
                {

                    if ( TransmitClientList( rkNodeID ) == false ) {

                        // something went wrong!
                        neolog << LogLevel( WARNING ) << "nw-server: error transmitting client list" << endl;

                    }

                }
                break;
            
                //--------------------------------------------------------------//
                case CTD_NM_NETWORK_SERVER_ACK_CLIENTOBJECT_ADDED:
                {

                    // this message can only be received by clients
                    neolog << LogLevel( WARNING ) << "  (NetworkServer::ProcessUnhandledPacket) this message must not be received by server but by clients!" << endl;
                
                }
                break;

                //--------------------------------------------------------------//
                //               initialization packets                         //
                //--------------------------------------------------------------//

                // broadcast this message to all clients and initialize server's remote client if not already done as
                //  this message is sent by every new connected client and broadcasted to all exisiting clients
                case CTD_NM_NETWORK_CLIENT_RECV_REMOTE_CLIENT_INITIALIZION_DATA:
                {

                    // NOTE: here some checks may be done on this packet before broatcasting it

                    tCTD_NM_NETWORK_CLIENT_RECV_REMOTE_CLIENT_INITIALIZION_DATA     *pkInitPacket = ( tCTD_NM_NETWORK_CLIENT_RECV_REMOTE_CLIENT_INITIALIZION_DATA* )pcPacketData;
                    Send( 
                        ( char* )pkInitPacket, 
                        pkInitPacket->m_lDataLength,
                        HIGH_PRIORITY,
                        RELIABLE_SEQUENCED, 
                        0, 
                        UNASSIGNED_PLAYER_ID,
                        true, 
                        true
                    );
                    
                    neolog << LogLevel( INFO ) << "nw-server: got client initialization data, network id: " << pkInitPacket->m_usNetworkID << endl;

                    // initialize remote client object on server
                    //------------------------------------------------------------------------------//

                    unsigned short  usNetworkID = pkInitPacket->m_usNetworkID;
                    NetworkObject   *pkNetworkObject = GET_OBJECT_FROM_ID( usNetworkID );

                    // this init data can be only for a remote client
                    assert( pkNetworkObject != NULL );
                    assert( pkNetworkObject->GetEntity()->GetNetworkingType() == stateREMOTE_CLIENTOBJECT );

                    // skip already initialized remote clients
                    if ( pkNetworkObject->m_bAlreadyInitialized == true ) {

                        break;

                    }

                    neolog << LogLevel( INFO ) << "nw-server:   initialize remote client object ..." << endl;

                    // send a system message conta hining initialization data to entity
                    pkInitPacket->m_ucDataType = CTD_NM_CMD_RECEIVE_INITIALIZION_DATA;
                    pkNetworkObject->GetEntity()->NetworkMessage( CTD_NM_NETWORK_SYSTEM, pkInitPacket );
                    pkNetworkObject->m_bAlreadyInitialized = true;

                    neolog << LogLevel( INFO ) << "nw-server:   initialization completed" << endl;

                    // here the integration negotiation is finish, now we can accept further clients
                    m_kPendingClient.m_bPending     = false;

                }
                break;

                default:
                    neolog << LogLevel( INFO ) << "nw-server: network management - unknown client object packet identifier received " << endl;
                    break;
        
            }
        }
        break;

        //-------------------------//       
        default:

            neolog << LogLevel( WARNING ) << "  (NetworkServer::ProcessUnhandledPacket) server received an undefined packet identifier: " << ucPacketIdentifier << endl;            
            break;

    }

}

// this is called if a new connected client requests for list of existing clients
bool NetworkServer::TransmitClientList( const PlayerID &rkNodeID )
{

    // prepare transmission of client list to requesting client
    //***************************************************************************************//

    bool bSuccess = false;

    // sent start of transmission signal

    tCTD_NM_SERVER_CMD_START_ADDING_REMOTE_CLIENTOBJECTS    kStartSequence;
    kStartSequence.m_usPacketIdentifier     = CTD_NM_NETWORK_SYSTEM;
    kStartSequence.m_ucDataType             = CTD_NM_NETWORK_SERVER_CMD_START_ADDING_REMOTE_CLIENTOBJECTS;
    kStartSequence.m_usCountOfClients       = ( unsigned short )m_pvpkClientList->size();
    bSuccess = Send( 
        ( char* )&kStartSequence, 
        sizeof( tCTD_NM_SERVER_CMD_START_ADDING_REMOTE_CLIENTOBJECTS ),
        HIGH_PRIORITY,
        RELIABLE_SEQUENCED, 
        0, 
        rkNodeID, 
        false, 
        true
        );

    if ( bSuccess == false ) {

        return false;

    }

    // now sent the list of existing client objects to requesting client
    size_t uiClients = m_pvpkClientList->size();
    tCTD_NM_SERVER_CMD_ADD_REMOTE_CLIENTOBJECT        kCommandAddRemoteClient;
    kCommandAddRemoteClient.m_usPacketIdentifier    = CTD_NM_NETWORK_SYSTEM;
    kCommandAddRemoteClient.m_ucDataType            = CTD_NM_NETWORK_SERVER_CMD_ADD_REMOTE_CLIENTOBJECT;
    for ( size_t uiClientCnt = 0; uiClientCnt < uiClients; uiClientCnt++ ) {
        
        strcpy( kCommandAddRemoteClient.m_pcEntityName, ( *m_pvpkClientList )[ uiClientCnt ].m_strEntityName.c_str() );
        kCommandAddRemoteClient.m_kNodeID               = ( *m_pvpkClientList )[ uiClientCnt ].m_kNodeID;
        kCommandAddRemoteClient.m_usNetworkID           = ( *m_pvpkClientList )[ uiClientCnt ].m_usNetworkID;
        strcpy( kCommandAddRemoteClient.m_pcNetworkNodeName, ( *m_pvpkClientList )[ uiClientCnt ].m_strClientName.c_str() );

        bSuccess = Send( 
            ( char* )&kCommandAddRemoteClient, 
            sizeof( tCTD_NM_SERVER_CMD_ADD_REMOTE_CLIENTOBJECT ),
            HIGH_PRIORITY,
            RELIABLE_SEQUENCED, 
            0, 
            rkNodeID, 
            false, 
            true
            );

        if ( bSuccess == false ) {

            return false;

        }

        neolog << LogLevel( INFO ) << "nw-server:  sending request to new client for adding remote client with id: " << kCommandAddRemoteClient.m_usNetworkID << endl;

    }

    // send end of transmission signal
    tCTD_NM_SERVER_CMD_END_ADDING_REMOTE_CLIENTOBJECTS  kEndSequence;
    kEndSequence.m_usPacketIdentifier       = CTD_NM_NETWORK_SYSTEM;
    kEndSequence.m_ucDataType               = CTD_NM_NETWORK_SERVER_CMD_END_ADDING_REMOTE_CLIENTOBJECTS;
    bSuccess = Send( 
        ( char* )&kEndSequence, 
        sizeof( tCTD_NM_SERVER_CMD_END_ADDING_REMOTE_CLIENTOBJECTS ),
        HIGH_PRIORITY,
        RELIABLE_SEQUENCED, 
        0, 
        rkNodeID, 
        false, 
        true
        );

    if ( bSuccess == false ) {

        return false;

    }

    return true;

}


// this is called if a new connected client requests for adding a client object such as player
bool NetworkServer::AddClientObject( const string &strEntityName, const PlayerID &rkNodeID, const string &strNetworkNodeName )
{
    
    // try to find the client entity
    EntityDescriptor        *pkDesc = m_pkFrameworkImpl->GetCurrentLevelSet()->GetPluginManager()->FindEntityDescriptor( strEntityName );
    if ( pkDesc == NULL ) {

        neolog << LogLevel( WARNING ) << "nw-server: entity representing remote client cannot be found, client disconnected!" << endl;
        
        // kick client!
        Kick( rkNodeID );
        return false;
    }

    // create a server object for new connected client, don't let NetworkObject automatically create an id and add it into search tree
    NetworkObject   *pkNewClientObject = new NetworkObject( false );

    // set object id for new client
    unsigned short usNetworkID = CreateNewNetworkID();
    pkNewClientObject->SetID( usNetworkID );

    // create and setup a new entity
    BaseEntity  *pkEntity = ( BaseEntity* )pkDesc->CreateEntityInstance();
    pkNewClientObject->SetEntity( pkEntity );
    // server client object have the type remote client11
    pkEntity->SetNetworkingType( stateREMOTE_CLIENTOBJECT );
    pkEntity->SetNetworkID( usNetworkID );
    pkEntity->Initialize();
    pkEntity->PostInitialize();
    m_pkFrameworkImpl->GetCurrentLevelSet()->GetRoom()->AttachNode( ( SceneNode* )pkEntity );

    // send a message to new client notifiying it that it is added to server
    //  now the client can create a local network object with here given network id
    tCTD_NM_SERVER_CLIENTOBJECT_ADDED       kNewClientPacket;
    kNewClientPacket.m_usPacketIdentifier   = CTD_NM_NETWORK_SYSTEM;
    kNewClientPacket.m_ucDataType           = CTD_NM_NETWORK_SERVER_ACK_CLIENTOBJECT_ADDED;
    kNewClientPacket.m_usNetworkID          = pkNewClientObject->GetID();
    Send( 
        ( char* )&kNewClientPacket, 
        sizeof( tCTD_NM_SERVER_CLIENTOBJECT_ADDED ),
        HIGH_PRIORITY,
        RELIABLE_SEQUENCED, 
        0, 
        rkNodeID,
        false, 
        true
    );

    // now send a message to all existing clients notifiying them that a new client is added to server
    //  all existing clients must add a ghost representing this new client
    size_t uiClients = m_pvpkClientList->size();
    tCTD_NM_SERVER_ADD_NEW_CLIENTOBJECT         kAddRemoteClientPacket;
    kAddRemoteClientPacket.m_usPacketIdentifier = CTD_NM_NETWORK_SYSTEM;
    kAddRemoteClientPacket.m_ucDataType         = CTD_NM_NETWORK_SERVER_CMD_ADD_NEW_CLIENTOBJECT;
    kAddRemoteClientPacket.m_usNetworkID        = pkNewClientObject->GetID();
    kAddRemoteClientPacket.m_kNodeID            = rkNodeID;
    strcpy( kAddRemoteClientPacket.m_pcEntityName, pkEntity->GetName().c_str() );
    strcpy( kAddRemoteClientPacket.m_pcNetworkNodeName, strNetworkNodeName.c_str() );
    for ( size_t uiClientCnt = 0; uiClientCnt < uiClients; uiClientCnt++ ) {
        
        Send( 
            ( char* )&kAddRemoteClientPacket, 
            sizeof( tCTD_NM_SERVER_ADD_NEW_CLIENTOBJECT ),
            HIGH_PRIORITY,
            RELIABLE_SEQUENCED, 
            0, 
            ( *m_pvpkClientList )[ uiClientCnt ].m_kNodeID, 
            false, 
            true
            );

    }

    // now command all client objects in net to send their initialization data via a broatcast packet
    tCTD_NM_NETWORK_SERVER_CMD_SEND_REMOTE_CLIENT_INITIALIZION_DATA     kCmdSendInitPacket;
    kCmdSendInitPacket.m_usPacketIdentifier                             = CTD_NM_NETWORK_SYSTEM;
    kCmdSendInitPacket.m_usNetworkID                                    = pkNewClientObject->GetID();
    kCmdSendInitPacket.m_ucDataType                                     = CTD_NM_NETWORK_SERVER_CMD_SEND_REMOTE_CLIENT_INITIALIZION_DATA;
    Send( 
        ( char* )&kCmdSendInitPacket, 
        sizeof( tCTD_NM_NETWORK_SERVER_CMD_SEND_REMOTE_CLIENT_INITIALIZION_DATA ),
        HIGH_PRIORITY,
        RELIABLE_SEQUENCED, 
        0, 
        UNASSIGNED_PLAYER_ID,
        true, 
        true
    );


    //*************************************************************************************//

    // after notifiying all other clients add the new client into internal list
    //  this list holds all connected clients
    //--------------------------------------------------------------------------//
    assert( ( m_pvpkClientList != NULL ) && "  (NetworkServer::AddClientObject) set server's client list pointer on initialization!" );
    ClientNode                      kClientNode;

    // take the pending client into internal client list
    kClientNode.m_kNodeID           = rkNodeID;
    kClientNode.m_usNetworkID       = pkNewClientObject->GetID();
    kClientNode.m_strClientName     = m_kPendingClient.m_strNetworkNodeName;
    kClientNode.m_strEntityName     = strEntityName;
    kClientNode.m_pkEntity          = pkEntity;
    m_pvpkClientList->push_back( kClientNode );
    //m_kPendingClient.m_bPending       = false;

    neolog << LogLevel( INFO ) << "nw-server:  new client object added, network id: " << pkNewClientObject->GetID() << endl;
    neolog << LogLevel( INFO ) << "              client name: " << kClientNode.m_strClientName << endl;
    neolog << LogLevel( INFO ) << "              object name: " << kClientNode.m_strEntityName << endl;
    neolog << LogLevel( INFO ) << "            total num of clients: " << ( unsigned int )m_pvpkClientList->size() << endl;

    return true;

}

// remove a client from internal list and destroy its entities
void NetworkServer::RemoveClient( const PlayerID &rkNodeID )
{

    vector< ClientNode >::iterator  pkListElem = m_pvpkClientList->begin();
    vector< ClientNode >::iterator  pkListEnd  = m_pvpkClientList->end();

    for ( ; pkListElem != pkListEnd; pkListElem++ ) {

        if ( pkListElem->m_kNodeID == rkNodeID ) {

            NetworkObject   *pkNetworkObject = GET_OBJECT_FROM_ID( pkListElem->m_usNetworkID );

            if ( pkNetworkObject == NULL ) {
                
                neolog << LogLevel( WARNING ) << "(NetworkServer::RemoveClient) attemp to remove a non-existing network object!" << endl;
                return;

            }

            BaseEntity   *pkEntity = pkNetworkObject->GetEntity();
            m_pkFrameworkImpl->GetCurrentLevelSet()->GetRoom()->DetachNode( ( SceneNode* )pkEntity );
            //pkEntity->DetachFromParent();


            neolog << LogLevel( INFO ) << "nw-server: removing client object ' " 
                << pkEntity->GetName() << " belonging to client ' " 
                << pkListElem->m_strClientName << " '" << endl;


            delete pkNetworkObject;
            delete pkEntity;
            m_pvpkClientList->erase( pkListElem );

        }

    }

    neolog << LogLevel( INFO ) << "nw-server: total num of remaining clients: " << ( unsigned int )m_pvpkClientList->size() << endl;

}

// this funtion created a unique network id for client objects which are requested by clients
unsigned short NetworkServer::CreateNewNetworkID()
{
    
    unsigned short usNetworkID = CTD_NETWORKID_CLIENT_OBJECTS_RANGE_BEGIN;

    vector< ClientNode >::iterator  pkListElem = m_pvpkClientList->begin();
    vector< ClientNode >::iterator  pkListEnd  = m_pvpkClientList->end();

    // search for first free id slot
    for ( ; pkListElem != pkListEnd; pkListElem++, usNetworkID++ ) {

        if ( pkListElem->m_usNetworkID != usNetworkID ) {

            break;

        }

    }

    return usNetworkID;
}

} // namespace CTD
