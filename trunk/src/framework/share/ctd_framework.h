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
 # neoengine, framework class
 #
 # all relevant engine resources can be accessed through this framework
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
 #  09/24/2004 boto       creation of Framework
 #
 ################################################################*/


#ifndef _CTD_FRAMEWORK_H_
#define _CTD_FRAMEWORK_H_

#include <string>

#include <ctd_frbase.h>
#include <ctd_baseentity.h>
#include <ctd_descriptor.h>
#include <ctd_plugin_defs.h>
#include <ctd_levelmanager.h>

namespace CTD
{

// simulation modes
typedef enum { stateSTANDALONE, stateSERVER, stateCLIENT } GameMode;

class FrameworkImpl;
class FrameworkImplClient;
class FrameworkImplServer;
class FrameworkImplStandalone;
template < class ImplementationT > class FrameworkBuilder;
class NetworkDevice;
class Settings;


//! Exception class for game framework
/**
* All kinds of framework exceptions make use of this class to describe their reason.
*/
class FrameworkException
{

    public:
    
        /**
        * Contructor
        * \param strMsg                         String describing the reason for exeption.
        */
                                                FrameworkException( const std::string& strMsg ) 
                                                { 
                                                    m_strExceptionMsg =std:: string("*** Framework exception: " ) + strMsg; 
                                                }

        /**
        * Get the reason for exception.
        * \return                               String describing the reason for exeption.
        */
        const std::string&                      GetMsg() { return m_strExceptionMsg; }

    private:

        //! String describing the reason for exeption
        std::string                             m_strExceptionMsg;

};


//! Framework class used by plugins.
/**
* This framework interface is used by plugins to access the most important framework services and engine configurations.
*/
class Framework
{

    public: 

        /**
        * Load a given level file
        * \return                               Instance of Level set where the level has been loaded to.
        */
        LevelSet*                               LoadLevel( const std::string &strLevelFile );

        /**
        * This function starts the game.
        * Call this function after initialization.
        */
        void                                    StartGame();


        /**
        * Shutdown simulation.
        */
        void                                    ExitGame();

        /**
        * Return the simulation mode ( standalone, server, client )
        * \return                               Game mode: CTD::stateSERVER, CTD::stateCLIENT, or CTD::stateSTANDALONE
        */
        GameMode                                GetGameMode();

        /**
        * Get absolut program execution root path.
        * \return                               Absolute root path 
        */
        const std::string&                      GetRootPath();

        /**
        * Returns the game settings instance
        * \return                               Game settings instance
        */
        Settings*                               GetGameSettings();

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
        CTD::BaseEntity*                        FindEntity( const std::string &strInstanceName, const std::string &strPluginName = std::string() );     

        /**
        * Send a message to a given plugin.
        * \param    iMsgId                      Message ID
        * \param    pMsgStruct                  Message specific data
        * \param    strPluginName               Plugin name, let it empty to send the message to all plugins.
        */
        void                                    SendPluginMessage( int iMsgId, void *pMsgStruct, const std::string &strPluginName = "" );

        /**
        * Send a message to all entities of given plugins. 
        * \param    iMsgId                      Message ID
        * \param    pMsgStruct                  Message specific data
        * \param    strPluginName               Plugin name, let it empty to send the message to all plugins' entities.
        */
        void                                    SendEntityMessage( int iMsgId, void *pMsgStruct, const std::string &strPluginName = "" );

        /**
        * Get render device
        * \return                               Render device
        */
        NeoEngine::RenderDevice*                GetRenderDevice();
        
        /**
        * Get name of render device
        * \return                               Name of Render device: opengl or d3d9
        */
        const std::string&                      GetRenderDeviceString();
        
        /**
        * Get render caps
        * \return                               Render caps
        */
        NeoEngine::RenderCaps&                  GetRenderCaps();
        
        /**
        * Usage of stencil buffer
        * \return                               Return true if stencil buffering is used
        */
        bool                                    UseStencilBuffer();

        /**
        * Get audio device
        * \return                               Audio device
        */
        NeoEngine::AudioDevice*                 GetAudioDevice();

        /**
        * Get audio device name
        * \return                               Name of audio device
        */
        const std::string&                      GetAudioDeviceString();

