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
 # neoengine, framework implementation class for server mode
 #
 #
 #   date of creation:  09/25/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 #
 ################################################################*/

#include <ctd_frbase.h>
#include "ctd_framework.h"
#include "ctd_framework_impl_server.h"
#include <ctd_levelloader.h>
#include <ctd_settings.h>
#include <ctd_network.h>
#include <ctd_levelmanager.h>

#include <neodevnull/link.h>

using namespace std;
using namespace NeoEngine;
using namespace NeoChunkIO;

//-------------------------------------------------------------------//

namespace CTD
{


FrameworkImplServer::FrameworkImplServer() 
{
    // set game mode to server
    m_eGameMode = stateSERVER;
}

FrameworkImplServer::~FrameworkImplServer()
{   
}


bool FrameworkImplServer::Initialize( int iArgc, char** ppcArgv )
{

    //parse arguments, if -level defined then launch the given level
    //-----------------------------------------------------//
    for( int iArg = 0; iArg < iArgc; ++iArg )
    {
        if( !strcmp( ppcArgv[ iArg ], "-level" ) )
        {
            ++iArg;

            if( iArg < iArgc )
            {
                m_strLevelName = ppcArgv[ iArg ];
                break;
            
            } else {

                throw( FrameworkException( "Use program arguments '-level <level file name>' " ) );
            }
        }
    }
    //-----------------------------------------------------//

    SetupRenderDevice();

    SetupNetworking();

    neolog << LogLevel( INFO ) << endl << "-----------------------------------------------" << endl << endl;

    // attach log file sink
    string strCfgAttribute;
    NeoEngine::Config *pkConfig = m_pkCore->GetConfig();
    if( pkConfig->GetValue( CTD_CFG_ATTR_LOGOUTPUT, &strCfgAttribute ) )
    {

        if( strCfgAttribute.find( "file" ) != string::npos ) {

            string strLogFile;
            strLogFile = CTD_LOG_FILE_SERVER;
            m_pkLogFile = new LogFileSink( strLogFile );
            neolog.AttachSink( m_pkLogFile );

        }

    }

    m_kTimer.Reset();
    m_kFPSTimer.Reset();

    return true;

}

void FrameworkImplServer::StartGame()
{

    // in client mode the level name is retrieved from server in function 'SetupNetworking'
    // load level
    if ( LoadLevel( m_strLevelName ) == false ) {

        throw( FrameworkException( "Cannot find level file '" + m_strLevelName + "'" ) );
    
    }
    //-----------------------------------------------------//

    neolog << LogLevel( INFO ) << "starting game" << endl << endl;

    // enter the game loop  
    GameLoop();

}

void FrameworkImplServer::ExitGame()
{

    m_bSimulationRun = false;

    FrameworkImpl::ExitGame();

}

void FrameworkImplServer::GameLoop()
{

    float           fDeltaTime;
    int             m_iFrames = 0;
    unsigned int    uiNumLevelSets = 0;
    unsigned int    uiLevelSetCnt  = 0;
    LevelSet**      apkLevelSets   = 0;

    while ( m_bSimulationRun != false ) {

        fDeltaTime = m_kTimer.GetDeltaTime();

        // do network processing
        m_pkNetworkDevice->UpdateServer( fDeltaTime );

        // update level sets
        apkLevelSets            = m_pkLevelManager->GetAllLevelSets();
        uiNumLevelSets          = m_pkLevelManager->GetLevelSetCount();
        uiLevelSetCnt           = 0;
        while ( uiLevelSetCnt < uiNumLevelSets ) {

            if ( apkLevelSets[ uiLevelSetCnt ]->GetUpdateFlag() == true ) {
           
                // update plugins
                apkLevelSets[ uiLevelSetCnt ]->GetPluginManager()->SendPluginMessage( CTD_PLUGIN_MSG_UPDATE, ( void* )NULL );

                // update all active entities in plugins
                apkLevelSets[ uiLevelSetCnt ]->GetPluginManager()->UpdateEntities( fDeltaTime );

                // upate room
                apkLevelSets[ uiLevelSetCnt ]->GetRoom()->Update( fDeltaTime );

            }

            uiLevelSetCnt++;

        }
        //----------------------------------------------------------------//

        // yield processor time
        Sleep( 5 );

    }

}

// setup networking
bool FrameworkImplServer::SetupNetworking()
{

    neolog << LogLevel( INFO ) << endl << "-----------------------------------------------" << endl << endl;

    bool            bServerHighPriority = false;
    int             iServerPort;
    unsigned short  usMaxConnections    = 15;
    string          strServerIP;
    string          strNodeName;

    GetGameSettings()->GetValue( CTD_ITOKEN_SERVERPORT, iServerPort );
    GetGameSettings()->GetValue( CTD_STOKEN_SERVERIP, strServerIP );
    GetGameSettings()->GetValue( CTD_STOKEN_PLAYERNAME, strNodeName );

    m_pkNetworkDevice   = new NetworkDevice( this );

    neolog << LogLevel( INFO ) << "CTD starting server ..." << endl;

    // setup server information
    m_pkNetworkDevice->SetupServerStaticData( strNodeName, m_strLevelName );

    m_pkNetworkDevice->Initialize( true );

    if ( m_pkNetworkDevice->StartServer( usMaxConnections, bServerHighPriority, iServerPort ) == false ) {

        neolog << LogLevel( ERROR ) << "*** nw server: server could not be started, exit..." << endl;
        return false;

    } 

    return true;

}

void FrameworkImplServer::SetupRenderDevice()
{

    if( !( m_pkRenderDevice = NeoEngine::Core::Get()->CreateRenderDevice( "null" ) ) )
    {
        
        throw( FrameworkException( "Cannot create null-device for rendering" ) );

    }

}

} // namespace CTD
