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
 # neoengine, menu item marker
 #
 # this class implements the a marker appearing when an item is marked
 #
 #
 #   date of creation:  05/27/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include "base.h"
#include "ctd_itemmarker.h"


using namespace std;
using namespace CTD;
using namespace NeoEngine;

namespace CTD_IPluginMenu
{


// plugin global entity descriptor for item marker
CTDItemMarkerDesc g_pkCTDItemMarkerEntity_desc;
//-------------------------------------------//


CTDItemMarker::CTDItemMarker()
{

	CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Menu) entity ' ItemMarker ' created " );

	m_pkMesh	= NULL;
	m_fWaveVar	= 0;

	// set the entity ( node ) name
	SetName( CTD_ENTITY_NAME_ItemMarker );

}

CTDItemMarker::~CTDItemMarker()
{

	CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Menu) entity ' ItemMarker ' destroyed " );

}

// init entity
void CTDItemMarker::Initialize() 
{ 

	if ( m_pkMesh == NULL ) {

		CTDCONSOLE_PRINT( LogLevel( WARNING ), " (Plugin Menu) entity ' ItemMarker ' could not load item marker's mesh, deactivating entity! " );
		Deactivate();

	}

	SetEntity( m_pkMesh );

}

void CTDItemMarker::UpdateEntity( float fDeltaTime ) 
{ 

	// softly move the marker up and down
	m_fWaveVar += 5.0f * fDeltaTime;
	if ( m_fWaveVar > 2.0f * PI )  {

		m_fWaveVar = 0;

	}

	Vector3d	kTranslation = GetTranslation();
	kTranslation.y += 0.1f * cosf( m_fWaveVar );
	SetTranslation( kTranslation );

	Rotate( Quaternion( EulerAngles( 0, fDeltaTime, 0 ) ) );

}

bool CTDItemMarker::Render( Frustum *pkFrustum, bool bForce ) 
{

	// render the marker mesh
	m_pkMesh->Render();

	return true;

}


int	CTDItemMarker::ParameterDescription( int iParamIndex, ParameterDescriptor *pkDesc )
{

	int iParamCount = 1;

	if (pkDesc == NULL) {

		return iParamCount;
	}

	switch( iParamIndex ) 
	{

	case 0:
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