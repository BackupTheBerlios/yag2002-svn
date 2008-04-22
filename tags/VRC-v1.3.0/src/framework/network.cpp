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
 # networking core basing on ReplicaNet
 #
 #   date of creation:  04/08/2004
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <base.h>
#include "network.h"
#include "log.h"
#include "utils.h"
#include "configuration.h"
#include <RNPlatform/Inc/FreewareCode.h>
#include <RNXPSockets/Inc/XPSocket.h>
#include <RNXPSession/Inc/XPSession.h>
#include <RNXPURL/Inc/XPURL.h>
#include <RNXPURL/Inc/DebugHandler.h>
#include <RNXPURL/Inc/TransportConfig.h>

// Visual debugger interface
class ApplicationDebugHandler : public RNReplicaNet::DebugHandler
{
    public:
                            ApplicationDebugHandler() {}

        virtual             ~ApplicationDebugHandler() {}

        void                DebugPrint( const char* p_text )
                            {
                                RNReplicaNet::DebugHandler::DebugPrint( p_text );
                            }
};

namespace yaf3d
{

// Implement the singleton NetworkDevice
YAF3D_SINGLETON_IMPL( NetworkDevice )


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

// resolve host name or validate ip address
std::string resolveHostName( const std::string& host )
{
    std::vector< std::string > fields;
    explode( host, ".", &fields );
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
        if ( XPSock_Resolve( &addr, host.c_str() ) < 0 )
            return "";

        std::stringstream hostip;
        hostip <<
                    static_cast< int >( addr.addr[ 0 ] ) << "." <<
                    static_cast< int >( addr.addr[ 1 ] ) << "." <<
                    static_cast< int >( addr.addr[ 2 ] ) << "." <<
                    static_cast< int >( addr.addr[ 3 ] );

        return hostip.str();
    }

    return host;
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
_serverSessionStable( false ),
_p_cbAuthentification( NULL )
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

    _serverIP            = "";

    log_debug << "NetworkDevice: successfully disconnected from network session" << std::endl;
}

void NetworkDevice::shutdown()
{
    log_info << "NetworkDevice: shutting down" << std::endl;

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

    _serverIP = "";

    // destroy singleton
    destroy();
}

void NetworkDevice::setupServer( int channel, const NodeInfo& nodeInfo  ) throw ( NetworkException )
{
    // do we already have a session created?
    assert( _p_session == NULL && "there is already a running session!" );

    log_info << "NetworkDevice: starting server, time: " << yaf3d::getTimeStamp() << std::endl;
    log_info << "NetworkDevice: networking protocol version: " << getProtocolVersionAsString( YAF3D_NETWORK_PROT_VERSION ) << std::endl;

    _p_session = new Networking;

    log_info << "NetworkDevice: server is starting network session: " << nodeInfo._nodeName << std::endl;

    RNReplicaNet::XPURL::RegisterDefaultTransports();

    _p_session->SetManualPoll();
    _p_session->SetPollLayerBelow( true );
    _p_session->SetLoadBalancing( true );
    _p_session->SetCanAcceptObjects( true );
    _p_session->SetCanBecomeMaster( true );
    _p_session->SetCanSpider( true );
    _p_session->SetDataRetention( true );
    _p_session->SetGameChannel( channel );
    _p_session->SetAllowConnections( true );
    _p_session->SetEncryption( true );

    // create the session
    _p_session->SessionCreate( nodeInfo._nodeName );

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

            throw NetworkException( "Problems starting server" );
        }
    }
    _serverSessionStable  = true;
    _mode = NetworkDevice::SERVER;

    std::string url = _p_session->SessionExportURL();
    log_info << "NetworkDevice: server session established: " << url << std::endl;

    // extract the ip from url and store it
    std::string ip( url.substr( url.find( "@" ) + 1 ) );
    ip.erase( ip.find( ":" ) );
    _serverIP = ip;

    // store the server node info
    _serverNodeInfo = nodeInfo;
}

