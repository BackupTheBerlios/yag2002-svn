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
 # neoengine,exit game item for 3d menu
 #  this item allows the termination of game
 #
 #
 #   date of creation:  08/04/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>
#include "ctd_exitgameitem.h"
#include <ctd_printf.h>
#include <ctd_settings.h>

using namespace std;
using namespace CTD;
using namespace NeoEngine;

namespace CTD_IPluginMenu
{

// plugin global entity descriptor for exit game item
CTDMenuExitGameItemDesc g_pkCTDMenuExitGameItemEntity_desc;
//-------------------------------------------//


CTDMenuExitGameItem::CTDMenuExitGameItem()
{

	CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Menu) entity ' MenuExitGameItem ' created " );

	CTDMenuItem::m_kPosition			= Vector3d( 0, 0, 0 );
	CTDMenuItem::m_kRotation			= Vector3d( 0, 0, 0 );
	CTDMenuItem::m_pkMesh				= NULL;
	m_fPosVar							= 0;
	
	// set item type
	CTDMenuItem::SetType( CTDMenuItem::eAction );

	// set the entity ( node ) name
	SetName( CTD_ENTITY_NAME_MenuExitGameItem );

}

CTDMenuExitGameItem::~CTDMenuExitGameItem()
{

	CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Menu) entity ' MenuExitGameItem ' destroyed " );

}

// init entity
void CTDMenuExitGameItem::Initialize() 
{ 

	// let father class initialize first
	CTDMenuItem::Initialize();

	// set initial position and orientation
	SetTranslation( m_kPosition );
	Quaternion	kRot( EulerAngles( m_kRotation.x * PI / 180.0f, m_kRotation.y * PI / 180.0f, m_kRotation.z * PI / 180.0f ) );
	SetRotation( kRot );

}

// post-init entity
void CTDMenuExitGameItem::PostInitialize()
{
	
	CTDMenuItem::PostInitialize();

	// if something went wrong durin father class initialization then
	//  this object will be deactivated
	if ( IsActive() == false ) {

		return;

	}

}

void CTDMenuExitGameItem::OnBeginFocus() 
{

	CTDMenuItem::OnBeginFocus();
	m_fPosVar		= 0;
	m_kCurrPosition	= m_kPosition;

}

void CTDMenuExitGameItem::OnEndFocus() 
{

	CTDMenuItem::OnEndFocus();
	SetTranslation( m_kPosition );

}

void CTDMenuExitGameItem::OnActivate() 
{

	if ( IsActive() == false ) {

		return;

	}

	Framework::Get()->ExitGame();

}

void CTDMenuExitGameItem::UpdateEntity( float fDeltaTime ) 
{ 

	if ( IsFocused() == false ) {

		return;

	}

	m_fPosVar += fDeltaTime;
	if ( m_fPosVar > ( 2.0f * PI ) ) {

		m_fPosVar -= ( 2.0f * PI );

	}
	m_kCurrPosition = m_kPosition + Vector3d( 0, 0.3f * cosf( m_fPosVar ), 0 );
	SetTranslation( m_kCurrPosition );

}

int	CTDMenuExitGameItem::Message( int iMsgId, void *pkMsgStruct ) 
{ 

	return 0;

}	


int	CTDMenuExitGameItem::ParameterDescription( int iParamIndex, ParameterDescriptor *pkDesc )
{

	// get the param count of father class
	int iGeneralParamCount = CTDMenuItem::ParameterDescription( 0, NULL );
	int iParamCount = iGeneralParamCount;

	if (pkDesc == NULL) {

		return iParamCount;
	}

	// general parameters are directed to father class
	if ( iParamIndex < iGeneralParamCount ) {

		CTDMenuItem::ParameterDescription( iParamIndex, pkDesc );
		return iParamCount;

	}

	return iParamCount;

}

}
