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
 # entity Network voice, used for server and client
 #
 #   date of creation:  01/28/2006
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _VRC_NETWORKVOICE_H_
#define _VRC_NETWORKVOICE_H_

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_voicenetwork.h"

namespace vrc
{

#define ENTITY_NAME_NETWORKVOICE    "NetworkVoice"

class VoiceGui;
class VoiceTransport;
class BaseVoiceInput;
class NetworkSoundCodec;
class BaseNetworkSoundImplementation;

//! NetworkVoice Entity
class EnNetworkVoice : 
    public yaf3d::BaseEntity, 
    public yaf3d::SessionNotifyCallback,
    public VoiceNetwork::CallbackHotspotChange,
    public gameutils::PlayerUtils::CallbackPlayerListChange
{
    public:
                                                    EnNetworkVoice();

        virtual                                     ~EnNetworkVoice();

        //! Initialize
        void                                        initialize();

        //! Call this when the voice configuration has been changed ( e.g. by voice gui ).
        void                                        updateConfiguration();

        //! Get input / output gains
        void                                        setInputOutputGain( float input, float output );

        //! This entity needs updating
        void                                        updateEntity( float deltaTime );

        //! Destroy the given sender, this is used by receiver.
        void                                        destroySender( unsigned int senderID );

    protected:

        //! Handle notifications
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        //! Setup the voice chat considering the settings in menu
        void                                        setupNetworkVoice();

        //! Create voice chat
        void                                        createVoiceChat( float inputgain, float outputgain );

        //! Destroy voice chat
        void                                        destroyVoiceChat();

        //! Update the sender list, 'joining' = true means that a new player has entered the voice hotspot
        //! Otherwise it means that an entity has left the hotspot.
        void                                        updateHotspot( yaf3d::BaseEntity* p_entity, bool joining );

        //! Functor for getting changed player list
        void                                        onVoiceChatPlayerListChanged( bool joining, yaf3d::BaseEntity* p_entity );

        //! Callback for getting changed hotspot ( see VoiceNetwork::CallbackHotspotChange )
        void                                        onHotspotChanged( bool joining, yaf3d::BaseEntity* p_entity );

        //! Called when the client gets disconnected from server
        void                                        onServerDisconnect( int /*sessionID*/ );

        //! Sound spot range, only players in this range transmit voice data
        float                                       _spotRange;

        //! Voice receiver
        BaseNetworkSoundImplementation*             _p_receiver;

        //! Voice network manager
        VoiceNetwork*                               _p_voiceNetwork;

        //! Voice gui
        VoiceGui*                                   _p_voiceGui;

        //! Used for transporting voice packets over network
        VoiceTransport*                             _p_transport;

        //! Sound compression codec
        NetworkSoundCodec*                          _p_codec;

        //! Voice input capture
        BaseVoiceInput*                             _p_soundInput;

        //! A type for mapping players and associated voice senders
        typedef std::map< yaf3d::BaseEntity*, BaseNetworkSoundImplementation* > SenderMap;

        //! Sender map
        SenderMap                                   _sendersMap;

        //! Sender input gain
        float                                       _inputGain;

        //! Mute the sender?
        bool                                        _inputMute;

        //! Identifies whether the voice chat is activated
        bool                                        _active;

        //! Flag showing if the client networking is established.
        bool                                        _networkingEstablished;
};

//! Entity type definition used for type registry
class NetworkVoiceEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    NetworkVoiceEntityFactory() : 
                                                     yaf3d::BaseEntityFactory
                                                     ( 
                                                       ENTITY_NAME_NETWORKVOICE, yaf3d::BaseEntityFactory::Client
                                                      )
                                                    {}

        virtual                                     ~NetworkVoiceEntityFactory() {}

        Macro_CreateEntity( EnNetworkVoice );
};

}

#endif // _VRC_NETWORKVOICE_H_
