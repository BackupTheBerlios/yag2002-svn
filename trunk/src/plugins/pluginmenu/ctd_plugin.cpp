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
 # neoengine, menu plugin
 #
 # this plugin provides all necessary entities for a 3d menu
 #  be aware that his special plugin is very tightly integrated into
 #  the framework; this is not a typical game code plugin!
 #
 #
 #   date of creation:  05/13/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include "base.h"
#include <ctd_link_menu.h>
#include <ctd_menu_defs.h>

#include <ctd_printf.h>
#include "ctd_menucontrol.h"
#include "ctd_menucam.h"
#include "ctd_menugroup.h"
#include "ctd_itemmarker.h"
#include "visuals/ctd_menuskybox.h"
#include "menuitems/ctd_outputitem.h"
#include "menuitems/ctd_startlevelitem.h"
#include "menuitems/ctd_exitgameitem.h"
#include "menuitems/ctd_inputitem.h"
#include "menuitems/ctd_changegroupitem.h"
#include "3dfonts/ctd_fontmgr.h"

#include <string>

using namespace std;
using namespace CTD;
using namespace NeoEngine;

namespace CTD_IPluginMenu
{

ModuleStatic        *g_pkStaticModule       = NULL;

// functions for getting and setting menu and game level sets
//---------------------------------------------------------//
LevelSet            *g_pkMenuLevelSet       = NULL;
LevelSet            *g_pkGameLevelSet       = NULL;

LevelSet* GetMenuLevelSet() 
{ 
    return g_pkMenuLevelSet; 
}

void  SetMenuLevelSet( LevelSet* pkLevelSet )
{
    g_pkMenuLevelSet = pkLevelSet;
}

CTD::LevelSet*     GetGameLevelSet()
{
    return g_pkGameLevelSet;
}

void SetGameLevelSet( LevelSet* pkLevelSet )
{
    g_pkGameLevelSet =  pkLevelSet;
}
//---------------------------------------------------------//


//------------------------------------------------------//
// this is the global instance ctdprintf object
CTDPrintf           g_CTDPrintf;
// this is a buffered ctdprintf object
CTDPrintf           g_CTDPrintfBuffered;
//-----------------------------------------------------//



// plugin interface function for retrieving plugin version
void GetVersion( std::string *pstrName, int *piMajor, int *piMinor, int *piRevision )
{
    *pstrName   = CTD_PLUGINNAME_MENU;
    *piMajor    = CTD_PLUGIN_MENU_VERSION_MAJ;
    *piMinor    = CTD_PLUGIN_MENU_VERSION_MIN;
    *piRevision = CTD_PLUGIN_MENU_VERSION_REVISION;
}

// plugin interface function for initialization
int Initialize()
{
    return 0;
}

// plugin interface function for shutdown
int Shutdown()
{

    return 0;
}

// plugin interface function for plugin global messaging
int Message( int iMsgId, void *pMsgStruct )
{

    switch ( iMsgId ) {

        // set engine interface
        case CTD_PLUGIN_MSG_INIZIALIZE:
        {
            // store menu room
            g_pkMenuLevelSet = Framework::Get()->GetCurrentLevelSet();

            // create menu camera
            Camera *pkCamera = new Camera( "_3dmenuCam" );
            pkCamera->SetRoom( g_pkMenuLevelSet->GetRoom() );

            g_pkMenuLevelSet->SetCamera( pkCamera );

	        //Set default view position
	        pkCamera->SetTranslation( Vector3d( 0.0f, 0.0f, 0.0f ) );

            // init ctd printf device
            CTDPRINTF_INIT( g_CTDPrintf, 50, 10, false );
            CTDPRINTF_INIT( g_CTDPrintfBuffered, 50, 50, true );
    
        }
        break;

        case CTD_PLUGIN_MSG_POSTINITIALIZE:
        {
            // notify all entities about entering the menu
            Framework::Get()->SendEntityMessage( CTD_ENTITY_ENTER_MENU, ( void* )NULL );
        }
        break;

        // 3d rendering phase
        case CTD_PLUGIN_MSG_RENDER_3D:

            break;

        // 2d rendering phase ( e.g. for huds )
        case CTD_PLUGIN_MSG_RENDER_2D:

            // print out ctd printfs
            CTDPRINTF_UPDATE( g_CTDPrintf );
            CTDPRINTF_UPDATE( g_CTDPrintfBuffered );

            break;

        // shutdown the plugin
        case CTD_PLUGIN_MSG_SHUTDOWN:

            break;

        default:

            ;
    }

    return 0;
}

// plugin interface function for retrieving count of entities in this plugin
int NumEntities()
{

    return 11;

}

// plugin interface function for retrieving entity descriptors in this plugin
EntityDescriptor* GetEntityDescriptor( int iNum )
{

    EntityDescriptor    *kDesc;

    switch(iNum) 
    {

    case    0:
        kDesc = &g_pkCTDMenuCameraEntity_desc;
        break;

    case    1:
        kDesc = &g_pkCTDMenuControlEntity_desc;
        break;

    case    2:
        kDesc = &g_pkCTDMenuOutputItemEntity_desc;
        break;

    case    3:
        kDesc = &g_pkCTDMenuInputItemEntity_desc;
        break;

    case    4:
        kDesc = &g_pkCTDMenuGroupEntity_desc;
        break;

    case    5:
        kDesc = &g_pkCTD3DFontMgrEntity_desc;
        break;

    case    6:
        kDesc = &g_pkCTDItemMarkerEntity_desc;
        break;

    case    7:
        kDesc = &g_pkCTDMenuChangeGroupItemEntity_desc;
        break;

    case    8:
        kDesc = &g_pkCTDMenuSkyBoxEntity_desc;

        break;
    case    9:
        kDesc = &g_pkCTDMenuStartLevelItemEntity_desc;

        break;
    case    10:
        kDesc = &g_pkCTDMenuExitGameItemEntity_desc;

        break;

    default:
        kDesc= NULL;
    }

    return kDesc;
}


// plugin interface function for global plugin's parameter descriptions
//  for pkDesc = NULL you must return the count of parameters!
int ParameterDescription( int iNum, ParameterDescriptor* pkDesc )
{

    int iparam_count = 0;

    if (pkDesc == NULL) {

        return iparam_count;
    }

    return iparam_count;

}

// static linking of this plugin
Link::Link()
{

    g_pkStaticModule = new ModuleStatic( "ctd_menu" );

    g_pkStaticModule->m_Symbols.Insert( "Initialize",           (void**)Initialize );
    g_pkStaticModule->m_Symbols.Insert( "Shutdown",             (void**)Shutdown   );
    g_pkStaticModule->m_Symbols.Insert( "GetVersion",           (void**)GetVersion );
    g_pkStaticModule->m_Symbols.Insert( "Message",              (void**)Message );
    g_pkStaticModule->m_Symbols.Insert( "NumEntities",          (void**)NumEntities );
    g_pkStaticModule->m_Symbols.Insert( "GetEntityDescriptor",  (void**)GetEntityDescriptor );
    g_pkStaticModule->m_Symbols.Insert( "ParameterDescription", (void**)ParameterDescription );

    NeoEngine::Core::GetModuleManager()->InsertModule( g_pkStaticModule );
}

} // namespace CTD_IPluginMenu
