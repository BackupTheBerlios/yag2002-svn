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

#include <ctd_base.h>
#include "ctd_network.h"
#include "ctd_log.h"
#include "ctd_utils.h"
#include "ctd_configuration.h"
#include <RNPlatform/Inc/FreewareCode.h>

using namespace std;
using namespace CTD;
using namespace RNReplicaNet;

CTD_SINGLETON_IMPL( NetworkDevice );


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
    _mode = NetworkingMode::NONE;
    _nodeInfo._levelName = "";
    _nodeInfo._nodeName  = "";
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

    _mode = NetworkingMode::NONE;

    // destroy singleton
    destroy();
}

bool NetworkDevice::setupServer( int channel, const NodeInfo& nodeInfo  )
{
    // do we already have a session created?
    assert( _p_session == NULL && "there is already a running session!" );

    CTD::log << CTD::Log::LogLevel( CTD::Log::L_DEBUG ) << "starting server, time: " << CTD::getTimeStamp() << endl;

    _nodeInfo  = nodeInfo;
    _p_session = new ReplicaNet;

    log << Log::LogLevel( Log::L_INFO ) << "nw server: starting network session: " << nodeInfo._nodeName << endl;

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
        CurrentThread::Sleep( 500 );
        tryCounter++;

        // try up to 10 seconds
        if ( tryCounter > 20 ) 
        {
            _p_session->Disconnect();
            delete _p_session;
            _p_session = NULL;   

            return false;
        }
	}
    _serverSessionStable  = true;
    _mode = NetworkingMode::SERVER;

    string url = _p_session->SessionExportURL();
    log << Log::LogLevel( Log::L_INFO ) << "nw server: session established: " << url << endl;
    return true;
}

bool NetworkDevice::setupClient( const string& serverIp, int channel, const NodeInfo& nodeInfo )
{
    // do we already have a session created?
    assert( _p_session == NULL && "there is already a running session!" );
    _nodeInfo  = nodeInfo;
    _p_session = new ReplicaNet;

    _p_session->SetManualPoll();
    //_p_session->SetGameChannel( channel );
    _p_session->SetLoadBalancing( false );
    _p_session->SetCanAcceptObjects( false );
    _p_session->SetCanBecomeMaster( false);
    _p_session->SetCanSpider( false );
    _p_session->SetDataRetention( true );
    _p_session->SetPreConnect( true );

    //! if the URL is empty then auto-search for sessions in loacal net
    string Url;
    if ( !serverIp.length() )
    {
        log << Log::LogLevel( Log::L_INFO ) << "nw client: trying to find sessions ..." << endl;
        _p_session->SessionFind();
        Sleep( 500 );
        Url = _p_session->SessionEnumerateFound();
        if ( Url.length() > 0 ) {
            log << Log::LogLevel( Log::L_INFO ) << "nw client: session found, url: '" + Url + "'" << endl;
        } 
        else 
        {
            log << Log::LogLevel( Log::L_WARNING ) << "nw client:   could not find any session!" << endl;
            _p_session->Disconnect();
            delete _p_session;
            _p_session = NULL;   
            return false;
        }
    }
    else
    {	
        //example url: "SESSION://UDP@127.0.0.1:32001/gameserver"}
        string servername;
        Configuration::get()->getSettingValue( CTD_GS_SERVER_NAME, servername );
        stringstream assembledUrl;
        assembledUrl << "SESSION://UDP@" << serverIp << ":" << channel << "/" << servername;
        Url = assembledUrl.str();
    }

    log << Log::LogLevel( Log::L_INFO ) << "nw client: try to join to session: " << Url << endl;
    _p_session->SessionJoin( Url );

    unsigned int tryCounter = 0;
    // wait for pre-connect state for 10 seconds
    while ( !_p_session->GetPreConnectStatus() && tryCounter < 100 ) 
    {
        _p_session->Poll();
        CurrentThread::Sleep( 100 );
        tryCounter++;
    }
    if ( tryCounter == 100 )
    {
        log << Log::LogLevel( Log::L_WARNING ) << "nw client: cannot connect to server: " << Url << endl;

        _p_session->Disconnect();
        delete _p_session;
        _p_session = NULL;   

        return false;
    }

    log << Log::LogLevel( Log::L_INFO ) << "nw client: negotiating with server ..." << endl;

    // begin to negotiate with server
    //-----------------------------//
    log << Log::LogLevel( Log::L_INFO ) << "nw client:  exchanging pre-connect data ..." << endl;

    PreconnectDataClient preconnectData;
    preconnectData._typeId = ( unsigned char )CTD_NW_PRECON_DATA_CLIENT;
    _p_session->DataSend( kReplicaNetUnknownUniqueID, &preconnectData, sizeof( PreconnectDataClient ), ReplicaNet::PacketType::kPacket_Reliable );

    int          sessionId;
    void*        p_buffer[ 512 ];
    int          bufferLength;
    bool         gotServerInfo = false;
    tryCounter = 0;
    while ( !gotServerInfo ) 
    {        
        _p_session->Poll();
        CurrentThread::Sleep( 100 );
        while ( _p_session->DataReceive( &sessionId, p_buffer, &bufferLength ) ) 
        {
            PreconnectDataServer* p_data = ( PreconnectDataServer* )p_buffer;
            if ( p_data->_typeId == ( unsigned char )CTD_NW_PRECON_DATA_SERVER ) 
            {
                p_data->_p_levelName[ 63 ]  = 0;
                p_data->_p_serverName[ 63 ] = 0;
                _nodeInfo._levelName = p_data->_p_levelName;
                _nodeInfo._nodeName  = p_data->_p_serverName;
                gotServerInfo = true;
            }
        }

        // try up to 10 seconds
        if ( tryCounter > 100 ) 
        {
            log << Log::LogLevel( Log::L_WARNING ) << "*** nw client: problems negotiating with server" << endl;

            _p_session->Disconnect();
            delete _p_session;
            _p_session = NULL;   

            return false;
        }
    }
    _clientSessionStable = true;
    _mode = NetworkingMode::CLIENT;

    return true;
}

