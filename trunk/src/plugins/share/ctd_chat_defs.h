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
 # neoengine, chat member plugin specific defines
 #
 #   date of creation:  08/13/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  08/13/2004 boto       creation of this header file
 #
 ################################################################*/


#ifndef _CTD_CHATMEMBER_DEFS_H_
#define _CTD_CHATMEMBER_DEFS_H_

namespace CTD_IPluginChat
{

// plugin name
#define CTD_PLUGINNAME_CHAT                         "Plugin -- Chat"

// versioning constants
#define CTD_PLUGIN_CHAT_VERSION_MAJ                 0
#define CTD_PLUGIN_CHAT_VERSION_MIN                 4
#define CTD_PLUGIN_CHAT_VERSION_REVISION            0


// entity names
//---------------------------------------------
#define CTD_ENTITY_NAME_ChatMember                  "ChatMember"
#define CTD_ENTITY_NAME_Gui                         "Gui"
#define CTD_ENTITY_NAME_3DMenuLoader                "3DMenuLoader"


// entity messages
//---------------------------------------------

// this message is sent by gui entity to chat member entity
#define CTD_ENTITY_GUI_SEND_TEXT                    0xA00001

// this message is sent by gui entity to chat member entity for locking the movement input during message editing
#define CTD_ENTITY_GUI_LOCK_MOVEMENT                0xA00002

// this message is sent by gui entity to chat member entity for freeing the movement input
#define CTD_ENTITY_GUI_FREE_MOVEMENT                0xA00003

// this message is sent by chat member entity to gui entity
#define CTD_ENTITY_CHATMEMBER_ADD_MSG               0xA00004

// network message structures
//---------------------------------------------

// message type position
#define CTD_NM_ChatMember_DT_POSITION_REQUEST       0x33
#define CTD_NM_ChatMember_DT_POSITION               0x11

#define CTD_NM_ChatMember_DT_ROTATION_REQUEST       0x44
#define CTD_NM_ChatMember_DT_ROTATION               0x22

#define CTD_NM_ChatMember_CHAT_MSG                  0x55
#define CTD_NM_ChatMember_CHAT_MSG_REQUEST          0x66

#define CTD_NM_ChatMember_INIT_REMOTE_CLIENT        0xE0

// a template structure for all kinds of observer messages
//  this is used to get the message id
typedef struct _CTD_NM_ChatMemberTemplate {

    //-------------------------------------------------------//
    // use of this macro as first element is mandatory for all 
    //  network message structs
    CTD_NW_EntityPacketHeader;

    unsigned char       m_ucDataType;
    //-------------------------------------------------------//

} tCTD_NM_ChatMemberTemplate;

// message structure for position updates
typedef struct _CTD_NM_ChatMemberPosition {

    //-------------------------------------------------------//
    // use of this macro as first element is mandatory for all 
    //  network message structs
    CTD_NW_EntityPacketHeader;

    unsigned char       m_ucDataType;
    //-------------------------------------------------------//

    float               m_fPosX;
    float               m_fPosY;
    float               m_fPosZ;

    float               m_fVelX;
    float               m_fVelY;
    float               m_fVelZ;

    unsigned char       m_stateTranslation;

} tCTD_NM_ChatMemberPosition;

// message structure for rotation updates
typedef struct _CTD_NM_ChatMemberRotation {

    //-------------------------------------------------------//
    // use of this macro as first element is mandatory for all 
    //  network message structs
    CTD_NW_EntityPacketHeader;

    unsigned char       m_ucDataType;
    //-------------------------------------------------------//

    float               m_fRotX;       // pitch
    float               m_fRotY;       // yaw

    float               m_fVelX;
    float               m_fVelY;

    unsigned char       m_stateRotation;

} tCTD_NM_ChatMemberRotation;

// this structure is used to send a chat text
//  set the data type to CTD_NM_ChatMember_CHAT_MSG
typedef struct _CTD_NM_SendChatMsg {

    //-------------------------------------------------------//
    // use of this macro as first element is mandatory for all 
    //  network message structs
    CTD_NW_EntityPacketHeader;

    unsigned char       m_ucDataType;
    //-------------------------------------------------------//
    
    char                m_pcChatText[ 256 ]; 
    
} tCTD_NM_SendChatMsg;

//--------------------------------------------------------//
// this structure is used to get remote clients initialized
typedef struct _CTD_NM_InitRemoteChatMember {

    //-------------------------------------------------------//
    // use of this macro as first element is mandatory for all 
    //  network message structs
    CTD_NW_SystemPacketHeader;
    //-------------------------------------------------------//
    unsigned short      m_usNetworkID;

    // length of following user data ( only for internal check )
    long                m_lDataLength;          
    // user data ( maximal CTD_NW_MAX_INITDATA_SIZE bytes )
    char                m_pcMeshFile[ 64 ];
    float               m_fPosX;
    float               m_fPosY;
    float               m_fPosZ;

    float               m_fRotX;
    float               m_fRotY;
    float               m_fRotZ;

} tCTD_NM_InitRemoteChatMember;
//--------------------------------------------------------//

} // namespace CTD_IPluginChat

#endif //_CTD_CHATMEMBER_DEFS_H_
