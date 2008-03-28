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

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_netvoice.h"
#include "vrc_voicesender.h"
#include "vrc_voicereceiver.h"
#include "vrc_voiceinput.h"
#include "vrc_natclient.h"
#include "vrc_codec.h"
#include <RNXPURL/Inc/NetworkEmulation.h>


//! Define this for getting voice input from file instead of microphone device; it is used for testing.
//#define INPUT_TEST
//! Test file name
#define INPUT_TEST_FILE     "voiceinput.wav"


namespace vrc
{

//! Implement and register the network voice entity factory
YAF3D_IMPL_ENTITYFACTORY( NetworkVoiceEntityFactory )

EnNetworkVoice::EnNetworkVoice():
_spotRange( 20.0f ),
_p_receiver( NULL ),
_p_voiceNetwork( NULL ),
_p_transport( NULL ),
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

    if ( _p_transport )
        delete _p_transport;
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

        case YAF3D_NOTIFY_NETWORKING_ESTABLISHED:

            if ( _p_transport )
                _p_transport->initialize();

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
        assert( !_p_transport && "transport layer already exists!" );
        // setup the transport layer
        _p_transport = new VoiceTransport;

        _inputGain = inputgain;

        // create a sound receiver
        _p_receiver = new VoiceReceiver( this, _p_transport );
        _p_receiver->initialize();
        _p_receiver->setOutputGain( outputgain );
        _p_receiver->setSpotRange( _spotRange );

        // create the network manager for voice chat clients
        _p_voiceNetwork = new VoiceNetwork;
        _p_voiceNetwork->initialize();
        _p_voiceNetwork->setHotspotRange( _spotRange );

        // create sound codec
        _p_codec = new NetworkSoundCodec;
        _p_codec->setEncoderComplexity( VOICE_CODEC_COMPLEXITY );
        _p_codec->setEncoderQuality( VOICE_CODEC_QUALITY );
        _p_codec->setupEncoder();

#ifdef INPUT_TEST
        _p_soundInput = new VoiceFileInput( INPUT_TEST_FILE, NULL, _p_codec );
        _p_soundInput->initialize();
        _p_soundInput->setInputGain( _inputGain );
        _p_soundInput->stop( true );
#else

        //unsigned int inputdevice;
        //yaf3d::Configuration::get()->getSettingValue( VRC_GS_VOICECHAT_INPUT_DEVICE, inputdevice );
        _p_soundInput = new VoiceMicrophoneInput( NULL, _p_codec );
        _p_soundInput->initialize();
        _p_soundInput->setInputGain( _inputGain );
        // we begin with input grabbing when at least one sender is active
        _p_soundInput->stop( true );
#endif
    }
    catch ( const NetworkSoundException& e )
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

    // set callback to get notifications on changed voice chat list
    vrc::gameutils::PlayerUtils::get()->registerCallbackVoiceChatPlayerListChanged( this, true );

    // set callback to get notifications on changed hotspot
    _p_voiceNetwork->registerCallbackHotspotChanged( this, true );

    // set active flag
    _active = true;
}

void EnNetworkVoice::onServerDisconnect( int /*sessionID*/ )
{
    if ( _active )
        destroyVoiceChat();
}

void EnNetworkVoice::destroyVoiceChat()
{
    assert( _active && "trying to destroy voice chat entity which has not been created before!" );

    try
    {
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
            _p_receiver = NULL;
        }

        // now destroy voice network
        if ( _p_voiceNetwork )
        {
            _p_voiceNetwork->registerCallbackHotspotChanged( this, false );
            _p_voiceNetwork->shutdown();
            delete _p_voiceNetwork;
            _p_voiceNetwork = NULL;
        }

        if ( _p_soundInput )
        {
            delete _p_soundInput;
            _p_soundInput = NULL;
        }

        if ( _p_codec )
        {
            delete _p_codec;
            _p_codec = NULL;
        }

        if ( _p_transport )
        {
            delete _p_transport;
            _p_transport = NULL;
        }
    }
    catch ( const NetworkSoundException& e )
    {
        log_error << ENTITY_NAME_NETWORKVOICE << ":" << getInstanceName() << "  a problem occurred during shutdown." << std::endl;
        log_error << "  reason: " << e.what() << std::endl;
    }

    // deregister from getting periodic updates
    yaf3d::EntityManager::get()->registerUpdate( this, false );

    // deregister from getting notifications on changed voice chat list
    vrc::gameutils::PlayerUtils::get()->registerCallbackVoiceChatPlayerListChanged( this, false );

    // reset active flag
    _active = false;
}

