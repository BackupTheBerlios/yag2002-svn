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
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_voicesender.h"
#include "vrc_voicetestutils.h"
#include "vrc_codec.h"
#include <RNXPSession/Inc/XPSession.h>

//! Timeout for connection request
#define VOICE_REQ_CON_TIMEOPUT  0.5f

namespace vrc
{

VoiceSender::VoiceSender( int sid, BaseVoiceInput* p_soundInput, VoiceTransport* p_transport ) :
_receiverSID( sid ),
_senderState( eStart ),
_p_soundInput( p_soundInput ),
_p_transport( p_transport ),
_pingTimer( 0.0f ),
_pongTimer( 0.0f ),
_respondTimer( 0.0f ),
_paketStamp( 0 ),
_isAlive( true )
{
    assert( p_soundInput && "invalid sound input" );
    _p_voicePacket = new VoicePaket;
    _p_voicePacket->_paketStamp = _paketStamp;
    _p_voicePacket->_senderID   = 0;

    // clean the receiver address, the XPAddress constructor seems not to clean the port!
    _receiverAddress.port      = 0;
    memset( _receiverAddress.addr, 0, sizeof( _receiverAddress.addr ) );
}

VoiceSender::~VoiceSender()
{
    log_verbose << "VoiceSender: shutdown" << std::endl;

    // shutdown
    shutdown();
}

void VoiceSender::shutdown()
{
    // we use this pointer for knowing if we have already been shut down
    if ( _p_voicePacket )
    {
        delete _p_voicePacket;
        _p_voicePacket = NULL;

        // set sender state to start in order to avoid further voice packet processing
        _senderState = eStart;

        // deregister from input stream
        _p_soundInput->registerStreamSink( this, false );

        // sleep a while in order to avoid a conflict with sender's audio codec and packet receive callback
        OpenThreads::Thread::microSleep( 50000 );

        if ( _p_transport )
        {
            // de-register from transport layer
            _p_transport->registerReceiver( this, false );
        }
    }
}

void VoiceSender::initialize() throw( NetworkSoundException )
{
    log_verbose << "VoiceSender: initializing ..." << std::endl;

    // register for getting audio input stream
    _p_soundInput->registerStreamSink( this, true );

    // get the sender socket
    _p_sendSocket = _p_transport->getSocket();

    // get peer's NAT information
    _p_transport->getClientInfo( _receiverSID, this );
}

void VoiceSender::recvClientAddress( int /*sid*/, const RNReplicaNet::XPAddress& address )
{
    //log_verbose << "  <- trying to connect the receiver: " << address.Export() << std::endl;
    _receiverAddress = address;
    _p_transport->registerReceiver( this, true );

    _senderState = eInitial;
}

void VoiceSender::update( float deltaTime )
{
    // if sender is dead then do not update, we will be removed soon
    if ( !_isAlive || ( _senderState == eStart ) )
        return;

    assert( _p_sendSocket && "invalid socket" );
    assert( _p_soundInput && "invalid sound input instance" );

    // handle the initial state
    if ( _senderState == eInitial )
    {
        _p_voicePacket->_typeId   = VOICE_PAKET_TYPE_CON_REQ;
        _p_voicePacket->_length   = 4;
        _p_voicePacket->_senderID = 0; // this must be 0 on connection!
        // put the player network id into data buffer
        int sid = yaf3d::NetworkDevice::get()->getSessionID();
        *( reinterpret_cast< int* >( _p_voicePacket->_p_buffer ) ) = sid;

        _p_sendSocket->Send( reinterpret_cast< char* >( _p_voicePacket ), VOICE_PAKET_HEADER_SIZE + _p_voicePacket->_length, _receiverAddress );
        _senderState = eRequestConnection;

        //log_verbose << "  <- requesting " << _receiverAddress.Export() << " for joining voice session " << " ... " << std::endl;

        return;
    }

    // check request timeout and try again
    if ( _senderState == eRequestConnection )
    {
        _respondTimer += deltaTime;
        if ( _respondTimer > VOICE_REQ_CON_TIMEOPUT )
        {
            _respondTimer = 0.0f;
            _senderState  = eInitial;
        }
    }

    // send out a ping to receiver
    _pingTimer += deltaTime;
    if ( _pingTimer > ( VOICE_LIFESIGN_PERIOD / 5.0f ) )
    {
        _pingTimer = 0.0f;
        _p_voicePacket->_length   = 0;
        _p_voicePacket->_typeId   = VOICE_PAKET_TYPE_CON_PING;
        _p_voicePacket->_senderID = _senderID;
        _p_sendSocket->Send( reinterpret_cast< char* >( _p_voicePacket ), VOICE_PAKET_HEADER_SIZE, _receiverAddress );
    }

    // check for receiver's pong
    _pongTimer += deltaTime;
    if ( _pongTimer > VOICE_LIFESIGN_PERIOD )
    {
        // lost the receiver
        log_verbose << "  <- voice chat receiver does not respond, going dead ..." << std::endl;
        _pongTimer = 0.0f;

        // retry to connect to receiver
        _senderState = eInitial;
    }
}

void VoiceSender::recvEncodedAudio( char* p_encodedaudio, unsigned int length, unsigned int encframesize )
{ // this operator is called by voice input instance ( encoded audio is passed in p_encodedaudio ), it runs in an own thread context!

    static VoicePaket packet;

    if ( ( _senderState != eConnectionReady ) || !_isAlive )
        return;

    // transmit encoded input over net
    packet._typeId  = VOICE_PAKET_TYPE_VOICE_DATA;
    packet._senderID= _senderID;

    int bufferlen = length;
    int bufferpos = 0;
    int packetlen = 0;
    int iencframesize = int( encframesize );
    while ( bufferlen >= iencframesize )
    {
        // we can put several encoded frames into one packet
        packetlen = std::min( ( VOICE_PAKET_MAX_BUF_SIZE / iencframesize ) * iencframesize, bufferlen ); // try to fit as much encoded frames as possible

        packet._paketStamp = ++_paketStamp;
        packet._length     = packetlen;
        memcpy( packet._p_buffer, &p_encodedaudio[ bufferpos ], packetlen );
        _p_sendSocket->Send( reinterpret_cast< char* >( &packet ), VOICE_PAKET_HEADER_SIZE + packetlen, _receiverAddress );

        bufferlen -= packetlen;
        bufferpos += packetlen;
    }
}

bool VoiceSender::recvPacket( VoicePaket* p_packet, const RNReplicaNet::XPAddress& /*senderaddr*/ )
{
    bool handled = true;

    // update protocol state
    switch ( _senderState )
    {
        case eRequestConnection:
        {
            if ( p_packet->_typeId == VOICE_PAKET_TYPE_CON_GRANT )
            {
                // store assigned sender ID
                _senderID = p_packet->_senderID;
                _senderState = eConnectionReady;
                std::stringstream msg;
                msg << "  <- receiver granted joining with ID: " << _senderID;
                log_debug << msg.str() << std::endl;
            }
            else if ( p_packet->_typeId == VOICE_PAKET_TYPE_CON_DENY )
            {
                log_debug << "  <- receiver denied joining " << std::endl;

                // kill this sender
                _isAlive = false;
                // set the state to Initial so the input callback gets inactive too
                _senderState = eInitial;
            }
            else
            {
                handled = false;
            }
        }
        break;

        // maintain the periodic alive signaling
        case eConnectionReady:
        {
            // check for receiver's pong paket
            // reset pong timer when a pong paket arrived
            if ( ( p_packet->_typeId == VOICE_PAKET_TYPE_CON_PONG ) && ( _senderID == p_packet->_senderID ) )
            {
                _pongTimer = 0.0f;
            }
            else
            {
                handled = false;
            }
        }
        break;

        default:
            handled = false;
    }

    return handled;
}

} // namespace vrc
