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


//! TODO:
//1. share the codec in senders just like the voice input
//   the codec should host the input, the sender should host the codec
//
//2. close connection before destroying a sender, the receiver is accumulating sound nodes without releasing them!

namespace vrc
{

VoiceSender::VoiceSender( const std::string& receiverIP, BaseVoiceInput* p_soundInput ) :
_receiverIP( receiverIP ),
_p_udpTransport( NULL ),
_senderState( Initial ),
_p_soundInput( p_soundInput )
{
    assert( receiverIP.length() && "invalid server IP" );
    assert( p_soundInput && "invalid sound input" );
    _p_voicePaket = new VoicePaket;
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
        yaf3d::Configuration::get()->getSettingValue( VRC_GS_VOICECHAT_CHAN_SND, channel );

        RNReplicaNet::XPURL::RegisterDefaultTransports();
        RNReplicaNet::XPURL xpurl;

        assert( xpurl.FindTransport( "UDP@" ) );

        _p_udpTransport = xpurl.FindTransport( "UDP@" )->Allocate();
        std::stringstream assembledUrl;
        assembledUrl << "UDP@" << _receiverIP << ":" << channel << "/" << VOICE_SERVER_NAME;
        _p_udpTransport->Connect( assembledUrl.str() );
    }
}

void VoiceSender::update( float deltaTime )
{
    assert( _p_soundInput && "invalid sound input instance" );

    // update the networking state
    updateNetwork();
}

void VoiceSender::operator ()( char* p_encodedaudio, unsigned short length )
{ // this operator is called by voice input instance

    if ( _senderState != ConnectionReady )
        return;

    // transmit encoded input over net
    _p_voicePaket->_typeId   = NETWORKSOUND_PAKET_TYPE_VOICE_DATA;
    _p_voicePaket->_length   = length;
    memcpy( _p_voicePaket->_p_buffer, p_encodedaudio, length );
    _p_udpTransport->SendCertain( reinterpret_cast< char* >( _p_voicePaket ), 4  + _p_voicePaket->_length );
}

void VoiceSender::updateNetwork()
{
    assert( _p_udpTransport && "network session is not available!" );

    // check if connection is established
    if ( _p_udpTransport->GetStatus() != RNReplicaNet::Transport::kTransport_EOK )
        return;

    if ( _senderState == Initial )
    {
        _p_voicePaket->_typeId   = NETWORKSOUND_PAKET_TYPE_CON_REQ;
        _p_voicePaket->_length   = 0;
        _p_udpTransport->SendReliable( reinterpret_cast< char* >( _p_voicePaket ), 4  + _p_voicePaket->_length );
        _senderState = RequestConnection;

        log_debug << "  -> requesting for connection to sound receiver ... " << std::endl;
    }

    static char s_buffer[ 512 ];
    if ( !_p_udpTransport->Recv( s_buffer, 512 ) )
        return;

    // update protocol state
    switch ( _senderState )
    {
        case RequestConnection:
        {
            VoicePaket* p_data = reinterpret_cast< VoicePaket* >( s_buffer );
            if ( p_data->_typeId == NETWORKSOUND_PAKET_TYPE_CON_GRANT )
            {
                _senderState = ConnectionReady;
                log_debug << "  receiver granted connection " << std::endl;
            }
            else if ( p_data->_typeId == NETWORKSOUND_PAKET_TYPE_CON_DENY )
            {
                _senderState = ConnectionDenied;
                log_debug << "  receiver denied connection " << std::endl;
            }
        }
        break;

        default:
            ;
    }
}

} // namespace vrc
