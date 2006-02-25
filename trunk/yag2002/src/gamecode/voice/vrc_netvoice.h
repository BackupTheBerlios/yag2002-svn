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
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
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

class BaseVoiceInput;
class NetworkSoundCodec;
class BaseNetworkSoundImplementation;

//! NetworkVoice Entity
class EnNetworkVoice : 
    public yaf3d::BaseEntity, 
    public VoiceNetwork::FunctorHotspotChange,
    public gameutils::PlayerUtils::FunctorPlayerListChange
{
    public:
                                                    EnNetworkVoice();

        virtual                                     ~EnNetworkVoice();

        //! Initialize
        void                                        initialize();

        //! This entity needs updating
        void                                        updateEntity( float deltaTime );

    protected:
        
        //! Handle notifications
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        //! Update the sender list, 'joining' = true means that a new player has entered the voice hotspot
        //! Otherwise it means that an entity has left the hotspot.
        void                                        updateHotspot( yaf3d::BaseEntity* p_entity, bool joining );

        //! Functor for getting changed player list
        void                                        operator()( bool localplayer, bool joining, yaf3d::BaseEntity* p_entity );

        //! Functor for getting changed hotspot
        void                                        operator()( bool joining, yaf3d::BaseEntity* p_entity );

        //! Sound spot range, only players in this range transmit voice data
        float                                       _spotRange;

        //! Enable / disable voice
        bool                                        _enableVoice;

        //! Voice receiver
        BaseNetworkSoundImplementation*             _p_receiver;

        //! Voice network manager
        VoiceNetwork*                               _p_voiceNetwork;

        //! Sound compression codec
        NetworkSoundCodec*                          _p_codec;

        //! Voice input capture
        BaseVoiceInput*                             _p_soundInput;

        //! A type for mapping players and associated voice senders
        typedef std::map< yaf3d::BaseEntity*, BaseNetworkSoundImplementation* > SenderMap;

        //! Sender map
        SenderMap                                   _sendersMap;
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
