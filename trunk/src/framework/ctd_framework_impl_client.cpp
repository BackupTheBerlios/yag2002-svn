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
 # neoengine, framework implementation class for client mode
 #
 #
 #   date of creation:  09/24/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 #
 ################################################################*/

#include <ctd_frbase.h>
#include "ctd_framework.h"
#include "ctd_framework_impl_client.h"
#include <ctd_keyconfig.h>
#include <ctd_levelloader.h>
#include <ctd_settings.h>
#include <ctd_network.h>
#include <ctd_levelmanager.h>

#include <neodevopengl/link.h>

#include <neodevdsound/link.h>
#include <neoacwav/link.h>
#include <neoacvorbis/link.h>


// we don't need glo headers here
#define CTD_NONEED_GLO_HEADERS
#include <widgets/ctd_widgets.h>

// default level file to load on start when nothing defined with -level argument
#define  CTD_MENU_LEVEL     "menu.lvl"

using namespace std;
using namespace NeoEngine;
using namespace NeoChunkIO;

//-------------------------------------------------------------------//

namespace CTD
{

FrameworkImplClient::FrameworkImplClient() 
{
    // set game mode to client
    m_eGameMode         = stateCLIENT;
    m_eClientNetworking = stateOff;
    m_pkWidgets         = NULL;
    m_bFullScreen       = false;
}

FrameworkImplClient::~FrameworkImplClient()
{
    if ( m_pkWidgets ) m_pkWidgets->Shutdown();    
}


bool FrameworkImplClient::Initialize( int iArgc, char** ppcArgv )
{

    // default elvel file to start is the menu level files
    m_strLevelName = CTD_MENU_LEVEL;

    //parse arguments, if -level defined
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

    SetupSoundDevice();

    neolog << LogLevel( INFO ) << endl << "-----------------------------------------------" << endl << endl;

    //--------------------------------------------------------//


    // attach log file sink
    string strCfgAttribute;
    NeoEngine::Config *pkConfig = m_pkCore->GetConfig();
    if( pkConfig->GetValue( CTD_CFG_ATTR_LOGOUTPUT, &strCfgAttribute ) )
    {

        if( strCfgAttribute.find( "file" ) != string::npos ) {

            string strLogFile;
            strLogFile = CTD_LOG_FILE_CLIENT;
            m_pkLogFile = new LogFileSink( strLogFile );
            neolog.AttachSink( m_pkLogFile );
       }

    }

    // initialze the widget system
    m_pkWidgets = Widgets::Get();
    m_pkWidgets->Initialize();

    // create a network device
    m_pkNetworkDevice   = new NetworkDevice( this );

    m_kTimer.Reset();
    m_kFPSTimer.Reset();

    return true;

}

void FrameworkImplClient::StartGame()
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

void FrameworkImplClient::ExitGame()
{

    m_bSimulationRun = false;

    FrameworkImpl::ExitGame();

}

void FrameworkImplClient::GameLoop()
{

    float           fDeltaTime;
    int             m_iFrames = 0;
    unsigned int    uiNumLevelSets = 0;
    unsigned int    uiLevelSetCnt  = 0;
    LevelSet**      apkLevelSets   = 0;

    NeoEngine::InputManager *pkInputMgr = m_pkCore->GetInputManager();

    // setup ortho projection matrix
    m_pkRenderDevice->SetOrthographicProjection( 0, 0, 1.0f, 1.0f, -100.0f, 100.0f );

    while ( m_bSimulationRun != false ) {

        fDeltaTime = m_kTimer.GetDeltaTime();

        // calculate fps
        //--------------------------------------------------------//
        ++m_iFrames;
        if( m_kFPSTimer.GetDeltaTime( false ) > 1.0f )
        {
            m_fFPS    = float( m_iFrames ) / m_kFPSTimer.GetDeltaTime( true );
            m_iFrames = 0;
        }

        // do network processing
        m_pkNetworkDevice->UpdateClient( fDeltaTime );

        // update and render level sets
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

            if ( apkLevelSets[ uiLevelSetCnt ]->GetRenderFlag() == true ) {

                // send a message to all plugins for 3d-rendering
                apkLevelSets[ uiLevelSetCnt ]->GetPluginManager()->SendPluginMessage( CTD_PLUGIN_MSG_RENDER_3D, ( void* )NULL );

            }

            uiLevelSetCnt++;

        }
        //----------------------------------------------------------------//

        // process the prerender queue, this phase is useful for mirror effects etc.
        //----------------------------------------------//
        static  vector< BaseEntity* >::iterator ipkPRenderQueueEnd;
        static  vector< BaseEntity* >::iterator ipkPRenderEntity;

        ipkPRenderEntity        = m_vpkPreRenderEntities.begin();
        ipkPRenderQueueEnd      = m_vpkPreRenderEntities.end();

        while ( ipkPRenderEntity != ipkPRenderQueueEnd ) {

            ( *ipkPRenderEntity )->PreRender();
            ipkPRenderEntity++;

        }

        // render camera view
        //--------------------------------------------------------//

        //clear target first
        if ( m_bUseStencilBuffer == true ) {

            m_pkRenderDevice->Clear( RenderDevice::COLORBUFFER | RenderDevice::ZBUFFER | RenderDevice::STENCILBUFFER, m_kBackgoundColor, 1.0f, 0 );

        } else {

            m_pkRenderDevice->Clear( RenderDevice::COLORBUFFER | RenderDevice::ZBUFFER, Color::BLACK, 1.0f, 0 );

        }

        m_pkRenderDevice->Begin( m_pkCurrentLevelSet->GetCamera()->GetViewMatrix() );

        m_pkCurrentLevelSet->GetCamera()->Render();

        m_pkRenderDevice->End();
        //--------------------------------------------------------//

        // start the 2d rendering phase
        //--------------------------------------------------------//
        m_pkRenderDevice->SetOrthographicProjection();

        // clear the z-buffer for 2d rendering, so no 3d objects from room can interfere into 2d primitives
        m_pkRenderDevice->Clear( RenderDevice::ZBUFFER, Color::BLACK, 1.0f, 0 );

        // in 2d rendering avoid primitive sorting, we want to render in the same order as we fill the pipeline
        m_pkRenderDevice->Begin( Matrix::IDENTITY, RenderDevice::NOSORT );

        // send a message to all plugins for 2d-rendering
        uiLevelSetCnt = 0;
        while ( uiLevelSetCnt < uiNumLevelSets ) {

            if ( apkLevelSets[ uiLevelSetCnt ]->GetRenderFlag() == true ) {

                // send a message to all plugins for 3d-rendering
                apkLevelSets[ uiLevelSetCnt ]->GetPluginManager()->SendPluginMessage( CTD_PLUGIN_MSG_RENDER_2D, ( void* )NULL );

            }

            uiLevelSetCnt++;

        }
        //----------------------------------------------------------------//

        // handle all registered entities
        static  vector< BaseEntity* >::iterator ipk2DRenderEnd;
        static  vector< BaseEntity* >::iterator ipk2DRenderEntity;

        ipk2DRenderEntity       = m_vpk2DRenderEntities.begin();
        ipk2DRenderEnd          = m_vpk2DRenderEntities.end();

        while ( ipk2DRenderEntity != ipk2DRenderEnd ) {

            ( *ipk2DRenderEntity )->Render2D();
            ipk2DRenderEntity++;

        }

        // now draw all widgets, if there are any
        m_pkWidgets->Update();
        //----------------------//

        m_pkRenderDevice->End();

        //--------------------------------------------------------//
        m_pkRenderDevice->SetPerspectiveProjection();

        //--------------------------------------------------------//

        m_pkRenderDevice->Flip();

        // do input processing at last, so shutdown requests can be immediately processed
        pkInputMgr->Process();

        // if not in full windowed mode then lets other applications live!
        if ( !m_bFullScreen ) {
            Sleep( 5 );
        }

    }

}

