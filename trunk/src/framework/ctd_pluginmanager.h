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
 # neoengine, module loader
 #
 # this class implements loading and managing custom modules (game plugins)
 #
 #
 #   date of creation:  12/03/2003
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  12/03/2003 boto       creation of CLevelLoader
 #
 #  12/05/2003 boto       implementation of plugin and entity initialization
 #
 #  02/16/2004 boto       bug-fix: loading and releasing plugins given lib name
 #                          instead of internal plugin name
 #
 #  04/14/2004 boto       added functions to find an entity descriptor given an entity name
 #
 #  13/05/2004 boto       - bug-fix: plugin path is not longer set in function LoadPlugin,
 #                           instead engine's module manager ist set up with all possible plugin paths
 #                           in ctd_frameworkcore.cpp
 #
 #                        - added sending message to entities in addition to plugins
 #
 #  09/24/2004 boto       redesigned
 #
 ################################################################*/

#ifndef _CTD_PLUGIN_MANAGER_H_
#define _CTD_PLUGIN_MANAGER_H_

#include <ctd_frbase.h>
#include <ctd_descriptor.h>
#include <ctd_baseentity.h>
#include <ctd_plugin_defs.h>


#include <string>
#include <vector>

namespace CTD 
{

// plugin interface function names and parameters / return values
//------------------------------------------//
#define CTD_PLUGIN_API_FC_GetVersion                "GetVersion"
#define CTD_PLUGIN_API_FC_CAST_GetVersion           ( void (__cdecl *)( std::string *,int *,int *,int * ) )

#define CTD_PLUGIN_API_FC_Initialize                "Initialize"
#define CTD_PLUGIN_API_FC_CAST_Initialize           ( int (__cdecl *)( ) )

#define CTD_PLUGIN_API_FC_Shutdown                  "Shutdown"
#define CTD_PLUGIN_API_FC_CAST_Shutdown             ( int (__cdecl *)( ) )

#define CTD_PLUGIN_API_FC_Message                   "Message"
#define CTD_PLUGIN_API_FC_CAST_Message              ( int (__cdecl *)( int, void * ) )

#define CTD_PLUGIN_API_FC_NumEntities               "NumEntities"
#define CTD_PLUGIN_API_FC_CAST_NumEntities          ( int (__cdecl *)( ) )

#define CTD_PLUGIN_API_FC_GetEntityDescriptor       "GetEntityDescriptor"
#define CTD_PLUGIN_API_FC_CAST_GetEntityDescriptor  ( EntityDescriptor* (__cdecl *)( int ) )

#define CTD_PLUGIN_API_FC_ParameterDescription      "ParameterDescription"
#define CTD_PLUGIN_API_FC_CAST_ParameterDescription ( int (__cdecl*)( int, ParameterDescriptor* ) )
//------------------------------------------//

// plugin class containing info and interfaces to a loaded plugin
class Plugin
{

    friend  class PluginManager;
    friend  class FrameworkImpl;


    public:

        /**
        * Get the plugin name.
        * \return Plugin name
        */
        std::string                     GetName() { return m_strPluginName; }

        /**
        * Get the library name of plugin.
        * \return Library name ( .dll or .so name )
        */
        std::string                     GetLibName() { return m_strLibName; }


        /**
        * Get the entity descriptor given its index.
        * \return CTDEntityDescriptor if one exsits, otherwise NULL
        */
        EntityDescriptor*               GetEntityDescriptor( unsigned int uiDescIndex );

        /**
        * Get the entity descriptor given its entity name.
        * \return CTDEntityDescriptor if one exsits, otherwise NULL
        */
        EntityDescriptor*               GetEntityDescriptor( const std::string &strEntityName );

        /**
        * Set for a given parameter its value. if 'pkEntity' is NULL then plugin's global parameters are set, otherwise the given plugin's parameters are set.
        * \return true if succesfulle, otherwise false
        */
        bool                            SetParameter( const std::string &strName, const std::string &strType, const std::string &strValue, BaseEntity *pkEntity = NULL );

        /**
        * Function pointer for interface function GetVersion ( this function is called by engine's module manager! )
        */

         void                           ( * Fp_GetVersion ) ( std::string *pstrName, int *piMajor, int *piMinor, int *piRevision );

        /**
        * Function pointer for interface function Initialize ( this function is called by engine's module manager! )
        */
         int                            ( * Fp_Initialize ) ( );

        /**
        * Function pointer for interface function Shutdown ( this function is called by engine's module manager! )
        */
         int                            ( * Fp_Shutdown ) ( );

        /**
        * Function pointer for interface function Message
        */
         int                            ( * Fp_Message ) ( int msg_id, void *msg_struct );

