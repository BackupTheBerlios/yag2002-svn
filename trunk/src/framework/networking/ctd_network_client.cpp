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

#include <string>
#include <time.h>

using namespace std;
using namespace NeoEngine;


namespace CTD
{

NetworkClient::NetworkClient( FrameworkImpl *pkFrameworkImpl, tStaticDataClient *pkStaticData ) 
{ 

    m_pkFrameworkImpl           = pkFrameworkImpl;
    m_pkStaticData              = pkStaticData; 
    m_bConnectionEstablished    = false;
    
    DistributedNetworkObjectManager::Instance()->RegisterRakClientInterface( this );

}

NetworkClient::~NetworkClient() 
{
}


void NetworkClient::Update()
{

    // dispatch received packets
    ProcessPackets( this ); 

}

void NetworkClient::Shutdown()
{ 
        
    Disconnect( CTD_NW_DISCONNECT_DURATION );

}

void NetworkClient::ReceiveRemoteDisconnectionNotification(Packet *pkPacket,RakClientInterface *pkClientInterface)
{

    neolog << LogLevel( INFO ) << "nw-client: remote client disconnected from server ( " 
        <<  pkPacket->playerId.binaryAddress 
        << "; port "
        << pkPacket->playerId.port
        << " )" << endl;

}


void NetworkClient::ReceiveRemoteConnectionLost(Packet *pkPacket,RakClientInterface *pkClientInterface)
{

    // print a time stamp
    time_t kTime;
    time( &kTime );
    neolog << LogLevel( INFO ) << "time: " << ctime( &kTime ) << endl;

    neolog << LogLevel( INFO ) << "nw-client:  remote client lost connection to server ( " 
        <<  pkPacket->playerId.binaryAddress 
        << "; port "
        << pkPacket->playerId.port
        << " )" << endl;


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

    m_bConnectionEstablished = true;

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
    m_pkFrameworkImpl->SendEntityMessage( CTD_NM_SYSTEM_NOTIFY_CONNECTION_RESUMED, ( void* )NULL );

}


void NetworkClient::ReceiveNoFreeIncomingConnections(Packet *pkPacket,RakClientInterface *pkClientInterface)
{

    // TODO: handle this event!?

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
    m_pkFrameworkImpl->SendEntityMessage( CTD_NM_SYSTEM_NOTIFY_DISCONNECTED, ( void* )NULL );

    m_bConnectionEstablished = false;
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
    m_pkFrameworkImpl->SendEntityMessage( CTD_NM_SYSTEM_NOTIFY_CONNECTION_LOST, ( void* )NULL );

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

    Disconnect( CTD_NW_DISCONNECT_DURATION );

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

}

} // namespace CTD