bool NetworkDevice::startClient()
{
    if ( !_clientSessionStable )
    {
        log << Log::LogLevel( Log::L_WARNING ) << "nw client: starting client without a stable session, cannot start client!" << endl;
        return false;
    }

    CTD::log << CTD::Log::LogLevel( CTD::Log::L_DEBUG ) << "starting client, time: " << CTD::getTimeStamp() << endl;

    _p_session->PreConnectHasFinished();

    log << Log::LogLevel( Log::L_INFO ) << "nw client: successfully integrated to network" << endl;
    stringstream msg;
    msg << "nw client: server name: '" << 
        _nodeInfo._nodeName  << 
        "', level name: '"   << 
        _nodeInfo._levelName << 
        "'" << endl;
    log << Log::LogLevel( Log::L_INFO ) << msg.str() << endl;
    //-----------------------------//

    unsigned int tryCounter = 0;
    log.enableSeverityLevelPrinting( false );
	while ( !_p_session->IsStable() )
	{
        _p_session->Poll();    // during this time we have to poll the session instance as we disabled the automatic poll!

        CurrentThread::Sleep( 200 );
        tryCounter++;
        log << Log::LogLevel( Log::L_DEBUG ) << ".";

        // try up to 10 seconds
        if ( tryCounter > 50 ) 
        {
            log << Log::LogLevel( Log::L_WARNING ) << "*** nw client: problems connecting to server" << endl;

            _p_session->Disconnect();
            delete _p_session;
            _p_session = NULL;   

            return false;
        }
    }    
    log << Log::LogLevel( Log::L_DEBUG ) << endl;
    log.enableSeverityLevelPrinting( true );

    string sessionurl = _p_session->SessionExportURL();
    log << Log::LogLevel( Log::L_INFO ) << "nw client: successfully joined to session: " << sessionurl << endl;

    return true;
}

NodeInfo* NetworkDevice::getNodeInfo()
{
    if ( _clientSessionStable || _serverSessionStable )
        return &_nodeInfo;

    return NULL;
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
        if ( p_data->_typeId == ( unsigned char )CTD_NW_PRECON_DATA_CLIENT ) 
        {
            log << Log::LogLevel( Log::L_INFO ) << "server: new client connecting ... " << endl;
            // send server node
            PreconnectDataServer sendData;
            sendData._typeId = ( unsigned char )CTD_NW_PRECON_DATA_SERVER;
            strcpy( sendData._p_levelName, _nodeInfo._levelName.c_str() );
            strcpy( sendData._p_serverName, _nodeInfo._nodeName.c_str() );        
            _p_session->DataSend( sessionId, ( void* )&sendData, sizeof( PreconnectDataServer ), ReplicaNet::PacketType::kPacket_Reliable );        
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