        /**
        * Function pointer for interface function NumEntities
        */
         int                            ( * Fp_NumEntities ) ( );

        /**
        * Function pointer for interface function GetEntityDescriptor
        */
         EntityDescriptor*              ( * Fp_GetEntityDescriptor ) ( int num );

        /**
        * Function pointer for interface function ParameterDescription
        */
         int                            ( * Fp_ParameterDescription ) ( int num, ParameterDescriptor* pd );

    protected:

        /**
        * Construction is only allowed to plugin manager.
        */
                                        Plugin();
        /**
        * Avoid the usage of copy constructor.
        */
                                        Plugin( Plugin& );

        /**
        * Avoid the assignment operator.
        */
        Plugin&                         operator = ( Plugin& );

        /**
        * Destruction is only allowed to plugin manager.
        */
                                        ~Plugin();

        /**
        * Plugin name
        */
        std::string                     m_strPluginName;

        /**
        * Library name
        */
        std::string                     m_strLibName;

        NeoEngine::ModulePtr            m_pkModule;

        /**
        * Version info
        */
        int                             m_iVersionMaj;
        int                             m_iVersionMin;
        int                             m_iVersionRevision;

        /**
        * Count of entities in plugin
        */
        unsigned int                    m_uiNumEntities;

        /**
        * Count of plugin's global parameters
        */
        unsigned int                    m_uiNumParameters;

        /**
        * This flag shows wether the plugin should get messages.
        */
        bool                            m_bReceiveMessages;

        /**
        * List of plugin entities
        */
        std::vector< BaseEntity * >     m_vpkEntities;

};


// class for plugin management
class PluginManager
{

    public:
                                        PluginManager();

                                        ~PluginManager();

        /**
        * Load a Plugin given its name.
        * \return Returns the plugin pointer, NULL if it could not be loaded.
        */
        Plugin*                         LoadPlugin( const std::string &strLibName );

        /**
        * Release a Plugin given its name. All entities are deleted.
        * \return  true if release was succesfull.
        */
        bool                            ReleasePlugin( const std::string &strLibName );

        /**
        * Release all Plugins.
        */
        void                            ReleaseAllPlugins();

        /**
        * Bind a plugin interface function such as GetVersion, Initialize, etc.
        * \return Returns the module pointer, falser if not all functions could be bounded.
        */
        bool                            BindFunction( Plugin *pkPlugin, const std::string &strFunctionName );

        /**
        * Add an entity into entity list of given plugin
        */
        void                            AddEntity( const Plugin *pkPlugin, const BaseEntity *pkEntity ) 
                                        { 
                                            assert ( pkPlugin );
                                            ( ( Plugin *)pkPlugin )->m_vpkEntities.push_back( ( BaseEntity* )pkEntity ); 
                                        }
        
        /**
        * Send a message to a given plugin.
        * \param    iMsgId              Message ID
        * \param    pMsgStruct          Message specific data
        * \param    strPluginName       Plugin name, let it empty to send the message to all plugins.
        */
        void                            SendPluginMessage( int iMsgId, void *pMsgStruct, const std::string &strPluginName = "" );

        /**
        * Send a message to all entities of given plugins. 
        * If bNetworkMessage is true then the entity function 'NetworkMessage' is called, otherwise 'Message' is called.
        * \param    iMsgId              Message ID
        * \param    pMsgStruct          Message specific data
        * \param    strPluginName       Plugin name, let it empty to send the message to all plugins' entities.
        * \param    bNetworkMessage     If true then the NetworkMessage method of entities is called.
        */
        void                            SendEntityMessage( int iMsgId, void *pMsgStruct, const std::string &strPluginName = "", bool bNetworkMessage = false );

        /**
        * Initialize plugins' entities.
        */
        void                            InitializeEntities();

        /**
        * Post-initialize plugins' entities.
        */
        void                            PostInitializeEntities();

        /**
        * Find an entity descriptor given its name and plugin name.
        * \praram   strEntiyName        Enity name
        * \param    strPluginName       Plugin name, if this is a blank string then all plugins are searched for given instance name.
        * \return                       Pointer to CTDEntityDescriptor object. NULL if entity could not be found.
        */
        EntityDescriptor*               FindEntityDescriptor( const std::string &strEntityName, const std::string &strPluginName = std::string() );

        /**
        * Update all active entities.
        */
        void                            UpdateEntities( float fDeltaTime );

    private:

        std::vector < Plugin* >         m_vpkPlugins;

    friend class FrameworkImpl;

};

} // namespace CTD 

#endif //_CTD_PLUGIN_MANAGER_H_