void NetworkDevice::setAuthCallback( CallbackAuthentification* p_cb )
{
    _p_cbAuthentification = p_cb;
}

void NetworkDevice::setupClient( const std::string& serverIp, int channel, NodeInfo& nodeInfo, const std::string& login, 
                                 const std::string& passwd, bool reguser, const std::string& name, const std::string& email ) throw ( NetworkException )
{
    log_info << "NetworkDevice: starting client, time: " << yaf3d::getTimeStamp() << std::endl;
    log_info << "NetworkDevice: networking protocol version: " << getProtocolVersionAsString( YAF3D_NETWORK_PROT_VERSION ) << std::endl;

#ifdef REPLICANET_VISUALDEBUGGER
    RNReplicaNet::XPURL::RegisterDebugHandler( new ApplicationDebugHandler() );
    RNReplicaNet::XPURL::GetDebugHandler()->EnableVisualDebuggerSocket( channel );
#endif

    // do we already have a session created?
    if( _p_session )
        disconnect();

    _p_session = new Networking;

    RNReplicaNet::XPURL::RegisterDefaultTransports();

    _p_session->SetManualPoll();
    //_p_session->SetGameChannel( channel );
    _p_session->SetLoadBalancing( false );
    _p_session->SetCanAcceptObjects( false );
    _p_session->SetCanBecomeMaster( false);
    _p_session->SetCanSpider( false );
    _p_session->SetDataRetention( true );
    _p_session->SetPreConnect( true );
    _p_session->SetEncryption( true );

    // some network emulation settings
    //bool enableNetworkEmulation = true;
    //if ( enableNetworkEmulation )
    //{
    //    RNReplicaNet::NetworkEmulation::SetEnabled( true );
    //    RNReplicaNet::NetworkEmulation::SetConnection( RNReplicaNet::NetworkEmulation::kModem56k, true );
    //    RNReplicaNet::NetworkEmulation::SetPacketLoss( 5 );
    //    RNReplicaNet::NetworkEmulation::SetAverageLatency( 0.1f );
    //    RNReplicaNet::NetworkEmulation::SetJitter( 0.02f );
    //}

    //! if the URL is empty or "localhost" then set it to "127.0.0.1"
    std::string Url, ip;
    if ( !serverIp.length() || ( serverIp == "localhost" ) )
        ip = "127.0.0.1";
    else
        ip = serverIp;

    log_debug << "NetworkDevice: trying to resolve host name '" << ip << "'" << std::endl;
    ip = resolveHostName( ip );

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
        std::string errtxt;
        unsigned int errid = _p_session->GetSessionErrorReason();
        switch( errid )
        {
            case RNReplicaNet::XPSession::kXPSession_EOK:
                errtxt = "Cannot find the server.";
               break;

            case RNReplicaNet::XPSession::kXPSession_EERROR:
                errtxt = "Undefined networking error.";
               break;

            case RNReplicaNet::XPSession::kXPSession_ETRANSPORT_CLOSED:
                errtxt = "Networking session disconnected or closed.";
               break;

            case RNReplicaNet::XPSession::kXPSession_ETRANSPORT_ERROR:
                errtxt = "Networking session has been abnormally terminated due to a transport timeout.";
               break;

            default:
                ;
        }
        _p_session->Disconnect();
        delete _p_session;
        _p_session = NULL;

        log_warning << "NetworkDevice: client cannot connect to server: " << Url << std::endl;
        log_warning << "               Reason: " << errtxt << std::endl;
        errtxt = "Problems connecting to server.\nReason: " + errtxt;
        throw NetworkException( errtxt );
    }

    log_info << "NetworkDevice: client is negotiating with server ..." << std::endl;

    // begin to negotiate with server
    //-----------------------------//
    log_info << "NetworkDevice: client is exchanging pre-connect data ..." << std::endl;

    PreconnectDataClient preconnectData;
    memset( &preconnectData, 0, sizeof( PreconnectDataClient ) );
    preconnectData._typeId = ( unsigned char )YAF3DNW_PRECON_DATA_CLIENT;
    preconnectData._state  = eConnecting;
 
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
            PreconnectDataServer* p_data = reinterpret_cast< PreconnectDataServer* >( p_buffer );
            if ( p_data->_typeId == static_cast< unsigned char >( YAF3DNW_PRECON_DATA_SERVER ) )
            {
                // check network protocol versions
                if ( p_data->_protocolVersion != YAF3D_NETWORK_PROT_VERSION )
                {
                    log_warning << "*** NetworkDevice: network protocol mismatch" << std::endl;
                    log_out << " version server: " << getProtocolVersionAsString( p_data->_protocolVersion ) << std::endl;
                    log_out << " version client: " << getProtocolVersionAsString( YAF3D_NETWORK_PROT_VERSION ) << std::endl;

                    _p_session->Disconnect();
                    delete _p_session;
                    _p_session = NULL;

                    std::string msg;
                    msg = "Network protocol version mismatch.\n";
                    msg += "Server's version: " + getProtocolVersionAsString( p_data->_protocolVersion ) + "\n";
                    msg += "Client's version: " + getProtocolVersionAsString( YAF3D_NETWORK_PROT_VERSION );
                    throw NetworkException( msg );
                }

                switch( p_data->_state )
                {
                    case eLogin:
                    {
                        PreconnectDataClient preconnectData;
                        memset( &preconnectData, 0, sizeof( PreconnectDataClient ) );
                        preconnectData._typeId = ( unsigned char )YAF3DNW_PRECON_DATA_CLIENT;

                        if ( !reguser )
                        {
                            preconnectData._state  = eLogin;

                            // check if the server needs authentification
                            if ( p_data->_needAuthentification )
                            {
                                if ( login.length() )
                                    strcpy_s( preconnectData._p_login, sizeof( preconnectData._p_login ) - 1, login.c_str() );

                                if ( passwd.length() )
                                    strcpy_s( preconnectData._p_passwd, sizeof( preconnectData._p_passwd ) - 1, passwd.c_str() );
                            }

                            _p_session->DataSend( RNReplicaNet::kReplicaNetUnknownUniqueID, &preconnectData, sizeof( PreconnectDataClient ), RNReplicaNet::ReplicaNet::kPacket_Reliable );
                            
                            // clear the preconnect struct for security reasons
                            memset( &preconnectData, 0, sizeof( PreconnectDataClient ) );
                        }
                        else
                        {
                            preconnectData._state = eRegister;
                            strcpy_s( preconnectData._p_name, sizeof( preconnectData._p_name ) - 1, name.c_str() );
                            strcpy_s( preconnectData._p_email, sizeof( preconnectData._p_email ) - 1, email.c_str() );
                            strcpy_s( preconnectData._p_login, sizeof( preconnectData._p_login ) - 1, login.c_str() );
                            strcpy_s( preconnectData._p_passwd, sizeof( preconnectData._p_passwd ) - 1, passwd.c_str() );
                            // send the registration packet
                            _p_session->DataSend( RNReplicaNet::kReplicaNetUnknownUniqueID, &preconnectData, sizeof( PreconnectDataClient ), RNReplicaNet::ReplicaNet::kPacket_Reliable );
                            // clear the preconnect struct for security reasons
                            memset( &preconnectData, 0, sizeof( PreconnectDataClient ) );
                        }
                    }
                    break;

                    case eLoginResult:
                    {
                        // check if server granted access
                        if ( p_data->_accessGranted )
                        {
                            p_data->_p_levelName[ sizeof( p_data->_p_levelName ) - 1 ]  = 0;
                            p_data->_p_serverName[ sizeof( p_data->_p_serverName ) - 1 ] = 0;
                            nodeInfo._accessGranted = true;

                            if ( p_data->_p_levelName[ 0 ] )
                                nodeInfo._levelName = p_data->_p_levelName;
                            
                            if ( p_data->_p_serverName[ 0 ] )
                                nodeInfo._nodeName  = p_data->_p_serverName;

                            // use the callback if any is set
                            if ( _p_cbAuthentification )
                                _p_cbAuthentification->authentificationResult( p_data->_userID );
                        }
                        else
                        {
                            nodeInfo._accessGranted = false;
                        }

                        gotServerInfo = true;
                        log_debug << "NetworkDevice: negotiation with server completed" << std::endl;
                    }

                    case eRegistrationResult:
                    {
                        // check if server could process the registration
                        if ( p_data->_accessGranted )
                        {
                            nodeInfo._accessGranted = true;
                            log_debug << "NetworkDevice: new user successfully registered" << std::endl;
                        }
                        else
                        {
                            nodeInfo._accessGranted = false;
                            log_debug << "NetworkDevice: could not register new user" << std::endl;
                        }
                        gotServerInfo = true;
                    }
                    break;

                    default:
                        log_error << "NetworkDevice: invalid connection state received from server!" << std::endl;
                }
            }
        }

        // try up to 10 seconds
        if ( tryCounter > 100 )
        {
            log_warning << "*** NetworkDevice: client has problems negotiating with server" << std::endl;

            _p_session->Disconnect();
            delete _p_session;
            _p_session = NULL;

            throw NetworkException( "Problems exchanging pre-connect data with server." );
        }

        ++tryCounter;
    }

    if ( nodeInfo._accessGranted )
    {
        _clientSessionStable = true;
        _mode = NetworkDevice::CLIENT;
        _serverIP = ip;

        std::stringstream msg;
        msg << "NetworkDevice: server name: '" <<
            nodeInfo._nodeName  <<
            "', level name: '"   <<
            nodeInfo._levelName <<
            "'" << std::endl;
        log_info << msg.str() << std::endl;
    }
    else
    {
        log_info << "NetworkDevice: server denied access to session, login '" << login << "'" << std::endl;
        _p_session->Disconnect();
    }
}

