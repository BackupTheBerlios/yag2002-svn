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
 # neoengine, framework implementation class
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
#include "ctd_framework_impl.h"
#include <ctd_keyconfig.h>
#include <ctd_levelloader.h>
#include <ctd_settings.h>
#include <ctd_network.h>
#include <ctd_levelmanager.h>


#include <neofcbzip2/link.h>
#include <neofczip/link.h>
#include <neoicpng/link.h>
#include <neoictga/link.h>
#include <neoicjpeg/link.h>
#include <neoicbmp/link.h>
#include <neoabt/link.h>
#include <neobsp/link.h>

using namespace std;
using namespace NeoEngine;
using namespace NeoChunkIO;

//-------------------------------------------------------------------//

// new vertex type supporting base and lightmap textures
class NormalDiffuseLightmapTexVertex
{
    public:

        /*! Position */
        NeoEngine::Vector3d                           m_kPosition;

        /*! Normal */
        NeoEngine::Vector3d                           m_kNormal;

        /*! Base texture coordinate */
        float                                         m_afBaseTexCoord[2];

        /*! Lightmap texture coordinate */
        float                                         m_afLmTexCoord[2];


        /*! Vertex declaration */
        static NeoEngine::VertexDeclaration           s_kDecl;
};

// register our new vertex type
VertexDeclaration NormalDiffuseLightmapTexVertex::s_kDecl( 4 );


