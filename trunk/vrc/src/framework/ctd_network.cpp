/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2002-2004, Ali Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU General Public License 
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public 
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
#include <RNPlatform/Inc/FreewareCode.h>

using namespace std;
using namespace CTD;
using namespace RNReplicaNet;

CTD_SINGLETON_IMPL( NetworkDevice );


NetworkDevice::NetworkDevice() :
_mode(NONE),
_p_session(NULL),
_clientSessionStable(false),
_serverSessionStable(false)
{
    FreewareSetRegistrationCode( "64B8EBDH-HJK8L6EA-8EBAC" );
}

NetworkDevice::~NetworkDevice()
{
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
    assert( _p_session == NULL );
    _nodeInfo  = nodeInfo;
    _p_session = new ReplicaNet;
    if ( !_p_session ) 
        return false;

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
            shutdown();
            return false;
        }
	}
    _serverSessionStable  = true;
    _mode = NetworkingMode::SERVER;
    return true;
}

bool NetworkDevice::setupClient( const string& URL, int channel, const NodeInfo& nodeInfo )
{
    // do we already have a session created?
    assert( _p_session == NULL );
    _nodeInfo  = nodeInfo;
    _p_session = new ReplicaNet;
    if ( !_p_session ) 
    {
        cout << "*** nw client: cannot create network session instance" << endl;
        return false;
    }

    _p_session->SetManualPoll();
    //_p_session->SetGameChannel( channel );
    _p_session->SetLoadBalancing( false );
    _p_session->SetCanAcceptObjects( false );
    _p_session->SetCanBecomeMaster( false);
    _p_session->SetCanSpider( false );
    _p_session->SetDataRetention( true );
    _p_session->SetPreConnect( true );

    //! TODO: the url must also be given by user
    cout << "nw client:   trying to find sessions ..." << endl;
    _p_session->SessionFind();
	Sleep(500);
    string Url = _p_session->SessionEnumerateFound();
    if ( Url.length() > 0 ) {
        cout << "nw client:   session found, url: '" +Url + "'" << endl;
    } 
    else 
    {
        cout << "nw client:   could not find any session!" << endl;
        shutdown();
        return false;
    }
    cout << "nw client:   join to session ..." << endl;
    _p_session->SessionJoin( Url );
    //_p_session->SessionJoin( URL );

    // wait for pre-connect state
    while ( !_p_session->GetPreConnectStatus() ) 
    {
        _p_session->Poll();
        CurrentThread::Sleep( 100 );
    }

    cout << "nw client:   negotiating with server ..." << endl;

    // begin to negotiate with server
    //-----------------------------//
    cout << "nw client:      exchanging pre-connect data ..." << endl;

    PreconnectDataClient preconnectData;
    preconnectData._typeId = ( unsigned char )CTD_NW_PRECON_DATA_CLIENT;
    _p_session->DataSend( kReplicaNetUnknownUniqueID, &preconnectData, sizeof( PreconnectDataClient ), ReplicaNet::PacketType::kPacket_Reliable );

    int          sessionId;
    void*        p_buffer[ 512 ];
    int          bufferLength;
    bool         gotServerInfo = false;
    unsigned int tryCounter    = 0;
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
            shutdown();
            cout << "*** nw client: problems negotiating with server" << endl;
            return false;
        }
    }
    _clientSessionStable = true;
    _mode = NetworkingMode::CLIENT;

    return true;
}

bool NetworkDevice::startClient()
{
    _p_session->PreConnectHasFinished();

    cout << "nw client: successfully integrated to network" << endl;
    stringstream msg;
    msg << "nw client: server name: '" << 
        _nodeInfo._nodeName  << 
        "', level name: '"   << 
        _nodeInfo._levelName << 
        "'" << endl;
    cout << msg.str() << endl;
    //-----------------------------//

    unsigned int tryCounter = 0;
	while ( !_p_session->IsStable() )
	{
        _p_session->Poll();    // during this time we have to poll the session instance as we disabled the automatic poll!

        CurrentThread::Sleep( 200 );
        tryCounter++;
        cout << ".";

        // try up to 10 seconds
        if ( tryCounter > 50 ) 
        {
            shutdown();
            cout << "*** nw client: problems connecting to server" << endl;
            return false;
        }
    }    
    cout << endl;
    cout << "nw client:   successfully joined to session ..." << endl;

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
