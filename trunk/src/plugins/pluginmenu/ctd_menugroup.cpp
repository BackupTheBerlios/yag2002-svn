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
 # neoengine, menu group
 #
 # this class implements the a menu group containing items
 #
 #
 #   date of creation:  05/15/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include "base.h"
#include "ctd_menugroup.h"
#include "ctd_menucontrol.h"
#include "menuitems/ctd_menuitem.h"


using namespace std;
using namespace CTD;
//using namespace CTD_IPluginMenu;
using namespace NeoEngine;

namespace CTD_IPluginMenu
{

// plugin global entity descriptor for menu group
CTDMenuGroupDesc	g_pkCTDMenuGroupEntity_desc;
//-------------------------------------------//


CTDMenuGroup::CTDMenuGroup()
{

	CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Menu) entity ' MenuGroup ' created " );

	// set the entity ( node ) name
	SetName( CTD_ENTITY_NAME_MenuGroup );

}

CTDMenuGroup::~CTDMenuGroup()
{

	CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Menu) entity ' MenuGroup ' destroyed " );

}

// post-init entity
void CTDMenuGroup::PostInitialize() 
{ 

	CTDCONSOLE_PRINT( LogLevel( INFO ), string( " (Plugin Menu) entity ' MenuGroup ' post initializing '" + GetInstanceName() + "' ..." ) );

	BaseEntity *pkMenuCtrl = Framework::Get()->FindEntity( CTD_ENTITY_NAME_MenuControl );

	if ( pkMenuCtrl == NULL ) {

		CTDCONSOLE_PRINT( LogLevel( WARNING ), " (Plugin Menu) entity ' MenuGroup ' cannot find menu control entity! entity deactivated." );
		Deactivate();
		return;

	}

	// search and register group's menu items
	vector< string >  vstrItemNames;
	Explode( " ", m_strItemNames, &vstrItemNames );

	// create a new menu group for registering in menu control
	CTDItemGroup	*pkMenuGroup = new CTDItemGroup;
	pkMenuGroup->m_uiGroupID	 = m_uiGroupID;
	pkMenuGroup->m_strGroupName  = m_strGroupName;

	// search all items and add them into menu group
	BaseEntity	*pkMenuItem  = NULL;
	size_t uiItems = vstrItemNames.size();
	size_t uiOrderingNum = 0;
	for ( size_t uiItemCnt = 0; uiItemCnt < uiItems; uiItemCnt++ ) {

		pkMenuItem = Framework::Get()->FindEntity( vstrItemNames[ uiItemCnt ] );
		
		if ( pkMenuItem == NULL ) {

			CTDCONSOLE_PRINT( LogLevel( WARNING ), string( " (Plugin Menu) entity ' MenuGroup ' cannot find menu item '" + vstrItemNames[ uiItemCnt ] + "', skipping item." ) );
			continue;
		}

		// set item's group id
		( ( CTDMenuItem* )pkMenuItem )->SetGroupID( m_uiGroupID );
		( ( CTDMenuItem* )pkMenuItem )->m_uiSelectionOrder = ( unsigned int )uiOrderingNum;
		uiOrderingNum++;

		pkMenuGroup->m_vpkMenuItems.push_back( ( CTDMenuItem* )pkMenuItem );
		CTDCONSOLE_PRINT( LogLevel( INFO ), string( "    registering menu item '" + pkMenuItem->GetInstanceName() + "'" ) );

	}

	// try to register this menu item in menu control entity
	//-----------------------------------------------------//
	pkMenuCtrl->Message( CTD_MENUCTRL_REGISTER_GROUP, &pkMenuGroup );
	//-----------------------------------------------------//

}


int	CTDMenuGroup::ParameterDescription( int iParamIndex, ParameterDescriptor *pkDesc )
{

	int iParamCount = 3;

	if (pkDesc == NULL) {

		return iParamCount;
	}

	switch( iParamIndex ) 
	{
	case 0:
		pkDesc->SetName( "Name" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_STRING );
		pkDesc->SetVar( &m_strGroupName );
		
		break;

	case 1:
		pkDesc->SetName( "ID" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_INTEGER );
		pkDesc->SetVar( &m_uiGroupID );
		
		break;

	case 2:
		pkDesc->SetName( "Items" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_STRING );
		pkDesc->SetVar( &m_strItemNames );
		
		break;

	default:
		return -1;
	}

	return iParamCount;

}

}