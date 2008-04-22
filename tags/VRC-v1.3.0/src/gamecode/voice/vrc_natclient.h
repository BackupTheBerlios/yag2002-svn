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
 # implementation of a NAT client used for getting NAT info from
 #  NAT server
 #
 #   date of creation:  02/20/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _VRC_NATCLIENT_H_
#define _VRC_NATCLIENT_H_

#include <vrc_main.h>
#include "vrc_natcommon.h"

namespace vrc
{

class VoiceTransport;

//! NAT client used by transport layer
class NATClient
{
    protected:

                                                    NATClient();

        virtual                                     ~NATClient();

        //! Initialize
        void                                        initialize();

        //! Call this method periodically on client
        void                                        update( float deltaTime );

        //! Request for client info with given SID, used only on clients in order to get info on peers.
        void                                        requestClientInfo( int sid, NATInfoCallback* p_cb );

        //! Register the client on server
        void                                        registerClient();

        //! Handle incomming network packets
        bool                                        receivePacket( char* p_buffer, int len, const RNReplicaNet::XPAddress& senderaddr );

        //! Get the socket
        RNReplicaNet::XPSocketUrgent*               getSocket() { return _p_socket; }

        //! Map for handling several simultanous requests < SID, Callback >
        std::map< int, NATInfoCallback* >           _requestMap;

        //! Communication socket
        RNReplicaNet::XPSocketUrgent*               _p_socket;

        //! Server address
        RNReplicaNet::XPAddress                     _serverAddress;

        //! Used if ip forwarding is enabled.
        int                                         _forwardedPort;

        //! General timer
        float                                       _timer;

        //! Internal client states
        enum ClientState
        {
            eUnknown        = 0x0,
            eInitialized    = 0x1,
            eReqClientInfo  = 0x2,
            eIdle           = 0x4
        };

        ClientState                                 _state;

    friend class VoiceTransport;
};

}

#endif // _VRC_NATCLIENT_H_
