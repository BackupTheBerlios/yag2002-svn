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

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_voicesender.h"
#include "vrc_codec.h"
#include <RNXPSession/Inc/XPSession.h>
#include <RNXPURL/Inc/TransportConfig.h>

namespace vrc
{

VoiceSender::VoiceSender( const std::string& receiverIP, BaseVoiceInput* p_soundInput ) :
_receiverIP( receiverIP ),
_p_udpTransport( NULL ),
_senderState( Initial ),
_p_soundInput( p_soundInput ),
_pingTimer( 0.0f ),
_pongTimer( 0.0f ),
_paketStamp( 0 ),
_senderID( 0 ),
_isAlive( true )
{
    assert( receiverIP.length() && "invalid server IP" );
    assert( p_soundInput && "invalid sound input" );
    _p_voicePaket = new VoicePaket;
    _p_voicePaket->_paketStamp = _paketStamp;
    _p_voicePaket->_senderID   = 0;
}

VoiceSender::~VoiceSender()
{
    shutdown();

    // deregister from input stream
    _p_soundInput->registerStreamSink( this, false );
}

void VoiceSender::shutdown()
{ 
    if ( _p_udpTransport )
    {
        delete _p_udpTransport;
        _p_udpTransport = NULL;
    }

    if ( _p_voicePaket )
    {
        delete _p_voicePaket;
        _p_voicePaket = NULL;
    }
}

void VoiceSender::initialize() throw( NetworkSoundExpection )
{
    if ( !_receiverIP.length() )
    {
        log_error << " cannot setup voice sender, invalid receiver ip" << std::endl;
        throw NetworkSoundExpection( "Invalid Receiver IP address" ) ;
    }

    // register for getting audio input stream
    _p_soundInput->registerStreamSink( this, true );

    // setup networking
    {
        int channel;
        yaf3d::Configuration::get()->getSettingValue( VRC_GS_VOICECHAT_CHANNEL, channel );

        // Enable packet buffering and set paket update time for transport layer
        RNReplicaNet::TransportConfig::SetPacketBufferEnable( true );
        RNReplicaNet::TransportConfig::SetPacketBufferTime( 1.0f / 60.0f );

        RNReplicaNet::XPURL::RegisterDefaultTransports();
        RNReplicaNet::XPURL xpurl;

        assert( xpurl.FindTransport( "UDP@" ) );

        _p_udpTransport = xpurl.FindTransport( "UDP@" )->Allocate();
        std::stringstream assembledUrl;
        assembledUrl << "UDP@" << _receiverIP << ":" << channel;// << "/" << VOICE_SERVER_NAME;
        RNReplicaNet::Transport::Error res = _p_udpTransport->Connect( assembledUrl.str() );
        if ( res != RNReplicaNet::Transport::kTransport_EOK )
            log_error << "  -> cannot connect to receiver on channel: " << assembledUrl.str() << std::endl;
        else
            log_debug << "  -> trying to connect the receiver: " << assembledUrl.str() << " ..." << std::endl;
    }
}

void VoiceSender::update( float deltaTime )
{
    // if sender is dead then do not update, we will be removed soon
    if ( !_isAlive )
        return;

    assert( _p_soundInput && "invalid sound input instance" );
    assert( _p_udpTransport && "network session is not available!" );

    // handle the initial state
    if ( _senderState == Initial )
    {
        // check if connection is established
        RNReplicaNet::Transport::Error res = _p_udpTransport->GetStatus();
        if ( res != RNReplicaNet::Transport::kTransport_EOK )
        {
            // check the transport connection timeout
            _pongTimer += deltaTime;
            if ( _pongTimer > VOICE_LIFESIGN_PERIOD )
            {
                log_debug << "  -> receiver does not respond, giving up! " << std::endl;
                _isAlive = false;
            }
            return;
        }
        _pongTimer = 0.0f;

        _p_voicePaket->_typeId    = NETWORKSOUND_PAKET_TYPE_CON_REQ;
        _p_voicePaket->_length    = 0;
        _p_udpTransport->SendReliable( reinterpret_cast< char* >( _p_voicePaket ), VOICE_PAKET_HEADER_SIZE + _p_voicePaket->_length );
        _senderState = RequestConnection;

        log_debug << "  -> ... connected, requesting for joining voice session ... " << std::endl;

        return;
    }

    // check for new arrived data pakets
    static char s_buffer[ 512 ];
    while ( _p_udpTransport->Recv( s_buffer, 512 ) > 0 )
    {
        // update protocol state
        switch ( _senderState )
        {
            case RequestConnection:
            {
                VoicePaket* p_data = reinterpret_cast< VoicePaket* >( s_buffer );
                if ( p_data->_typeId == NETWORKSOUND_PAKET_TYPE_CON_GRANT )
                {
                    // store assigned sender ID
                    _senderID = p_data->_senderID;
                    _senderState = ConnectionReady;
                    std::stringstream msg;
                    msg << "  -> receiver granted joining with ID: " << _senderID;
                    log_debug << msg.str() << std::endl;
                }
                else if ( p_data->_typeId == NETWORKSOUND_PAKET_TYPE_CON_DENY )
                {
                    _senderState = ConnectionDenied;
                    log_debug << "  -> receiver denied joining " << std::endl;
                }
            }
            break;

            // maintain the periodic alive signaling
            case ConnectionReady:
            {
                // check for receiver's pong paket
                // reset pong timer when a pong paket arrived
                VoicePaket* p_data = reinterpret_cast< VoicePaket* >( s_buffer );
                if ( ( p_data->_typeId == NETWORKSOUND_PAKET_TYPE_CON_PONG ) && ( _senderID == p_data->_senderID ) )
                {
                    _pongTimer = 0.0f;
                }
            }
            break;

            default:
                ;
        }
    }

    // send out a ping to receiver
    _pingTimer += deltaTime;
    if ( _pingTimer > ( VOICE_LIFESIGN_PERIOD / 10.0f ) )
    {
        _pingTimer = 0.0f;
        _p_voicePaket->_length   = 0;
        _p_voicePaket->_typeId   = NETWORKSOUND_PAKET_TYPE_CON_PING;
        _p_voicePaket->_senderID = _senderID;
        _p_udpTransport->SendReliable( reinterpret_cast< char* >( _p_voicePaket ), VOICE_PAKET_HEADER_SIZE );
    }

    //!FIXME: checking for lifesign, i.e. checking the ping/pong timers, causes a problem 
    //        when dragging the app window the gameloop stucks which causes lifetime exceeding!

    // check for receiver's pong
    _pongTimer += deltaTime;
    if ( _pongTimer > VOICE_LIFESIGN_PERIOD )
    {
        // lost the receiver
        log_verbose << "  -> voice chat receiver does not respond, going dead ..." << std::endl;
        _pongTimer = 0.0f;

        // set the alive flag to false, this flag is polled by netvoice entity and handled appropriately
        _isAlive = false;
        // set the state to Initial so the input functor gets inactive too
        _senderState = Initial;
    }
}

void VoiceSender::operator ()( char* p_encodedaudio, unsigned short length )
{ // this operator is called by voice input instance ( encoded audio is passed in p_encodedaudio )

    if ( _senderState != ConnectionReady )
        return;

    // transmit encoded input over net
    _p_voicePaket->_paketStamp = ++_paketStamp;
    _p_voicePaket->_typeId     = NETWORKSOUND_PAKET_TYPE_VOICE_DATA;
    _p_voicePaket->_senderID   = _senderID;
    _p_voicePaket->_length     = length;
    memcpy( _p_voicePaket->_p_buffer, p_encodedaudio, length );
    _p_udpTransport->Send( reinterpret_cast< char* >( _p_voicePaket ), VOICE_PAKET_HEADER_SIZE + _p_voicePaket->_length );
}

} // namespace vrc
