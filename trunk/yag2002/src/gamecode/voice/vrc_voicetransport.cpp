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
 # network layer for transporting voice packets
 #
 #   date of creation:  02/24/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_voicetransport.h"

namespace vrc
{

VoiceTransport::VoiceTransport() :
 _p_NATClient( NULL ),
 _p_socket( NULL )
{
}

VoiceTransport::~VoiceTransport()
{
    if ( _p_NATClient )
        delete _p_NATClient;
}

void VoiceTransport::initialize()
{
    _p_NATClient = new NATClient;
    _p_NATClient->initialize();
    _p_socket = _p_NATClient->getSocket();
}

void VoiceTransport::update( float deltaTime )
{
    // check if the transport layer is initialized
    if ( !_p_socket )
        return;

    // pick up incomming network packets and distribute them, these can be nat packets or voice receiver packets.
    RNReplicaNet::XPAddress senderaddr;
    static char             s_buffer[ 512 ];
    int                     len = 0;
    std::vector< CallbackVoiceData* >::iterator p_beg, p_end;
    while ( len = _p_socket->Recv( s_buffer, sizeof( s_buffer ), &senderaddr ) )
    {
        // handle NAT related packets
        bool natpacket = _p_NATClient->receivePacket( s_buffer, len, senderaddr );

        // if the packet was not a NAT packet then pass it to voice packet receivers
        if ( !natpacket )
        {
            VoicePaket* p_voicepacket = reinterpret_cast< VoicePaket* >( s_buffer );
            if ( len > int( sizeof( VoicePaket ) ) )
            {
                log_error << "VoiceTransport: receiver packet; unexpected packet length " << len << std::endl;
                continue;
            }

            p_beg = _recvCallbacks.begin(), p_end = _recvCallbacks.end();
            for ( ; p_beg != p_end; ++p_beg )
            {
                // check if the packet is handled by the receiver
                if ( ( *p_beg )->recvPacket( p_voicepacket, senderaddr ) )
                    break;
            }
        }
    }

    // now update the nat client
    _p_NATClient->update( deltaTime );
}

RNReplicaNet::XPSocketUrgent* VoiceTransport::getSocket()
{
    return _p_socket;
}

void VoiceTransport::registerReceiver( CallbackVoiceData* p_cb, bool reg )
{
    std::vector< CallbackVoiceData* >::iterator p_beg = _recvCallbacks.begin(), p_end = _recvCallbacks.end();
    if ( reg )
    {
        for ( ; p_beg != p_end; ++p_beg )
        {
            if ( *p_beg == p_cb )
            {
                log_warning << "VoiceTransport: receiver is already registered on socket!" << std::endl;
                return;
            }
        }

        if ( p_beg == p_end )
            _recvCallbacks.push_back( p_cb );
    }
    else
    {
        for ( ; p_beg != p_end; ++p_beg )
        {
            if ( *p_beg == p_cb )
                break;
        }

        if ( p_beg != p_end )
        {
            _recvCallbacks.erase( p_beg );
        }
        else
        {
            log_warning << "VoiceTransport: receiver was not previously registered on socket!" << std::endl;
        }
    }
}

void VoiceTransport::getClientInfo( int sid, CallbackVoiceData* p_cb )
{
    log_verbose << "VoiceTransport: requesting client info for sid " << sid << std::endl;

    _p_NATClient->requestClientInfo( sid, this );
    _sidRecvMap[ sid ] = p_cb;
}

void VoiceTransport::receiveAddressMap( int sid, RNReplicaNet::XPAddress& address )
{
    if ( _sidRecvMap.find( sid ) == _sidRecvMap.end() )
    {
        log_error << "VoiceTransport: cannot find receiver callback!" << std::endl;
        return;
    }

    // use the callback
    _sidRecvMap[ sid ]->recvClientAddress( sid, address );
}

} // namespace vrc
