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
 #   date of creation:  04/08/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  04/23/2004 boto       creation of CTDNetworkClient
 #
 #  09/24/2004 boto       redesigned
 #
 ################################################################*/

#ifndef _CTD_NETWORK_CLIENT_H_
#define _CTD_NETWORK_CLIENT_H_


#include "ctd_network_defs.h"

#include <ctd_baseentity.h>
#include <ctd_pluginmanager.h>

#include <Source/PacketEnumerations.h>
#include <Source/RakClient.h>
#include "raknet_Multiplayer.h"

#include <string>
#include <vector>

namespace CTD
{

class FrameworkImpl;

// client core
class NetworkClient  : public Multiplayer< RakClientInterface >, public RakClient
{

    protected:

                                        NetworkClient( 
                                                          FrameworkImpl             *pkFrameworkImpl,
                                                          tStaticDataClient         *pkStaticData,
                                                          std::vector< ClientNode > *pvpkClientList
                                                         );
                                        

                                        ~NetworkClient();


        // raknet specific functions
        //-------------------------------------------------------------------------------------------------------------------------//

        void                            ReceiveRemoteDisconnectionNotification(Packet *pkPacket,RakClientInterface *pkClientInterface);

        void                            ReceiveRemoteConnectionLost(Packet *pkPacket,RakClientInterface *pkClientInterface);

        void                            ReceiveRemoteNewIncomingConnection(Packet *pkPacket,RakClientInterface *pkClientInterface);

        void                            ReceiveConnectionRequestAccepted(Packet *pkPacket,RakClientInterface *pkClientInterface);

        void                            ReceiveConnectionResumption(Packet *pkPacket,RakClientInterface *pkClientInterface);

        void                            ReceiveNoFreeIncomingConnections(Packet *pkPacket,RakClientInterface *pkClientInterface);

        void                            ReceiveDisconnectionNotification(Packet *pkPacket,RakClientInterface *pkClientInterface);

        void                            ReceiveConnectionLost(Packet *pkPacket,RakClientInterface *pkClientInterface);

        void                            ReceivedStaticData(Packet *pkPacket,RakClientInterface *pkClientInterface);

        void                            ReceiveInvalidPassword(Packet *pkPacket,RakClientInterface *pkClientInterface);

        void                            ReceiveModifiedPacket(Packet *pkPacket,RakClientInterface *pkClientInterface);

        void                            ReceiveRemotePortRefused(Packet *pkPacket,RakClientInterface *pkClientInterface);

        void                            ReceiveVoicePacket(Packet *pkPacket,RakClientInterface *pkClientInterface);

        void                            ProcessUnhandledPacket( char *pcPacketData, unsigned char ucPacketIdentifier, const PlayerID &pkNodeID, RakClientInterface *pkClientInterface );

        // protocol stack specific functions and variables
        //-------------------------------------------------------------------------------------------------------------------------//

        //! Update client, message dispatching, etc.
        void                            Update();

        //! Add a new client object
        bool                            AddRemoteClientObject( const std::string &strEntityName, unsigned short usNetworkID, const PlayerID &kNodeID, const std::string &strNetworkNodeName );

        //! Remove a client from internal list and destroy its entities
        void                            RemoveClient( const PlayerID &kNodeID );

        //! Enqueue a client which wants to be integrated into the network
        void                            EnqueueRequestingClientObject( NetworkObject *pkQueuedClientObject ) { m_pkQueuedClientObject = pkQueuedClientObject; }

        //-------------------------------------------------------------------------------------------//
        //! Indicated whether a connection could be established with a new client 
        bool                            IsConnectionEstablished() { return m_bConnectionEstablished; }

        //! Shutdown networking
        void                            Shutdown();

        //! Set ready flag signalizing that entity messages can / cannot be dispatched
        //!  e.g. after negotiating with server this flas is set to true by network device
        void                            SetReady( bool bReady ) { m_bReady = bReady; }

        //! Shows wether server could create the requested client object
        bool                            IsClientObjectCreated() { return m_bClientObjectCreated; }

        //! Call this after successfully creation of a client object on server
        void                            ResetClientObjectCreated() { m_bClientObjectCreated = false; }
        
        //! Connection established
        bool                            m_bConnectionEstablished;

        //! Flag indicating that network messages can be dispatched
        bool                            m_bReady;

        //! Flag to indicate that the requested client object is created on server
        bool                            m_bClientObjectCreated;

        //! Client list transmission initiated
        bool                            m_bCListTransInitiated;

        //! Count of clients to trasmit
        unsigned int                    m_uiCListCount;
        
        //! Timer for checking client list transmission timeout
        NeoEngine::Timer                m_kCListTransTimer;

        //! Client's static data
        tStaticDataClient               *m_pkStaticData;
        
        //! Client list
        std::vector< ClientNode >       *m_pvpkClientList;

        //! Instance of framework implementation
        FrameworkImpl                   *m_pkFrameworkImpl;
        
        //! This client object is queued by network device which is requesting the server for adding it into network session
        //!  This queue has a depth of 1, so adding requests are processed sequentially.
        NetworkObject                   *m_pkQueuedClientObject;


    friend class NetworkDevice;

};
//--------------------------------------------------------------------------------------//

} // namespace CTD

#endif //_CTD_NETWORK_CLIENT_H_
