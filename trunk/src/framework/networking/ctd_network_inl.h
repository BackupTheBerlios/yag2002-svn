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
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  04/08/2004 boto       creation of CTDNetworking
 #
 #  09/24/2004 boto       redesigned
 #
 ################################################################*/

#ifndef _CTD_NETWORK_H_
#error "do not include this file! this is an inline header."
#endif

// plugin-public networking funtions
//-------------------------------------------------------------------------------------------------------//
inline bool NetworkDevice::SendServer( unsigned short usNetworkID, char *pcData, const long lLength, PacketPriority ePriority, PacketReliability eReliability, char cOrderingStream, PlayerID kNodeId, bool bBroadcast, bool bSecured )
{

    assert( ( m_pkServer != NULL ) && " (NetworkDevice::SendServer) server core ist not initialized!" );

    // network id 0 is invalid
    //assert ( usNetworkID != 0 );

    // wrap the packet and send it out
    ( ( tCTDEntityPacketHeader* )pcData )->m_ucPacketIdentifier     = CTD_NM_NETWORK_ENTITY;
    ( ( tCTDEntityPacketHeader* )pcData )->m_usNetworkID            = usNetworkID;


    // sent the packet
    return m_pkServer->Send( 
                            ( char* )pcData,
                            lLength,
                            ePriority,
                            eReliability,
                            cOrderingStream,
                            kNodeId,
                            bBroadcast,
                            bSecured
                            );


}

inline bool NetworkDevice::SendClient( unsigned short usNetworkID, char *pcData, const long lLength, PacketPriority ePriority, PacketReliability eReliability, char cOrderingStream )
{

    assert( ( m_pkClient != NULL ) && " (NetworkDevice::SendClient) client core ist not initialized!" );

    // network id 0 is invalid
    //assert ( usNetworkID != 0 );

    // wrap the packet and send it out
    ( ( tCTDEntityPacketHeader* )pcData )->m_ucPacketIdentifier     = CTD_NM_NETWORK_ENTITY;
    ( ( tCTDEntityPacketHeader* )pcData )->m_usNetworkID            = usNetworkID;

    // sent the packet
    return m_pkClient->Send( 
                            ( char* )pcData,
                            lLength,
                            ePriority,
                            eReliability,
                            cOrderingStream 
                            );


}
