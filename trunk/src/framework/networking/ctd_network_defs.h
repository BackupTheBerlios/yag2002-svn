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


#include <ctd_baseentity.h>
#include <ctd_pluginmanager.h>

#include <Source/NetworkTypes.h>
#include "raknet_NetworkObject.h"
#include "raknet_Multiplayer.h"

#include <string>
#include <vector>

namespace CTD
{

//------------------------------------------------//
#define CTD_NM_NETWORK_CLIENT_REQ_CLIENTLIST                                0x11
#define CTD_NM_NETWORK_CLIENT_REQ_ADD_CLIENTOBJECT                          0x12

#define CTD_NM_NETWORK_CLIENT_RECV_REMOTE_CLIENT_INITIALIZION_DATA          0x13
#define CTD_NM_NETWORK_SERVER_CMD_SEND_REMOTE_CLIENT_INITIALIZION_DATA      0x15

#define CTD_NM_NETWORK_SERVER_ACK_CLIENTOBJECT_ADDED                        0x16
#define CTD_NM_NETWORK_SERVER_CMD_ADD_NEW_CLIENTOBJECT                      0x17

#define CTD_NM_NETWORK_SERVER_CMD_START_ADDING_REMOTE_CLIENTOBJECTS         0x21
#define CTD_NM_NETWORK_SERVER_CMD_ADD_REMOTE_CLIENTOBJECT                   0x22
#define CTD_NM_NETWORK_SERVER_CMD_END_ADDING_REMOTE_CLIENTOBJECTS           0x23

// timeout for trying to connect to a server
#define CTD_CLIENT_CONNECTION_TIMEOUT                       10.0f
// number of retries to add a client object into network session
#define CTD_CLIENT_ADD_CLIENTOBJECT_RETRIES                 3
// timeout for trying to add a client object ( e.g. player ) to network session
#define CTD_CLIENT_ADD_CLIENTOBJECT_TIMEOUT                 5.0f
// timeout for client list transmission
#define CTD_CLIENT_GET_REMOTE_CLIENTOBJECTS_TIMEOUT         15.0f
// timeout for integration of a pending client
#define CTD_PENDING_CLIENT_TIMEOUT                          8.0f

// connecting clients get ids beginning at 10000
#define CTD_NETWORKID_CLIENT_OBJECTS_RANGE_BEGIN            10000



// static data exchanged uppon client connection
typedef struct _StaticDataServer {

    char            m_pcNetworkNodeName[ 256 ];
    char            m_pcLevelName[ 256 ];
    
} tStaticDataServer;

typedef struct _StaticDataClient {

    PlayerID        m_kNodeID;
    char            m_pcNetworkNodeName[ 256 ];
    char            m_pcLevelName[ 256 ];       // this information will be received from server
    
} tStaticDataClient;
//---------------------------------------------------------------//


// this class represents one single client successfully connected to server
class ClientNode
{
    
    public:

                                        ClientNode() { m_pkEntity = NULL; m_kNodeID = UNASSIGNED_PLAYER_ID; }
                                        ~ClientNode() {};

