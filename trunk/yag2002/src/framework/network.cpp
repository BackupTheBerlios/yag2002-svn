/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2007, A. Botorabi
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
 # networking core basing on ReplicaNet
 #
 #   date of creation:  04/08/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>
#include "network.h"
#include "log.h"
#include "utils.h"
#include "configuration.h"
#include <RNPlatform/Inc/FreewareCode.h>
#include <RNXPSockets/Inc/XPSocket.h>


namespace yaf3d
{

// Implement the singleton NetworkDevice
YAF3D_SINGLETON_IMPL( NetworkDevice );


// helper function for converting the protocol version to a string
std::string getProtocolVersionAsString( unsigned int version )
{
    std::stringstream str;
    str << ( ( version & 0x00FF0000 ) >> 16 );
    str << ".";
    str << ( ( version & 0x0000FF00 ) >> 8 );
    str << ".";
    str << ( version & 0x000000FF );

    return str.str();
}

Networking::Networking() :
_numSessions( 0 )
{
}

Networking::~Networking()
{
}

void Networking::registerSessionNotify( SessionNotifyCallback* p_cb )
{
    std::vector< SessionNotifyCallback* >::iterator p_beg = _sessionCallbacks.begin(), p_end = _sessionCallbacks.end();
    for ( ; p_beg != p_end; ++p_beg )
        if ( *p_beg == p_cb )
            break;

    assert( ( p_beg == p_end ) && "session callback already exists!" );

    _sessionCallbacks.push_back( p_cb );
}

void Networking::deregisterSessionNotify( SessionNotifyCallback* p_cb )
{
    std::vector< SessionNotifyCallback* >::iterator p_beg = _sessionCallbacks.begin(), p_end = _sessionCallbacks.end();
    for ( ; p_beg != p_end; ++p_beg )
        if ( *p_beg == p_cb )
            break;

    assert( ( p_beg != p_end ) && "session callback does not exist!" );

    _sessionCallbacks.erase( p_beg );
}

void Networking::getSessionIDs( std::vector< int >& ids )
{
    ids = _sessionIDs;
}

void Networking::JoinerSessionIDPost( const int sessionID )
{
    THREADSAFELOCKCLASS( _mutex );
    _sessionIDs.push_back( sessionID );
    ++_numSessions;

    log_debug << "NetworkDevice: client with session ID " << sessionID << " joined" << std::endl;

    // notify registered callback objects
    std::vector< SessionNotifyCallback* >::iterator p_beg = _sessionCallbacks.begin(), p_end = _sessionCallbacks.end();
    for ( ; p_beg != p_end; ++p_beg )
        ( *p_beg )->onSessionJoined( sessionID );
}

void Networking::LeaverSessionIDPost( const int sessionID )
{
    THREADSAFELOCKCLASS( _mutex );

    // check if we have been disconnected from server ( because of server shutdown or network disturbance )
    if ( sessionID == GetMasterSessionID() )
    {
        _numSessions = 0;
        std::vector< SessionNotifyCallback* >::iterator p_cbbeg = _sessionCallbacks.begin(), p_cbend = _sessionCallbacks.end();
        for ( ; p_cbbeg != p_cbend; ++p_cbbeg )
            ( *p_cbbeg )->onServerDisconnect( sessionID );

        _sessionIDs.clear();
        return;
    }

    std::vector< int >::iterator p_beg = _sessionIDs.begin(), p_end = _sessionIDs.end();
    for ( ; p_beg != p_end; ++p_beg )
        if ( *p_beg == sessionID )
            break;

    if ( p_beg == p_end )
    {
        log_warning << "NetworkDevice: a session is leaving which has an unregistered ID: " << sessionID << std::endl;
        return;
    }
    else
    {
        log_debug << "NetworkDevice: leaving network session (" << sessionID << ")" << std::endl;
    }

    _sessionIDs.erase( p_beg );
    _numSessions--;

    // notify registered callback objects
    std::vector< SessionNotifyCallback* >::iterator p_cbbeg = _sessionCallbacks.begin(), p_cbend = _sessionCallbacks.end();
    for ( ; p_cbbeg != p_cbend; ++p_cbbeg )
        ( *p_cbbeg )->onSessionLeft( sessionID );

}

NetworkDevice::NetworkDevice() :
_mode( NONE ),
_p_session( NULL ),
_clientSessionStable( false ),
_serverSessionStable( false )
{
    FreewareSetRegistrationCode( "64B8EBDH-HJK8L6EA-8EBAC" );
}

NetworkDevice::~NetworkDevice()
{
}

void NetworkDevice::disconnect()
{
    if ( _p_session ) 
    {
        _p_session->Disconnect();
        delete _p_session;
        _p_session = NULL;
    }
    _clientSessionStable = false;
    _serverSessionStable = false;
    _mode = NetworkDevice::NONE;
    _nodeInfo._levelName = "";
    _nodeInfo._nodeName  = "";

    log_debug << "NetworkDevice: successfully disconnected from network session" << std::endl;
}

void NetworkDevice::shutdown()
{
    if ( _p_session )
    {
        _p_session->Disconnect();
        delete _p_session;
        _p_session = NULL;
    }
    _clientSessionStable = false;
    _serverSessionStable = false;

    RNReplicaNet::XPURL::ShutdownNetwork();
    RNReplicaNet::PlatformHeap::ForceFree();

    _mode = NetworkDevice::NONE;

    // destroy singleton
    destroy();
}

void NetworkDevice::setupServer( int channel, const NodeInfo& nodeInfo  ) throw ( NetworkExpection )
{
    // do we already have a session created?
    assert( _p_session == NULL && "there is already a running session!" );

    log_info << "NetworkDevice: starting server, time: " << yaf3d::getTimeStamp() << std::endl;
    log_info << "NetworkDevice: networking protocol version: " << getProtocolVersionAsString( YAF3D_NETWORK_PROT_VERSION ) << std::endl;

    _nodeInfo  = nodeInfo;
    _p_session = new Networking;

    log_info << "NetworkDevice: server is starting network session: " << nodeInfo._nodeName << std::endl;

    _p_session->SetManualPoll();
    _p_session->SetLoadBalancing( true );
    _p_session->SetCanAcceptObjects( true );
    _p_session->SetCanBecomeMaster( true );
    _p_session->SetCanSpider( true );
    _p_session->SetDataRetention( true );
    _p_session->SetGameChannel( channel );
    _p_session->SessionCreate( _nodeInfo._nodeName );

    unsigned int tryCounter = 0;
    while ( !_p_session->IsStable() )
    {
        RNReplicaNet::CurrentThread::Sleep( 500 );
        ++tryCounter;

        // try up to 10 seconds
        if ( tryCounter > 20 )
        {
            _p_session->Disconnect();
            delete _p_session;
            _p_session = NULL;

            throw NetworkExpection( "Problems starting server" );
        }
    }
    _serverSessionStable  = true;
    _mode = NetworkDevice::SERVER;

    std::string url = _p_session->SessionExportURL();
    log_info << "NetworkDevice: server session established: " << url << std::endl;
}

void NetworkDevice::setupClient( const std::string& serverIp, int channel, const NodeInfo& nodeInfo ) throw ( NetworkExpection )
{
    log_info << "NetworkDevice: starting client, time: " << yaf3d::getTimeStamp() << std::endl;
    log_info << "NetworkDevice: networking protocol version: " << getProtocolVersionAsString( YAF3D_NETWORK_PROT_VERSION ) << std::endl;

    // do we already have a session created?
    assert( _p_session == NULL && "there is already a running session!" );
    _nodeInfo  = nodeInfo;
    _p_session = new Networking;

    _p_session->SetManualPoll();
    //_p_session->SetGameChannel( channel );
    _p_session->SetLoadBalancing( false );
    _p_session->SetCanAcceptObjects( false );
    _p_session->SetCanBecomeMaster( false);
    _p_session->SetCanSpider( false );
    _p_session->SetDataRetention( true );
    _p_session->SetPreConnect( true );

    //! if the URL is empty or "localhost" then set it to "127.0.0.1"
    std::string Url, ip;
    if ( !serverIp.length() || ( serverIp == "localhost" ) )
        ip = "127.0.0.1";
    else
        ip = serverIp;

    // further analyse the server address for hostname instead of host ip
    {
        std::vector< std::string > fields;
        explode( ip, ".", &fields );
        // check for alphanumerics
        bool alphachar = false;
        for ( std::size_t cnt = 0; cnt < fields.size(); ++cnt )
        {
            for ( std::size_t i = 0; i < fields[ cnt ].size(); ++i )
            {
                if ( !isdigit( fields[ cnt ][ i ] ) )
                {
                    alphachar = true;
                    break;
                }
            }
            if ( alphachar )
                break;
        }
        if ( alphachar )
        {
            // try to resolve the host name
            RNReplicaNet::XPAddress addr;
            log_debug << "NetworkDevice: trying to resolve host name '" << ip << "'" << std::endl;
            if ( XPSock_Resolve( &addr, ip.c_str() ) < 0 )
            {
                delete _p_session;
                _p_session = NULL;
                throw NetworkExpection( "Cannot resolve hostname '" + serverIp + "'.\nTry its IP address." );
            }

            std::stringstream hostip;
            hostip << 
                        static_cast< int >( addr.addr[ 0 ] ) << "." << 
                        static_cast< int >( addr.addr[ 1 ] ) << "." << 
                        static_cast< int >( addr.addr[ 2 ] ) << "." << 
                        static_cast< int >( addr.addr[ 3 ] );
                
            ip = hostip.str();
            log_debug << "NetworkDevice: resolved host name to '" << ip << "'" << std::endl;
        }
    }

    //assemble the url; example url: "SESSION://UDP@127.0.0.1:32001/gameserver"}
    std::string servername;
    Configuration::get()->getSettingValue( YAF3D_GS_SERVER_NAME, servername );
    std::stringstream assembledUrl;
    assembledUrl << "SESSION://UDP@" << ip << ":" << channel << "/" << servername;
    Url = assembledUrl.str();

    log_info << "NetworkDevice: client tries to join to session: " << Url << std::endl;
    _p_session->SessionJoin( Url );

    unsigned int tryCounter = 0;
    // wait for pre-connect state for 10 seconds
    while ( !_p_session->GetPreConnectStatus() && tryCounter < 100 ) 
    {
        _p_session->Poll();
        RNReplicaNet::CurrentThread::Sleep( 100 );
        ++tryCounter;
    }
    if ( tryCounter == 100 )
    {
        log_warning << "NetworkDevice: client cannot connect to server: " << Url << std::endl;

        _p_session->Disconnect();
        delete _p_session;
        _p_session = NULL;

        throw NetworkExpection( "Problems connecting to server." );
    }

    log_info << "NetworkDevice: client is negotiating with server ..." << std::endl;

    // begin to negotiate with server
    //-----------------------------//
    log_info << "NetworkDevice: client is exchanging pre-connect data ..." << std::endl;

    PreconnectDataClient preconnectData;
    preconnectData._typeId = ( unsigned char )YAF3DNW_PRECON_DATA_CLIENT;
    _p_session->DataSend( RNReplicaNet::kReplicaNetUnknownUniqueID, &preconnectData, sizeof( PreconnectDataClient ), RNReplicaNet::ReplicaNet::kPacket_Reliable );

    int          sessionId;
    void*        p_buffer[ 512 ];
    int          bufferLength;
    bool         gotServerInfo = false;
    tryCounter = 0;
    while ( !gotServerInfo )
    {
        _p_session->Poll();
        RNReplicaNet::CurrentThread::Sleep( 100 );
        while ( _p_session->DataReceive( &sessionId, p_buffer, &bufferLength ) ) 
        {
            PreconnectDataServer* p_data = ( PreconnectDataServer* )p_buffer;
            if ( p_data->_typeId == ( unsigned char )YAF3DNW_PRECON_DATA_SERVER ) 
            {
                // check network protocol versions
                if ( p_data->_protocolVersion != YAF3D_NETWORK_PROT_VERSION )
                {
                    log_warning << "*** NetworkDevice: network protocol mismatch" << std::endl;
                    log << " version server: " << getProtocolVersionAsString( p_data->_protocolVersion ) << std::endl;
                    log << " version client: " << getProtocolVersionAsString( YAF3D_NETWORK_PROT_VERSION ) << std::endl;

                    _p_session->Disconnect();
                    delete _p_session;
                    _p_session = NULL;

                    std::string msg;
                    msg = "Network protocol version mismatch.\n";
                    msg += "Server's version: " + getProtocolVersionAsString( p_data->_protocolVersion ) + "\n";
                    msg += "Client's version: " + getProtocolVersionAsString( YAF3D_NETWORK_PROT_VERSION );
                    throw NetworkExpection( msg );
                }

                p_data->_p_levelName[ 63 ]  = 0;
                p_data->_p_serverName[ 63 ] = 0;
                _nodeInfo._levelName = p_data->_p_levelName;
                _nodeInfo._nodeName  = p_data->_p_serverName;
                gotServerInfo = true;

                log_debug << "NetworkDevice: client got preconnect data from server" << std::endl;
            }
        }

        // try up to 10 seconds
        if ( tryCounter > 100 )
        {
            log_warning << "*** NetworkDevice: client has problems negotiating with server" << std::endl;

            _p_session->Disconnect();
            delete _p_session;
            _p_session = NULL;

            throw NetworkExpection( "Problems exchanging pre-connect data with server." );
        }

        ++tryCounter;
    }
    _clientSessionStable = true;
    _mode = NetworkDevice::CLIENT;
}

void NetworkDevice::startClient() throw ( NetworkExpection )
{
    if ( !_clientSessionStable )
    {
        log_warning << "NetworkDevice: starting client without a stable session, cannot start client!" << std::endl;

        throw NetworkExpection( "Internal error: starting client without a stable session." );
    }

    log_debug << "NetworkDevice: starting client, time: " << yaf3d::getTimeStamp() << std::endl;

    _p_session->PreConnectHasFinished();

    log_info << "NetworkDevice: client successfully integrated to network" << std::endl;
    std::stringstream msg;
    msg << "nw client: server name: '" <<
        _nodeInfo._nodeName  <<
        "', level name: '"   <<
        _nodeInfo._levelName <<
        "'" << std::endl;
    log_info << msg.str() << std::endl;
    //-----------------------------//

    unsigned int tryCounter = 0;
    while ( !_p_session->IsStable() )
    {
        _p_session->Poll();    // during this time we have to poll the session instance as we disabled the automatic poll!

        RNReplicaNet::CurrentThread::Sleep( 200 );
        ++tryCounter;

        log.enableSeverityLevelPrinting( false );
        log << ".";
        log.enableSeverityLevelPrinting( true );

        // try up to 10 seconds
        if ( tryCounter > 50 ) 
        {
            log_warning << "*** NetworkDevice: client has problems connecting to server" << std::endl;

            _p_session->Disconnect();
            delete _p_session;
            _p_session = NULL;

            throw NetworkExpection( "Problems connecting to server" );
        }
    }
    log_debug << std::endl;

    std::string sessionurl = _p_session->SessionExportURL();
    log_info << "NetworkDevice: client successfully joined to session: " << sessionurl << std::endl;
}

void NetworkDevice::registerSessionNotify( SessionNotifyCallback* p_cb )
{
    assert( _p_session && "no valid session exists!" );
    _p_session->registerSessionNotify( p_cb );
}

void NetworkDevice::deregisterSessionNotify( SessionNotifyCallback* p_cb )
{
    assert( _p_session && "no valid session exists!" );
    _p_session->deregisterSessionNotify( p_cb );
}

void NetworkDevice::getSessionIDs( std::vector< int >& ids )
{
    assert( _p_session && "no valid session exists!" );
    _p_session->getSessionIDs( ids );
}

NodeInfo* NetworkDevice::getNodeInfo()
{
    if ( _clientSessionStable || _serverSessionStable )
        return &_nodeInfo;

    return NULL;
}

int NetworkDevice::getSessionID()
{
    assert( _p_session && "no valid session exists!" );
    return _p_session->GetSessionID();
}

void NetworkDevice::getObjects( std::vector< RNReplicaNet::ReplicaObject* >& objs )
{
    assert( _p_session && "there is no valid session!" );
    _p_session->ObjectListBeginIterate();

    RNReplicaNet::ReplicaObject* p_obj = NULL;
    do 
    {
        p_obj = _p_session->ObjectListIterate();
        if ( !p_obj )
            objs.push_back( p_obj );

    } while( p_obj );

    _p_session->ObjectListFinishIterate();
}

void NetworkDevice::updateServer( float deltaTime )
{
    // poll the netwroking core
    _p_session->Poll();

    // check for pre-connection requests
    int         sessionId;
    void*       p_buffer[ 512 ];
    int         bufferLength;
    while ( _p_session->DataReceive( &sessionId, p_buffer, &bufferLength ) ) 
    {
        PreconnectDataClient* p_data = ( PreconnectDataClient* )p_buffer;
        if ( p_data->_typeId == ( unsigned char )YAF3DNW_PRECON_DATA_CLIENT ) 
        {
            log_info << "NetworkDevice: server is requested for a new client connection ... " << std::endl;
            // send server node
            PreconnectDataServer sendData;
            sendData._typeId = ( unsigned char )YAF3DNW_PRECON_DATA_SERVER;
            strcpy( sendData._p_levelName, _nodeInfo._levelName.c_str() );
            strcpy( sendData._p_serverName, _nodeInfo._nodeName.c_str() );
            sendData._protocolVersion = YAF3D_NETWORK_PROT_VERSION;
            _p_session->DataSend( sessionId, ( void* )&sendData, sizeof( PreconnectDataServer ), RNReplicaNet::ReplicaNet::kPacket_Reliable );
        }
    }
}

void NetworkDevice::updateClient( float fDeltaTime ) 
{ 
    // poll the netwroking core
    if ( _p_session )
        _p_session->Poll();
}
void NetworkDevice::lockObjects()
{
    if ( _p_session )
        _p_session->LockObjects();
}

void NetworkDevice::unlockObjects()
{
    if ( _p_session )
        _p_session->UnLockObjects();
}

} // namespace yaf3d
