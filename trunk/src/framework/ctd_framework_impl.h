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
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  09/24/2004 boto       creation of FrameworkImpl
 #
 ################################################################*/


#ifndef _CTD_FRAMEWORK_IMPL_H_
#define _CTD_FRAMEWORK_IMPL_H_

#include <string>

#include <ctd_frbase.h>
#include <ctd_framework_builder.h>
#include <ctd_pluginmanager.h>
#include <ctd_levelloader.h>
#include <ctd_keyconfig.h>

namespace CTD
{

#define CTD_APP_TITLE               "VirtualChatRoom VCR 1.0"
#define CTD_SETTINGS_FILE           "settings.cfg"
#define CTD_LOG_FILE_SERVER         "server.log"
#define CTD_LOG_FILE_CLIENT         "client.log"
#define CTD_CONFIG_FILE_NAME        "./engine.cfg"

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


class LevelSet;
class LevelManager;
class Settings;

//! Base class for all implementations of Framework
class FrameworkImpl
{

    protected:  


                                                FrameworkImpl();    

        virtual                                 ~FrameworkImpl();

        /**
        * Game's main loop
        */
        virtual void                            GameLoop() = 0;

        /**
        * Initialize simulation.
        */
        virtual bool                            Initialize( int iArgc = 0, char** ppcArgv = NULL ) = 0;

        /**
        * This function starts the game.
        * Call this function after initialization.
        */
        virtual void                            StartGame() = 0;

        /**
        * Setup networking.
        *  This method must be overwritten by client implementation.
        * \param iClientPort                    Client port
        * \param iServerPort                    Server port
        * \param strServerIP                    Server IP
        * \param strNodeName                    Network node name
        * \return                               Returns true if networking setup was successfull.
        */
        virtual bool                            InitiateClientNetworking( int iClientPort, int iServerPort, const std::string& strServerIP, const std::string& strNodeName ) { return false; }

        /**
        * Start networking for client mode. Call this method after a successful initiation of client networking by calling 'InitiateClientNetworking'.
        *  This method must be overwritten by client implementation.
        * \return                               false if an error occures
        */
        virtual bool                            StartClientNetworking() { return false; }


        /**
        * Shutdown networking for client mode.
        *  This method must be overwritten by client implementation.
        */
        virtual void                            ShutdownClientNetworking() {}

        /**
        * Register an entity for pre-rendering callbacks. The entity function PreRender will be called in this rendering phase.
        */
        virtual void                            RegisterPreRenderEntity( BaseEntity *pkEntity ) = 0;

        /**
        * Register an entity for 2D rendering callbacks. The entity function Render2D will be called in 2D rendering phase.
        */
        virtual void                            Register2DRendering( BaseEntity *pkEntity ) = 0;

        /**
        * Shutdown simulation.
        */
        virtual void                            ExitGame();

        /**
        * Setup render device
        */
        virtual void                            SetupRenderDevice();

        /**
        * Setup sound device
        */
        virtual void                            SetupSoundDevice();

        /**
        * Load engine configuration
        * \param strFileName                    Configuration file name
        */
        void                                    LoadConfig( const std::string& strFileName );

        /**
        * Returns the game settings instance
        * \return                               Game settings instance
        */
        Settings*                               GetGameSettings();

        /**
        * Load a given level file
        * \param strLevelFile                   Level file name
        * \param bInitializePlugins             If true all loaded plugins are initialized.
        * \param pkLevelSet                     LevelSet to load to. If NULL then a new LevelSet is created.
        * \return                               Returns the level set created during loading.
        */
        LevelSet*                               LoadLevel( const std::string &strLevelFile, bool bInitializePlugins = true, LevelSet* pkLevelSet = NULL );

        /**
        * Send a message to a given plugin.
        * \param iMsgId                         Message ID
        * \param pMsgStruct                     Message specific data
        * \param strPluginName                  Plugin name, let it empty to send the message to all plugins in all level sets.
        */
        void                                    SendPluginMessage( int iMsgId, void *pMsgStruct, const std::string &strPluginName = "" );

