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
#include "ctd_network_server.h"
#include "ctd_network_client.h"
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
        m_pkServer  = new NetworkServer( m_pkFrameworkImpl, &m_kStaticDataServer );

        // set static data for server
        m_pkServer->SetStaticServerData( (char*)&m_kStaticDataServer, sizeof( tStaticDataServer ) );


    } else {

        assert( ( m_bClientStaticDataReady == true ) && "(NetworkDevice::Initialize) first setup client's static data!" );

        m_strNetworkNodeName            = m_kStaticDataClient.m_pcNetworkNodeName;

        m_pkClient  = new NetworkClient( m_pkFrameworkImpl, &m_kStaticDataClient );

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

// update funtions for server and client
void NetworkDevice::UpdateServer( float fDeltaTime ) 
{ 

    if ( !m_pkServer ) {

        return;

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

}

} // namespace CTD
