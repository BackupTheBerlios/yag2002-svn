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
 # neoengine, networking defines and structs
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
 #  04/08/2004 boto       creation of first packet layouts
 #
 #
 ################################################################*/

#ifndef _CTD_NETWORK_DEFS_H_
#define _CTD_NETWORK_DEFS_H_

#include <include/NetworkTypes.h>

namespace CTD
{

// max duration used for sending out last packets in send queue before disconnecting
#define CTD_NW_DISCONNECT_DURATION      5

//---------------------------------------------------------------//
// static data exchanged uppon client connection
typedef struct _StaticDataServer {

    char                m_pcNetworkNodeName[ 256 ];
    char                m_pcLevelName[ 256 ];
    
} tStaticDataServer;

typedef struct _StaticDataClient {

    PlayerID            m_kNodeID;
    char                m_pcNetworkNodeName[ 256 ];
    char                m_pcLevelName[ 256 ];       // this information will be received from server
    
} tStaticDataClient;
//---------------------------------------------------------------//


// this describes the packet header for entity messages containing packet identifier and data to be sent
typedef struct _CTDEntityPacketHeader {

    unsigned char       m_ucPacketIdentifier;   /* must be set to CTD_NM_NETWORK_ENTITY */
    unsigned short      m_usNetworkID;
    char*               m_pcData;

} tCTDEntityPacketHeader;


// this describes the packet header for client messages containing packet identifier, object id, packet data, etc.
typedef struct _CTDNetworkMgrPacketHeader {

    unsigned char       m_ucPacketIdentifier;   // must be set to CTD_NM_NETWORK_MANAGEMENT
    unsigned char       m_ucDataType;
    char*               m_pcData;

} tCTDNetworkMgrPacketHeader;

} // namespace CTD

#endif //_CTD_NETWORK_DEFS_H_
