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
 # neoengine, output item for 3d menu
 #
 #
 #   date of creation:  05/14/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>
#include "ctd_outputitem.h"
#include <ctd_printf.h>

using namespace std;
using namespace CTD;
using namespace NeoEngine;

namespace CTD_IPluginMenu
{

// plugin global entity descriptor for output item
CTDMenuOutputItemDesc g_pkCTDMenuOutputItemEntity_desc;
//-------------------------------------------//


CTDMenuOutputItem::CTDMenuOutputItem()
{

	CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Menu) entity ' MenuOutputItem ' created " );

	CTDMenuItem::m_kPosition			= Vector3d( 0, 0, 0 );
	CTDMenuItem::m_kRotation			= Vector3d( 0, 0, 0 );
	CTDMenuItem::m_pkMesh				= NULL;
	m_eState							= eIdle;
	m_fBlendFactor						= 0;
	m_fDistanceOnFocused				= CTD_DISTANCE_TO_CAMERA_WHEN_FOCUSED;
	
	// set item type
	CTDMenuItem::SetType( CTDMenuItem::eOutput );

	// set the entity ( node ) name
	SetName( CTD_ENTITY_NAME_MenuOutputItem );

}

CTDMenuOutputItem::~CTDMenuOutputItem()
{

	CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Menu) entity ' MenuOutputItem ' destroyed " );

}

// init entity
void CTDMenuOutputItem::Initialize() 
{ 

	// let father class initialize first
	CTDMenuItem::Initialize();

	// set initial position and orientation
	SetTranslation( m_kPosition );
	Quaternion	kRot( EulerAngles( m_kRotation.x * PI / 180.0f, m_kRotation.y * PI / 180.0f, m_kRotation.z * PI / 180.0f ) );
	SetRotation( kRot );

	m_kCurrentPos	= m_kPosition;


}

// post-init entity
void CTDMenuOutputItem::PostInitialize()
{
	
	CTDMenuItem::PostInitialize();

	// if something went wrong durin father class initialization then
	//  this object will be deactivated
	if ( IsActive() == false ) {

		return;

	}

}

void CTDMenuOutputItem::OnBeginFocus() 
{

	if ( IsActive() == false ) {

		return;

	}

	// call father class' OnBeginFocus function to set some internal states
	CTDMenuItem::OnBeginFocus();

	m_eState			= eStartFocus;
	m_fBlendFactor		= 0;
	Camera	 *pkCamera	= GetMenuLevelSet()->GetCamera();
	m_kDestPos		= ( pkCamera->GetTranslation() + pkCamera->GetRotation() * Vector3d( 0, 0, -m_fDistanceOnFocused ) );

}

void CTDMenuOutputItem::OnEndFocus() 
{

	if ( IsActive() == false ) {

		return;

	}

	// call father class' OnEndFocus function to set some internal states
	CTDMenuItem::OnEndFocus();

	m_fBlendFactor	= 0;
	m_eState		= eLostFocus;

}

void CTDMenuOutputItem::OnInput( CTDMenuControl::CTDCtrlKeys eCtrlKey, unsigned int iKeyData, char cKeyData ) 
{

}


void CTDMenuOutputItem::UpdateEntity( float fDeltaTime ) 
{ 

	switch ( m_eState ){

		case eIdle:

			break;

		case eStartFocus:

			m_kCurrentPos = m_kCurrentPos + ( m_kDestPos - m_kCurrentPos ) * m_fBlendFactor;

			if ( m_fBlendFactor > 1.0f ) {

				m_fBlendFactor	= 0;
				m_eState		= eActive;

			} else {

				m_fBlendFactor += fDeltaTime;
				SetTranslation( m_kCurrentPos );

			}
			break;

		case eActive:
		{
				
		}
		break;

		case eLostFocus:
		{
			m_kCurrentPos = m_kCurrentPos - ( m_kCurrentPos - m_kPosition ) * m_fBlendFactor;

			if ( m_fBlendFactor > 1.0f ) {

				m_fBlendFactor	= 0;
				m_eState		= eIdle;

			} else {

				m_fBlendFactor += fDeltaTime;
				SetTranslation( m_kCurrentPos );

			}
		
		}
		break;

		default:
			break;

	}

}

int	CTDMenuOutputItem::Message( int iMsgId, void *pkMsgStruct ) 
{ 

	return 0;

}	


int	CTDMenuOutputItem::ParameterDescription( int iParamIndex, ParameterDescriptor *pkDesc )
{

	// get the param count of father class
	int iGeneralParamCount = CTDMenuItem::ParameterDescription( 0, NULL );
	int iParamCount = iGeneralParamCount + 1;

	if (pkDesc == NULL) {

		return iParamCount;
	}

	// general parameters are directed to father class
	if ( iParamIndex < iGeneralParamCount ) {

		CTDMenuItem::ParameterDescription( iParamIndex, pkDesc );
		return iParamCount;

	}

	// correct the parameter index
	iParamIndex -= iGeneralParamCount;

	switch( iParamIndex ) 
	{

	case 0:
		pkDesc->SetName( "DistOnFocus" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_FLOAT );
		pkDesc->SetVar( &m_fDistanceOnFocused );
		
		break;

	default:
		return -1;
	}

	return iParamCount;

}

}
