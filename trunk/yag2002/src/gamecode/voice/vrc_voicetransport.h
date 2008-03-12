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
 # network layer for transporting voice packets
 #
 #   date of creation:  02/24/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _VRC_VOICETRANSPORT_H_
#define _VRC_VOICETRANSPORT_H_

#include <vrc_main.h>
#include "vrc_natcommon.h"
#include "vrc_natclient.h"

namespace vrc
{

//! Callback class used for receiving voice packets or client information
class CallbackVoiceData
{
    public:

                                                    CallbackVoiceData() {}

        virtual                                     ~CallbackVoiceData() {}

        /**! Method called when new data received on receiver socket. Override this operator in derived class if needed.
             Return true if the packet has been handled, otherwise the packet is passed to next registered receiver.
        */
        virtual bool                                recvPacket( VoicePaket* p_packet, const RNReplicaNet::XPAddress& senderaddr ) { return false; }

        //! Method called when client information received. Override this operator in derived class if needed.
        virtual void                                recvClientAddress( int /*sid*/, const RNReplicaNet::XPAddress& /*address*/ ) {}
};


//! Voice transport class
class VoiceTransport: public NATInfoCallback
{
    public:

                                                    VoiceTransport();

        virtual                                     ~VoiceTransport();

        //! Initialize
        void                                        initialize();

        //! Call this method periodically on client
        void                                        update( float deltaTime );

        //! Register / de-register a voice packet receiver. On receipt of new voice packets the callback's method 'recvPacket' is called.
        void                                        registerReceiver( CallbackVoiceData* p_cb, bool reg = true );

//! TODO: we should better implement a send method instead of exposing the socket!
        //! Get the raw socket. This socket must be used for sending voice packets to other clients.
        RNReplicaNet::XPSocketUrgent*               getSocket();

        //! Get NAT information of client with given session ID, this is used by sender.
        //! When the information gets available then the the callback with sid and address is called.
        void                                        getClientInfo( int sid, CallbackVoiceData* p_cb );

    protected:

        //! Overriden method from NATInfoCallback for getting mapped address of other clients
        void                                        receiveAddressMap( int sid, RNReplicaNet::XPAddress& address );

        //! NAT client used for retrieving information on clients' NAT
        NATClient*                                  _p_NATClient;

        //! Receiver socket
        RNReplicaNet::XPSocketUrgent*               _p_socket;

////! Receiver callback, there can be only one data receiver.
//CallbackVoiceData*                          _p_receiverCallback;

        //! Map for < sid / voice packet receiver callback >
        std::map< int, CallbackVoiceData* >         _sidRecvMap;

        //! Voice packet receiving callbacks
        std::vector< CallbackVoiceData* >           _recvCallbacks;
};

}

#endif // _VRC_VOICETRANSPORT_H_