void NetworkDevice::startClient() throw ( NetworkException )
{
    if ( !_clientSessionStable )
    {
        log_warning << "NetworkDevice: starting client without a stable session, cannot start client!" << std::endl;

        throw NetworkException( "Internal error: starting client without a stable session." );
    }

    log_debug << "NetworkDevice: starting client, time: " << yaf3d::getTimeStamp() << std::endl;

    _p_session->PreConnectHasFinished();

    log_info << "NetworkDevice: client successfully integrated to network" << std::endl;

    // Enable packet buffering and set paket update time for transport layer
    RNReplicaNet::TransportConfig::SetPacketBufferEnable( true );
    RNReplicaNet::TransportConfig::SetPacketBufferTime( 1.0f / 60.0f );

    unsigned int tryCounter = 0;
    while ( !_p_session->IsStable() )
    {
        _p_session->Poll();    // during this time we have to poll the session instance as we disabled the automatic poll!

        RNReplicaNet::CurrentThread::Sleep( 200 );
        ++tryCounter;

        log_out.enableSeverityLevelPrinting( false );
        log_out << ".";
        log_out.enableSeverityLevelPrinting( true );

        // try up to 10 seconds
        if ( tryCounter > 50 )
        {
            log_warning << "*** NetworkDevice: client has problems connecting to server" << std::endl;

            _p_session->Disconnect();
            delete _p_session;
            _p_session = NULL;

            throw NetworkException( "Problems connecting to server" );
        }
    }
    log_debug << std::endl;

    log_info << "NetworkDevice: client successfully joined to session: " << _p_session->SessionExportURL() << std::endl;
}

