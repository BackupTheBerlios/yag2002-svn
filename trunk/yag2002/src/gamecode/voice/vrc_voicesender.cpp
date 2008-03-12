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

//! TODO: we should move this to general header and choose a better name for it
#define VOICE_RESPOND_TIMEOPUT  0.5f


namespace vrc
{

VoiceSender::VoiceSender( int sid, BaseVoiceInput* p_soundInput, VoiceTransport* p_transport ) :
_receiverSID( sid ),
_senderState( Start ),
_p_soundInput( p_soundInput ),
_p_transport( p_transport ),
_pingTimer( 0.0f ),
_pongTimer( 0.0f ),
_respondTimer( 0.0f ),
_paketStamp( 0 ),
_isAlive( true )
{
    assert( p_soundInput && "invalid sound input" );
    _p_voicePaket = new VoicePaket;
    _p_voicePaket->_paketStamp = _paketStamp;
    _p_voicePaket->_senderID   = 0;
}

VoiceSender::~VoiceSender()
{
    if ( _p_transport )
    {
        // de-register from transport layer
        _p_transport->registerReceiver( this, false );
    }

    // deregister from input stream
    _p_soundInput->registerStreamSink( this, false );

    // shutdown
    shutdown();
}

void VoiceSender::shutdown()
{
    if ( _p_voicePaket )
    {
        delete _p_voicePaket;
        _p_voicePaket = NULL;
    }
}

void VoiceSender::initialize() throw( NetworkSoundException )
{
    // register for getting audio input stream
    _p_soundInput->registerStreamSink( this, true );

    // get the sender socket
    _p_sendSocket = _p_transport->getSocket();

    // get peer's NAT information
    _p_transport->getClientInfo( _receiverSID, this );
}

void VoiceSender::recvClientAddress( int sid, const RNReplicaNet::XPAddress& address )
{
    log_verbose << "  <- trying to connect the receiver: " << address.Export() << std::endl;
    _receiverAddress = address;
    _p_transport->registerReceiver( this, true );

    _senderState = Initial;
}

static int pakets = 0;
static float t = 0.0f;


void VoiceSender::update( float deltaTime )
{
    // if sender is dead then do not update, we will be removed soon
    if ( !_isAlive )
        return;

//! TODO: remove this
#if 0
t += deltaTime;
if ( t > 1.0f )
{
    log_verbose << "sender time: " << t << ", packets sent: " << pakets << std::endl;
    t -= 1.0f;
    pakets = 0;
}
#endif

    assert( _p_sendSocket && "invalid socket" );
    assert( _p_soundInput && "invalid sound input instance" );

    // handle the initial state
    if ( _senderState == Initial )
    {
        _p_voicePaket->_typeId   = VOICE_PAKET_TYPE_CON_REQ;
        _p_voicePaket->_length   = 4;
        _p_voicePaket->_senderID = 0; // this must be 0 on connection!
        // put the player network id into data buffer
        int sid = yaf3d::NetworkDevice::get()->getSessionID();
        *( reinterpret_cast< int* >( _p_voicePaket->_p_buffer ) ) = sid;

        _p_sendSocket->Send( reinterpret_cast< char* >( _p_voicePaket ), VOICE_PAKET_HEADER_SIZE + _p_voicePaket->_length, _receiverAddress );
        _senderState = RequestConnection;

        log_verbose << "  <- request for joining voice session from: " << _receiverAddress.Export() << " ... " << std::endl;

        return;
    }

    // check request timeout and try again
    if ( _senderState == RequestConnection )
    {
        _respondTimer += deltaTime;
        if ( _respondTimer > VOICE_RESPOND_TIMEOPUT )
        {
            _respondTimer = 0.0f;
            _senderState  = Initial;
        }
    }

    // send out a ping to receiver
    _pingTimer += deltaTime;
    if ( _pingTimer > ( VOICE_LIFESIGN_PERIOD * 0.5f ) )
    {
        _pingTimer = 0.0f;
        _p_voicePaket->_length   = 0;
        _p_voicePaket->_typeId   = VOICE_PAKET_TYPE_CON_PING;
        _p_voicePaket->_senderID = _senderID;
        _p_sendSocket->Send( reinterpret_cast< char* >( _p_voicePaket ), VOICE_PAKET_HEADER_SIZE, _receiverAddress );
    }

    // check for receiver's pong
    _pongTimer += deltaTime;
    if ( _pongTimer > VOICE_LIFESIGN_PERIOD )
    {
        // lost the receiver
        log_verbose << "  <- voice chat receiver does not respond, going dead ..." << std::endl;
        _pongTimer = 0.0f;

        // set the alive flag to false, this flag is polled by netvoice entity and handled appropriately
        _isAlive = false;
        // set the state to Initial so the input functor gets inactive too
        _senderState = Initial;
    }
}

void VoiceSender::operator ()( char* p_encodedaudio, unsigned short length )
{ // this operator is called by voice input instance ( encoded audio is passed in p_encodedaudio ), it runs in an own thread context!

    static VoicePaket packet;

    if ( ( _senderState != ConnectionReady ) || !_isAlive )
        return;

    // wrap-around in ring buffer
    if ( ( _sendBufferWritePos + length ) > VOICE_SEND_RING_BUF_SIZE )
    {
        int lentail = VOICE_SEND_RING_BUF_SIZE - _sendBufferReadPos;
        memcpy( _p_sendBuffer, &_p_sendBuffer[ _sendBufferReadPos ], lentail );
        memcpy( &_p_sendBuffer[ lentail ], p_encodedaudio, length );
        _sendBufferWritePos = length + lentail;
        _sendBufferReadPos  = 0;
    }
    else
    {
        memcpy( &_p_sendBuffer[ _sendBufferWritePos ], p_encodedaudio, length );
        _sendBufferWritePos += length;
    }

    // we use a ring buffer in order to efficiently load the network packets, thus every packet carries 4 encoded frames
    if ( ( _sendBufferWritePos - _sendBufferReadPos ) > VOICE_SEND_PACKET_BUF_SIZE )
    {
        // transmit encoded input over net
        packet._typeId     = VOICE_PAKET_TYPE_VOICE_DATA;
        packet._paketStamp = ++_paketStamp;
        packet._senderID   = _senderID;
        packet._length     = VOICE_SEND_PACKET_BUF_SIZE;
        memcpy( packet._p_buffer, &_p_sendBuffer[ _sendBufferReadPos ], VOICE_SEND_PACKET_BUF_SIZE );

        _sendBufferReadPos += VOICE_SEND_PACKET_BUF_SIZE;

        _p_sendSocket->Send( reinterpret_cast< char* >( &packet ), VOICE_PAKET_HEADER_SIZE + packet._length, _receiverAddress );

pakets++;
    }
}

bool VoiceSender::recvPacket( VoicePaket* p_packet, const RNReplicaNet::XPAddress& senderaddr )
{
    bool handled = true;

    // update protocol state
    switch ( _senderState )
    {
        case RequestConnection:
        {
            if ( p_packet->_typeId == VOICE_PAKET_TYPE_CON_GRANT )
            {
                // store assigned sender ID
                _senderID = p_packet->_senderID;
                _senderState = ConnectionReady;
                std::stringstream msg;
                msg << "  <- receiver granted joining with ID: " << _senderID;
                log_debug << msg.str() << std::endl;
            }
            else if ( p_packet->_typeId == VOICE_PAKET_TYPE_CON_DENY )
            {
                log_debug << "  <- receiver denied joining " << std::endl;

                // kill this sender
                _isAlive = false;
                // set the state to Initial so the input functor gets inactive too
                _senderState = Initial;
            }
            else
            {
                handled = false;
            }
        }
        break;

        // maintain the periodic alive signaling
        case ConnectionReady:
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
