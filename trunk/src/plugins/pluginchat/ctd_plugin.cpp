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
 # neoengine, chat plugin's global interface
 #
 # this interface also provides info about containing entities
 #
 #
 #   date of creation:  08/13/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>
#include "ctd_link_chat.h"

#include "ctd_setup.h"
#include "ctd_member.h"
#include "ctd_gui.h"

#include <string>

using namespace std;
using namespace CTD;
using namespace NeoEngine;


namespace CTD_IPluginChat {

ModuleStatic		*g_pkStaticModule		= NULL;

//  interface function for retrieving plugin version
void GetVersion( std::string *pstrName, int *piMajor, int *piMinor, int *piRevision )
{
	*pstrName   = CTD_PLUGINNAME_CHAT;
	*piMajor    = CTD_PLUGIN_CHAT_VERSION_MAJ;
	*piMinor    = CTD_PLUGIN_CHAT_VERSION_MIN;
	*piRevision = CTD_PLUGIN_CHAT_VERSION_REVISION;
}

//  interface function for initialization
int Initialize()
{
	return 0;
}

//  interface function for shutdown
int Shutdown()
{

	delete g_pkInputListener;
	delete g_pkInputGroup;

	return 0;

}

//  interface function for plugin global messaging
int Message( int iMsgId, void *pMsgStruct )
{

	switch ( iMsgId ) {

		// set engine interface
		case CTD_PLUGIN_MSG_INIZIALIZE:
		{

            // store the current levelset
            SetLevelSet( Framework::Get()->GetCurrentLevelSet() );

            if ( Framework::Get()->GetGameMode() != stateSERVER ) {

			    // setup fog
			    //FogMode kFogmode;
			    //kFogmode.m_iMode		= FogMode::LINEAR;
			    //kFogmode.m_fDensity		= 0.010f;
			    //kFogmode.m_fStart		= g_fNearplane;
			    //kFogmode.m_fEnd			= g_fFarplane / 3.0f;
			    //kFogmode.m_iHint		= FogMode::PERPIXEL;
			    //kFogmode.m_kColor		= Color( 0.8f, 0.8f, 0.8f );
			    //Framework::Get()->GetRenderDevice()->SetFog( kFogmode );
			    Framework::Get()->SetBackgroundColor( Color( 12.0f / 255.0f, 12.0f / 255.0f, 85.0f / 255.0f, 1.0f ) );
			    // create and setup a camera 
			    CreateCamera();
			    // setup input listener
			    CreateInputListener();

            }

            // init ctd printf device
			CTDPRINTF_INIT( g_CTDPrintf, 10, 10, false );
			CTDPRINTF_INIT( g_CTDPrintfNwStats, 500, 10, false );

		}		
		break;

		// 3d rendering phase
		case CTD_PLUGIN_MSG_RENDER_3D:

			break;

		// 2d rendering phase ( e.g. for huds )
		case CTD_PLUGIN_MSG_RENDER_2D:

			//Framework::Get()->GetCore()->GetProfileManager()->Render();
	
			// print out ctd printfs
			CTDPRINTF_UPDATE( g_CTDPrintf );
			CTDPRINTF_UPDATE( g_CTDPrintfNwStats );

			break;

		default:

			;
	}

	return 0;
}

//  interface function for retrieving count of entities in this plugin
int NumEntities()
{

	return 2;

}

//  interface function for retrieving entity descriptors in this plugin
EntityDescriptor* GetEntityDescriptor( int iNum )
{

	EntityDescriptor	*ed;

	switch(iNum) 
	{

	case	0:
		ed = &g_pkChatMemberEntity_desc;
		break;

	case	1:
		ed = &g_pkGuiEntity_desc;
		break;

	default:
		ed = NULL;
	}

	return ed;
}


//  interface function for global plugin's parameter descriptions
//  for pkDesc = NULL you must return the count of parameters!
int ParameterDescription( int iNum, ParameterDescriptor* pkDesc )
{

	int iparam_count = 3;

	if (pkDesc == NULL) {

		return iparam_count;
	}

	switch(iNum) 
	{
	case 0:
		pkDesc->SetName( "FOV angle" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_FLOAT );
		pkDesc->SetVar( &g_fFOVAngle );
		break;

	case 1:
		pkDesc->SetName( "Nearplane" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_FLOAT );
		pkDesc->SetVar( &g_fNearplane );
		break;

	case 2:
		pkDesc->SetName( "Farplane" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_FLOAT );
		pkDesc->SetVar( &g_fFarplane );
		break;

	default:
		return -1;
	}

	return iparam_count;

}

// static linking of this plugin
Link::Link()
{

	g_pkStaticModule = new ModuleStatic( "ctd_chat" );

	g_pkStaticModule->m_Symbols.Insert( "Initialize",           (void**)Initialize );
	g_pkStaticModule->m_Symbols.Insert( "Shutdown",             (void**)Shutdown   );
	g_pkStaticModule->m_Symbols.Insert( "GetVersion",           (void**)GetVersion );
	g_pkStaticModule->m_Symbols.Insert( "Message",              (void**)Message );
	g_pkStaticModule->m_Symbols.Insert( "NumEntities",          (void**)NumEntities );
	g_pkStaticModule->m_Symbols.Insert( "GetEntityDescriptor",  (void**)GetEntityDescriptor );
	g_pkStaticModule->m_Symbols.Insert( "ParameterDescription", (void**)ParameterDescription );

	NeoEngine::Core::GetModuleManager()->InsertModule( g_pkStaticModule );
}

} // namespace CTD_IPluginChat
