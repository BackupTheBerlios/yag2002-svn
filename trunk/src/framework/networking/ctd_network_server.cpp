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
#include <include/NetworkObject.h>

#include <string>
#include <time.h>

using namespace std;
using namespace NeoEngine;

namespace CTD
{

NetworkServer::NetworkServer( FrameworkImpl *pkFrameworkImpl, tStaticDataServer *pkStaticData )
{

    m_pkFrameworkImpl   = pkFrameworkImpl;
    m_pkStaticData      = pkStaticData;

    DistributedNetworkObjectManager::Instance()->RegisterRakServerInterface( this );

}

void NetworkServer::Update()
{

    // dispatch received packets
    ProcessPackets( this ); 

}

void NetworkServer::Shutdown()
{ 

    Disconnect( CTD_NW_DISCONNECT_DURATION );

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

}

void NetworkServer::ReceiveRemoteNewIncomingConnection(Packet *pkPacket,RakServerInterface *pkServerInterface)
{

    neolog << LogLevel( INFO ) << "nw-server: remote client ( " 
        <<  pkPacket->playerId.binaryAddress 
        << "; port "
        << pkPacket->playerId.port
        << " ) requests for connection." << endl;


}

void NetworkServer::ReceiveRemoteDisconnectionNotification(Packet *pkPacket,RakServerInterface *pkServerInterface)
{
}

void NetworkServer::ReceiveRemoteConnectionLost(Packet *pkPacket,RakServerInterface *pkServerInterface)
{
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

}

void NetworkServer::ReceivedStaticData(Packet *pkPacket,RakServerInterface *pkServerInterface)
{

    tStaticDataClient   *pkData = ( tStaticDataClient* )( ( char* )( pkPacket->data + sizeof(unsigned char) ) );
    // terminate string avoiding a possible stacke overflow ( crash attacks )!
    pkData->m_pcNetworkNodeName[ 255 ]      = '\0';

    neolog << LogLevel( INFO ) << "nw-server: initialization information received " << "from client: " << pkData->m_pcNetworkNodeName << endl;  

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

}

void NetworkServer::ReceiveRemotePortRefused(Packet *pkPacket,RakServerInterface *pkServerInterface)
{
}

void NetworkServer::ReceiveVoicePacket(Packet *pkPacket,RakServerInterface *pkServerInterface)
{
}

void NetworkServer::ProcessUnhandledPacket( char *pcPacketData, unsigned char ucPacketIdentifier, const PlayerID &rkNodeID, RakServerInterface *pkServerInterface )
{
}

} // namespace CTD