        /**
        * Set room. It is possible to handle with more than one room in an application.
        * \param pkRoom                         Set a NeoEngine room instance which is handled in game loop.
        */
        void                                    SetRoom( NeoEngine::Room* pkRoom );

        /**
        * Get level manager
        * \return                               Level manager
        */
        LevelManager*                           GetLevelManager();

        /**
        * Switch to given level set. In game loop the new level set will be considered for rendering
        * \param pkLevelSet                     Level set which should be switched to
        */
        void                                    SwitchToLevelSet( LevelSet* pkLevelSet );

        /**
        * Get current level set. After loading a new level the new created level set can be later retrieved by this method.
        * The method LoadLevel also returns the curren level set.
        * \return                               Current level set
        */
        LevelSet*                               GetCurrentLevelSet();

        /** 
        * Set background color. The screen will be cleared with this color in every frame.
        */
        void                                    SetBackgroundColor( NeoEngine::Color kColor );

        /** 
        * Get background color. The screen will be cleared with this color in every frame.
        * \return                               Background color
        */
        NeoEngine::Color                        GetBackgroundColor();
          
        /**
        * Initiate networking for client mode
        * \param iClientPort                    Client port
        * \param iServerPort                    Server port
        * \param strServerIP                    Server IP
        * \param strNodeName                    Network node name
        * \param strLevelName                   If networking has been started successful then this string contains the level name to be started
        * \return                               Returns true if networking setup was successfull.
        */
        bool                                    InitiateClientNetworking( int iClientPort, int iServerPort, const std::string& strServerIP, const std::string& strNodeName, std::string& strLevelName );

        /**
        * Start networking for client mode. Call this method after a successful initiation of client networking by calling 'InitiateClientNetworking'.
        * \return                               false if an error occures
        */
        bool                                    StartClientNetworking();

        /**
        * Shutdown networking for client mode.
        */
        void                                    ShutdownClientNetworking();

        /**
        * Get networking device
        * \return                               Networking device 
        */
        NetworkDevice*                          GetNetworkDevice();

        /**
        * Shutdown the network device in emergent cases.
        * This function can be called in application when an exception occurs and the application cannot shutdown in normal way.
        */
        void                                    EmergencyShutdownNetworkDevice();

        /**
        * Register an entity for pre-rendering callbacks. The entity function PreRender will be called in this rendering phase.
        */
        void                                    RegisterPreRenderEntity( CTD::BaseEntity *pkEntity );

        /**
        * Register an entity for 2D rendering callbacks. The entity function Render2D will be called in 2D rendering phase.
        */
        void                                    Register2DRendering( BaseEntity *pkEntity );

        /**
        * Get current fps.
        * \return  FPS
        */
        const float&                            GetFPS();

        /**
        * Get the client player, it is the responsibility of client to set a valid scene node as player!
        */
        BaseEntity*                             GetPlayer();

        /**
        * Set the client player, it is the responsibility of client to set a valid scene node as player!
        */
        void                                    SetPlayer( BaseEntity *pkPlayer );

        /**
        * Get a singlton instance of framework created by FrameworkBuilder
        */
        static Framework*                       Get()
                                                {
                                                    if ( !s_pkSingletonFramework ) {
                                                        throw( FrameworkException( "Framework builder has not been finalized!" ) );
                                                    }
                                                    return s_pkSingletonFramework; 
                                                }

    protected:

        //! Only FrameworkBuilder is allowed to create and destroy an object of this class 

                                                Framework( FrameworkImpl *pkFrameworkImpl );    
                                                
                                                ~Framework();

                                                Framework( Framework& );

        Framework&                              operator = ( Framework& );

        //! Singlton instance of framework
        static Framework                        *s_pkSingletonFramework;

    private:

        FrameworkImpl                           *m_pkImpl;


    friend class FrameworkBuilder< FrameworkImplClient >;       
    friend class FrameworkBuilder< FrameworkImplServer >;       
    friend class FrameworkBuilder< FrameworkImplStandalone >;
    friend class LevelLoader;

};


} // namespace CTD

#endif // _CTD_FRAMEWORK_H_

