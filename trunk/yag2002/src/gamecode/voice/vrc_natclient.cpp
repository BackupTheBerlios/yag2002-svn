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
 # implementation of a NAT client used for getting NAT info from
 #  NAT server
 #
 #   date of creation:  02/20/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_natclient.h"

//! Various timeouts
#define TIMEOUT_REQ_CLIENT_INFO   1.0f
#define TIMEOUT_REGISTER_CLIENT   1.0f

namespace vrc
{

NATClient::NATClient() :
 _p_socket( NULL ),
 _state( eUnknown ),
 _timer( 0.0f )
{
    assert( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Client && "this object must be created only on a client!" );
}

NATClient::~NATClient()
{
    if ( _p_socket )
        delete _p_socket;
}

void NATClient::initialize()
{
    // create a socket
    std::string serverip;
    int         voiceChannel;
    serverip = yaf3d::NetworkDevice::get()->getServerIP();
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_VOICECHAT_CHANNEL, voiceChannel );
    std::stringstream assembledUrl;
    assembledUrl << serverip << ":" << voiceChannel;
    RNReplicaNet::XPAddress addr( assembledUrl.str() );

    _serverAddress = addr;

    _p_socket = new RNReplicaNet::XPSocketUrgent;
    int result = _p_socket->Create( 0 );
    if ( ( result != XPSOCK_EOK ) && ( result != XPSOCK_EWOULDBLOCK ) )
    {
        log_error << "NATClient: could not create socket, error " <<  RNReplicaNet::XPSocket::GetLastError() << std::endl;
        delete _p_socket;
        _p_socket = NULL;
        return;
    }

    // setup the state and initial timeout
    _state = eInitialized;
    _timer = TIMEOUT_REGISTER_CLIENT;
}

void NATClient::update( float deltaTime )
{
    if ( !_p_socket )
        return;

    static tNATData s_data;

    _timer += deltaTime;

    memset( &s_data, 0, sizeof( s_data ) );
    switch( _state )
    {
        // send registration packet until we are registered
        case eInitialized:
        {
            if ( _timer > TIMEOUT_REGISTER_CLIENT )
            {
                // send out a packet to the NAT server
                s_data._cmd    = NAT_REGISTER_CLIENT;
                s_data._ownSID = yaf3d::NetworkDevice::get()->getSessionID();
                _p_socket->Send( reinterpret_cast< char* >( &s_data ), sizeof( s_data ), _serverAddress );
                _timer = 0.0f;
            }
        }
        break;

        case eReqClientInfo:
        {
            if ( _timer > TIMEOUT_REQ_CLIENT_INFO )
            {
                std::map< int, NATInfoCallback* >::iterator p_beg = _requestMap.begin(), p_end = _requestMap.end();
                if ( p_beg == p_end )
                {
                    _state = eIdle;
                    break;
                }

                for ( ; p_beg != p_end; ++p_beg )
                {
                    s_data._cmd     = NAT_GET_CLIENT_INFO;
                    s_data._ownSID  = yaf3d::NetworkDevice::get()->getSessionID();
                    s_data._peerSID = p_beg->first;
                    _p_socket->Send( reinterpret_cast< char* >( &s_data ), sizeof( s_data ), _serverAddress );
                    _timer = 0.0f;

                    //! TODO: implement a mechanism for a maximal try count, otherwise the requests runs for ever!
                }
            }
        }
        break;

        case eIdle:
        {
            if ( _timer > NAT_CLIENT_LIFESIGN_PERIOD )
            {
                // send out lifesign
                s_data._cmd    = NAT_CLIENT_LIFESIGN;
                s_data._ownSID = yaf3d::NetworkDevice::get()->getSessionID();
                _p_socket->Send( reinterpret_cast< char* >( &s_data ), sizeof( s_data ), _serverAddress );

                _timer = 0.0f;
            }
        }
        break;

        default:
            break;
    }
}

bool NATClient::receivePacket( char* p_buffer, int len, const RNReplicaNet::XPAddress& senderaddr )
{
    tNATData* p_data = reinterpret_cast< tNATData* >( p_buffer );
    if ( ( p_data->_cmd & NAT_CMD_MASK ) != NAT_CMD_MASK )
        return false;

    // this packet comes from other clients, not from NAT server
    if ( p_data->_cmd == NAT_PUNCH_HOLE )
    {
        log_verbose << "NATClient: hole punching packet received from: " << senderaddr.Export() << std::endl;
        return true;
    }

    if ( _serverAddress != senderaddr )
        return false;

    switch ( p_data->_cmd )
    {
        case NAT_ACK_REGISTER_CLIENT:
        {
            _state = eIdle;
            log_verbose << "NATClient: registered on server" << std::endl;
        }
        break;

        case NAT_RCV_CLIENT_INFO:
        {
            // just to be sure
            if ( _requestMap.find( p_data->_peerSID ) == _requestMap.end() )
            {
                log_error << "NATClient: received unrequested client info, peer sid " << p_data->_peerSID << std::endl;
                break;
            }

            RNReplicaNet::XPAddress mappedaddress( p_data->_peerPort, p_data->_peerIP[ 0 ], p_data->_peerIP[ 1 ], p_data->_peerIP[ 2 ], p_data->_peerIP[ 3 ] );

            log_verbose << "NATClient: receiving NAT info for sid " << p_data->_peerSID << std::endl;

            // forward the mapped address to recipient
            _requestMap[ p_data->_peerSID ]->receiveAddressMap( p_data->_peerSID, mappedaddress );

            // remove the callback from internal map
            _requestMap.erase( _requestMap.find( p_data->_peerSID ) );

            // continue idling
            _state = eIdle;
        }
        break;

        case NAT_RCV_CLIENT_QUERY:
        {
            log_verbose << "NATClient: client has been queried by sid " << p_data->_peerSID << std::endl;

            if ( yaf3d::NetworkDevice::get()->getSessionID() != p_data->_ownSID )
            {
                log_warning << "NATClient: wrong query destination " << p_data->_ownSID << std::endl;
            }

            // send a dummy packet to peer client in order to punch a hole in our fw
            tNATData data;
            memset( &data, 0, sizeof( data ) );
            data._cmd = NAT_PUNCH_HOLE;

            RNReplicaNet::XPAddress peeraddr( p_data->_peerPort, p_data->_peerIP[ 0 ], p_data->_peerIP[ 1 ], p_data->_peerIP[ 2 ], p_data->_peerIP[ 3 ] );
            _p_socket->Send( reinterpret_cast< char* >( &data ), sizeof( data ), peeraddr );

            log_verbose << "NATClient: punching a hole for peer: " << peeraddr.Export() << std::endl;
        }
        break;

        default:
        {
            log_warning << "NATClient: received unknown command packet from NAT server" << std::endl;
            return false;
        }
    }

    return true;
}

void NATClient::requestClientInfo( int sid, NATInfoCallback* p_cb )
{
    // store the request in lookup table for later handling of the response
    _requestMap[ sid ] = p_cb;
    // set proper state
    _state = eReqClientInfo;
    // set the timer so the packet is sent immediately on next update
    _timer = TIMEOUT_REQ_CLIENT_INFO;
}

} // namespace vrc