        /**
        * Send a broad-cast message to all plugin entities
        * \param iMagId                         Message ID
        * \param pMsgStruct                     Message specific data
        * \param strPluginName                  Plugin name, let it empty to send the message to all level sets' plugins' entities.
        * \param bNetworkMessage                If true then the NetworkMessage method of entities is called.
        */
        void                                    SendEntityMessage( int iMsgId, void* pMsgStruct, const std::string &strPluginName = "" );


        /**
        * Get the current level set
        * \return                               Current LevelSet
        */
        LevelSet*                               GetCurrentLevelSet()    { return m_pkCurrentLevelSet; }

        /**
        * Return the simulation mode ( standalone, server, client )
        */
        GameMode                                GetGameMode()           { return m_eGameMode; }

        /**
        * Get absolut root path.
        * \return                               Absolute root path 
        */
        const std::string&                      GetRootPath()           { return m_strRootPath; }

        /**
        * Get a key code given its name
        * \return                               Returns false if the key code does not exitst or key configuration does not exit
        */
        bool                                    GetKeyCode( const std::string &strKeyName , int &iKeyCode );                                                

        /**
        * Find an entity given its instance name.
        * \param     strInstanceName            Instance name
        * \param     strPluginName              Plugin name, if this is a blank string then all plugins are searched for given instance name.
        * \return                               Pointer to BaseEntity object. NULL if entity could not be found.
        */
        BaseEntity*                             FindEntity( const std::string &strInstanceName, const std::string &strPluginName = std::string() );     

        /**
        * Add a new entity
        * \param pkLevelSet                     LevelSet the entity is added to
        * \param pkEntity                       Entity to be added
        * \param strPluginName                  Name of plugin to add the entity to. if empty then the entity is added to first plugin.
        * \return                               true if successfull added, false if this entity already exists in level set
        */
        bool                                    AddEntiy( LevelSet* pkLevelSet, CTD::BaseEntity* pkEntity, const string &strPluginName = "" );

        /**
        * Remove an entity from levelsets, rooms and eventually delete it
        * \param pkEntity                       Entity to be removed, all levelsets are searched and updated
        * \return                               Number of entities removed from all levelsets
        */
        unsigned int                            RemoveEntiy( CTD::BaseEntity* pkEntity );

        /**
        * Find an entity in given plugin manager
        * \param     pkPluginMgr                Plugin manager to be queried 
        * \param     strInstanceName            Instance name
        * \param     strPluginName              Plugin name, if this is a blank string then all plugins are searched for given instance name.
        * \return                               Pointer to BaseEntity object. NULL if entity could not be found.
        */
        BaseEntity*                             SearchEntity( PluginManager *pkPluginMgr, const std::string &strInstanceName, const std::string &strPluginName );

        /**
        * Get render device
        * \return                               Render device
        */
        NeoEngine::RenderDevice*                GetRenderDevice()       { return m_pkRenderDevice; }
        
        /**
        * Get name of render device
        * \return                               Name of Render device: opengl or d3d9
        */
        const std::string&                      GetRenderDeviceString() { return m_strRenderDevice; }
        
        /**
        * Get render caps
        * \return                               Render caps
        */
        NeoEngine::RenderCaps&                  GetRenderCaps()         { return m_kRenderCaps; }
        
        /**
        * Usage of stencil buffer
        * \return                               Return true if stencil buffering is used
        */
        bool                                    UseStencilBuffer()      { return m_bUseStencilBuffer; }

        /**
        * Get audio device
        * \return                               Audio device
        */
        NeoEngine::AudioDevice*                 GetAudioDevice()        { return m_pkAudioDevice; }

        /**
        * Get audio device name
        * \return                               Name of audio device
        */
        const std::string&                      GetAudioDeviceString()  { return m_strAudioDevice; }

        /** 
        * Set background color. The screen will be cleared with this color in every frame.
        */
        void                                    SetBackgroundColor( NeoEngine::Color kColor ) { m_kBackgoundColor = kColor; }

        /** 
        * Get background color. The screen will be cleared with this color in every frame.
        * \return                               Background color
        */
        const NeoEngine::Color&                 GetBackgroundColor()    { return m_kBackgoundColor; }
                                                
        /**
        * Get networking device
        * \return                               Networking device 
        */
        NetworkDevice*                          GetNetworkDevice()      { return m_pkNetworkDevice; }

        /**
        * Shutdown the network device in emergent cases.
        */
        void                                    EmergencyShutdownNetworkDevice();

