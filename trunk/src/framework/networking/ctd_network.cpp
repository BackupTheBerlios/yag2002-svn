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
 # this core uses RakNet and provides networking services
 #
 #
 #   date of creation:  04/08/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_frbase.h>
#include "ctd_network.h"

#include <ctd_framework_impl.h>
#include <ctd_descriptor.h>
#include <ctd_levelmanager.h>

#include "raknet_NetworkObject.h"

#include <string>

using namespace std;
using namespace NeoEngine;

namespace CTD
{

NetworkDevice::NetworkDevice( FrameworkImpl *pkFrameworkImpl ) 
{   

    m_pkServer                  = NULL;
    m_pkClient                  = NULL;

    assert( pkFrameworkImpl );

    m_pkFrameworkImpl           = pkFrameworkImpl;

    strcpy( m_kStaticDataServer.m_pcNetworkNodeName, "NoName Server" );
    strcpy( m_kStaticDataServer.m_pcLevelName, "No Level running" );
    strcpy( m_kStaticDataClient.m_pcNetworkNodeName, "NoName Client" );
    strcpy( m_kStaticDataClient.m_pcLevelName, "No Level running" );

    
    m_fClientMinUpdatePeriod    = CTD_CLIENT_UPDATE_PERIOD_THROTTLE;
    m_fPassedUpdateTime         = 0;


    m_bConnectionEstablished    = false;

    m_bServerStaticDataReady    = false;
    m_bClientStaticDataReady    = false;

}

NetworkDevice::~NetworkDevice()
{
    Shutdown();
}

void NetworkDevice::Shutdown()
{

    if ( m_pkServer ) {

        m_pkServer->Shutdown(); 
        delete m_pkServer;
        m_pkServer = NULL;

    }

    if ( m_pkClient ) {

        m_pkClient->Shutdown();
        delete m_pkClient;
        m_pkClient = NULL;

    }

}

void NetworkDevice::SetupServerStaticData( const string &strNodeName, const string &strLevelName )
{

    tStaticDataServer   *pkServerData = GetStaticDataServer();
    strcpy( pkServerData->m_pcNetworkNodeName, strNodeName.c_str() );
    strcpy( pkServerData->m_pcLevelName, strLevelName.c_str() );

    m_bServerStaticDataReady = true;

}

void NetworkDevice::SetupClientStaticData( const string &strNodeName )
{

    tStaticDataClient   *pkDataClient   = GetStaticDataClient();
    strcpy( pkDataClient->m_pcNetworkNodeName, strNodeName.c_str() );

    m_bClientStaticDataReady = true;

}

void NetworkDevice::Initialize( bool bServer )
{

    if ( bServer == true ) {

        assert( ( m_bServerStaticDataReady == true ) && "(NetworkDevice::Initialize) first setup server's static data!" );

        m_strNetworkNodeName        = m_kStaticDataServer.m_pcNetworkNodeName;

        // setup server object
        m_pkServer  = new NetworkServer( m_pkFrameworkImpl, &m_kStaticDataServer, &m_vstrClientList );

        // set static data for server
        m_pkServer->SetStaticServerData( (char*)&m_kStaticDataServer, sizeof( tStaticDataServer ) );


    } else {

        assert( ( m_bClientStaticDataReady == true ) && "(NetworkDevice::Initialize) first setup client's static data!" );

        m_strNetworkNodeName            = m_kStaticDataClient.m_pcNetworkNodeName;

        m_pkClient  = new NetworkClient( m_pkFrameworkImpl, &m_kStaticDataClient, &m_vstrClientList );

        m_kStaticDataClient.m_kNodeID   = m_pkClient->GetPlayerID();

        m_pkClient->SetStaticClientData( m_pkClient->GetPlayerID(), ( char* )&m_kStaticDataClient, sizeof( tStaticDataClient ) );

    }

}

bool NetworkDevice::StartServer( unsigned short usMaxNumberOfClients, bool bHightPriority, unsigned short usServerPort )
{

    assert( m_pkServer && "CTD network device: first initialize device before starting server!" );

    bool    bSuccess = m_pkServer->Start( usMaxNumberOfClients, 0, bHightPriority, usServerPort );
        
    return bSuccess;

}

bool NetworkDevice::ConnectClient( char *pcHost, unsigned short usServerPort, unsigned short usClientPort, bool bHighPriority )
{

    assert( m_pkClient && "CTD network device: first initialize device before connecting client!" );
    
    bool    bSuccess =  m_pkClient->Connect( pcHost, usServerPort, usClientPort, 0, bHighPriority );

    return bSuccess;

}

bool NetworkDevice::EstablishConnection()
{

    assert( m_pkClient != NULL && " client network is not set up! " );

    Timer   kTimer, kSecTimer;
    float fTime     = kTimer.GetDeltaTime( true );
    float fSecTime  = kSecTimer.GetDeltaTime( true );

    // wait for establishing client connection
    while ( m_pkClient->IsConnectionEstablished() == false ) {

        // call the client with 2 x minimum delta time to process the packets in every call
        UpdateClient( 2.0f * CTD_CLIENT_UPDATE_PERIOD_THROTTLE ); 

        fTime     = kTimer.GetDeltaTime( false );
        fSecTime  = kSecTimer.GetDeltaTime( false );

        if ( fSecTime > 1.0f ) {

            neolog << LogLevel( INFO ) << ".";
            fTime     = kSecTimer.GetDeltaTime( true );

        }

        // check for connection timeout
        if ( fTime > CTD_CLIENT_CONNECTION_TIMEOUT ) {

            neolog << LogLevel( INFO ) << " timeout" << endl;
            return false;

        }
    }

    neolog << LogLevel( INFO ) << "nw-device: successfully connected to server " << endl;

    return true;

}

// this function is called by LevelLoader and enables entities for networking if they request for it
//  by returning 'stateSERVEROBJECT' in function Entity::GetNetworkingType()
unsigned short NetworkDevice::AddServerObject( BaseEntity *pkEntity )
{

    // create a new server object and set its entity
    NetworkObject   *pkNetObject = new NetworkObject;
    pkNetObject->SetEntity( pkEntity );
    pkEntity->SetNetworkID( pkNetObject->GetID() );
    pkEntity->SetNetworkingType( stateSERVEROBJECT );
    m_pkFrameworkImpl->AddNetworkServerObject( pkEntity );

    return pkNetObject->GetID();

}

// this function is called to insert the network objects into room manager
void NetworkDevice::ActivateServerObjects()
{

    size_t uiEntities = m_pkFrameworkImpl->m_vpkNetworkServerObjects.size();
    for ( size_t uiEntityCnt = 0; uiEntityCnt < uiEntities; uiEntityCnt++ ) {

        BaseEntity  *pkEntity = m_pkFrameworkImpl->m_vpkNetworkServerObjects[ uiEntityCnt ];
        m_pkFrameworkImpl->GetCurrentLevelSet()->GetRoom()->AttachNode( ( SceneNode* )pkEntity );

    }

}

void NetworkDevice::AddClientObject( BaseEntity *pkEntity )
{

    assert( ( m_pkFrameworkImpl->m_eGameMode != stateSERVER ) && " (NetworkDevice::AddNetworkActor) do not try to add a client object in server mode!" );
    m_pkFrameworkImpl->AddNetworkClientObject( pkEntity );

}


//  create the network client objects in negotiation with server and add them into room manager
void NetworkDevice::ActivateClientObjects()
{

    size_t uiClientObjects = m_pkFrameworkImpl->m_vpkNetworkClientObjects.size();
    for ( size_t uiClientObjectCnt = 0; uiClientObjectCnt < uiClientObjects; uiClientObjectCnt++ ) {

        BaseEntity* pkEntity = m_pkFrameworkImpl->m_vpkNetworkClientObjects[ uiClientObjectCnt ];

        // we need the initialization to be able to send initialization data
        m_pkFrameworkImpl->GetCurrentLevelSet()->GetRoom()->AttachNode( ( SceneNode* )pkEntity );
        pkEntity->Initialize();

        // try to integrate the client object into network session
        unsigned int uiNumRetries = CTD_CLIENT_ADD_CLIENTOBJECT_RETRIES;
        while ( ( uiNumRetries > 0 ) && ( CreateClientObject( pkEntity ) == false ) ) {

            uiNumRetries--;
            neolog << LogLevel( WARNING ) << "nw-device: could not create client object on server, try again ..." << endl;

        }

        if ( uiNumRetries == 0 ) {

            m_pkFrameworkImpl->GetCurrentLevelSet()->GetRoom()->DetachNode( ( SceneNode* )pkEntity );

            // FIXME: this client must be also removed from client list!
            
            neolog << LogLevel( WARNING ) << "nw-device: cannot create client object on server, giving up!" << endl;

        } else {

            pkEntity->Activate();
            pkEntity->PostInitialize();

            // send connection notification to all entities
            neolog << LogLevel( INFO ) << "nw-device: sending notification to all entities... " << endl;
            m_pkFrameworkImpl->SendPluginMessage( CTD_NM_SYSTEM_NOTIFY_CONNECTION_ESTABLISHED, ( void* )NULL );
            m_pkFrameworkImpl->SendEntityMessage( CTD_NM_SYSTEM_NOTIFY_CONNECTION_ESTABLISHED, ( void* )NULL, "", true ); // empty plugin name means a message broadcast

        }
    }
}

// requests the server for list of already connected remote clients
bool NetworkDevice::GetRemoteClients() 
{
    
    neolog << LogLevel( INFO ) << "nw-device: getting remote client objects ..." << endl;

    tCTD_NM_NETWORK_CLIENT_REQ_CLIENTLIST       kRequestClientList;
    kRequestClientList.m_usPacketIdentifier     = CTD_NM_NETWORK_SYSTEM;
    kRequestClientList.m_ucDataType             = CTD_NM_NETWORK_CLIENT_REQ_CLIENTLIST;

    bool bSuccess = m_pkClient->Send( 
        ( char* )&kRequestClientList, 
        sizeof( tCTD_NM_NETWORK_CLIENT_REQ_CLIENTLIST ),
        HIGH_PRIORITY,
        RELIABLE_SEQUENCED, 
        0
    );

    if ( bSuccess == false ) {

        neolog << LogLevel( WARNING ) << "nw-device: error sending request for client list" << endl;

    }

    return bSuccess;
}


void NetworkDevice::RemoveServerObject( unsigned short usNetworkID )
{

}

// this function is called in client mode, it tries to add a client object to server and other clients
bool NetworkDevice::CreateClientObject( BaseEntity *pkActorkEntity )
{

    assert( m_pkClient && " (NetworkDevice::AddRemoteActor() you must first initialize and establish a connection to server!" );

    if ( m_pkClient->IsConnectionEstablished() == false ) {

        neolog << LogLevel( WARNING ) << "nw-device: error: request for adding a client object cannot be processed as no connection to server established!";
        return false;

    }

    if ( pkActorkEntity->GetName().length() == 0 ) {

        neolog << LogLevel( WARNING ) << "nw-device:  error: client object entity has no valid name, it cannot be added as client object to server and clients!";
        return false;

    }

    // avoid auto-creation of network id, this id will be received from server
    NetworkObject   *pkNetworkObject = new NetworkObject( false );
    pkNetworkObject->SetEntity( pkActorkEntity );
    // enqueue this new created server object so the client core will be able to set
    //  its network id when it is retrieved from server
    m_pkClient->EnqueueRequestingClientObject( pkNetworkObject );

    tCTD_NM_CLIENT_ADD_CLIENTOBJECT_REQEUST      kRequestAddClientObject;
    kRequestAddClientObject.m_usPacketIdentifier = CTD_NM_NETWORK_SYSTEM;
    kRequestAddClientObject.m_ucDataType         = CTD_NM_NETWORK_CLIENT_REQ_ADD_CLIENTOBJECT;
    strcpy( kRequestAddClientObject.m_pcEntityName, pkActorkEntity->GetName().c_str() );
    strcpy( kRequestAddClientObject.m_pcNetworkNodeName, GetNetworkNodeName().c_str() );
    bool bSuccess = m_pkClient->Send( 
        ( char* )&kRequestAddClientObject, 
        sizeof( tCTD_NM_CLIENT_ADD_CLIENTOBJECT_REQEUST ),
        HIGH_PRIORITY,
        RELIABLE_SEQUENCED, 
        0
    );

    if ( bSuccess == false ) {

        neolog << LogLevel( WARNING ) << "nw-device: error sending client object creation request" << endl;
        delete pkNetworkObject;
        return false;

    }


    neolog << LogLevel( INFO ) << "nw-device: requesting server for adding a client object ...." << endl;

    Timer   kTimer, kSecTimer;
    float fTime     = kTimer.GetDeltaTime( true );
    float fSecTime  = kSecTimer.GetDeltaTime( true );
    

    // wait until server created a client object or timeout occurs
    while ( m_pkClient->IsClientObjectCreated() == false ) {

        // call the client with 2 x minimum delta time to process the packets in every call
        UpdateClient( 2.0f * CTD_CLIENT_UPDATE_PERIOD_THROTTLE );

        fTime     = kTimer.GetDeltaTime( false );
        fSecTime  = kSecTimer.GetDeltaTime( false );

        if ( fSecTime > 1.0f ) {

            neolog << LogLevel( INFO ) << ".";
            fTime     = kSecTimer.GetDeltaTime( true );
        }

        // check for connection timeout
        if ( fTime > CTD_CLIENT_ADD_CLIENTOBJECT_TIMEOUT ) {

            neolog << LogLevel( INFO ) << " timeout" << endl;
            delete pkNetworkObject;
            m_pkClient->ResetClientObjectCreated();
            return false;

        }
    }

    m_pkClient->ResetClientObjectCreated();

    neolog << LogLevel( INFO ) << endl;

    return true;

}


// update funtions for server and client
void NetworkDevice::UpdateServer( float fDeltaTime ) 
{ 

    if ( !m_pkServer ) {

        return;

    }

    // check the timeout for pending client
    //  if during negotiation a client fails then the server will lock and accept no further clients! this timeout releases the pending status of server.
    if ( m_pkServer->m_kPendingClient.m_bPending == true ) {

        if ( m_pkServer->m_kPendingClient.m_pkPendingTimer.GetDeltaTime( false ) > CTD_PENDING_CLIENT_TIMEOUT ) {

            m_pkServer->Kick( m_pkServer->m_kPendingClient.m_kNodeID );
            m_pkServer->m_kPendingClient.m_bPending = false;
            m_pkServer->RemoveClient( m_pkServer->m_kPendingClient.m_kNodeID );
            neolog << LogLevel( INFO ) << "nw-device: timeout for integration of pending client reached, kick the client '" << m_pkServer->m_kPendingClient.m_strNetworkNodeName << "'" << endl; 

        }
    
    }

    m_pkServer->Update();

}

void NetworkDevice::UpdateClient( float fDeltaTime ) 
{ 

    if ( !m_pkClient ) {

        return;

    }

    m_fPassedUpdateTime += fDeltaTime;

    // don't update every frame if the machine can perform hight amount of frames per second ( throttle for very fast machines )
    if ( m_fPassedUpdateTime > m_fClientMinUpdatePeriod ) {

        m_pkClient->Update();
        m_fPassedUpdateTime = 0.0f;

    }

    BaseEntity  *pkEntity;
    // update remote client entities
    std::vector< ClientNode >::iterator pkClientNode    = m_pkClient->m_pvpkClientList->begin();
    std::vector< ClientNode >::iterator pkClientNodeEnd = m_pkClient->m_pvpkClientList->end();
    while ( pkClientNode != pkClientNodeEnd ) {

        pkEntity = ( *pkClientNode ).m_pkEntity;
        if ( pkEntity->IsActive() == true ) {
        
            pkEntity->UpdateEntity( fDeltaTime );

        }
        pkClientNode++;

    }

    // update client entities
    std::vector< BaseEntity* >::iterator    pkClient    = m_pkFrameworkImpl->m_vpkNetworkClientObjects.begin();
    std::vector< BaseEntity* >::iterator    pkClientEnd = m_pkFrameworkImpl->m_vpkNetworkClientObjects.end();
    while ( pkClient != pkClientEnd ) {

        pkEntity = ( *pkClient );
        if ( pkEntity->IsActive() == true ) {
        
            pkEntity->UpdateEntity( fDeltaTime );

        }
        pkClient++;

    }

}

} // namespace CTD
