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

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_netvoice.h"
#include "vrc_voicesender.h"
#include "vrc_voicereceiver.h"
#include "vrc_voiceinput.h"
#include "vrc_codec.h"
#include <RNXPURL/Inc/NetworkEmulation.h>

namespace vrc
{

//! Implement and register the network voice entity factory
YAF3D_IMPL_ENTITYFACTORY( NetworkVoiceEntityFactory )

EnNetworkVoice::EnNetworkVoice():
_spotRange( 20.0f ),
_p_receiver( NULL ),
_p_voiceNetwork( NULL ),
_p_codec( NULL ),
_p_soundInput( NULL ),
_inputGain( 1.0f ),
_active( false )
{
    // register entity attributes
    getAttributeManager().addAttribute( "spotRange", _spotRange  );
}

EnNetworkVoice::~EnNetworkVoice()
{
    if ( _active )
        destroyVoiceChat();
}

void EnNetworkVoice::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle notifications
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_MENU_LEAVE:

            // reflect the settings in menu
            setupNetworkVoice();
            break;

        case YAF3D_NOTIFY_SHUTDOWN:
            break;

        default:
            ;
    }
}

void EnNetworkVoice::initialize()
{
    // setup voice chat
    setupNetworkVoice();

    // register entity in order to get notifications (e.g. from menu entity)
    yaf3d::EntityManager::get()->registerNotification( this, true );

    // some network emulation settings
    //bool enableNetworkEmulation = true;
    //if ( enableNetworkEmulation )
    //{
    //    RNReplicaNet::NetworkEmulation::SetEnabled( true );
    //    RNReplicaNet::NetworkEmulation::SetConnection( RNReplicaNet::NetworkEmulation::kModem56k, true );
    //    RNReplicaNet::NetworkEmulation::SetPacketLoss( 5 );
    //    RNReplicaNet::NetworkEmulation::SetAverageLatency( 0.1f );
    //    RNReplicaNet::NetworkEmulation::SetJitter( 0.02f );
    //}
}

void EnNetworkVoice::setupNetworkVoice()
{
    // check if voice chat is enabled
    bool voicechatenable;
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_VOICECHAT_ENABLE, voicechatenable );
    if ( voicechatenable )
    {
        // get further menu settings for voice chat
        float outputgain;
        yaf3d::Configuration::get()->getSettingValue( VRC_GS_VOICE_OUTPUT_GAIN, outputgain );
        yaf3d::Configuration::get()->getSettingValue( VRC_GS_VOICE_INPUT_GAIN, _inputGain );
        unsigned int  inputdev;
        yaf3d::Configuration::get()->getSettingValue( VRC_GS_VOICECHAT_INPUT_DEVICE, inputdev );

        // is voice chat already active?
        if ( !_active )
        {
            createVoiceChat( _inputGain, outputgain );
        }
        else
        {
            // just update the input and output gain
            _p_soundInput->setInputGain( _inputGain );
            _p_receiver->setOutputGain( outputgain );
            _p_soundInput->setInputDevice( inputdev );
        }
    }
    else if ( !voicechatenable && _active )
    {
        destroyVoiceChat();
    }
}

void EnNetworkVoice::createVoiceChat( float inputgain, float outputgain )
{
    assert( !_active && "trying to destroy voice chat which has already been created before!" );

    try
    {
        _inputGain = inputgain;

        // create a sound receiver
        _p_receiver = new VoiceReceiver();
        _p_receiver->initialize();
        _p_receiver->setOutputGain( outputgain );

        // create the network manager for voice chat clients
        _p_voiceNetwork = new VoiceNetwork;
        _p_voiceNetwork->initialize();
        _p_voiceNetwork->setHotspotRange( _spotRange );

        // create sound codec
        _p_codec = new NetworkSoundCodec;
        _p_codec->setEncoderComplexity( 9 );
        _p_codec->setEncoderQuality( 10 );
        _p_codec->setupEncoder();

        unsigned int inputdevice;
        yaf3d::Configuration::get()->getSettingValue( VRC_GS_VOICECHAT_INPUT_DEVICE, inputdevice );
        _p_soundInput = new VoiceMicrophoneInput( NULL, _p_codec );
        if ( inputdevice != VRC_GS_VOICECHAT_INPUT_DEVICE_NA )
        {
            _p_soundInput->initialize();
            _p_soundInput->setInputGain( _inputGain );
            // we begin with input grabbing when at least one sender is active
            _p_soundInput->stop( true );
        }
    }
    catch ( const NetworkSoundExpection& e )
    {
        log_error << ENTITY_NAME_NETWORKVOICE << ":" << getInstanceName() << "  could not be initialized." << std::endl;
        log_error << "  reason: " << e.what() << std::endl;

        if ( _p_receiver )
        {
            delete _p_receiver;
            _p_receiver = NULL;
        }

        if ( _p_voiceNetwork )
        {
            delete _p_voiceNetwork;
            _p_voiceNetwork = NULL;
        }

        return;
    }

    // register for getting periodic updates
    yaf3d::EntityManager::get()->registerUpdate( this, true );

    // set functor to get notifications on changed voice chat list
    vrc::gameutils::PlayerUtils::get()->registerFunctorVoiceChatPlayerListChanged( this, true );

    // set functor to get notifications on changed hotspot
    _p_voiceNetwork->registerFunctorHotspotChanged( this, true );

    // set active flag
    _active = true;
}

