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
 # neoengine, start level item for 3d menu
 #  this item allows immediately starting of preset level
 #
 #   date of creation:  07/29/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>
#include "ctd_startlevelitem.h"
#include <ctd_printf.h>
#include <ctd_settings.h>

using namespace std;
using namespace CTD;
using namespace NeoEngine;

namespace CTD_IPluginMenu
{

// plugin global entity descriptor for start level item
CTDMenuStartLevelItemDesc g_pkCTDMenuStartLevelItemEntity_desc;
//-------------------------------------------//


CTDMenuStartLevelItem::CTDMenuStartLevelItem()
{

	CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Menu) entity ' MenuStartLevelItem ' created " );

	CTDMenuItem::m_kPosition			= Vector3d( 0, 0, 0 );
	CTDMenuItem::m_kRotation			= Vector3d( 0, 0, 0 );
	CTDMenuItem::m_pkMesh				= NULL;

	m_fPosVar							= 0;
	
	// set item type
	CTDMenuItem::SetType( CTDMenuItem::eAction );

	// set the entity ( node ) name
	SetName( CTD_ENTITY_NAME_MenuStartLevelItem );

}

CTDMenuStartLevelItem::~CTDMenuStartLevelItem()
{

	CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Menu) entity ' MenuStartLevelItem ' destroyed " );

}

// init entity
void CTDMenuStartLevelItem::Initialize() 
{ 

	// let father class initialize first
	CTDMenuItem::Initialize();

	// set initial position and orientation
	SetTranslation( m_kPosition );
	Quaternion	kRot( EulerAngles( m_kRotation.x * PI / 180.0f, m_kRotation.y * PI / 180.0f, m_kRotation.z * PI / 180.0f ) );
	SetRotation( kRot );

}

// post-init entity
void CTDMenuStartLevelItem::PostInitialize()
{
	
	CTDMenuItem::PostInitialize();

	// if something went wrong durin father class initialization then
	//  this object will be deactivated
	if ( IsActive() == false ) {

		return;

	}

}

void CTDMenuStartLevelItem::OnBeginFocus() 
{

	CTDMenuItem::OnBeginFocus();
	m_fPosVar		= 0;
	m_kCurrPosition	= m_kPosition;

}

void CTDMenuStartLevelItem::OnEndFocus() 
{

	CTDMenuItem::OnEndFocus();
	SetTranslation( m_kPosition );

}

void CTDMenuStartLevelItem::OnActivate() 
{

	if ( IsActive() == false ) {

		return;

	}

	string strLevelFile;
    bool   bClient = false;

    Settings::Get()->GetValue( CTD_BTOKEN_CLIENT, bClient );

    if ( bClient ) {

        // get the level name through server connection
        strLevelFile = StartNetworking();
        if ( strLevelFile.length() == 0 ) {

            // todo: here a message box should appear informing about problems with connecting to server
            return;

        }

    } else {

        Settings::Get()->GetValue( CTD_STOKEN_LEVEL, strLevelFile );

    }

    // try to load the level
    LevelSet *pkGameLevelSet = Framework::Get()->LoadLevel( strLevelFile );
    if ( pkGameLevelSet == NULL ) {

		// cannot load level!
        return;

	}

    if ( bClient ) {

        // start the client networking
        Framework::Get()->StartClientNetworking();

    }

    // store game's level set for later switching between menu and game
    SetGameLevelSet( pkGameLevelSet );

    // switch to loaded level set
    Framework::Get()->SwitchToLevelSet( pkGameLevelSet );

    // deactivate menu level set for rendering and updating, till next 3d menu call
    GetMenuLevelSet()->SetRenderFlag( false );
    GetMenuLevelSet()->SetUpdateFlag( false );

}

string CTDMenuStartLevelItem::StartNetworking()
{

    string strLevelName;

    CTDCONSOLE_PRINT( LogLevel( INFO ), "setup networking ..." );

    int             iServerPort;
    int             iClientPort;
    string          strServerIP;
    string          strNodeName;

    Settings::Get()->GetValue( CTD_ITOKEN_CLIENTPORT, iClientPort );
    Settings::Get()->GetValue( CTD_ITOKEN_SERVERPORT, iServerPort );
    Settings::Get()->GetValue( CTD_STOKEN_SERVERIP,   strServerIP );
    Settings::Get()->GetValue( CTD_STOKEN_PLAYERNAME, strNodeName );


    if ( Framework::Get()->InitiateClientNetworking( iClientPort, iServerPort, strServerIP, strNodeName, strLevelName ) == false ) {
    
        CTDCONSOLE_PRINT( LogLevel( INFO ), "Problems with setting up networking" );
        return "";

    }
    
    return strLevelName;

}

void CTDMenuStartLevelItem::UpdateEntity( float fDeltaTime ) 
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

int	CTDMenuStartLevelItem::Message( int iMsgId, void *pkMsgStruct ) 
{ 

	return 0;

}	


int	CTDMenuStartLevelItem::ParameterDescription( int iParamIndex, ParameterDescriptor *pkDesc )
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
