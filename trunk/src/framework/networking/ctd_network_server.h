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
 #   date of creation:  04/08/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  04/23/2004 boto       creation of CTDNetworkServer
 #
 #  09/24/2004 boto       redesigned
 #
 ################################################################*/

#ifndef _CTD_NETWORK_SERVER_H_
#define _CTD_NETWORK_SERVER_H_

#include "ctd_network_defs.h"
#include <ctd_baseentity.h>
#include <ctd_pluginmanager.h>
#include <Source/PacketEnumerations.h>
#include <Source/RakServer.h>
#include "raknet_Multiplayer.h"

#include <string>
#include <vector>

namespace CTD
{

// a new client will be stored in this pending class and
//  processed in next steps
class PendingClient {

    public:
                                PendingClient() { m_bPending = false; }
                                ~PendingClient() {};

        bool                    m_bPending;
        PlayerID                m_kNodeID;
        std::string             m_strNetworkNodeName;
        NeoEngine::Timer        m_pkPendingTimer;

};

class FrameworkImpl;
//--------------------------------------------------------------------------------------//
// server core
class NetworkServer  : public Multiplayer< RakServerInterface >, public RakServer
{

    friend class NetworkDevice;


    private:

                                        NetworkServer( FrameworkImpl                    *pkFrameworkImpl,
                                                       tStaticDataServer                *pkStaticData, 
                                                       std::vector< ClientNode >        *pvpkClientList )
                                        {
                                            m_pkFrameworkImpl   = pkFrameworkImpl;
                                            m_pkStaticData      = pkStaticData; 
                                            m_pvpkClientList    = pvpkClientList;
                                            m_bServerRunning    = true;
                                        }

                                        ~NetworkServer() {}

        // raknet specific functions
        //-------------------------------------------------------------------------------------------------------------------------//

        void                            ReceiveRemoteDisconnectionNotification(Packet *pkPacket,RakServerInterface *pkServerInterface);

        void                            ReceiveRemoteConnectionLost(Packet *pkPacket,RakServerInterface *pkServerInterface);

        void                            ReceiveRemoteNewIncomingConnection(Packet *pkPacket,RakServerInterface *pkServerInterface);

        void                            ReceiveNewIncomingConnection(Packet *pkPacket,RakServerInterface *pkServerInterface);

        void                            ReceiveConnectionResumption(Packet *pkPacket,RakServerInterface *pkServerInterface);

        void                            ReceiveNoFreeIncomingConnections(Packet *pkPacket,RakServerInterface *pkServerInterface);

        void                            ReceiveDisconnectionNotification(Packet *pkPacket,RakServerInterface *pkServerInterface);

        void                            ReceiveConnectionLost(Packet *pkPacket,RakServerInterface *pkServerInterface);

        void                            ReceivedStaticData(Packet *pkPacket,RakServerInterface *pkServerInterface);

        void                            ReceiveInvalidPassword(Packet *pkPacket,RakServerInterface *pkServerInterface);

        void                            ReceiveModifiedPacket(Packet *pkPacket,RakServerInterface *pkServerInterface);

        void                            ReceiveRemotePortRefused(Packet *pkPacket,RakServerInterface *pkServerInterface);

        void                            ReceiveVoicePacket(Packet *pkPacket,RakServerInterface *pkServerInterface);

        void                            ProcessUnhandledPacket( char *pcPacketData, unsigned char ucPacketIdentifier, const PlayerID &rkNodeID, RakServerInterface *pkServerInterface);


        // protocol stack specific functions and variables
        //-------------------------------------------------------------------------------------------------------------------------//

        void                            Update();

        // shutdown networking
        void                            Shutdown();


        // this function starts transmitting a list of clients to requesting ( new ) client
        bool                            TransmitClientList( const PlayerID &rkNodeID );

        // this is called if a new connected client requests for adding a client object such as player
        bool                            AddClientObject( const std::string &strEntityName, const PlayerID &rkNodeID, const std::string &strNetworkNodeName );

        // remove a client from internal list and destroy its entities
        void                            RemoveClient( const PlayerID &kNodeID );

        // this funtion created a unique network id for client objects which are requested by clients
        unsigned short                  CreateNewNetworkID();

        tStaticDataServer               *m_pkStaticData;

        std::vector< ClientNode >       *m_pvpkClientList;
        
        FrameworkImpl                   *m_pkFrameworkImpl;
        
        PendingClient                   m_kPendingClient;

        bool                            m_bServerRunning;

};

} // namespace CTD

#endif //_CTD_NETWORK_SERVER_H_