void EnNetworkVoice::destroyVoiceChat()
{
    assert( _active && "trying to destroy voice chat entity which has not been created before!" );

    try
    {
        // remove functor in player utils
        vrc::gameutils::PlayerUtils::get()->registerFunctorVoiceChatPlayerListChanged( this, false );

        SenderMap::iterator p_beg = _sendersMap.begin(), p_end = _sendersMap.end();
        for ( ; p_beg != p_end; ++p_beg )
        {
            delete p_beg->second;
        }
        _sendersMap.clear();

        if ( _p_receiver )
        {
            _p_receiver->shutdown();
            delete _p_receiver;
        }

        // now destroy voice network
        if ( _p_voiceNetwork )
        {
            _p_voiceNetwork->registerFunctorHotspotChanged( this, false );
            _p_voiceNetwork->shutdown();
            delete _p_voiceNetwork;
        }

        if ( _p_soundInput )
        {
            delete _p_soundInput;
        }

        if ( _p_codec )
            delete _p_codec;
    }
    catch ( const NetworkSoundExpection& e )
    {
        log_error << ENTITY_NAME_NETWORKVOICE << ":" << getInstanceName() << "  a problem occurred during shutdown." << std::endl;
        log_error << "  reason: " << e.what() << std::endl;
    }

    // reset active flag
    _active = false;
}

void EnNetworkVoice::updateEntity( float deltaTime )
{
    // update the voice network state
    _p_voiceNetwork->update( deltaTime );

    // update senders
    SenderMap::iterator p_beg = _sendersMap.begin(), p_end = _sendersMap.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        VoiceSender* p_sender = static_cast< VoiceSender* >( p_beg->second );
        // check for dead senders
        if ( !p_sender->isAlive() )
        {
            delete p_sender;
            _sendersMap.erase( p_beg );
            p_beg = _sendersMap.begin();
            p_end = _sendersMap.end();
            continue;
        }
        else
        {
            p_beg->second->update( deltaTime );
        }
    }

    // update audio input
    _p_soundInput->update();

    // update our receiver
    if ( _p_receiver )
        _p_receiver->update( deltaTime );

    //! TODO: remove this diagnostics stuff later
    //*******************************************
    static float t = 0.0f;
    t += deltaTime;
    if ( t > 2.0f )
    {
        t = 0.0f;
        std::stringstream msg;
        msg << "current active senders: " << _sendersMap.size();
        log_verbose << msg.str() << std::endl;
    }
    //*******************************************
}

void EnNetworkVoice::updateHotspot( yaf3d::BaseEntity* p_entity, bool joining )
{
    assert( p_entity && "invalid entity" );

    SenderMap::iterator hit = _sendersMap.find( p_entity );
    if ( joining )
    {
        if ( hit != _sendersMap.end() )
        {
            log_warning << "EnNetworkVoice: entity is already in internal hotspot map!" << std::endl;
            return;
        }

        // determine receiver's ip
        VoiceNetwork::VoiceClientMap::const_iterator vmapend = _p_voiceNetwork->getHotspot().end();
        VoiceNetwork::VoiceClientMap::const_iterator vmapent = _p_voiceNetwork->getHotspot().find( p_entity );
        if ( vmapent == vmapend )
        {
            log_error << " EnNetworkVoice: internal error, voice client's receiver ip cannot be determined." << std::endl;
        }
        else
        {
            // create a new sender
            std::string receiverIP = vmapent->second;
            BaseNetworkSoundImplementation* p_sender = new VoiceSender( receiverIP, _p_soundInput );
            p_sender->initialize();
            _sendersMap[ p_entity ] = p_sender;

            // continue grabbing input if it was stopped before
            if ( _sendersMap.size() == 1 )
                _p_soundInput->stop( false );
        }
    }
    else
    {
        if ( hit == _sendersMap.end() )
        {
            log_warning << "EnNetworkVoice: entity is not in internal hotspot map!" << std::endl;
            return;
        }

        // destroy the sender
        hit->second->shutdown();
        delete hit->second;
        _sendersMap.erase( hit );
        // stop input grabbing when no senders exist
        if ( _sendersMap.size() == 0 )
            _p_soundInput->stop( true );
    }
}

void EnNetworkVoice::operator()( bool localplayer, bool joining, yaf3d::BaseEntity* p_entity )
{
    // let the voice network manager update its internal client list when a new client is joining 
    if ( joining )
    {
        _p_voiceNetwork->updateVoiceClients( p_entity, true );
    }
    else
    {
        _p_voiceNetwork->updateVoiceClients( p_entity, false );

        // remove player from sender map
        SenderMap::iterator player = _sendersMap.find( p_entity );
        if ( player != _sendersMap.end() )
        {
            BaseNetworkSoundImplementation* p_sender = player->second;
            delete p_sender;
            _sendersMap.erase( player );
        }
        else
        {
            log_error << "VoiceNetwork: leaving player could not be found in sender map" << std::endl;
        }    
    }
}

void EnNetworkVoice::operator()( bool joining, yaf3d::BaseEntity* p_entity )
{
    // update the hotspot when clients enter / leave the hotspot range
    updateHotspot( p_entity, joining );
}

} // namespace vrc