        /**
        * Get current fps.
        * \return  FPS
        */
        float                                   GetFPS()                { return m_fFPS; }

        /**
        * Get the client player, it is the responsibility of client to set a valid scene node as player!
        */
        BaseEntity*                             GetPlayer()             { return m_pkPlayer; }

        /**
        * Set the client player, it is the responsibility of client to set a valid scene node as player!
        */
        void                                    SetPlayer( BaseEntity *pkPlayer )   { m_pkPlayer = pkPlayer; }

        /**
        * Add a client object during level loading.
        */
        void                                    AddNetworkClientObject( BaseEntity* pkEntity ) 
                                                { 
                                                    m_vpkNetworkClientObjects.push_back( pkEntity ); 
                                                }

        /**
        * Add a server object during level loading.
        */
        void                                    AddNetworkServerObject( BaseEntity* pkEntity ) 
                                                { 
                                                    m_vpkNetworkServerObjects.push_back( pkEntity ); 
                                                }



        //! If false the simulation shuts down
        bool                                    m_bSimulationRun;
        
        //! Game mode ( server, client, standalone )
        GameMode                                m_eGameMode;

        //! Name of game level to start
        std::string                             m_strLevelName;

        //! Log file
        NeoEngine::LogFileSink                  *m_pkLogFile;

        //! This is the unique network id which is given by server
        unsigned short                          m_usNetworkID;

        //! NeoEngine core
        NeoEngine::Core                         *m_pkCore;

        //! Render device
        NeoEngine::RenderDevice                 *m_pkRenderDevice;
        
        //! Name of render device ( e.g. opengl )
        std::string                             m_strRenderDevice;

        //! Screen height
        unsigned int                            m_uiScreenHeight;
        
        //! Screen width
        unsigned int                            m_uiScreenWidth;
        
        //! Color depth
        unsigned int                            m_uiColorDepth;

        //! Full screen flag
        bool                                    m_bFullScreen;

        //! Dynamic shadow depth
        float                                   m_fShadowDepth;

        //! Graphics refresh rate
        unsigned int                            m_uiRefreshRate;

        //! Audio device instance
        NeoEngine::AudioDevice                  *m_pkAudioDevice;

        //! Name of audio device ( dsound or alsa )
        std::string                             m_strAudioDevice;

        //! Level manager instance
        LevelManager                            *m_pkLevelManager;
       
        //! Current level set
        LevelSet                                *m_pkCurrentLevelSet;

        // Scene background color ( also fog color ) 
        NeoEngine::Color                        m_kBackgoundColor;

        //! List of entities registered for prerender-phase
        std::vector< BaseEntity* >              m_vpkPreRenderEntities;

        //! List of entities registered for 2D render-phase
        std::vector< BaseEntity* >              m_vpk2DRenderEntities;

        //! Actual render caps
        NeoEngine::RenderCaps                   m_kRenderCaps;

        //! Indicates the usage of stencil buffer
        bool                                    m_bUseStencilBuffer;

        //! Game settings instance
        Settings                                *m_pkGameSettings;

        //! An instance of key configuration class
        KeyConfig                               *m_pkKeyConfiguration;

        //! Name of key configuration file
        std::string                             m_strKeyConfigFile;

        //! Network device instance
        NetworkDevice                           *m_pkNetworkDevice;

        //! Game timer
        NeoEngine::Timer                        m_kTimer;
        
        //! Frame timer
        NeoEngine::Timer                        m_kFPSTimer;

        //! Current FPS
        float                                   m_fFPS;

        //! Program execution root path
        std::string                             m_strRootPath;
        
        //! Game Packages
        std::string                             m_strPackages;

        //! Client's player instance
        BaseEntity                              *m_pkPlayer;

        //! All network client objects are stored in this lists
        //!  for later initializations and updating
        std::vector< BaseEntity* >              m_vpkNetworkClientObjects;
        
        //! All network server objects are stored in this lists
        //!  for later initializations and updating
        std::vector< BaseEntity* >              m_vpkNetworkServerObjects;


    friend class Framework;
    friend class NetworkDevice;
    friend class NetworkServer;
    friend class NetworkClient;
    friend class LevelLoader;

};

} // namespace CTD

#endif // _CTD_FRAMEWORK_IMPL_H_

