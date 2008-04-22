/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2006, A. Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU Lesser General Public 
 *  License version 2.1 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public 
 *  License along with this program; if not, write to the Free 
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 * 
 ****************************************************************/

/*###############################################################
 # common NAT structures and defines
 #
 #   date of creation:  02/20/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _VRC_NATCOMMON_H_
#define _VRC_NATCOMMON_H_

#include <RNXPSockets/Inc/XPSockets.h>
#include "vrc_networksoundimpl.h"

namespace vrc
{

//! Command mask for NAT packets
#define NAT_CMD_MASK                    0xAFBEEF00

//! NAT commands
#define NAT_REGISTER_CLIENT             0xAFBEEF05
#define NAT_ACK_REGISTER_CLIENT         0xAFBEEF11
#define NAT_GET_CLIENT_INFO             0xAFBEEF22
#define NAT_RCV_CLIENT_INFO             0xAFBEEF33
#define NAT_RCV_CLIENT_QUERY            0xAFBEEF44
#define NAT_CLIENT_LIFESIGN             0xAFBEEF55
#define NAT_PUNCH_HOLE                  0xAFBEEF99

//! NAT client lifesign period in seconds
#define NAT_CLIENT_LIFESIGN_PERIOD      20.0f


//! Life-sign signaling period ( in seconds ) used in voice paket transmission protocol
#define VOICE_LIFESIGN_PERIOD                   5.0f

//! Paket structure and type ids for transmitting voice stream
#define VOICE_PAKET_TYPE_CON_REQ         0x0010  /* Sender requests receiver for connection */
#define VOICE_PAKET_TYPE_CON_GRANT       0x0020  /* Receiver grants connection from sender */
#define VOICE_PAKET_TYPE_CON_DENY        0x0030  /* Receiver denies connection from sender */
#define VOICE_PAKET_TYPE_CON_CLOSE       0x0040  /* Sender closes connection */
#define VOICE_PAKET_TYPE_CON_PING        0x0050  /* Sender pings receiver */
#define VOICE_PAKET_TYPE_CON_PONG        0x0060  /* Receiver pongs sender */
#define VOICE_PAKET_TYPE_VOICE_DATA      0x0070  /* voice data buffer */


//! Voice paket header size ( in bytes )
#define VOICE_PAKET_HEADER_SIZE          14

//! The total paket size must not exceed 512 bytes!
struct VoicePaket
{
    //! Paket id, it must not be NAT_CMD_MASK
    unsigned int        _typeId;
    
    //! Time stamp for detecting lost pakets
    unsigned int        _paketStamp;

    //! Unique sender ID, 0 means unassigned
    unsigned int        _senderID;

    //! Sound data length
    unsigned short      _length;

    //! Sound data buffer
    char                _p_buffer[ VOICE_PAKET_MAX_BUF_SIZE ];
};


//! NAT data packet type
typedef struct tNATData
{
    //! Command
    int                 _cmd;

    //! Sender's session ID
    int                 _ownSID;

    //! Peer's session ID
    int                 _peerSID;

    //! Peer's port
    int                 _peerPort;

    //! Peer's IP
    unsigned char       _peerIP[ 4 ];

    //! Local port
    int                 _localPort;

    //! Local IP
    unsigned char       _localIP[ 4 ];

    //! Non-zero if port forwarding is enabled
    int                 _fwdPort;

} tNATData;


//! Callback class for getting client info, used by transport layer.
class NATInfoCallback
{
    public:
                        NATInfoCallback() : _cntRetries( 0 ) {}

        virtual         ~NATInfoCallback() {}

        //! Called when client's NAT information received ( mapped address and port ).
        virtual void    receiveAddressMap( int sid, RNReplicaNet::XPAddress& address ) = 0;

        //! Count of retries
        unsigned int    _cntRetries;
};

}

#endif // _VRC_NATCOMMON_H_
