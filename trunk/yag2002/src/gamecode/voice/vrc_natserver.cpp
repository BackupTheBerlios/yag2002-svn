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
 # entity NAT server used for establishing peer2peer connections
 #  between voice clients
 #
 #   date of creation:  02/14/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_natserver.h"
#include <RNReplicaNet/Inc/DataBlock_Function.h>
#include <RNXPSockets/Inc/XPSockets.h>


namespace vrc
{

//! Implement and register the network voice entity factory
YAF3D_IMPL_ENTITYFACTORY( NATServerEntityFactory )

EnNATServer::EnNATServer() :
 _p_socket( NULL )
{
    if ( yaf3d::GameState::get()->getMode() != yaf3d::GameState::Server )
        log_warning << "NATServer: this entity can be created only on server!" << std::endl;
}

EnNATServer::~EnNATServer()
{
    if ( _p_socket )
    {
        _p_socket->Close();
        delete _p_socket;

        // deregister for getting network session notifications ( client left notification )
        yaf3d::NetworkDevice::get()->deregisterSessionNotify( this );
    }

    // clean up the sid / transport map
    std::map< int, NATInfo* >::iterator p_natclient = _sidNATInfoMap.begin(), p_end = _sidNATInfoMap.end();
    for ( ; p_natclient != p_end; ++p_natclient )
        delete p_natclient->second;
}

void EnNATServer::initialize()
{
    // assemble the NAT udp listener address ( NAT server )
    int voicechannel = 0;
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_VOICE_PORT, voicechannel );

    _p_socket = new RNReplicaNet::XPSocketUrgent;
    int result = _p_socket->Create( voicechannel );
    if ( ( result != XPSOCK_EOK ) && ( result != XPSOCK_EWOULDBLOCK ) )
    {
        log_error << "NATServer: could not create socket, error " <<  RNReplicaNet::XPSocket::GetLastError() << std::endl;
        delete _p_socket;
        _p_socket = NULL;
        return;
    }

    // register for getting network session notifications ( client left notification )
    yaf3d::NetworkDevice::get()->registerSessionNotify( this );

    // we need updating
    yaf3d::EntityManager::get()->registerUpdate( this, true );
}

void EnNATServer::updateEntity( float /*deltaTime*/ )
{
    static tNATData s_data;

    RNReplicaNet::XPAddress senderaddr;
    while ( _p_socket->Recv( reinterpret_cast< char* >( &s_data ), sizeof( s_data ), &senderaddr ) )
    {
        switch ( s_data._cmd )
        {
            case NAT_REGISTER_CLIENT:
            {
                RNReplicaNet::XPAddress localaddr( s_data._localPort, s_data._localIP[ 0 ], s_data._localIP[ 1 ], s_data._localIP[ 2 ], s_data._localIP[ 3 ] );

                NATInfo* p_natc = new NATInfo( senderaddr, localaddr, s_data._fwdPort );
                _sidNATInfoMap[ s_data._ownSID ] = p_natc;

                log_verbose << "NATServer: client registered, sid " << s_data._ownSID << ", port " << senderaddr.port << std::endl;

                // send acknowledgement
                s_data._cmd = NAT_ACK_REGISTER_CLIENT;
                _p_socket->Send( reinterpret_cast< char* >( &s_data ), sizeof( s_data ), senderaddr );
            }
            break;

            case NAT_CLIENT_LIFESIGN:
            {
                //log_verbose << "NATServer: got lifesign, sid " << s_data._ownSID << ", " << senderaddr.Export() << std::endl;
            }
            break;

            case NAT_GET_CLIENT_INFO:
            {
                std::map< int, NATInfo* >::iterator p_queried = _sidNATInfoMap.find( s_data._peerSID );
                if ( p_queried == _sidNATInfoMap.end() )
                {
                    log_verbose << "NATServer: have no info on client with sid " << s_data._peerSID << ", requester sid: " << s_data._ownSID << std::endl;
                    return;
                }

                // check if both clients are behind the same nat
                bool localclients = false;

                if ( ( p_queried->second->_mappedAddress.addr[ 0 ] == senderaddr.addr[ 0 ] ) &&
                     ( p_queried->second->_mappedAddress.addr[ 1 ] == senderaddr.addr[ 1 ] ) &&
                     ( p_queried->second->_mappedAddress.addr[ 2 ] == senderaddr.addr[ 2 ] ) &&
                     ( p_queried->second->_mappedAddress.addr[ 3 ] == senderaddr.addr[ 3 ] )
                   )
                {
                    localclients = true;
                }

                if ( localclients )
                {
                    // send back the local address info to requester
                    s_data._peerPort = p_queried->second->_localAddress.port;
                    memcpy( s_data._peerIP, p_queried->second->_localAddress.addr, sizeof( s_data._peerIP ) );
                }
                else
                {
                    // send back the mapped address info to requester
                    s_data._fwdPort  = p_queried->second->_forwardedPort;
                    s_data._peerPort = p_queried->second->_mappedAddress.port;
                    memcpy( s_data._peerIP, p_queried->second->_mappedAddress.addr, sizeof( s_data._peerIP ) );
                }

                s_data._cmd = NAT_RCV_CLIENT_INFO;
                _p_socket->Send( reinterpret_cast< char* >( &s_data ), sizeof( s_data ), senderaddr );

                // we have also to notify the queried client about beeing queried, thus the queried client can punch a hole for the requester
                if ( localclients )
                {
                    std::map< int, NATInfo* >::iterator p_requester = _sidNATInfoMap.find( s_data._ownSID );
                    if ( p_requester == _sidNATInfoMap.end() )
                    {
                        log_verbose << "NATServer: have no info on requesting client with sid " << s_data._ownSID << std::endl;
                    }
                    else
                    {
                        s_data._peerPort = p_requester->second->_localAddress.port;
                        memcpy( s_data._peerIP, p_requester->second->_localAddress.addr, sizeof( s_data._peerIP ) );
                    }
                }
                else
                {
                    s_data._peerPort = senderaddr.port;
                    memcpy( s_data._peerIP, senderaddr.addr, sizeof( s_data._peerIP ) );
                }

                s_data._peerSID = s_data._ownSID;
                s_data._ownSID  = p_queried->first;
                s_data._cmd     = NAT_RCV_CLIENT_QUERY;

                //! TODO: check if we have to guarantee that this packet arrives!
                _p_socket->Send( reinterpret_cast< char* >( &s_data ), sizeof( s_data ), p_queried->second->_mappedAddress );

                log_verbose << "NATServer: sending requested NAT info: " << senderaddr.Export() << " to sid: " << s_data._ownSID << std::endl;
            }
            break;

            default:
            {
                log_warning << "NATServer: received unknown command: " << s_data._cmd << std::endl;
            }
        }
    }
}

void EnNATServer::onSessionLeft( int sessionID )
{
    // on server remove the sid from sid/port map
    if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server )
    {
        std::map< int, NATInfo* >::iterator p_natclient = _sidNATInfoMap.find( sessionID );

        if ( p_natclient != _sidNATInfoMap.end() )
        {
            log_verbose << "NATServer: remove NAT entry of sid " << sessionID << ", total entries: " << _sidNATInfoMap.size() - 1 << std::endl;
            delete p_natclient->second;
            _sidNATInfoMap.erase( p_natclient );
        }
        else
        {
            log_warning << "NATServer: leaving client could not be found in sid/port map: sid " << sessionID << std::endl;
        }
    }
}

} // namespace vrc
