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
 # neoengine, menu camera
 #
 # this class implements the camera control for 3d menu
 #
 #
 #   date of creation:  05/13/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>
#include "ctd_menuitem.h"
#include <ctd_printf.h>

using namespace std;
using namespace CTD;
using namespace NeoEngine;

namespace CTD_IPluginMenu
{

CTDMenuItem::CTDMenuItem()
{

	m_kPosition					= Vector3d( 0, 0, 0 );
	m_kRotation					= Vector3d( 0, 0, 0 );
	m_pkMesh					= NULL;
	m_uiGroupID					= 0;
	m_uiSelectionOrder			= 0;
	m_strParameterName			= CTD_MENUITEM_UNKNOWN_PARAM_NAME;
	m_bIsFocused				= false;

}

CTDMenuItem::~CTDMenuItem()
{

}

// init entity
void CTDMenuItem::Initialize() 
{ 

	if ( m_pkMesh == NULL ) {

		string strMsg = " (Plugin Menu) entity ' " + GetName() + "::" + GetInstanceName() + " ' has no mesh assigned, deactivated!";
		CTDCONSOLE_PRINT( LogLevel( INFO ), strMsg );
		Deactivate();

	}

	SetEntity( m_pkMesh );

}

bool CTDMenuItem::Render( Frustum *pkFrustum, bool bForce ) 
{

	// render the mesh
	m_pkMesh->Render();

	return true;

}

int	CTDMenuItem::ParameterDescription( int iParamIndex, ParameterDescriptor *pkDesc )
{

	int iParamCount = 3;

	if (pkDesc == NULL) {

		return iParamCount;
	}

	switch( iParamIndex ) 
	{
	case 0:
		pkDesc->SetName( "Position" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_VECTOR3 );
		pkDesc->SetVar( &m_kPosition );
		
		break;
	case 1:
		pkDesc->SetName( "Rotation" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_VECTOR3 );
		pkDesc->SetVar( &m_kRotation );
		
		break;

	case 2:
		pkDesc->SetName( "Mesh" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_STATIC_MESH );
		pkDesc->SetVar( &m_pkMesh );
		break;

	default:
		return -1;
	}

	return iParamCount;

}

}
