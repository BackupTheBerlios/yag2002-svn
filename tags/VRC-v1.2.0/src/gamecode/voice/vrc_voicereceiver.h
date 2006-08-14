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
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_VOICERECEIVER_H_
#define _VRC_VOICERECEIVER_H_

#include <vrc_main.h>
#include <speex/speex.h>
#include "vrc_networksoundimpl.h"

namespace vrc
{

class SoundNode;

//! Class for receiving, decoding and playing incoming voice pakets
class VoiceReceiver : public BaseNetworkSoundImplementation
{
    public:


                                                    VoiceReceiver();

        virtual                                     ~VoiceReceiver();
        
    protected:

        //! Initialize the voice server
        void                                        initialize() throw( NetworkSoundExpection );

        //! Set output gain
        void                                        setOutputGain( float gain );

        //! Set the spot range
        void                                        setSpotRange( float range );

        //! Shutdown the voice server
        void                                        shutdown();

        //! Update server
        void                                        update( float deltaTime );

        //! Setup FMOD
        void                                        setupSound() throw( NetworkSoundExpection );

        //! Create a sound node for a new client with given session ID
        SoundNode*                                  createSoundNode();

        //! Setup ReplicaNet
        void                                        setupNetwork()  throw( NetworkSoundExpection );

        //! A map for connected senders ( udp transport / sound node )
        typedef std::map< RNReplicaNet::Transport*, SoundNode* > SenderMap;
        SenderMap                                   _soundNodeMap;

        //! Unique sender ID for connecting senders
        unsigned int                                _senderID;

        //! UDP Transport
        RNReplicaNet::Transport*                    _p_udpTransport;

        //! Sound system
        FMOD::System*                               _p_soundSystem;
    
        //! Output gain
        float                                       _outputGain;

        //! Sport range is used for calculating the voice volume attenuation depending on player distance to each other
        float                                       _spotRange;

        //! Cut-off range
        float                                       _cutoffRange;
};

} // namespace vrc

#endif // _VRC_VOICERECEIVER_H_