namespace CTD
{

#define CTD_CFG_ATTR_LOGLEVEL       "LogLevel"
#define CTD_CFG_ATTR_LOGOUTPUT      "LogOutput"
#define CTD_CFG_ATTR_PACKAGES       "Packages"
#define CTD_CFG_ATTR_KEY_CONFIG     "KeyConfigFile"

#define CTD_CFG_ATTR_RENDER_DEVICE  "RenderDevice"
#define CTD_CFG_ATTR_SCREEN_W       "RenderWidth"
#define CTD_CFG_ATTR_SCREEN_H       "RenderHeight"
#define CTD_CFG_ATTR_COLOR_DEPTH    "RenderDepth"
#define CTD_CFG_ATTR_FULLSCREEN     "RenderFullscreen"
#define CTD_CFG_ATTR_STENCILBUFF    "StencilBuffer"
#define CTD_CFG_ATTR_SHADOW_DEPTH   "ShadowDepth"
#define CTD_CFG_ATTR_REFRESHRATE    "Refreshrate"

// engine's config file related defines
#define CTD_CONFIG_FILE_NAME        "./engine.cfg"

FrameworkImpl::FrameworkImpl()
{

    m_pkCore                = NULL;
    m_pkRenderDevice        = NULL;
    m_bUseStencilBuffer     = false;
    m_pkAudioDevice         = NULL;
    m_pkNetworkDevice       = NULL;
    m_pkLevelManager        = NULL;
    m_pkCurrentLevelSet     = NULL;
    m_pkKeyConfiguration    = NULL;
    m_kBackgoundColor       = Color::BLACK;

    m_fFPS                  = 0.0f;
    m_bSimulationRun        = true;
    m_eGameMode             = stateSTANDALONE;

    m_uiScreenHeight        = 480;
    m_uiScreenWidth         = 640;
    m_uiColorDepth          = 16;
    m_uiRefreshRate         = 60;
    m_bFullScreen           = false;
    m_fShadowDepth          = 10.0f;


    m_pkCore = NeoEngine::Core::Get();

    neolog.SetLogThreshold( DEBUG );
    neolog.AttachSink( m_pkCore->GetStdoutSink() );

    char    pcCurrDir[256];
#ifdef WIN32
    GetCurrentDirectory( 256, pcCurrDir );
    m_strRootPath = pcCurrDir;
#elif defined(POSIX) || defined(__APPLE__)
    m_strRootPath = getcwd( pcCurrDir, 1 );
#endif
    
    // initialize engine core
    m_pkCore->Initialize( 0, NULL );
    
    // initialize chunk types
    NeoChunkIO::Core::Get()->Initialize();

    // setup our new vertex type ( this is used for lightmapping )
    //*****************************************************************************************//
    NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[0].m_uiType   = VertexElement::FLOAT3;
    NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[0].m_uiUsage  = VertexElement::POSITION;
    NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[0].m_uiOffset = 0;

    NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[1].m_uiType   = VertexElement::FLOAT3;
    NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[1].m_uiUsage  = VertexElement::NORMAL;
    NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[1].m_uiOffset = 12;

    NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[2].m_uiType   = VertexElement::FLOAT2;
    NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[2].m_uiUsage  = VertexElement::TEXCOORD;
    NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[2].m_uiIndex  = 0;
    NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[2].m_uiOffset = 24;

    NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[3].m_uiType   = VertexElement::FLOAT2;
    NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[3].m_uiUsage  = VertexElement::TEXCOORD;
    NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[3].m_uiIndex  = 1;
    NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[3].m_uiOffset = 32;
    //*****************************************************************************************//

    // load game settings
    Settings::Get()->Load( CTD_SETTINGS_FILE );

    // load engine configuration
    LoadConfig( CTD_CONFIG_FILE_NAME );

    // add compression codecs
     m_pkCore->GetFileManager()->LoadCodec( "zip" );
     m_pkCore->GetFileManager()->LoadCodec( "bzip2" );

    // setup data paths
    //--------------------------------------------------------//
    neolog << LogLevel( INFO ) << "setup packages ..." << endl;
    vector< string >    vstrPackages;
    Explode( " ", m_strPackages, &vstrPackages );
    for ( unsigned int uiPathCnt = 0; uiPathCnt < vstrPackages.size(); uiPathCnt++ ) {

        m_pkCore->GetFileManager()->AddPackage( vstrPackages[ uiPathCnt ] );

    }
    
    // create level manager
    m_pkLevelManager = new LevelManager;

    // create the key configuration instance
    m_pkKeyConfiguration = new KeyConfig;
    m_pkKeyConfiguration->Initialize( m_strKeyConfigFile );


}

FrameworkImpl::~FrameworkImpl()
{

    m_pkLevelManager->DestroyAll();
    delete m_pkLevelManager;

    delete m_pkKeyConfiguration;

    Settings::Get()->Shutdown();

    m_vpkNetworkClientObjects.clear();
    m_vpkNetworkServerObjects.clear();

    if ( m_pkNetworkDevice ) delete m_pkNetworkDevice;

    m_pkCore->DeleteAudioDevice( m_pkAudioDevice );
    m_pkCore->DeleteRenderDevice( m_pkRenderDevice );
    m_pkCore->Shutdown();
    
    NeoChunkIO::Core::Get()->Shutdown();

}

void FrameworkImpl::SendPluginMessage( int iMsgId, void *pMsgStruct, const string &strPluginName )
{

    unsigned int    uiNumLevelSets = m_pkLevelManager->GetLevelSetCount();
    LevelSet**      apkLevelSets   = m_pkLevelManager->GetAllLevelSets();
    unsigned int    uiLevelSetCnt  = 0;
    while ( uiLevelSetCnt < uiNumLevelSets ) {

        apkLevelSets[ uiLevelSetCnt ]->m_pkPluginMgr->SendPluginMessage( iMsgId, pMsgStruct, strPluginName );
        uiLevelSetCnt++;
    }

}

void FrameworkImpl::SendEntityMessage( int iMsgId, void* pMsgStruct, const string &strPluginName, bool bNetworkMessage )
{

    unsigned int    uiNumLevelSets = m_pkLevelManager->GetLevelSetCount();
    LevelSet**      apkLevelSets   = m_pkLevelManager->GetAllLevelSets();
    unsigned int    uiLevelSetCnt  = 0;
    while ( uiLevelSetCnt < uiNumLevelSets ) {

        apkLevelSets[ uiLevelSetCnt ]->m_pkPluginMgr->SendEntityMessage( iMsgId, pMsgStruct, strPluginName, bNetworkMessage );
        uiLevelSetCnt++;

    }

}

void FrameworkImpl::ExitGame()
{

    SendEntityMessage( CTD_ENTITY_EXIT_GAME, ( void* )NULL );
    SendPluginMessage( CTD_PLUGIN_MSG_SHUTDOWN, ( void* )NULL );

}

void FrameworkImpl::LoadConfig( const string& strFileName )
{

    string strCfgAttribute;
    
    File *pkFile = new File( "", strFileName , std::ios_base::binary | std::ios_base::in, false, 0 );   

    NeoEngine::Config *pkConfig = m_pkCore->GetConfig();
    if( !pkConfig->Load( pkFile ) )
    {
        neolog << LogLevel( ERROR ) << "*** unable to load config file" << endl;
        delete pkFile;
        throw( FrameworkException( "Cannot find engine configuration file '" + strFileName + "'" ) );

    }

    if( pkConfig->GetValue( CTD_CFG_ATTR_LOGLEVEL, &strCfgAttribute ) )
    {
        if( strCfgAttribute == "debug" )
            neolog.SetLogThreshold( DEBUG );
        else if( strCfgAttribute == "info" )
            neolog.SetLogThreshold( INFO );
        else if( strCfgAttribute == "warning" )
            neolog.SetLogThreshold( WARNING );
        else if( strCfgAttribute == "error" )
            neolog.SetLogThreshold( ERROR );
        else if( strCfgAttribute == "panic" )
            neolog.SetLogThreshold( PANIC );
    }

    // attach sinks!
    //-------------------------------------------------------------//
    neolog.DetachSink( m_pkCore->GetStdoutSink() );

    if( pkConfig->GetValue( CTD_CFG_ATTR_LOGOUTPUT, &strCfgAttribute ) )
    {
        if( strCfgAttribute.find( "stdout" ) != string::npos )
            neolog.AttachSink( m_pkCore->GetStdoutSink() );
        
        if( strCfgAttribute.find( "file" ) != string::npos ) {

            string strLogFile;
            if ( m_eGameMode == stateSERVER ) {

                strLogFile = CTD_LOG_FILE_SERVER;

            } else {

                strLogFile = CTD_LOG_FILE_CLIENT;

            }

            m_pkLogFile = new LogFileSink( strLogFile );
            neolog.AttachSink( m_pkLogFile );
        }

        if( strCfgAttribute.find( "console" ) != string::npos )
            neolog.AttachSink( ( LogSink* )m_pkCore->GetConsole() );
    }   
    //-------------------------------------------------------------//

    if( pkConfig->GetValue( CTD_CFG_ATTR_RENDER_DEVICE, &strCfgAttribute ) ) {

        m_strRenderDevice = strCfgAttribute;

    }

    if( pkConfig->GetValue( CTD_CFG_ATTR_KEY_CONFIG, &strCfgAttribute ) ) {

        m_strKeyConfigFile = strCfgAttribute;

    }

    if( pkConfig->GetValue( CTD_CFG_ATTR_PACKAGES, &strCfgAttribute ) ) {

        m_strPackages = strCfgAttribute;

    }

    if( pkConfig->GetValue( CTD_CFG_ATTR_SCREEN_W, &strCfgAttribute ) ) {

        m_uiScreenWidth = atoi(strCfgAttribute.c_str());

    }
    if( pkConfig->GetValue( CTD_CFG_ATTR_SCREEN_H, &strCfgAttribute ) ) {

        m_uiScreenHeight = atoi(strCfgAttribute.c_str());

    }
    if( pkConfig->GetValue( CTD_CFG_ATTR_COLOR_DEPTH, &strCfgAttribute ) ) {

        m_uiColorDepth = atoi(strCfgAttribute.c_str());

    }
    if( pkConfig->GetValue( CTD_CFG_ATTR_REFRESHRATE, &strCfgAttribute ) ) {

        m_uiRefreshRate = atoi(strCfgAttribute.c_str());

    }

    if( pkConfig->GetValue( CTD_CFG_ATTR_FULLSCREEN, &strCfgAttribute ) ) {

        if (strCfgAttribute == "false") {

            m_bFullScreen = false;

        } else {

            if (strCfgAttribute == "true") {

                m_bFullScreen = true;
            
            }
        
        }

    }
    if( pkConfig->GetValue( CTD_CFG_ATTR_STENCILBUFF, &strCfgAttribute ) ) {

        if (strCfgAttribute == "false") {

            m_bUseStencilBuffer = false;

        } else {

            if (strCfgAttribute == "true") {

                m_bUseStencilBuffer = true;
            
            }
        
        }

    }

    if( pkConfig->GetValue( CTD_CFG_ATTR_SHADOW_DEPTH, &strCfgAttribute ) ) {

        m_fShadowDepth = ( float )atof( strCfgAttribute.c_str() );

    }
    //--------------------------------------------------------//


    delete pkFile;
    
}

void FrameworkImpl::SetupRenderDevice()
{

    if( !( m_pkRenderDevice = NeoEngine::Core::Get()->CreateRenderDevice( m_strRenderDevice.c_str() ) ) ) {

        throw( FrameworkException( "Unable to create the graphics device." ) );

    }

    // set render caps
    //-----------------------------------------------------//
    RenderResolution render_res( m_uiScreenWidth, m_uiScreenHeight, m_uiColorDepth );
    render_res.m_uiRefreshRate = m_uiRefreshRate;
    if ( m_bFullScreen == true ) {
    
        m_kRenderCaps.Set( NeoEngine::RenderCaps::FULLSCREEN );

    } else {

        m_kRenderCaps.Reset( NeoEngine::RenderCaps::FULLSCREEN) ;

    }

    if ( m_bUseStencilBuffer == true ) {
    
        m_kRenderCaps.Set( NeoEngine::RenderCaps::STENCILBUFFER );

    } else {

        m_kRenderCaps.Reset( NeoEngine::RenderCaps::STENCILBUFFER );

    }


    // open device with required caps
    if( !m_pkRenderDevice->Open( RenderWindow( CTD_APP_TITLE, m_kRenderCaps, render_res ) ) ) {

        throw( FrameworkException( "Unable to open the graphics device." ) );

    }

    if ( m_bUseStencilBuffer == true ) {

        m_pkRenderDevice->m_pkShadowGenerator = new StencilShadowGenerator( m_fShadowDepth );
    }

    m_kRenderCaps = m_pkRenderDevice->GetCaps();


    // setup image codecs
    //--------------------------------------------------------//
    vector< string > vstrCodecs;
    string           strCodecs;

    if( NeoEngine::Core::Get()->GetConfig()->GetValue( "ImageCodecs", &strCodecs ) ) {

        Explode( " ", strCodecs, &vstrCodecs );

    }

    vector< string >::iterator pstrCodec    = vstrCodecs.begin();
    vector< string >::iterator pstrCodecEnd = vstrCodecs.end();

    for(; pstrCodec != pstrCodecEnd; ++pstrCodec) {

        if( pstrCodec->length() ) {

            m_pkRenderDevice->LoadImageCodec(*pstrCodec);
        
        }
    
    }
    //--------------------------------------------------------//

    // setup texture filtering
    //--------------------------------------------------------//

    //Get texture filtering modes from config
    string strFiltering;
    Texture::TEXTUREFILTERMODE eTMode = Texture::LINEAR;

    if( NeoEngine::Core::Get()->GetConfig()->GetValue( "TextureFilter", &strFiltering ) )
    {
        if( strFiltering == "bilinear" )
            eTMode = Texture::BILINEAR;
        else
        if( strFiltering == "trilinear" )
            eTMode = Texture::TRILINEAR;
    }

    m_pkRenderDevice->SetDefaultTextureFiltering( eTMode );

    // load system's default font
    NeoEngine::Core::Get()->GetConsole()->LoadDefaultFont();

}


void FrameworkImpl::SetupSoundDevice()
{

#ifdef WIN32
    m_strAudioDevice    = "dsound";
#else
    m_strAudioDevice    = "alsa";
#endif
    
    if( !( m_pkAudioDevice = NeoEngine::Core::Get()->CreateAudioDevice( m_strAudioDevice ) ) ) {

        throw( FrameworkException( "Unable to create audio device '" + m_strAudioDevice + "'" ) );

    } else {

        if( !m_pkAudioDevice->Open( m_pkRenderDevice->GetWindow() ) ) {

            throw( FrameworkException( "Unable to open audio device '" + m_strAudioDevice + "'" ) );

        }
    
    }

    // setup audio codecs
    //--------------------------------------------------------//
    vector< string > vstrAudioCodecs;
    string           strAudioCodecs;

    if( NeoEngine::Core::Get()->GetConfig()->GetValue( "AudioCodecs", &strAudioCodecs ) ) {

        Explode( " ", strAudioCodecs, &vstrAudioCodecs );

    }

    vector< string >::iterator pstrAudioCodec    = vstrAudioCodecs.begin();
    vector< string >::iterator pstrAudioCodecEnd = vstrAudioCodecs.end();

    for(; pstrAudioCodec != pstrAudioCodecEnd; ++pstrAudioCodec) {

        if( pstrAudioCodec->length() ) {

            m_pkAudioDevice->LoadSoundCodec( *pstrAudioCodec );
        
        }
    
    }

}

LevelSet* FrameworkImpl::LoadLevel( const std::string &strLevelFile ) 
{ 
    
    File *pkFile = NeoEngine::Core::Get()->GetFileManager()->GetByName( strLevelFile );
    if ( !pkFile || pkFile->Open( "", strLevelFile, ios_base::in | ios_base::binary ) == false ) {

        neolog << NeoEngine::LogLevel( NeoEngine::ERROR ) << "CTD error loading level '" << strLevelFile << "'" << endl;
        delete pkFile;
        return false;

   }
    
    LevelSet      *pkLevelSet       = new LevelSet;
    LevelLoader   *pkLevelLoader    = new LevelLoader( pkLevelSet, this );

    if ( pkLevelLoader->Load( pkFile, 0 ) == false) {

        delete pkLevelLoader;
        delete pkLevelSet;
        delete pkFile;
        return NULL;

    }

    // set the current level set, thus the plugins can access to it during initialization
    m_pkCurrentLevelSet = pkLevelSet;

    // add the new set to level manager
    m_pkLevelManager->Add( pkLevelSet );
    
    // first send a message to all plugins for initializing them
    neolog << LogLevel( INFO ) << "  CTD initializing plugins ..." << endl;
    pkLevelSet->m_pkPluginMgr->SendPluginMessage( CTD_PLUGIN_MSG_INIZIALIZE, ( void* )NULL );

    // then initialize entities in plugins
    neolog << LogLevel( INFO ) << "  CTD initializing entites ..." << endl;
    pkLevelSet->m_pkPluginMgr->InitializeEntities();

    // post-initialize entities in plugins; this is needed for entities to have the chance for attaching them to eachother
    neolog << LogLevel( INFO ) << "  CTD post-initializing entites ..." << endl;
    pkLevelSet->m_pkPluginMgr->PostInitializeEntities();

    // send post initialize message when all things are already initilaized
    neolog << LogLevel( INFO ) << "  CTD post-initializing plugins ..." << endl;
    pkLevelSet->m_pkPluginMgr->SendPluginMessage( CTD_PLUGIN_MSG_POSTINITIALIZE, ( void* )NULL );

    neolog << LogLevel( INFO ) << "-----------------------------------------------" << endl << endl;

    delete pkLevelLoader;
    delete pkFile;


    return pkLevelSet;

}

void FrameworkImpl::EmergencyShutdownNetworkDevice()
{

    if ( m_pkNetworkDevice ) {

        m_pkNetworkDevice->Shutdown();

    }

}

bool FrameworkImpl::GetKeyCode( const std::string &strKeyName , int &iKeyCode ) 
{

    // find the requested key name in key list
    unsigned int uiNumKeys = ( unsigned int )m_pkKeyConfiguration->m_vkKeyBindings.size();
    for ( unsigned int uiKeyCnt = 0; uiKeyCnt < uiNumKeys; uiKeyCnt++ ) {

        if ( m_pkKeyConfiguration->m_vkKeyBindings[ uiKeyCnt ].m_strKeyName == strKeyName ) {

            iKeyCode = m_pkKeyConfiguration->m_vkKeyBindings[ uiKeyCnt ].m_iKeyCode;
            return true;

        }
    }

    return false;

}

BaseEntity* FrameworkImpl::FindEntity( const std::string &strInstanceName, const std::string &strPluginName )
{

    BaseEntity* pkEntity = NULL;

    unsigned int    uiNumLevelSets = m_pkLevelManager->GetLevelSetCount();
    LevelSet**      apkLevelSets   = m_pkLevelManager->GetAllLevelSets();
    unsigned int    uiLevelSetCnt  = 0;
    while ( uiLevelSetCnt < uiNumLevelSets ) {

        pkEntity = SearchEntity( apkLevelSets[ uiLevelSetCnt ]->m_pkPluginMgr, strInstanceName, strPluginName );

        if ( pkEntity != NULL ) {

            return pkEntity;

        }

        uiLevelSetCnt++;

    }

    return NULL;

}

BaseEntity* FrameworkImpl::SearchEntity( PluginManager *pkPluginMgr, const string &strInstanceName, const string &strPluginName )
{

    Plugin          *pkPlugin = NULL;
    BaseEntity      *pkEntity = NULL;

    // check wether plugin name is given, if so search only in that plugin for the entity
    if ( strPluginName.length() > 0 ) {

        // search for plugin by name
        for ( unsigned int i = 0; i < pkPluginMgr->m_vpkPlugins.size(); i++ ) {

            if ( strPluginName == pkPluginMgr->m_vpkPlugins[i]->GetName() ) {

                pkPlugin = pkPluginMgr->m_vpkPlugins[i];
                break;

            }
        }

        // could we find plugin?
        if ( pkPlugin == NULL ) {

            return NULL;

        } else {

            for ( unsigned int j = 0; j < pkPlugin->m_vpkEntities.size(); j++ ) {

                if ( strInstanceName == pkPlugin->m_vpkEntities[j]->GetInstanceName() ) {

                    pkEntity = pkPlugin->m_vpkEntities[j];
                    break;
                }

            }

            // could we find the entity?
            if ( pkEntity == NULL ) {

                return NULL;

            } else {

                return pkEntity;
            }
        }

    }
    else
    {
        // search for instance name in all plugins, check for unique instance name
        unsigned int uiInstCounter = 0;
        for ( unsigned int i = 0; i < pkPluginMgr->m_vpkPlugins.size(); i++ ) {

            for ( unsigned int j = 0; j < pkPluginMgr->m_vpkPlugins[i]->m_vpkEntities.size(); j++ ) {
                
                if ( strInstanceName == pkPluginMgr->m_vpkPlugins[i]->m_vpkEntities[j]->GetInstanceName() ) {

                    pkEntity = pkPluginMgr->m_vpkPlugins[i]->m_vpkEntities[j];
                    uiInstCounter++;
                }
            }
        }

        if ( uiInstCounter > 1 ) {

            NeoEngine::neolog << NeoEngine::LogLevel( NeoEngine::WARNING ) << " searching for an ununiqe entity instance name '" << strInstanceName.c_str() << " '" << std::endl;
            return NULL;

        } 

        return pkEntity;

    }

    return NULL;
}

} // namespace CTD
