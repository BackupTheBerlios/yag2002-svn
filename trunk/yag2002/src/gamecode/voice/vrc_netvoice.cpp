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

namespace vrc
{

//! Implement and register the network voice entity factory
YAF3D_IMPL_ENTITYFACTORY( NetworkVoiceEntityFactory )

EnNetworkVoice::EnNetworkVoice():
_spotRange( 20.0f ),
_p_receiver( NULL ),
_p_voiceNetwork( NULL ),
_p_codec( NULL ),
_p_soundInput( NULL )
{
    // register entity attributes
    getAttributeManager().addAttribute( "spotRange",       _spotRange  );
    getAttributeManager().addAttribute( "testSoundFile",   _testFile   );
}

EnNetworkVoice::~EnNetworkVoice()
{
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
        }

        // remove callbacks
        vrc::gameutils::PlayerUtils::get()->registerFunctorVoiceChatPlayerListChanged( this, false );
        _p_voiceNetwork->registerFunctorHotspotChanged( this, false );

        // now destroy voice network
        if ( _p_voiceNetwork )
        {
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
}

void EnNetworkVoice::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle notifications
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_MENU_LEAVE:
            break;

        case YAF3D_NOTIFY_SHUTDOWN:
            break;

        default:
            ;
    }
}

void EnNetworkVoice::initialize()
{
    try
    {
        // create a sound receiver
        _p_receiver = new VoiceReceiver();
        _p_receiver->initialize();

        _p_voiceNetwork = new VoiceNetwork;
        _p_voiceNetwork->initialize();
        _p_voiceNetwork->setHotspotRange( _spotRange );

        // create sound compression codec
        _p_codec = new NetworkSoundCodec;
        _p_codec->setEncoderComplexity( 9 );
        _p_codec->setEncoderQuality( 10 );
        _p_codec->setupEncoder();

        // create a file input as voice source for testing if a test file is given
        // the file must have following format: PCM16, mono, 8 kHz
        // if no test file given then we take the microphone as input ( normal case )
        if ( _testFile.length() )
            _p_soundInput = new VoiceFileInput( _testFile, NULL, _p_codec );
        else
            _p_soundInput = new VoiceMicrophoneInput( NULL, _p_codec );

        _p_soundInput->initialize();
        // we begin with input grabbing when at least one sender is active
        _p_soundInput->stop( true );
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

    // set callback to get notifications on changed voice chat list
    vrc::gameutils::PlayerUtils::get()->registerFunctorVoiceChatPlayerListChanged( this, true );

    // set callback to get notifications on changed hotspot
    _p_voiceNetwork->registerFunctorHotspotChanged( this, true );
}

void EnNetworkVoice::updateEntity( float deltaTime )
{
    // update the voice network state
    _p_voiceNetwork->update( deltaTime );

    // update senders
    SenderMap::iterator p_beg = _sendersMap.begin(), p_end = _sendersMap.end();
    for ( ; p_beg != p_end; ++p_beg )
        p_beg->second->update( deltaTime );

    // update audio input
    _p_soundInput->update();

    // update our receiver
    if ( _p_receiver )
        _p_receiver->update( deltaTime );
}

void EnNetworkVoice::updateHotspot( yaf3d::BaseEntity* p_entity, bool joining )
{
    assert( p_entity && "invalid entity" );

    SenderMap::iterator hit = _sendersMap.find( p_entity );
    if ( joining )
    {
        assert( ( hit == _sendersMap.end() ) && "entity is already in internal hotspot map!" );

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
        assert( ( hit != _sendersMap.end() ) && "entity is not in internal hotspot map!" );

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
    // let the voice network manager update its internal client list when 
    if ( joining )
        _p_voiceNetwork->updateVoiceClients( p_entity, true );
    else
        _p_voiceNetwork->updateVoiceClients( p_entity, false );
}

void EnNetworkVoice::operator()( bool joining, yaf3d::BaseEntity* p_entity )
{
    updateHotspot( p_entity, joining );
}

} // namespace vrc
