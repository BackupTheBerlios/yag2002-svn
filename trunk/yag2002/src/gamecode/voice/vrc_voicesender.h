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
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_VOICESENDER_H_
#define _VRC_VOICESENDER_H_

#include <vrc_main.h>
#include <speex/speex.h>
#include "vrc_voiceinput.h"
#include "vrc_networksoundimpl.h"

namespace vrc
{

class NetworkSoundCodec;

//! Class for encoding sound data and sending voice pakets
class VoiceSender : public BaseNetworkSoundImplementation, public BaseVoiceInput::FCaptureInput
{
    public:

        //! Construct a sender for given receiver and sound input capture
                                                    VoiceSender( const std::string& receiverIP, BaseVoiceInput* p_soundInput );

        virtual                                     ~VoiceSender();
        
    protected:

        //! Initialize the voice server
        void                                        initialize() throw( NetworkSoundExpection );

        //! Shutdown the voice server
        void                                        shutdown();

        //! Update client
        void                                        update( float deltaTime );

        //! Setup FMOD
        void                                        setupSound() throw( NetworkSoundExpection );

        //! Update sound system
        void                                        updateSound();

        //! Setup ReplicaNet
        void                                        setupNetwork()  throw( NetworkSoundExpection );

        //! Update ReplicaNet
        void                                        updateNetwork();

        //! Functor for grabbing the sound input, see class BaseVoiceInput
        void                                        operator ()( char* p_encodedaudio, unsigned short length );

        //! Receiver IP
        std::string                                 _receiverIP;

        //! UDP Transport
        RNReplicaNet::Transport*                    _p_udpTransport;

        //! Network states of sender
        enum
        {
            Initial = 0x10,
            RequestConnection,
            ConnectionReady,
            ConnectionDenied
        }                                           _senderState;

        //! Paket for sending and voice data over network
        VoicePaket*                                 _p_voicePaket;

        //! Sound input
        BaseVoiceInput*                             _p_soundInput;

    friend class BaseVoiceInput::FCaptureInput;
};

} // namespace vrc

#endif // _VRC_VOICESENDER_H_
