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
 # neoengine, visuals plugin's global interface
 #
 # this interface also provides info about containing entities
 # visuals plugin contains several entities for decorating a map
 #
 #   date of creation:  12/14/2003
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include "base.h"
#include "ctd_link_visuals.h"

#include "ctd_staticmesh.h"
#include "ctd_animatedmesh.h"
#include "ctd_skybox.h"
#include "ctd_sprite.h"
#include "particles/ctd_fountain.h"
#include "ctd_pointlight.h"

#include <string>

using namespace std;
using namespace CTD;
using namespace NeoEngine;

namespace CTD_IPluginVisuals {


ModuleStatic			*g_pkStaticModule		= NULL;


// interface function for retrieving plugin version
void GetVersion( std::string *pstrName, int *piMajor, int *piMinor, int *piRevision )
{
	*pstrName   = CTD_PLUGINNAME_VISUALS;
	*piMajor    = CTD_PLUGIN_VISUALS_VERSION_MAJ;
	*piMinor    = CTD_PLUGIN_VISUALS_VERSION_MIN;
	*piRevision = CTD_PLUGIN_VISUALS_VERSION_REVISION;
}

// interface function for initialization
int Initialize()
{
	return 0;
}

// interface function for shutdown
int Shutdown()
{
	return 0;
}

// interface function for plugin global messaging
int Message( int iMsgId, void *pMsgStruct )
{

	switch ( iMsgId ) {

		// set engine interface
        case CTD_PLUGIN_MSG_INIZIALIZE:
	
			break;

		// 3d rendering phase
		case CTD_PLUGIN_MSG_RENDER_3D:

			break;

		// 2d rendering phase ( e.g. for huds )
		case CTD_PLUGIN_MSG_RENDER_2D:

			break;

		default:

			;
	}

    // this plugin needs not notifications anymore
    return CTD_PLUGIN_MSG_NO_NOTIFICATION;
}

// interface function for retrieving count of entities in this plugin
int NumEntities()
{

	return 6;

}

// interface function for retrieving entity descriptors in this plugin
EntityDescriptor* GetEntityDescriptor( int iNum )
{

	EntityDescriptor	*pkDesc;

	switch(iNum) 
	{

	case	0:
		pkDesc = &g_pkStaticMeshEntity_desc;
		break;
	
	case	1:
		pkDesc = &g_pkAnimatedMeshEntity_desc;
		break;

	case	2:
		pkDesc = &g_pkSkyBoxEntity_desc;
		break;

	case	3:
		pkDesc = &g_pkSpriteEntity_desc;
		break;

	case	4:
		pkDesc = &g_pkParticleFountainEntity_desc;
		break;

	case	5:
		pkDesc = &g_pkPointLightEntity_desc;
		break;

	default:
		pkDesc = NULL;
	}

	return pkDesc;
}


// interface function for global plugin's parameter descriptions
//  for pkDesc = NULL you must return the count of parameters!
int ParameterDescription( int iNum, ParameterDescriptor* pkDesc )
{

	int iParamCount = 0;

	if (pkDesc == NULL) {

		return iParamCount;
	}
	return iParamCount;

}

// static linking of this plugin
Link::Link()
{

	g_pkStaticModule = new ModuleStatic( "ctd_visuals" );

	g_pkStaticModule->m_Symbols.Insert( "Initialize",           (void**)Initialize );
	g_pkStaticModule->m_Symbols.Insert( "Shutdown",             (void**)Shutdown   );
	g_pkStaticModule->m_Symbols.Insert( "GetVersion",           (void**)GetVersion );
	g_pkStaticModule->m_Symbols.Insert( "Message",              (void**)Message );
	g_pkStaticModule->m_Symbols.Insert( "NumEntities",          (void**)NumEntities );
	g_pkStaticModule->m_Symbols.Insert( "GetEntityDescriptor",  (void**)GetEntityDescriptor );
	g_pkStaticModule->m_Symbols.Insert( "ParameterDescription", (void**)ParameterDescription );

	NeoEngine::Core::GetModuleManager()->InsertModule( g_pkStaticModule );
}

}; // namespace CTD_IPluginVisuals