void EnNetworkVoice::updateEntity( float deltaTime )
{
    // update the voice network state
    if ( _p_voiceNetwork )
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
            break;
        }
        else
        {
            p_beg->second->update( deltaTime );
        }
    }
}

void EnNetworkVoice::destroySender( unsigned int senderID )
{
    SenderMap::iterator p_sender = _sendersMap.begin(), p_senderEnd = _sendersMap.end();
    for( ; p_sender != p_senderEnd; ++p_sender )
    {
        VoiceSender* p_voicesender = dynamic_cast< VoiceSender* >( p_sender->second );
        assert( p_voicesender && "invalid object type!" );
        if ( p_voicesender->getSenderID() == senderID )
        {
            p_voicesender->die();
            break;
        }
    }
}

void EnNetworkVoice::updateHotspot( yaf3d::BaseEntity* p_entity, bool joining )
{
    assert( p_entity && "invalid entity" );

    SenderMap::iterator p_hit = _sendersMap.find( p_entity );
    if ( joining )
    {
        if ( p_hit != _sendersMap.end() )
        {
            log_warning << "EnNetworkVoice: entity is already in internal hotspot map!" << std::endl;
            return;
        }

        // determine receiver's ip
        VoiceNetwork::VoiceClientMap::const_iterator p_vmapend = _p_voiceNetwork->getHotspot().end();
        VoiceNetwork::VoiceClientMap::const_iterator p_vmapent = _p_voiceNetwork->getHotspot().find( p_entity );
        if ( p_vmapent == p_vmapend )
        {
            log_error << " EnNetworkVoice: internal error, voice client's receiver address cannot be determined." << std::endl;
        }
        else
        {
            // create a new sender
            int sid = p_vmapent->second;
            BaseNetworkSoundImplementation* p_sender = new VoiceSender( sid, _p_soundInput, _p_transport );
            p_sender->initialize();
            _sendersMap[ p_entity ] = p_sender;

            // continue grabbing input if it was stopped before
            if ( _sendersMap.size() == 1 )
                _p_soundInput->stop( false );
        }
    }
    else
    {
        // check if the sender is in hotspot map, if so remove it
        if ( p_hit != _sendersMap.end() )
        {
            // destroy the sender
            p_hit->second->shutdown();
            delete p_hit->second;
            _sendersMap.erase( p_hit );
            // stop input grabbing when no senders exist
            if ( _sendersMap.size() == 0 )
                _p_soundInput->stop( true );
        }
    }
}

void EnNetworkVoice::onVoiceChatPlayerListChanged( bool joining, yaf3d::BaseEntity* p_entity )
{
    log_verbose << "EnNetworkVoice: voice chat player changed, player " << p_entity->getInstanceName() << ( joining ? " joining" : " leaving" ) << std::endl;

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

            // remove also the player from voice receiver immediately
            _p_receiver->removePlayer( p_entity );
        }
        else
        {
            log_error << "EnNetworkVoice: leaving player could not be found in sender map" << std::endl;
        }
    }
}

void EnNetworkVoice::onHotspotChanged( bool joining, yaf3d::BaseEntity* p_entity )
{
    // update the hotspot when clients enter / leave the hotspot range
    updateHotspot( p_entity, joining );
}

} // namespace vrc
