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

namespace vrc
{

VoiceSender::VoiceSender( const std::string& receiverIP, BaseVoiceInput* p_soundInput ) :
_receiverIP( receiverIP ),
_p_udpTransport( NULL ),
_senderState( Initial ),
_p_soundInput( p_soundInput ),
_pingTimer( 0.0f ),
_pongTimer( 0.0f ),
_paketStamp( 0 )
{
    assert( receiverIP.length() && "invalid server IP" );
    assert( p_soundInput && "invalid sound input" );
    _p_voicePaket = new VoicePaket;
    _p_voicePaket->_paketStamp = _paketStamp;
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

        RNReplicaNet::XPURL::RegisterDefaultTransports();
        RNReplicaNet::XPURL xpurl;

        assert( xpurl.FindTransport( "UDP@" ) );

        _p_udpTransport = xpurl.FindTransport( "UDP@" )->Allocate();
        std::stringstream assembledUrl;
        assembledUrl << "UDP@" << _receiverIP << ":" << channel << "/" << VOICE_SERVER_NAME;
        _p_udpTransport->Connect( assembledUrl.str() );
        log_debug << "  -> trying to connect the receiver: " << assembledUrl.str() << " ..." << std::endl;
    }
}

void VoiceSender::update( float deltaTime )
{
    assert( _p_soundInput && "invalid sound input instance" );
    assert( _p_udpTransport && "network session is not available!" );

    // check for new arrived data pakets
    bool        datareceived = false;
    static char s_buffer[ 512 ];
    if ( !_p_udpTransport->Recv( s_buffer, 512 ) )
        datareceived = true;

    // update protocol state
    switch ( _senderState )
    {
        case Initial:
        {
            // check if connection is established
            if ( _p_udpTransport->GetStatus() != RNReplicaNet::Transport::kTransport_EOK )
                break;
            _p_voicePaket->_typeId    = NETWORKSOUND_PAKET_TYPE_CON_REQ;
            _p_voicePaket->_length    = 0;
            _p_udpTransport->SendReliable( reinterpret_cast< char* >( _p_voicePaket ), VOICE_PAKET_HEADER_SIZE + _p_voicePaket->_length );
            _senderState = RequestConnection;

            log_debug << "  -> ... connected, requesting for joining voice session ... " << std::endl;
        }
        break;

        case RequestConnection:
        {
            if ( !datareceived ) 
                break;

            VoicePaket* p_data = reinterpret_cast< VoicePaket* >( s_buffer );
            if ( p_data->_typeId == NETWORKSOUND_PAKET_TYPE_CON_GRANT )
            {
                _senderState = ConnectionReady;
                log_debug << "  -> receiver granted joining " << std::endl;
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
            // send out a ping to receiver
            _pingTimer += deltaTime;
            if ( _pingTimer > VOICE_LIFESIGN_PERIOD )
            {
                _pingTimer = 0.0f;
                _p_voicePaket->_length = 0;
                _p_voicePaket->_typeId = NETWORKSOUND_PAKET_TYPE_CON_PING;
                _p_udpTransport->SendCertain( reinterpret_cast< char* >( _p_voicePaket ), VOICE_PAKET_HEADER_SIZE );
            }

            // check for receiver's pong
            _pongTimer += deltaTime;
            if ( _pongTimer > ( VOICE_LIFESIGN_PERIOD * 5.0f ) )
            {
                // lost the receiver
                //! TODO: figure out an appropriate strategy for this situation
                log_error << "voice chat receiver does not respond!" << std::endl;
                _pongTimer = 0.0f;
            }

            if ( !datareceived ) 
                break;

            // reset pong timer when a pong paket arrived
            VoicePaket* p_data = reinterpret_cast< VoicePaket* >( s_buffer );
            if ( p_data->_typeId == NETWORKSOUND_PAKET_TYPE_CON_PONG )
                _pongTimer = 0.0f;
        }
        break;

        default:
            ;
    }
}

void VoiceSender::operator ()( char* p_encodedaudio, unsigned short length )
{ // this operator is called by voice input instance ( encoded audio is passed in p_encodedaudio )

    if ( _senderState != ConnectionReady )
        return;

    // transmit encoded input over net
    _p_voicePaket->_paketStamp = ++_paketStamp;
    _p_voicePaket->_typeId     = NETWORKSOUND_PAKET_TYPE_VOICE_DATA;
    _p_voicePaket->_length     = length;
    memcpy( _p_voicePaket->_p_buffer, p_encodedaudio, length );
    _p_udpTransport->SendReliable( reinterpret_cast< char* >( _p_voicePaket ), VOICE_PAKET_HEADER_SIZE + _p_voicePaket->_length );
}

} // namespace vrc
