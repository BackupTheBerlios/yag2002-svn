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
 # voice receiver is responsible for collecting incoming voice 
 #  over netweork
 #
 #   date of creation:  01/19/2006
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _VRC_VOICERECEIVER_H_
#define _VRC_VOICERECEIVER_H_

#include <vrc_main.h>
#include <speex/speex.h>
#include "vrc_networksoundimpl.h"
#include "vrc_voicetransport.h"
#include <RNLobby/Inc/TransportNATUDP.h>

namespace vrc
{

class SoundNode;
class EnNetworkVoice;
class ReceiverUpdateThread;

//! Class for receiving, decoding and playing incoming voice pakets
class VoiceReceiver : public BaseNetworkSoundImplementation, public CallbackVoiceData
{
    public:

        explicit                                    VoiceReceiver( EnNetworkVoice* p_network, VoiceTransport* p_transport );

        virtual                                     ~VoiceReceiver();

    protected:

        //! Initialize the voice server
        void                                        initialize() throw( NetworkSoundException );

        //! Set output gain
        void                                        setOutputGain( float gain );

        //! Set the spot range
        void                                        setSpotRange( float range );

        //! Shutdown the voice server
        void                                        shutdown();

        //! Update server, call this in gaim loop
        void                                        update( float deltaTime );

        //! Setup FMOD
        void                                        setupSound() throw( NetworkSoundException );

        //! Create a sound node for a new client with given session ID
        SoundNode*                                  createSoundNode();

        //! Setup ReplicaNet
        void                                        setupNetwork()  throw( NetworkSoundException );

        //! Destroy the given connection, note: this call modifies _soundNodeMap!
        void                                        destroyConnection( unsigned int senderID );

        //! Overriden method called when new data received on socket.
        bool                                        recvPacket( VoicePaket* p_packet, const RNReplicaNet::XPAddress& senderaddr );

        //! Overriden method called when a voice chat player left the session ( see BaseNetworkSoundImplementation ).
        void                                        removePlayer( yaf3d::BaseEntity* p_entity );

        //! Get the sound data mutex
        static OpenThreads::Mutex&                   getSoundDataMutex() { return _s_sndDataMutex; }

        //! A map for connected senders ( sender ID / sound node )
        typedef std::map< unsigned int, SoundNode* > SenderMap;

        SenderMap                                   _soundNodeMap;

        //! Transport layer
        VoiceTransport*                             _p_transport;

        //! Sound system
        FMOD::System*                               _p_soundSystem;

        //! Voice network
        EnNetworkVoice*                             _p_network;

        //! Receiver updating is done in an own thread
        ReceiverUpdateThread*                       _p_receiverUpdateThread;

        //! Sound data mutex
        static OpenThreads::Mutex                   _s_sndDataMutex;

        //! Sound node manipulation mutex
        OpenThreads::Mutex                          _soundMapMutex;

        //! Senders' lifesign check timer
        float                                       _lifesignCheck;

        //! Output gain
        float                                       _outputGain;

        //! Sport range is used for calculating the voice volume attenuation depending on player distance to each other
        float                                       _spotRange;

        //! Cut-off range
        float                                       _cutoffRange;

    //! The voice playing callback function has access to internal methods.
    friend FMOD_RESULT F_CALLBACK voiceReceiverReadPCM( FMOD_SOUND* p_sound, void* p_data, unsigned int datalen );
};

} // namespace vrc

#endif // _VRC_VOICERECEIVER_H_
