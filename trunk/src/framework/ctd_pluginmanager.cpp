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
 # neoengine, plugin manager
 #
 # this class implements loading and managing custom modules (plugins)
 #
 #
 #   date of creation:  12/03/2003
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 #
 ################################################################*/

#include "ctd_frbase.h"
#include "ctd_pluginmanager.h"
#include <ctd_descriptor.h>
#include <ctd_baseentity.h>


#include <string>


using namespace std;
using namespace NeoEngine;
using namespace NeoChunkIO;

namespace CTD
{

// implementation of plugin class
Plugin::Plugin() 
{   

    m_bReceiveMessages          = true;

    m_strPluginName             = "not loaded";
    m_iVersionMaj               = -1;
    m_iVersionMin               = -1;
    m_iVersionRevision          = -1;

    m_uiNumEntities              = 0;
    m_uiNumParameters            = 0;

    m_pkModule                  = NULL;
    Fp_GetVersion               = NULL;
    Fp_Initialize               = NULL;
    Fp_Message                  = NULL;
    Fp_NumEntities              = NULL;
    Fp_GetEntityDescriptor      = NULL;
    Fp_ParameterDescription     = NULL;

};

Plugin::~Plugin()
{
}

// set for a given parameter its value.
bool Plugin::SetParameter( const std::string &strName, const std::string &strType, const std::string &strValue, BaseEntity *pkEntity )
{// we assume that this function is called only by plugin manager and the plugin is successfully loaded!


    static ParameterDescriptor      kParamDesc;
    ParameterDescriptor::m_ePdType  eParamType = ParameterDescriptor::CTD_PD_UNKNOWN;
    Vector3d                kVector;

    // decode the parameter type
    if ( strType == CTD_PD_TYPE_INTEGER ) {

        eParamType = ParameterDescriptor::CTD_PD_INTEGER;

    }
    else    
    if ( strType == CTD_PD_TYPE_FLOAT ) {

        eParamType = ParameterDescriptor::CTD_PD_FLOAT;

    }
    else    
    if ( strType == CTD_PD_TYPE_BOOL ) {

        eParamType = ParameterDescriptor::CTD_PD_BOOL;

    }
    else    
    if ( strType == CTD_PD_TYPE_VECTOR3 ) {

        eParamType = ParameterDescriptor::CTD_PD_VECTOR3;

    }
    else    
    if ( strType == CTD_PD_TYPE_STRING ) {

        eParamType = ParameterDescriptor::CTD_PD_STRING;

    }

    else    
    if ( strType == CTD_PD_TYPE_STATIC_MESH ) {

        eParamType = ParameterDescriptor::CTD_PD_STATIC_MESH;

    }

    if ( eParamType == ParameterDescriptor::CTD_PD_UNKNOWN ) {

        neolog << LogLevel( WARNING ) << " CTD warning (plugin manager) unsuported parameter type for ' " << strName.c_str() << " '" << endl; 

        return false;
    }
        

    // find the parameter by name ( names must be unique ) and set its value
    int i, iParamCount;
    // determine the number of parameters
    if ( pkEntity == NULL ) {

        iParamCount = m_uiNumParameters;

    } else {

        iParamCount = pkEntity->ParameterDescription( 0, NULL );

    }
    for ( i = 0; i < iParamCount; i++ ) {

        if ( pkEntity == NULL ) {
        
            Fp_ParameterDescription( i, &kParamDesc );

        } else {

            pkEntity->ParameterDescription( i, &kParamDesc );

        }

        // if paramter name / type found then set its value
        if ( ( kParamDesc.GetName() == strName ) && ( kParamDesc.GetType() == eParamType ) ) {

            if ( eParamType == ParameterDescriptor::CTD_PD_INTEGER ) {
            
                int *piVar = (int*)kParamDesc.GetVar(); 
                *piVar = atoi( strValue.c_str() );
                neolog << LogLevel( INFO ) << "      setting parameter : " << strName << " = " << *piVar << endl; 

            }
            else
            if ( eParamType == ParameterDescriptor::CTD_PD_FLOAT ) {
            
                float   *pfVar = (float*)kParamDesc.GetVar(); 
                *pfVar = (float)atof( strValue.c_str() );
                neolog << LogLevel( INFO ) << "      setting parameter : " << strName << " = " << *pfVar << endl; 
            }
            else
            if ( eParamType == ParameterDescriptor::CTD_PD_BOOL ) {
            
                bool    *pbVar = (bool*)kParamDesc.GetVar();
                // compare case insensitive
                if ( 
                    toupper(strValue[0]) == 'T'  &&
                    toupper(strValue[1]) == 'R' &&
                    toupper(strValue[2]) == 'U' &&
                    toupper(strValue[3]) == 'E'
                    ) {     

                    *pbVar = true;
                    neolog << LogLevel( INFO ) << "      setting parameter : " << strName << " = " << "true" << endl; 

                } else {

                    *pbVar = false;
                    neolog << LogLevel( INFO ) << "      setting parameter : " << strName << " = " << "false" << endl; 

                }
            }
            else
            if ( eParamType == ParameterDescriptor::CTD_PD_VECTOR3 ) {
            
                Vector3d    *pvVar = (Vector3d*)kParamDesc.GetVar();
                if (sscanf(strValue.c_str(), "%f %f %f", &kVector.x, &kVector.y, &kVector.z ) == 3) { 
                
                    *pvVar = kVector;
                    neolog << LogLevel( INFO ) << "      setting parameter : " << strName << " = " << kVector.x << ", " << kVector.y << ", " << kVector.z << endl; 

                } else {

                    neolog << LogLevel( WARNING ) << " CTD warning (plugin manager) vector must have three components " << endl; 
                    return false;

                }
            }
            else
            if ( eParamType == ParameterDescriptor::CTD_PD_STRING ) {
            
                string  *pfVar = ( string* )kParamDesc.GetVar(); 
                *pfVar = string( strValue );
                neolog << LogLevel( INFO ) << "      setting parameter : " << strName << " = '" << pfVar->c_str() << "'" << endl; 

            }
            else
            if ( eParamType == ParameterDescriptor::CTD_PD_STATIC_MESH ) {
            

                Scene *pkScene = new Scene;
                if ( pkScene->Load( strValue ) == false ) {
                    
                    neolog << LogLevel( WARNING ) << " CTD warning (plugin manager) mesh file could not be opened " << endl; 
                
                    delete pkScene;
                    return false;

                }

                MeshEntity *pkMesh = new MeshEntity( pkScene->GetMeshes()[0] );
                pkMesh->GenerateBoundingVolume();
                pkMesh->GetMesh()->SetName( strValue );
                // get the mesh variable and assign the new loaded mesh to it
                MeshEntity **pVar = ( MeshEntity** )kParamDesc.GetVar();
                *pVar = pkMesh;

                neolog << LogLevel( INFO ) << "      setting parameter : " << strName << " = " << pkMesh->GetMesh()->GetName().c_str() << endl; 

                delete pkScene;

            }

            break;

        }
    }

    // check wether we found the parameter in plugin
    if ( i == iParamCount ) {

        neolog << 
            LogLevel( WARNING ) << " CTD warning (plugin manager) could not find parameter '" << 
            strName.c_str() << " '" << " with type ' " << 
            strType.c_str() << " '" << endl; 
        
        return false;

    }

    return true;
}

EntityDescriptor* Plugin::GetEntityDescriptor( unsigned int uiDescIndex )
{

    if ( uiDescIndex > m_uiNumEntities ) {

        return NULL;

    }

    return Fp_GetEntityDescriptor( uiDescIndex );

}


EntityDescriptor* Plugin::GetEntityDescriptor( const string &strEntityName )
{

    if ( strEntityName.length() == 0 ) {

        return NULL;

    }

    EntityDescriptor *pkDesc = NULL;

    for ( unsigned int uiEntityCnt = 0; uiEntityCnt < m_uiNumEntities; uiEntityCnt++ ) {

        pkDesc = GetEntityDescriptor( uiEntityCnt );

        if ( pkDesc->GetEntityName() == strEntityName ) {

            return pkDesc;

        }
    }

    return NULL;
}

//#######################################################################################//

// implementation of plugin manager class
PluginManager::PluginManager()
{

}


PluginManager::~PluginManager()
{


    ModulePtr   pkModule;

    // freeing the entities is the job of room manger as they are attached into room as nodes

    // free up the loaded modules
    for ( unsigned int i = 0; i < m_vpkPlugins.size(); i++ ) {

        pkModule = m_vpkPlugins[i]->m_pkModule;
        NeoEngine::Core::Get()->GetModuleManager()->RemoveModule( pkModule );

    }

    // free up plugins and their entities
    ReleaseAllPlugins();

}

// initialize plugins' entities.
void PluginManager::InitializeEntities()
{

    for ( unsigned int i = 0; i < m_vpkPlugins.size(); i++ ) {

        for ( unsigned int j = 0; j < m_vpkPlugins[i]->m_vpkEntities.size(); j++ ) {

            m_vpkPlugins[i]->m_vpkEntities[j]->Initialize();

        }
    }

}

// post-initialize plugins' entities.
void PluginManager::PostInitializeEntities()
{

    for ( unsigned int i = 0; i < m_vpkPlugins.size(); i++ ) {

        for ( unsigned int j = 0; j < m_vpkPlugins[i]->m_vpkEntities.size(); j++ ) {

            m_vpkPlugins[i]->m_vpkEntities[j]->PostInitialize();

        }
    }

}

// load a module given its name + path
Plugin* PluginManager::LoadPlugin( const string &strLibName )
{

    Plugin  *pkPlugin = NULL;

    // check wether plugin already loaded
    for ( unsigned int uiPluginCnt = 0; uiPluginCnt < m_vpkPlugins.size(); uiPluginCnt++ ) {

        if ( m_vpkPlugins[ uiPluginCnt ]->GetLibName() == strLibName ) {

            return m_vpkPlugins[ uiPluginCnt ];
        }
    }

    // try to load the plugin
    ModulePtr pkModule;
    pkModule = NeoEngine::Core::Get()->GetModuleManager()->LoadModule( strLibName );

    if( !pkModule ) {

            neolog << LogLevel( WARNING ) << "*** CTD unable to localize plugin " << strLibName.c_str() << endl;
            return NULL;

    }

    pkPlugin = new Plugin;
    pkPlugin->m_pkModule = pkModule;
    pkPlugin->m_strLibName = strLibName;

    // bind plugin's interface functions
    BindFunction( pkPlugin, CTD_PLUGIN_API_FC_GetVersion           );
    BindFunction( pkPlugin, CTD_PLUGIN_API_FC_Initialize           );
    BindFunction( pkPlugin, CTD_PLUGIN_API_FC_Shutdown             );
    BindFunction( pkPlugin, CTD_PLUGIN_API_FC_Message              );
    BindFunction( pkPlugin, CTD_PLUGIN_API_FC_NumEntities          );
    BindFunction( pkPlugin, CTD_PLUGIN_API_FC_GetEntityDescriptor  );
    BindFunction( pkPlugin, CTD_PLUGIN_API_FC_ParameterDescription );

    // check wether all interface functions are found
    if ( ( pkPlugin->Fp_GetVersion == NULL )           ||
         ( pkPlugin->Fp_Initialize == NULL )           ||
         ( pkPlugin->Fp_Shutdown == NULL )             ||
         ( pkPlugin->Fp_Message == NULL )              ||
         ( pkPlugin->Fp_NumEntities == NULL )          ||
         ( pkPlugin->Fp_GetEntityDescriptor == NULL )  ||
         ( pkPlugin->Fp_ParameterDescription == NULL ) 
        ) 
    {

        neolog << LogLevel( WARNING ) << "*** CTD  skipping this plugin " << endl;

        delete pkPlugin;

        return NULL;
    }

    // initialize plugin
    //-----------------------------------------------------------------------------------------------//

    // get and store plugin name and version info
    pkPlugin->Fp_GetVersion( &pkPlugin->m_strPluginName, &pkPlugin->m_iVersionMaj, &pkPlugin->m_iVersionMin, &pkPlugin->m_iVersionRevision );
    // get count of entities in plugin
    pkPlugin->m_uiNumEntities = pkPlugin->Fp_NumEntities();
    // get count of plugin's global parameters ( pass NULL for parameter descriptor pointer to get number of params )
    pkPlugin->m_uiNumParameters = pkPlugin->Fp_ParameterDescription( 0, NULL );

    //-----------------------------------------------------------------------------------------------//

    m_vpkPlugins.push_back( pkPlugin );

    neolog << LogLevel( INFO ) << " CTD Plugin Manager: plugin ' " << pkPlugin->GetName().c_str() << " ' ( version " 
        << pkPlugin->m_iVersionMaj << "." << pkPlugin->m_iVersionMin << "." << pkPlugin->m_iVersionRevision << " )"
        << " successfully loaded" << endl;


    return pkPlugin;

}

bool PluginManager::ReleasePlugin( const string &strLibName )
{

    // search for plugin name
    Plugin  *pkPlugin = NULL;
    for ( unsigned int uiPluginCnt = 0; uiPluginCnt < m_vpkPlugins.size(); uiPluginCnt++ ) {

        if ( m_vpkPlugins[ uiPluginCnt ]->GetLibName() == strLibName ) {

            pkPlugin = m_vpkPlugins[ uiPluginCnt ];
            break;
        }
    }

    if ( pkPlugin == NULL ) {

        neolog << LogLevel( WARNING ) << " *** CTD Plugin Manager: request for releasing a non-extisting plugin ' " << strLibName.c_str() << " '" << endl;
        return false;

    }


    // delete all entities
    SceneNode   *pkNode;
    for ( unsigned int uiEntityCnt = 0; uiEntityCnt < pkPlugin->m_vpkEntities.size(); uiEntityCnt++ ) {
    
        pkNode = ( SceneNode* )pkPlugin->m_vpkEntities[ uiEntityCnt ];
        delete pkNode;

    }

    return true;
}

void PluginManager::ReleaseAllPlugins()
{

    for ( unsigned int uiPluginCnt = 0; uiPluginCnt < m_vpkPlugins.size(); uiPluginCnt++ ) {

        ReleasePlugin( m_vpkPlugins[ uiPluginCnt ]->GetLibName() );

    }

}


// bind a plugin intterface function given its name
bool PluginManager::BindFunction( Plugin *pkPlugin, const std::string &strFunctionName )
{

    ModSymbol kSymGetName;

    // bind function GetVersion
    if ( strFunctionName == CTD_PLUGIN_API_FC_GetVersion ) {

        // get symbol
        kSymGetName = pkPlugin->m_pkModule->LookupSymbol( CTD_PLUGIN_API_FC_GetVersion );

        if( !kSymGetName ) {

            goto ErrorBinding;

        } else {

            // get function pointer
            pkPlugin->Fp_GetVersion = CTD_PLUGIN_API_FC_CAST_GetVersion ( kSymGetName );

        }

    } 

    // bind function Initialize
    if ( strFunctionName == CTD_PLUGIN_API_FC_Initialize ) {

        // get symbol
        kSymGetName = pkPlugin->m_pkModule->LookupSymbol( CTD_PLUGIN_API_FC_Initialize );

        if( !kSymGetName ) {

            goto ErrorBinding;

        } else {

            // get function pointer
            pkPlugin->Fp_Initialize = CTD_PLUGIN_API_FC_CAST_Initialize ( kSymGetName );

        }

    } 
    // bind function Initialize
    if ( strFunctionName == CTD_PLUGIN_API_FC_Shutdown ) {

        // get symbol
        kSymGetName = pkPlugin->m_pkModule->LookupSymbol( CTD_PLUGIN_API_FC_Shutdown );

        if( !kSymGetName ) {

            goto ErrorBinding;

        } else {

            // get function pointer
            pkPlugin->Fp_Shutdown = CTD_PLUGIN_API_FC_CAST_Shutdown ( kSymGetName );

        }

    } 

    // bind function Message
    if ( strFunctionName == CTD_PLUGIN_API_FC_Message ) {

        // get symbol
        kSymGetName = pkPlugin->m_pkModule->LookupSymbol( CTD_PLUGIN_API_FC_Message );

        if( !kSymGetName ) {

            goto ErrorBinding;

        } else {

            // get function pointer
            pkPlugin->Fp_Message = CTD_PLUGIN_API_FC_CAST_Message ( kSymGetName );

        }

    } 

    // bind function NumEntities
    if ( strFunctionName == CTD_PLUGIN_API_FC_NumEntities ) {

        // get symbol
        kSymGetName = pkPlugin->m_pkModule->LookupSymbol( CTD_PLUGIN_API_FC_NumEntities );

        if( !kSymGetName ) {

            goto ErrorBinding;

        } else {

            // get function pointer
            pkPlugin->Fp_NumEntities = CTD_PLUGIN_API_FC_CAST_NumEntities ( kSymGetName );

        }

    } 

    // bind function GetEntityDescriptor
    if ( strFunctionName == CTD_PLUGIN_API_FC_GetEntityDescriptor ) {

        // get symbol
        kSymGetName = pkPlugin->m_pkModule->LookupSymbol( CTD_PLUGIN_API_FC_GetEntityDescriptor );

        if( !kSymGetName ) {

            goto ErrorBinding;

        } else {

            // get function pointer
            pkPlugin->Fp_GetEntityDescriptor = CTD_PLUGIN_API_FC_CAST_GetEntityDescriptor ( kSymGetName );

        }

    } 

    // bind function ParameterDescription
    if ( strFunctionName == CTD_PLUGIN_API_FC_ParameterDescription ) {

        // get symbol
        kSymGetName = pkPlugin->m_pkModule->LookupSymbol( CTD_PLUGIN_API_FC_ParameterDescription );

        if( !kSymGetName ) {

            goto ErrorBinding;

        } else {

            // get function pointer
            pkPlugin->Fp_ParameterDescription = CTD_PLUGIN_API_FC_CAST_ParameterDescription ( kSymGetName );

        }

    }

    return true;

ErrorBinding:
    neolog << LogLevel( WARNING ) << "*** CTD could not bind plugin interface function ' " << strFunctionName.c_str() << " '" << endl;
    return false;

}


EntityDescriptor* PluginManager::FindEntityDescriptor( const string &strEntityName, const string &strPluginName )
{

    Plugin              *pkPlugin       = NULL;
    EntityDescriptor    *pkTmpDesc      = NULL;
    EntityDescriptor    *pkDesc         = NULL;
    size_t              uiPlugins       = m_vpkPlugins.size();
    size_t              uiEntities      = 0;
    unsigned int        bEntitiesFound  = 0;

    // go through all loaded plugins
    for ( size_t uiPluginCnt = 0; uiPluginCnt < uiPlugins; uiPluginCnt++ ) {

        pkPlugin   = m_vpkPlugins[ uiPluginCnt ];
        uiEntities = pkPlugin->m_vpkEntities.size();

        // if a plugin name is provided then search only in that plugin, otherwise search in all plugins
        if ( strPluginName.length() > 0 ) {

            if ( pkPlugin->GetName() != strPluginName ) {

                continue;

            }

        }

        // if bEntitiesFound is > 1 then we have non-unique entity names! this cannot be tollerated!
        pkTmpDesc = pkPlugin->GetEntityDescriptor( strEntityName );
        if ( pkTmpDesc != NULL ) {

            pkDesc = pkTmpDesc;
            bEntitiesFound++;

        }

    }

    // entity named ( describing its type ) must be unique!
    if ( bEntitiesFound > 1 ) {

        neolog << LogLevel( WARNING ) << " ***warning( plugin manager ): requesting for descriptor of non-uniqe entity name!" << endl;
        pkDesc = NULL;

    }

    return pkDesc;

}

void PluginManager::UpdateEntities( float fDeltaTime )
{

    Plugin          *pkPlugin = NULL;
    BaseEntity  *pkEntity = NULL;
    size_t              uiPlugins = m_vpkPlugins.size();
    size_t              uiEntities = 0;

    for ( unsigned int i = 0; i < uiPlugins; i++ ) {

        pkPlugin   = m_vpkPlugins[ i ];
        uiEntities = pkPlugin->m_vpkEntities.size();
        for ( unsigned int j = 0; j < uiEntities; j++ ) {

            pkEntity = pkPlugin->m_vpkEntities[ j ];
            if ( pkEntity->IsActive() == true ) {
            
                pkEntity->UpdateEntity( fDeltaTime );

            }

        }

    }

}

void PluginManager::SendPluginMessage( int iMsgId, void *pMsgStruct, const string &strPluginName )
{

    bool bBoradcast = ( strPluginName.length() == 0 );
    bool bNBPluginFound = false; // becomes true in non-broadcast case when the pluging with given name has been found

    std::vector< Plugin* >::iterator    pkPlugin    = m_vpkPlugins.begin();
    std::vector< Plugin* >::iterator    pkPluginEnd = m_vpkPlugins.end();
    while( pkPlugin != pkPluginEnd ) {

        if ( !bBoradcast ) {
            
            if ( ( *pkPlugin )->GetName() != strPluginName ) {

                continue;

            } else {

                bNBPluginFound = true;

            }

        }

        if ( ( *pkPlugin )->m_bReceiveMessages == false ) {

            pkPlugin++;
            continue;

        }

        if ( ( *pkPlugin )->Fp_Message( iMsgId, pMsgStruct ) == CTD_PLUGIN_MSG_NO_NOTIFICATION ) {

            ( *pkPlugin )->m_bReceiveMessages = false;
            pkPlugin++;
            continue;

        }

        // in non-broadcast sending when the plugin has been found and we delivered the message then we are finish
        if ( bNBPluginFound ) {

            break;

        }

        pkPlugin++;

    }

}

// be aware that sending messages to entites does not consider entity's state active/inactive
void PluginManager::SendEntityMessage( int iMsgId, void *pMsgStruct, const string &strPluginName, bool bNetworkMessage )
{

    bool bBoradcast     = ( strPluginName.length() == 0 );
    bool bNBPluginFound = false; // becomes true in non-broadcast case when the pluging with given name has been found

    std::vector< Plugin* >::iterator    pkPlugin    = m_vpkPlugins.begin();
    std::vector< Plugin* >::iterator    pkPluginEnd = m_vpkPlugins.end();
    std::vector< BaseEntity* >::iterator    pkEntiy;
    std::vector< BaseEntity* >::iterator    pkEntiyEnd;
    Plugin      *pkCurrPlugin;

    while ( pkPlugin != pkPluginEnd ) {

        pkCurrPlugin    = *pkPlugin;

        if ( !bBoradcast ) {
            
            if ( ( *pkPlugin )->GetName() != strPluginName ) {

                pkPlugin++;
                continue;

            } else {

                bNBPluginFound = true;

            }

        }

        pkEntiy    = pkCurrPlugin->m_vpkEntities.begin();
        pkEntiyEnd = pkCurrPlugin->m_vpkEntities.end();

        while ( pkEntiy != pkEntiyEnd ) {

            if ( bNetworkMessage == true ) {

                ( *pkEntiy )->NetworkMessage( iMsgId, pMsgStruct );

            } else {

                ( *pkEntiy )->Message( iMsgId, pMsgStruct );

            }

            pkEntiy++;

        }

        // in non-broadcast sending when the plugin has been found and we delivered the message then we are finish
        if ( bNBPluginFound ) {

            break;

        }

        pkPlugin++;

    }

}

} // namespace CTD
