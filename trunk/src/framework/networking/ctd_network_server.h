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
#include <include/PacketEnumerations.h>
#include <include/RakServer.h>
#include <include/Multiplayer.h>

#include <string>
#include <vector>

namespace CTD
{

class FrameworkImpl;
//--------------------------------------------------------------------------------------//
// server core
class NetworkServer  : public Multiplayer< RakServerInterface >, public RakServer
{

    protected:

                                        NetworkServer( FrameworkImpl *pkFrameworkImpl, tStaticDataServer *pkStaticData );

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

        //! Update server
        void                            Update();

        //! Shutdown networking
        void                            Shutdown();

        //! Server's static data
        tStaticDataServer               *m_pkStaticData;

        //! Instance of framework implementation
        FrameworkImpl                   *m_pkFrameworkImpl;

    friend class NetworkDevice;

};

} // namespace CTD

#endif //_CTD_NETWORK_SERVER_H_