const std::string& NetworkDevice::getServerIP()
{
    return _serverIP;
}

std::string NetworkDevice::getClientIP( int sessionID )
{
    if ( !_p_session )
        return std::string( "" );

    // extract the ip from url
    std::string url = _p_session->GetURLFromSessionID( sessionID );
    if ( url.empty() )
        return std::string( "" );

    std::string ip( url.substr( url.find( "@" ) + 1 ) );
    ip.erase( ip.find( ":" ) );

    return ip;
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

int NetworkDevice::getSessionID()
{
    assert( _p_session && "no valid session exists!" );
    return _p_session->GetSessionID();
}

void NetworkDevice::getObjects( std::vector< RNReplicaNet::ReplicaObject* >& objs )
{
    assert( _p_session && "there is no valid session!" );

    lockObjects();
    _p_session->ObjectListBeginIterate();

    RNReplicaNet::ReplicaObject* p_obj = NULL;
    do
    {
        p_obj = _p_session->ObjectListIterate();
        if ( !p_obj )
            objs.push_back( p_obj );

    } while( p_obj );

    _p_session->ObjectListFinishIterate();
    unlockObjects();
}

void NetworkDevice::updateServer( float /*deltaTime*/ )
{
    // poll the netwroking core
    _p_session->Poll();

    // check for pre-connection requests
    int         sessionId;
    static char s_buffer[ 512 ];
    int         bufferLength;
    while ( _p_session->DataReceive( &sessionId, s_buffer, &bufferLength ) )
    {
        PreconnectDataClient* p_data = reinterpret_cast< PreconnectDataClient* >( s_buffer );
        if ( p_data->_typeId == static_cast< unsigned char >( YAF3DNW_PRECON_DATA_CLIENT ) )
        {
            switch ( p_data->_state )
            {
                case eConnecting:
                {
                    std::string conurl = _p_session->GetURLFromSessionID( sessionId );
                    log_info << "NetworkDevice: server is requested for a new client connection, url  " << conurl << std::endl;

                    // send paket
                    PreconnectDataServer sendData;
                    memset( &sendData, 0, sizeof( PreconnectDataServer ) );
                    sendData._typeId = static_cast< unsigned char >( YAF3DNW_PRECON_DATA_SERVER );
                    sendData._protocolVersion = YAF3D_NETWORK_PROT_VERSION;

                    // need for auth?
                    if ( _serverNodeInfo.needAuthentification() )
                        sendData._needAuthentification = true;
                    else
                        sendData._needAuthentification = false;

                    sendData._state = eLogin;

                    _p_session->DataSend( sessionId, reinterpret_cast< void* >( &sendData ), sizeof( PreconnectDataServer ), RNReplicaNet::ReplicaNet::kPacket_Reliable );
                }
                break;

                case eLogin:
                {
                    // check if login and passwd exist
                    std::string login;
                    std::string passwd;
                    if ( p_data->_p_login[ 0 ] )
                    {
                        p_data->_p_login[ sizeof(  p_data->_p_login ) - 1 ] = 0;
                        login =  p_data->_p_login;
                        memset( p_data->_p_login, 0, sizeof(  p_data->_p_login ) );
                    }

                    if ( p_data->_p_passwd[ 0 ] )
                    {
                        p_data->_p_passwd[ sizeof(  p_data->_p_passwd ) - 1 ] = 0;
                        passwd =  p_data->_p_passwd;
                        memset( p_data->_p_passwd, 0, sizeof(  p_data->_p_passwd ) );
                    }

                    // send paket
                    PreconnectDataServer sendData;
                    memset( &sendData, 0, sizeof( PreconnectDataServer ) );
                    sendData._typeId = static_cast< unsigned char >( YAF3DNW_PRECON_DATA_SERVER );
                    sendData._protocolVersion = YAF3D_NETWORK_PROT_VERSION;
                    
                    // need for auth?
                    if ( _serverNodeInfo.needAuthentification() )
                        sendData._needAuthentification = true;
                    else
                        sendData._needAuthentification = false;

                    sendData._state = eLoginResult;

                    sendData._userID = static_cast< unsigned int >( -1 );
                    sendData._state  = eLoginResult;

                    // need for auth?
                    if ( _serverNodeInfo.needAuthentification() && _p_cbAuthentification )
                    {
                        // check the type of user ID!
                        unsigned int userID = static_cast< unsigned int >( -1 );
                        if ( !_p_cbAuthentification->authentify( sessionId, login, passwd, userID ) )
                        {
                            // access denied
                            sendData._accessGranted = false;
                            _p_session->DataSend( sessionId, reinterpret_cast< void* >( &sendData ), sizeof( PreconnectDataServer ), RNReplicaNet::ReplicaNet::kPacket_Reliable );
                            log_info << "NetworkDevice: access denied to user '" << login << "'" << std::endl;
                        }
                        else
                        {
                            // send server node
                            sendData._accessGranted = true;
                            sendData._userID        = userID;
                            strcpy( sendData._p_levelName, _serverNodeInfo._levelName.c_str() );
                            strcpy( sendData._p_serverName, _serverNodeInfo._nodeName.c_str() );
                            _p_session->DataSend( sessionId, reinterpret_cast< void* >( &sendData ), sizeof( PreconnectDataServer ), RNReplicaNet::ReplicaNet::kPacket_Reliable );
                            log_info << "NetworkDevice: access granted to user '" << login << "'" << std::endl;
                        }
                    }
                    else
                    {
                        // send server node
                        sendData._accessGranted = true;
                        strcpy( sendData._p_levelName, _serverNodeInfo._levelName.c_str() );
                        strcpy( sendData._p_serverName, _serverNodeInfo._nodeName.c_str() );
                        _p_session->DataSend( sessionId, reinterpret_cast< void* >( &sendData ), sizeof( PreconnectDataServer ), RNReplicaNet::ReplicaNet::kPacket_Reliable );
                        log_info << "NetworkDevice: access granted to user" << std::endl;
                    }

                    memset( &sendData, 0, sizeof( PreconnectDataServer ) );
                }
                break;

                case eRegister:
                {
                    if ( _p_cbAuthentification )
                    {
                        PreconnectDataServer sendData;
                        memset( &sendData, 0, sizeof( PreconnectDataServer ) );
                        sendData._typeId = static_cast< unsigned char >( YAF3DNW_PRECON_DATA_SERVER );
                        sendData._protocolVersion = YAF3D_NETWORK_PROT_VERSION;
                        
                        // need for auth?
                        if ( _serverNodeInfo.needAuthentification() )
                            sendData._needAuthentification = true;
                        else
                            sendData._needAuthentification = false;

                        sendData._state = eRegistrationResult;

                        p_data->_p_login[ sizeof( p_data->_p_login ) - 1 ] = 0;
                        p_data->_p_passwd[ sizeof( p_data->_p_passwd ) - 1 ] = 0;
                        p_data->_p_name[ sizeof( p_data->_p_name ) - 1 ] = 0;
                        p_data->_p_email[ sizeof( p_data->_p_email ) - 1 ] = 0;

                        if ( !_p_cbAuthentification->registerUser( p_data->_p_name, p_data->_p_login, p_data->_p_passwd, p_data->_p_email ) )
                        {
                            // registration failed
                            sendData._accessGranted = false;
                            log_info << "NetworkDevice: registration failed for user '" << p_data->_p_login << "'" << std::endl;
                        }
                        else
                        {
                            sendData._accessGranted = true;
                            log_info << "NetworkDevice: registration successful for user '" << p_data->_p_login << "'" << std::endl;
                        }
                        _p_session->DataSend( sessionId, reinterpret_cast< void* >( &sendData ), sizeof( PreconnectDataServer ), RNReplicaNet::ReplicaNet::kPacket_Reliable );
                    }
                }
                break;

                default:

                    log_error << "NetworkDevice: invalid connection state received from client!" << std::endl;
            }
        }
    }
}

void NetworkDevice::updateClient( float /*fDeltaTime*/ )
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
