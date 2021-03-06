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
 # voice sender is responsible for creating encoded voice pakets
 #  and sending them over netweork
 #
 #   date of creation:  01/28/2006
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#ifndef _VRC_VOICESENDER_H_
#define _VRC_VOICESENDER_H_

#include <vrc_main.h>
#include <speex/speex.h>
#include "vrc_codec.h"
#include "vrc_voiceinput.h"
#include "vrc_networksoundimpl.h"
#include "vrc_voicetransport.h"

namespace vrc
{


class NetworkSoundCodec;

//! Class for encoding sound data and sending voice pakets
class VoiceSender : public BaseNetworkSoundImplementation, public BaseVoiceInput::CallbackInputStream, public CallbackVoiceData
{
    public:

        //! Construct a sender for given receiver and sound input capture
                                                    VoiceSender( int sid, BaseVoiceInput* p_soundInput, VoiceTransport* p_transport );

        virtual                                     ~VoiceSender();

        //! Returns flase if the sender is dead, i.e. it had lost connection to receiver
        inline bool                                 isAlive() const;

        //! Return true if the connection is established, otherwise false
        inline bool                                 isConnected() const;

        //! Let the sender die.
        inline void                                 die();

    protected:

        //! Initialize the voice server
        void                                        initialize() throw( NetworkSoundException );

        //! Shutdown the voice server
        void                                        shutdown();

        //! Update client
        void                                        update( float deltaTime );

        //! Setup FMOD
        void                                        setupSound() throw( NetworkSoundException );

        //! Setup ReplicaNet
        void                                        setupNetwork()  throw( NetworkSoundException );

        //! Callback for grabbing the sound input, see class BaseVoiceInput ( CallbackInputStream )
        void                                        recvEncodedAudio( char* p_encodedaudio, unsigned int length, unsigned int encframesize );

        //! Callback for getting client NAT information ( CallbackVoiceData ).
        void                                        recvClientAddress( int sid, const RNReplicaNet::XPAddress& address );

        //! Callback for receiving voice packets
        bool                                        recvPacket( VoicePaket* p_packet, const RNReplicaNet::XPAddress& senderaddr );

        //! Receiver SID
        int                                         _receiverSID;

        //! Network states of sender
        enum
        {
            eStart = 0x10,
            eInitial,
            eRequestConnection,
            eConnectionReady
        }                                           _senderState;

        //! Packet for sending and voice data over network
        VoicePaket*                                 _p_voicePacket;

        //! Sound input
        BaseVoiceInput*                             _p_soundInput;

        //! Transport layer
        VoiceTransport*                             _p_transport;

        //! Sender socket
        RNReplicaNet::XPSocketUrgent*               _p_sendSocket;

        //! Peer address
        RNReplicaNet::XPAddress                     _receiverAddress;

        //! Used for alive-signaling in voice trasmission protocol
        float                                       _pingTimer;

        //! Used for alive-signaling in voice trasmission protocol
        float                                       _pongTimer;

        //! Timer used for various respond timeouts.
        float                                       _respondTimer;

        //! Paket stamp for voice transmission
        unsigned int                                _paketStamp;

        //! Alive flag set when the sender loses the connection to receiver
        bool                                        _isAlive;

    friend class BaseVoiceInput::CallbackInputStream;
};

inline bool VoiceSender::isAlive() const
{
    return _isAlive;
}

inline bool VoiceSender::isConnected() const
{
    return _senderState == eConnectionReady;
}

inline void VoiceSender::die()
{
    _isAlive = false;
}

} // namespace vrc

#endif // _VRC_VOICESENDER_H_
