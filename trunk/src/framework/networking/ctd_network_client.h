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
#include <include/PacketEnumerations.h>
#include <include/RakClient.h>
#include <include/Multiplayer.h>

#include <string>
#include <vector>

namespace CTD
{

class FrameworkImpl;

// client core
class NetworkClient  : public Multiplayer< RakClientInterface >, public RakClient
{

    protected:

                                        NetworkClient( FrameworkImpl *pkFrameworkImpl, tStaticDataClient *pkStaticData );
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

        //! Shutdown networking
        void                            Shutdown();

        //! Return true if a connection to server is established
        bool                            IsConnectionEstablished() { return m_bConnectionEstablished; }

        //! Connection flag
        bool                            m_bConnectionEstablished;

        //! Client's static data
        tStaticDataClient               *m_pkStaticData;
                        
        //! Instance of framework implementation
        FrameworkImpl                   *m_pkFrameworkImpl;

    friend class NetworkDevice;

};
//--------------------------------------------------------------------------------------//

} // namespace CTD

#endif //_CTD_NETWORK_CLIENT_H_
