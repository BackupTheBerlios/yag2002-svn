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
 # neoengine, common plugin defines
 #
 #   date of creation:  12/01/2003
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  12/01/2003 boto       creation of ctd_plugin_defs.h
 #
 #  04/09/2004 boto       added network msg id
 #
 #  09/24/2004 boto       redesigned
 #
 ################################################################*/


#ifndef _CTD_PLUGIN_DEFS_H_
#define _CTD_PLUGIN_DEFS_H_

namespace CTD
{


// default game settings
//--------------------------------------------//
// default game settings tokens
#define CTD_STOKEN_PLAYERNAME                                               "playername"
#define CTD_STOKEN_LEVEL                                                    "level"
#define CTD_BTOKEN_NETWORKING                                               "networking"
#define CTD_BTOKEN_SERVER                                                   "server"
#define CTD_BTOKEN_CLIENT                                                   "client"
#define CTD_STOKEN_SERVERNAME                                               "servername"
#define CTD_STOKEN_CLIENTNAME                                               "clientname"
#define CTD_STOKEN_SERVERIP                                                 "serverip"
#define CTD_ITOKEN_CLIENTPORT                                               "clientport"
#define CTD_ITOKEN_SERVERPORT                                               "serverport"

// default settings values
#define CTD_SVALUE_PLAYERNAME                                               "noname"
#define CTD_SVALUE_LEVEL                                                    "noname"
#define CTD_SVALUE_SERVER                                                   "localhost"
#define CTD_BVALUE_NETWORKING                                               true
#define CTD_BVALUE_SERVER                                                   false
#define CTD_BVALUE_CLIENT                                                   true
#define CTD_SVALUE_SERVERNAME                                               "ctd-server"
#define CTD_SVALUE_SERVERIP                                                 "localhost"
#define CTD_SVALUE_CLIENTNAME                                               "ctd-client"
#define CTD_IVALUE_SERVERPORT                                               32000
#define CTD_IVALUE_CLIENTPORT                                               32001


// plugin message ids
//------------------------------------------------//

//! return this value in plugin message and the plugin message function will no longer be called
#define CTD_PLUGIN_MSG_NO_NOTIFICATION                                      0xFFFFFFFF
//! init engine interface
#define CTD_PLUGIN_MSG_INIZIALIZE                                           0xFF000000
//! post initialize
#define CTD_PLUGIN_MSG_POSTINITIALIZE                                       0xFF000005
//! rendering phase for 3d
#define CTD_PLUGIN_MSG_RENDER_3D                                            0xFF000010
//! rendering phase for 2d
#define CTD_PLUGIN_MSG_RENDER_2D                                            0xFF000020
//! stepping
#define CTD_PLUGIN_MSG_UPDATE                                               0xFF000030
//! shutdown
#define CTD_PLUGIN_MSG_SHUTDOWN                                             0xFF000040
//------------------------------------------------//

//! system notifications for entities
//------------------------------------------------//
//!  application begins pausing 
#define CTD_ENTITY_PAUSING_BEGIN                                            0xFE000000
//!  application ends pausing
#define CTD_ENTITY_PAUSING_END                                              0xFE000001
//!  user entered menu
#define CTD_ENTITY_ENTER_MENU                                               0xFE000002
//!  user exits menu
#define CTD_ENTITY_EXIT_MENU                                                0xFE000003
//! exit game 
#define CTD_ENTITY_EXIT_GAME                                                0xFE000004
//------------------------------------------------//

// networking specific ids and macros
//------------------------------------------------//
// packet identifiers for network management and entity messages
//  these are passed to entities via message id parameter
#define CTD_NM_NETWORK_SYSTEM                                               0xF0
#define CTD_NM_NETWORK_ENTITY                                               0xF1

// network system messages directed to all loaded entities in client mode
//-----------------------------------------------//

// this message signalizes that we have been integrated into network session
#define CTD_NM_SYSTEM_NOTIFY_CONNECTION_ESTABLISHED                         0xF2
// this message signalizes that we lost connection
#define CTD_NM_SYSTEM_NOTIFY_CONNECTION_LOST                                0xF3
// this message signalizes that we resumed connection
#define CTD_NM_SYSTEM_NOTIFY_CONNECTION_RESUMED                             0xF4
// this message signalizes that we have been disconnected from server
#define CTD_NM_SYSTEM_NOTIFY_DISCONNECTED                                   0xF5


// use this macro in network struct declarations as leading element.
#define CTD_NW_EntityPacketHeader   \
    unsigned char       m_ucPacketIdentifier;   /* must be set to CTD_NM_NETWORK_ENTITY */\
    unsigned short      m_usNetworkID;

#define CTD_NW_SystemPacketHeader   \
    unsigned char       m_ucPacketIdentifier;   /* must be set to CTD_NM_NETWORK_MANAGEMENT */\
    unsigned char       m_ucDataType;


// this notification is sent by framework to all client object entities on all machines 
//  forcing them to send their initialization data to all existing remote client entities
// these ids are also passed to client objects or remote client object entities via id parameter
#define CTD_NM_CMD_SEND_INITIALIZION_DATA                                   0xE0
#define CTD_NM_CMD_RECEIVE_INITIALIZION_DATA                                0xE1
//------------------------------------------------//


} // namespace CTD

#endif //_CTD_PLUGIN_DEFS_H_