        std::string                     m_strClientName;
        std::string                     m_strEntityName;
        BaseEntity                      *m_pkEntity;
        PlayerID                        m_kNodeID;
        unsigned short                  m_usNetworkID;

};


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

//####################################################////####################################################//

// this packet is sent by server notifying the connecting client for initiation of sending remote clients' information.
//  then for every connected client a packet tCTD_NM_SERVER_CMD_ADD_REMOTE_CLIENTOBJECT will follow.
//  the end of transmission is signalized by packet tCTD_NM_SERVER_CMD_END_ADDING_REMOTE_CLIENTOBJECTS.
typedef struct _CTD_NM_SERVER_CMD_START_ADDING_REMOTE_CLIENTOBJECTS {

    unsigned char       m_usPacketIdentifier;   // set this to CTD_NM_NETWORK_MANAGEMENT
    //-----------------------------//
    unsigned char       m_ucDataType;           // set this to CTD_NM_NETWORK_SERVER_CMD_START_ADDING_REMOTE_CLIENTOBJECTS
    unsigned short      m_usCountOfClients;     // this determines the count of following client information packets

} tCTD_NM_SERVER_CMD_START_ADDING_REMOTE_CLIENTOBJECTS;

// this struct is send by server to connecting client when it has just been created on server. this
//  struct commands to all clients to create existing client objects as ghosts in their local session; the 
//  connecting client gets this command several times ( once for every already existing client ). 
typedef struct _CTD_NM_SERVER_CMD_ADD_REMOTE_CLIENTOBJECT {

    unsigned char       m_usPacketIdentifier;   // set this to CTD_NM_NETWORK_MANAGEMENT
    //-----------------------------//
    unsigned char       m_ucDataType;           // set this to CTD_NM_NETWORK_SERVER_CMD_ADD_REMOTE_CLIENTOBJECT
    unsigned short      m_usCountOfClients;     // this determines the count of following client information packets
    char                m_pcEntityName[ 128 ];
    unsigned short      m_usNetworkID;
    PlayerID            m_kNodeID;
    char                m_pcNetworkNodeName[ 256 ];

} tCTD_NM_SERVER_CMD_ADD_REMOTE_CLIENTOBJECT;

// this struct is send to a new client after sending a list of existing clients
typedef struct _CTD_NM_SERVER_CMD_END_ADDING_REMOTE_CLIENTOBJECTS {

    unsigned char       m_usPacketIdentifier;   // set this to CTD_NM_NETWORK_MANAGEMENT
    //-----------------------------//
    unsigned char       m_ucDataType;           // set this to CTD_NM_NETWORK_SERVER_CMD_END_ADDING_REMOTE_CLIENTOBJECTS

} tCTD_NM_SERVER_CMD_END_ADDING_REMOTE_CLIENTOBJECTS;

//####################################################////####################################################//

// this struct is send to server requesting for a list of existing clients
typedef struct _CTD_NM_NETWORK_CLIENT_REQ_CLIENTLIST {

    unsigned char       m_usPacketIdentifier;   // set this to CTD_NM_NETWORK_MANAGEMENT
    //-----------------------------//
    unsigned char       m_ucDataType;           // set this to CTD_NM_NETWORK_CLIENT_REQ_CLIENTLIST

} tCTD_NM_NETWORK_CLIENT_REQ_CLIENTLIST;

// this struct is send by a client to server to signalize a request for adding a client object into 
//  network session. the entity name is used to instantiate the requested entity. after this
//  the server sends the struct _CTD_NM_SERVER_CLIENTOBJECT_ADDED below to all clients.
typedef struct _CTD_NM_CLIENT_ADD_CLIENTOBJECT_REQEUST {

    unsigned char       m_usPacketIdentifier;   // set this to CTD_NM_NETWORK_MANAGEMENT
    //-----------------------------//
    unsigned char       m_ucDataType;           // set this to CTD_NM_NETWORK_CLIENT_REQ_ADD_CLIENTOBJECT
    char                m_pcEntityName[ 128 ];
    char                m_pcNetworkNodeName[ 256 ];

} tCTD_NM_CLIENT_ADD_CLIENTOBJECT_REQEUST;

// this struct is send by server to all clients ( including the requesting one ), signalizing that a new client
//  is added to server simulation; the clients must create a new ghost representing this new client.
//  the variable m_usNetworkID contains the network id which the clients
//  should set in their local network object as the same client object must have the same network id in all clients.
typedef struct _CTD_NM_SERVER_CLIENTOBJECT_ADDED {

    unsigned char       m_usPacketIdentifier;   // set this to CTD_NM_NETWORK_MANAGEMENT
    //-----------------------------//
    unsigned char       m_ucDataType;           // set this to CTD_NM_NETWORK_SERVER_ACK_CLIENTOBJECT_ADDED
    char                m_pcEntityName[ 128 ];
    unsigned short      m_usNetworkID;

} tCTD_NM_SERVER_CLIENTOBJECT_ADDED;

// this struct is send by server to existing clients to command them adding a successfully connected new client
typedef struct _CTD_NM_SERVER_ADD_NEW_CLIENTOBJECT {

    unsigned char       m_usPacketIdentifier;   // set this to CTD_NM_NETWORK_MANAGEMENT
    //-----------------------------//
    unsigned char       m_ucDataType;           // set this to CTD_NM_NETWORK_SERVER_CMD_ADD_NEW_CLIENTOBJECT
    char                m_pcEntityName[ 128 ];
    unsigned short      m_usNetworkID;
    PlayerID            m_kNodeID;
    char                m_pcNetworkNodeName[ 256 ];

} tCTD_NM_SERVER_ADD_NEW_CLIENTOBJECT;


#define  CTD_NW_MAX_INITDATA_SIZE   1024    // maximal initialization data size     
// this struct is used to command all client objects ( not remote client objects ) for sending their initialization data
//  after a new client is connected
typedef struct _CTD_NM_NETWORK_SERVER_CMD_SEND_REMOTE_CLIENT_INITIALIZION_DATA {

    unsigned char       m_usPacketIdentifier;   // set this to CTD_NM_NETWORK_MANAGEMENT
    unsigned char       m_ucDataType;           // set this to CTD_NM_NETWORK_SERVER_CMD_SEND_REMOTE_CLIENT_INITIALIZION_DATA
    //-----------------------------//
    unsigned short      m_usNetworkID;          // set this to new client's network id

} tCTD_NM_NETWORK_SERVER_CMD_SEND_REMOTE_CLIENT_INITIALIZION_DATA;

// this struct is used by clients to send their object initialization data to all new created ghosts
typedef struct _CTD_NM_NETWORK_CLIENT_RECV_REMOTE_CLIENT_INITIALIZION_DATA {

    unsigned char       m_usPacketIdentifier;   // set this to CTD_NM_NETWORK_MANAGEMENT
    unsigned char       m_ucDataType;           // set this to CTD_NM_NETWORK_CLIENT_RECV_REMOTE_CLIENT_INITIALIZION_DATA
    //-----------------------------//
    unsigned short      m_usNetworkID;          // set this to new client's network id
    long                m_lDataLength;          // length of m_pcInitData
    char                m_pcInitData[ CTD_NW_MAX_INITDATA_SIZE ];

} tCTD_NM_NETWORK_CLIENT_RECV_REMOTE_CLIENT_INITIALIZION_DATA;

//---------------------------------------------------------------//

} // namespace CTD

#endif //_CTD_NETWORK_DEFS_H_
