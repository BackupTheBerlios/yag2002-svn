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
 # this core uses RakNet and provides network client services
 #
 #
 #   date of creation:  04/23/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/


#include <ctd_frbase.h>
#include "ctd_network_client.h"
#include <ctd_network.h>
#include <ctd_framework_impl.h>
#include <ctd_levelmanager.h>

#include <ctd_descriptor.h>

#include "raknet_NetworkObject.h"

#include <string>
#include <time.h>

using namespace std;
using namespace NeoEngine;

namespace CTD
{

NetworkClient::NetworkClient( FrameworkImpl *pkFrameworkImpl, tStaticDataClient *pkStaticData, vector< ClientNode > *pvpkClientList ) 
{ 

    m_pkFrameworkImpl           = pkFrameworkImpl;
    m_pkStaticData              = pkStaticData; 
    m_pvpkClientList            = pvpkClientList;
    m_pkQueuedClientObject      = NULL;
    m_bConnectionEstablished    = false; 
    m_bReady                    = false;
    m_bClientObjectCreated      = false; 
    m_bCListTransInitiated      = false;
    m_uiCListCount              = 0;

}

NetworkClient::~NetworkClient() 
{
}


void NetworkClient::Update()
{

    // dispatch received packets
    ProcessPackets( this ); 


    // check timeout for receiving client list on connection
    if ( ( m_bCListTransInitiated == true ) && ( m_kCListTransTimer.GetDeltaTime( false ) > CTD_CLIENT_GET_REMOTE_CLIENTOBJECTS_TIMEOUT ) ) {

        m_bCListTransInitiated      = false;
        m_bConnectionEstablished    = false;

        // delete all network objects and enities
        NetworkObject           *pkObject;
        BaseEntity          *pkEntity;
        size_t uiClients = m_pvpkClientList->size();
        for ( size_t uiClientCnt = 0; uiClientCnt < uiClients; uiClientCnt++ ) {

            pkObject = GET_OBJECT_FROM_ID( ( *m_pvpkClientList )[ uiClientCnt ].m_usNetworkID );
            assert( pkObject != NULL && " (NetworkClient::Update()): error deleting network object" );

            pkEntity = pkObject->GetEntity();
            delete pkEntity;
            delete pkObject;

        }
        m_pvpkClientList->empty();

        // disconnect from server
        neolog << LogLevel( INFO ) << "nw-client: time out receiving remote clients, disconnecting from server"  << endl;
        Disconnect();

    }

}

void NetworkClient::Shutdown()
{ 
        
    Disconnect();

}

void NetworkClient::ReceiveRemoteDisconnectionNotification(Packet *pkPacket,RakClientInterface *pkClientInterface)
{

    neolog << LogLevel( INFO ) << "nw-client: remote client disconnected from server ( " 
        <<  pkPacket->playerId.binaryAddress 
        << "; port "
        << pkPacket->playerId.port
        << " )" << endl;

    RemoveClient( pkPacket->playerId );

}


void NetworkClient::ReceiveRemoteConnectionLost(Packet *pkPacket,RakClientInterface *pkClientInterface)
{

    // print a time stamp
    time_t kTime;
    time( &kTime );
    neolog << LogLevel( INFO ) << "time: " << ctime( &kTime ) << endl;

    neolog << LogLevel( INFO ) << "nw-client: lost remote connection to server ( " 
        <<  pkPacket->playerId.binaryAddress 
        << "; port "
        << pkPacket->playerId.port
        << " )" << endl;

    RemoveClient( pkPacket->playerId );


}

void NetworkClient::ReceiveConnectionRequestAccepted(Packet *pkPacket,RakClientInterface *pkClientInterface)
{
    // print a time stamp
    time_t kTime;
    time( &kTime );
    neolog << LogLevel( INFO ) << "time: " << ctime( &kTime ) << endl;

    neolog << LogLevel( INFO ) << "nw-client: successfully connected to server ( " 
        <<  pkPacket->playerId.binaryAddress 
        << "; port "
        << pkPacket->playerId.port
        << " )" << endl;
    neolog << LogLevel( INFO ) << "nw-client: waiting for server information" << endl; 

}

void NetworkClient::ReceiveRemoteNewIncomingConnection(Packet *pkPacket,RakClientInterface *pkClientInterface)
{
    
    neolog << LogLevel( INFO ) << "nw-client: new remote client connecting ( " 
        <<  pkPacket->playerId.binaryAddress 
        << "; port "
        << pkPacket->playerId.port
        << " )." << endl;   

}


void NetworkClient::ReceiveConnectionResumption(Packet *pkPacket,RakClientInterface *pkClientInterface)
{

    // it is up to the plugins to react to this state!
    m_pkFrameworkImpl->SendPluginMessage( CTD_NM_SYSTEM_NOTIFY_CONNECTION_RESUMED, ( void* )NULL );
    m_pkFrameworkImpl->SendEntityMessage( CTD_NM_SYSTEM_NOTIFY_CONNECTION_RESUMED, ( void* )NULL, "", true ); // empty plugin name means a message broadcast

}


void NetworkClient::ReceiveNoFreeIncomingConnections(Packet *pkPacket,RakClientInterface *pkClientInterface)
{

    // FIXME: handle this event!

}

void NetworkClient::ReceiveDisconnectionNotification(Packet *pkPacket,RakClientInterface *pkClientInterface)
{

    // print a time stamp
    time_t kTime;
    time( &kTime );
    neolog << LogLevel( INFO ) << "time: " << ctime( &kTime ) << endl;

    neolog << LogLevel( INFO ) << "nw-client: we have been disconnected ( " 
        <<  pkPacket->playerId.binaryAddress 
        << "; port "
        << pkPacket->playerId.port
        << " )" << endl;    

    // it is up to the plugins to react on this state!
    m_pkFrameworkImpl->SendPluginMessage( CTD_NM_SYSTEM_NOTIFY_DISCONNECTED, ( void* )NULL );
    m_pkFrameworkImpl->SendEntityMessage( CTD_NM_SYSTEM_NOTIFY_DISCONNECTED, ( void* )NULL, "", true ); // empty plugin name means a message broadcast

}


void NetworkClient::ReceiveConnectionLost(Packet *pkPacket,RakClientInterface *pkClientInterface)
{

    // print a time stamp
    time_t kTime;
    time( &kTime );
    neolog << LogLevel( INFO ) << "time: " << ctime( &kTime ) << endl;

    neolog << LogLevel( INFO ) << "nw-client: we have lost connection to server ( " 
        <<  pkPacket->playerId.binaryAddress 
        << "; port "
        << pkPacket->playerId.port
        << " )" << endl;    

    // it is up to the plugins to react on this state!
    m_pkFrameworkImpl->SendPluginMessage( CTD_NM_SYSTEM_NOTIFY_CONNECTION_LOST, ( void* )NULL );
    m_pkFrameworkImpl->SendEntityMessage( CTD_NM_SYSTEM_NOTIFY_CONNECTION_LOST, ( void* )NULL, "", true ); // empty plugin name means a message broadcast

}


void NetworkClient::ReceivedStaticData(Packet *pkPacket,RakClientInterface *pkClientInterface)
{

    neolog << LogLevel( INFO ) << "nw-client: static data received ( " 
        <<  pkPacket->playerId.binaryAddress 
        << "; port "
        << pkPacket->playerId.port
        << " )" << endl;    

    // get the static data containing information about the server state
    tStaticDataServer   *pkData = ( tStaticDataServer* )( ( char* )( pkPacket->data + sizeof(unsigned char) ) );

    // store the level name currently running on server
    pkData->m_pcLevelName[ 255 ] = '\0';
    strcpy( m_pkStaticData->m_pcLevelName, pkData->m_pcLevelName );

    neolog << LogLevel( INFO ) << "   server name: " << pkData->m_pcNetworkNodeName << endl;
    neolog << LogLevel( INFO ) << "   level name: " << pkData->m_pcLevelName << endl;

    // signalize to network device that a connection is successfully established
    //  now the negotiation of simulation state follows.
    m_bConnectionEstablished = true;

}


void NetworkClient::ReceiveInvalidPassword(Packet *pkPacket,RakClientInterface *pkClientInterface)
{

}


void NetworkClient::ReceiveModifiedPacket(Packet *pkPacket,RakClientInterface *pkClientInterface)
{

    neolog << LogLevel( INFO ) << "nw-client: got modified packet ( " 
        <<  pkPacket->playerId.binaryAddress 
        << "; port "
        << pkPacket->playerId.port
        << " ) disconnecting due to security issues." << endl;

    Disconnect();
    RemoveClient( pkPacket->playerId );

}


void NetworkClient::ReceiveRemotePortRefused(Packet *pkPacket,RakClientInterface *pkClientInterface)
{

    neolog << LogLevel( INFO ) << "nw-client: server port refused ( " 
        <<  pkPacket->playerId.binaryAddress 
        << "; port "
        << pkPacket->playerId.port
        << " ) left the session." << endl;  

}

void NetworkClient::ReceiveVoicePacket(Packet *pkPacket,RakClientInterface *pkClientInterface)
{

}


void NetworkClient::ProcessUnhandledPacket( char *pcPacketData, unsigned char ucPacketIdentifier, const PlayerID &pkNodeID, RakClientInterface *pkClientInterface )
{

    if ( m_bConnectionEstablished == false ) {

        return;

    }

    switch( ucPacketIdentifier ) {

        //-------------------------//
        case CTD_NM_NETWORK_ENTITY:
        {

            // don't receiver entity messages if we are still negotiating with server
            if ( m_bReady == false ) {

                return;

            }

            tCTDEntityPacketHeader  *pkPacketHeader  = ( tCTDEntityPacketHeader* )pcPacketData;

            unsigned short  usNetworkID = pkPacketHeader->m_usNetworkID;

            NetworkObject   *pkNetworkObject = GET_OBJECT_FROM_ID( usNetworkID );

            // check for valid ids!
            if ( pkNetworkObject == NULL ) {

                neolog << LogLevel( WARNING ) << "  (NetworkClient::ProcessUnhandledPacket) client received an undefined object id: " << usNetworkID << endl;
                return;
            }

            // send the message to entity
            pkNetworkObject->GetEntity()->NetworkMessage( CTD_NM_NETWORK_ENTITY, pkPacketHeader );

        }
        break;

        // client object related messages
        //################################################################################################//
        case CTD_NM_NETWORK_SYSTEM:
        {

            tCTDNetworkMgrPacketHeader  *pkPacketHeader  = ( tCTDNetworkMgrPacketHeader* )pcPacketData;
            unsigned char  ucDataType   = pkPacketHeader->m_ucDataType;
            switch ( ucDataType ) {

                //--------------------------------------------------------------//
                case CTD_NM_NETWORK_SERVER_CMD_START_ADDING_REMOTE_CLIENTOBJECTS:
                {

                    tCTD_NM_SERVER_CMD_START_ADDING_REMOTE_CLIENTOBJECTS    *pkCommandStartAddingRemoteClients = ( tCTD_NM_SERVER_CMD_START_ADDING_REMOTE_CLIENTOBJECTS* )pcPacketData;

                    // reset a timer and await the client objects in following packtets
                    //  the end of client list is signalized with CTD_NM_NETWORK_SERVER_CMD_END_ADDING_REMOTE_CLIENTOBJECTS
                    //  but check also the timer! we don't want to get stucked at this point if something with transmision goes wrong ( server crash, etc. )
                    m_bCListTransInitiated  = true;
                    m_uiCListCount          = pkCommandStartAddingRemoteClients->m_usCountOfClients;
                    m_kCListTransTimer.Reset();
                    neolog << LogLevel( INFO ) << "nw-client: receiving client list ... " <<  endl;


                }
                break;
                //--------------------------------------------------------------//
                //  this message means for non-requesting clients that they have to add a remote client object ( a ghost )
                case CTD_NM_NETWORK_SERVER_CMD_ADD_REMOTE_CLIENTOBJECT:
                {

                    tCTD_NM_SERVER_CMD_ADD_REMOTE_CLIENTOBJECT  *pkCommandAddRemoteClient = ( tCTD_NM_SERVER_CMD_ADD_REMOTE_CLIENTOBJECT* )pcPacketData;
                    pkCommandAddRemoteClient->m_pcNetworkNodeName[ 255 ] = '\0';
                    // add a ghost
                    if ( AddRemoteClientObject( 
                        pkCommandAddRemoteClient->m_pcEntityName, 
                        pkCommandAddRemoteClient->m_usNetworkID,
                        pkCommandAddRemoteClient->m_kNodeID,
                        pkCommandAddRemoteClient->m_pcNetworkNodeName   ) == false ) {

                        // something went wrong, our requested client object cannot be locally created !?
                        neolog << LogLevel( WARNING ) << "nw-client:  requested remote client object with following id could not be locally created: " << pkCommandAddRemoteClient->m_usNetworkID << endl;

                    } else {

                        neolog << LogLevel( INFO ) << "nw-client: remote client object created with id: " << pkCommandAddRemoteClient->m_usNetworkID << endl;
                        m_uiCListCount--;
                    }
                }
                break;

                //--------------------------------------------------------------//
                case CTD_NM_NETWORK_SERVER_CMD_END_ADDING_REMOTE_CLIENTOBJECTS:
                {

                    // reset the identifier for end of client list reception
                    m_bCListTransInitiated = false;
                    if ( m_uiCListCount > 0 ) {

                        neolog << LogLevel( WARNING ) << "nw-client: warning: wrong count of remote clients received" << endl;

                    } else {

                        neolog << LogLevel( INFO ) << "nw-client: end of client list" <<  endl;
                    
                    }

                }
                break;
                //--------------------------------------------------------------//
                case CTD_NM_NETWORK_CLIENT_REQ_ADD_CLIENTOBJECT:
                {

                    // this message can only be received by server
                    neolog << LogLevel( WARNING ) << "  (NetworkClient::ProcessUnhandledPacket) this message must not be received by clients but by server!" << endl;

                }
                break;

                case CTD_NM_NETWORK_SERVER_ACK_CLIENTOBJECT_ADDED:
                {

                    tCTD_NM_SERVER_CLIENTOBJECT_ADDED   *pkAcknowledgePacket = ( tCTD_NM_SERVER_CLIENTOBJECT_ADDED* )pcPacketData;

                    // only one request after the other; did we request for adding a client object at all!?
                    if ( m_bClientObjectCreated == true ) {
                        
                        neolog << LogLevel( WARNING ) << "nw-client: client got an unrequested acknowledge from server for adding a client object! " << endl;

                    } else {

                        // set local client object's network id, etc.
                        assert ( ( m_pkQueuedClientObject != NULL ) && " (NetworkClient::ProcessUnhandledPacket) no queued client object entity to set network id for!" );
                        m_pkQueuedClientObject->SetID( pkAcknowledgePacket->m_usNetworkID );
                        m_pkQueuedClientObject->GetEntity()->SetNetworkID( pkAcknowledgePacket->m_usNetworkID );
                        neolog << LogLevel( INFO ) << "nw-client: we have been integrated into network with id: " << pkAcknowledgePacket->m_usNetworkID << endl;

                        // notify network device about successfull creation of requested client object on server
                        m_bClientObjectCreated = true;
                    }                   

                }
                break;
                //--------------------------------------------------------------//
                case CTD_NM_NETWORK_SERVER_CMD_ADD_NEW_CLIENTOBJECT:
                {

                    // this message arrives at exisiting clients when a new client successfully connected
                    tCTD_NM_SERVER_ADD_NEW_CLIENTOBJECT *pkCmdPacket = ( tCTD_NM_SERVER_ADD_NEW_CLIENTOBJECT* )pcPacketData;
                    pkCmdPacket->m_pcNetworkNodeName[ 255 ] = '\0';
                    pkCmdPacket->m_pcEntityName[ 127 ] = '\0'; 
                    
                    neolog << LogLevel( WARNING ) << "nw-client: trying to add a new remote client ... " << endl;
                    // add a ghost
                    if ( AddRemoteClientObject( 
                        pkCmdPacket->m_pcEntityName, 
                        pkCmdPacket->m_usNetworkID,
                        pkCmdPacket->m_kNodeID,
                        pkCmdPacket->m_pcNetworkNodeName ) == false ) {

                        // something went wrong, our requested client object cannot be locally created !?
                        neolog << LogLevel( WARNING ) << "nw-client:  error:  requested remote client object with following id could not be locally created: " << pkCmdPacket->m_usNetworkID << endl;

                    } else {

                        neolog << LogLevel( INFO ) << "nw-client: a remote client object added to network with id: " << pkCmdPacket->m_usNetworkID << endl;

                    }

                }
                break;
                //--------------------------------------------------------------//
                // command to send client object's initialization data
                case CTD_NM_NETWORK_SERVER_CMD_SEND_REMOTE_CLIENT_INITIALIZION_DATA:
                {
                    
                    // send the message to entity and receive init data
                    tCTD_NM_NETWORK_CLIENT_RECV_REMOTE_CLIENT_INITIALIZION_DATA     kInitPacket;

                    // get init data from client entity
                    //--------------------------------//
                    // fill some defaults
                    kInitPacket.m_ucDataType            = CTD_NM_CMD_SEND_INITIALIZION_DATA;
                    kInitPacket.m_lDataLength           = sizeof( tCTD_NM_NETWORK_CLIENT_RECV_REMOTE_CLIENT_INITIALIZION_DATA );

                    // send a system message to entity forcing it to fill its init data into kInitPacket
                    m_pkFrameworkImpl->GetPlayer()->NetworkMessage( CTD_NM_NETWORK_SYSTEM, &kInitPacket );

                    kInitPacket.m_usPacketIdentifier    = CTD_NM_NETWORK_SYSTEM;
                    kInitPacket.m_ucDataType            = CTD_NM_NETWORK_CLIENT_RECV_REMOTE_CLIENT_INITIALIZION_DATA;
                    kInitPacket.m_usNetworkID           = m_pkFrameworkImpl->GetPlayer()->GetNetworkID();
                    //--------------------------------//

                    // send initialization data to server, the server will broadcast this to all clients
                    Send(       
                        ( char* )&kInitPacket, 
                        kInitPacket.m_lDataLength,
                        HIGH_PRIORITY,
                        RELIABLE_SEQUENCED, 
                        0
                        );
        
                    neolog << LogLevel( INFO ) << "nw-client: send initialization data to server ... " << endl;


                }
                break;

                // remote client object's initialization data received from server
                case CTD_NM_NETWORK_CLIENT_RECV_REMOTE_CLIENT_INITIALIZION_DATA:
                {

                    // got remote client init data, set it into local ghost
                    
                    tCTD_NM_NETWORK_CLIENT_RECV_REMOTE_CLIENT_INITIALIZION_DATA     *pkInitPacket = ( tCTD_NM_NETWORK_CLIENT_RECV_REMOTE_CLIENT_INITIALIZION_DATA* )pcPacketData;

                    unsigned short  usNetworkID = pkInitPacket->m_usNetworkID;

                    //  cancel echos
                    if ( usNetworkID == m_pkFrameworkImpl->GetPlayer()->GetNetworkID() ) {

                        //neolog << LogLevel( DEBUG ) << "nw-client: received echo of initialization data " << endl;
                        break;

                    }

                    // do not initialize already exisiting local remote clients
                    vector< ClientNode >::iterator  pkListElem = m_pvpkClientList->begin();
                    vector< ClientNode >::iterator  pkListEnd  = m_pvpkClientList->end();
                    for ( ; pkListElem != pkListEnd; pkListElem++ ) {

                        if ( pkListElem->m_usNetworkID == usNetworkID ) {

                            break;

                        }

                    }

                    NetworkObject   *pkNetworkObject = GET_OBJECT_FROM_ID( usNetworkID );

                    // check for valid ids!
                    if ( pkNetworkObject == NULL ) {

                        neolog << LogLevel( WARNING ) << "  (NetworkClient::ProcessUnhandledPacket) client received initialization data for an undefined object id: " << usNetworkID << endl;
                        return;
                    }


                    // this init data can be only for a remote client
                    assert( pkNetworkObject->GetEntity()->GetNetworkingType() == stateREMOTE_CLIENTOBJECT );

                    neolog << LogLevel( INFO ) << "nw-client: getting initialization data for remote client: '" << 
                        pkNetworkObject->GetEntity()->GetName() << 
                        "' ( network id: " << usNetworkID << " ) " << endl;

                    // send a system message containing initialization data to entity
                    pkInitPacket->m_ucDataType = CTD_NM_CMD_RECEIVE_INITIALIZION_DATA;
                    pkNetworkObject->GetEntity()->NetworkMessage( CTD_NM_NETWORK_SYSTEM, pkInitPacket );

                }
                break;
                //--------------------------------------------------------------//
                default:
                {
                    neolog << LogLevel( INFO ) << "nw-client: network management - unknown client object packet identifier received " << endl;
                    break;
                }
            }
                
        }
        break;
        //################################################################################################//

        //-------------------------//
        default:

            neolog << LogLevel( WARNING ) << "  (NetworkClient::ProcessUnhandledPacket) client received an undefined packet identifier: " << ucPacketIdentifier << endl;            
            break;

    }

}

bool NetworkClient::AddRemoteClientObject( const string &strEntityName, unsigned short usNetworkID, const PlayerID &kNodeID, const string &strNetworkNodeName )
{

    // try to find the client entity
    EntityDescriptor        *pkDesc = m_pkFrameworkImpl->GetCurrentLevelSet()->GetPluginManager()->FindEntityDescriptor( strEntityName );
    if ( pkDesc == NULL ) {

        neolog << LogLevel( WARNING ) << "nw-client: entity representing remote client cannot be found, client disconnected!" << endl;
        return false;

    }

    // create a network object for new connected client, don't let NetworkObject automatically create an id and add it into search tree
    NetworkObject   *pkNewClientObject = new NetworkObject( false );

    // set object id for new client
    pkNewClientObject->SetID( usNetworkID );

    BaseEntity  *pkEntity = ( BaseEntity* )pkDesc->CreateEntityInstance();
    pkNewClientObject->SetEntity( pkEntity );
    pkEntity->SetNetworkID( usNetworkID );
    pkEntity->SetNetworkingType( stateREMOTE_CLIENTOBJECT );
    pkEntity->Initialize();
    pkEntity->PostInitialize();
    m_pkFrameworkImpl->GetCurrentLevelSet()->GetRoom()->AttachNode( ( SceneNode* )pkEntity );

    // add the client into internal list
    //  this list holds all connected clients
    //--------------------------------------------------------------------------//
    assert( ( m_pvpkClientList != NULL ) && "set server's client list pointer on initialization!" );
    ClientNode                      kClientNode;
    kClientNode.m_kNodeID           = kNodeID;
    kClientNode.m_usNetworkID       = pkNewClientObject->GetID();
    kClientNode.m_strClientName     = strNetworkNodeName;
    kClientNode.m_pkEntity          = pkEntity;
    m_pvpkClientList->push_back( kClientNode );

    neolog << LogLevel( INFO ) << "nw-client: remote client object ' " << pkEntity->GetName() << " ' created" << endl;
    neolog << LogLevel( INFO ) << "           total num of clients: " << ( unsigned int )m_pvpkClientList->size() << endl;

    return true;

}

// remove a client from internal list and destroy its entities
void NetworkClient::RemoveClient( const PlayerID &rkNodeID )
{

    vector< ClientNode >::iterator  pkListElem = m_pvpkClientList->begin();
    vector< ClientNode >::iterator  pkListEnd  = m_pvpkClientList->end();

    for ( ; pkListElem != pkListEnd; pkListElem++ ) {

        if ( pkListElem->m_kNodeID == rkNodeID ) {

            NetworkObject   *pkNetworkObject = GET_OBJECT_FROM_ID( pkListElem->m_usNetworkID );

            assert ( ( pkNetworkObject != NULL ) && "(NetworkServer::RemoveClient) attemp to remove a non-existing network object!" );

            BaseEntity   *pkEntity = pkNetworkObject->GetEntity();
            m_pkFrameworkImpl->GetCurrentLevelSet()->GetRoom()->DetachNode( ( SceneNode* )pkEntity );
            //pkEntity->DetachFromParent();

            neolog << LogLevel( INFO ) << "nw-client: removing client object ' " 
                << pkEntity->GetName() << " belonging to client ' " 
                << pkListElem->m_strClientName << " '" << endl;


            delete pkNetworkObject;
            delete pkEntity;
            m_pvpkClientList->erase( pkListElem );

        }

    }

    neolog << LogLevel( INFO ) << "nw-client: total num of remaining remote clients: " << ( unsigned int )m_pvpkClientList->size() << endl;

}

} // namespace CTD