// setup networking
bool FrameworkImplClient::InitiateClientNetworking( int iClientPort, int iServerPort, const string& strServerIP, const string& strNodeName )
{

    if ( m_eClientNetworking != stateOff ) {

        neolog << LogLevel( WARNING ) << " Cannot initiate client networking; it is already running!" << endl;
        return false;

    }

    bool   bClientHighPriority = false;

    neolog << LogLevel( INFO ) << "nw-client: connecting to server ..." << endl;

    // setup client information
    m_pkNetworkDevice->SetupClientStaticData( strNodeName );

    m_pkNetworkDevice->Initialize( false );

    if ( m_pkNetworkDevice->ConnectClient( ( char* )strServerIP.c_str(), iServerPort, iClientPort, bClientHighPriority ) == false ) {

        neolog << LogLevel( WARNING ) << "*** nw client: cannot find server" << endl;
        return false;

    }

    // wait here until the connection is established and negotiation with server is done
    if ( m_pkNetworkDevice->EstablishConnection() == false ) {

        neolog << LogLevel( WARNING ) << "nw client: cannot get server information" << endl;
        return false;

    }

    // set the level name to be loaded received from server
    m_strLevelName  = m_pkNetworkDevice->GetLevelName();

    m_eClientNetworking = stateInitiated;

    return true;

}

bool FrameworkImplClient::StartClientNetworking()
{

    if ( m_eClientNetworking != stateInitiated ) {

        neolog << LogLevel( WARNING ) << " Cannot start client networking; it must first be initiated!" << endl;
        return false;

    }

    m_eClientNetworking = stateStarted;

    return true;

}

void FrameworkImplClient::ShutdownClientNetworking()
{

    if ( m_eClientNetworking == stateOff ) {

        neolog << LogLevel( WARNING ) << " Cannot shutdown client networking; it has not been initiated!" << endl;
        return;

    }

    m_pkNetworkDevice->Shutdown();
    m_eClientNetworking = stateOff;

}

} // namespace CTD
