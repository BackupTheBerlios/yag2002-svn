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
 # neoengine, framework implementation class for standalone mode
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
#include "ctd_framework_impl_standalone.h"
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

using namespace std;
using namespace NeoEngine;
using namespace NeoChunkIO;

//-------------------------------------------------------------------//

namespace CTD
{

FrameworkImplStandalone::FrameworkImplStandalone() 
{
    // set game mode to client
    m_eGameMode = stateSTANDALONE;
}

FrameworkImplStandalone::~FrameworkImplStandalone()
{
    m_pkWidgets->Shutdown();    
}


bool FrameworkImplStandalone::Initialize( int iArgc, char** ppcArgv )
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

    SetupSoundDevice();

    neolog << LogLevel( INFO ) << endl << "-----------------------------------------------" << endl << endl;

    //--------------------------------------------------------//

    // initialze the widget system
    m_pkWidgets = Widgets::Get();
    m_pkWidgets->Initialize();

    m_kTimer.Reset();
    m_kFPSTimer.Reset();

    return true;

}

void FrameworkImplStandalone::StartGame()
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

void FrameworkImplStandalone::ExitGame()
{

    m_bSimulationRun = false;

    FrameworkImpl::ExitGame();

}

void FrameworkImplStandalone::GameLoop()
{

    float           fDeltaTime;
    int             m_iFrames      = 0;
    unsigned int    uiNumLevelSets = 0;
    unsigned int    uiLevelSetCnt  = 0;
    LevelSet**      apkLevelSets   = 0;

    NeoEngine::InputManager *pkInputMgr = m_pkCore->GetInputManager();

    // setup ortho projection matrix
    m_pkRenderDevice->SetOrthographicProjection( 0, 0, 1.0f, 1.0f, -100.0f, 100.0f );

    while ( m_bSimulationRun != false ) {

        fDeltaTime = m_kTimer.GetDeltaTime();

         // calculate fps
        ++m_iFrames;
        if( m_kFPSTimer.GetDeltaTime( false ) > 1.0f )
        {
            m_fFPS    = float( m_iFrames ) / m_kFPSTimer.GetDeltaTime( true );
            m_iFrames = 0;
        }

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

        // now dras all widgets, if there are any
        m_pkWidgets->Update();
        //----------------------//

        m_pkRenderDevice->End();

        //--------------------------------------------------------//
        m_pkRenderDevice->SetPerspectiveProjection();

        //--------------------------------------------------------//

        m_pkRenderDevice->Flip();

        // do input processing at last, so shutdown requests can be immediately processed
        pkInputMgr->Process();

    }

}


} // namespace CTD
