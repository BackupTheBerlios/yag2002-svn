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
 # neoengine, misc functions for setting up chat plugin
 #
 # it provides input listener and chat setup funtion
 #
 #
 #   date of creation:  12/02/2003
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>

#include "ctd_setup.h"

using namespace NeoEngine;
using namespace std;
using namespace CTD;

namespace CTD_IPluginChat {

//----------------------------------------------------------------//
int								g_aiKeys[ CTD_KEYFLAG_ARRAY_NUM ] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  0, 0, 0, 0 };
InputGroup						*g_pkInputGroup		= NULL;
CTDInputListener				*g_pkInputListener	= NULL;
bool							g_bLockMovementInut = false;

LevelSet*                       s_pkLevelSet        = NULL;
//----------------------------------------------------------------//

void SetLevelSet( CTD::LevelSet* pkLevelSet )
{
    s_pkLevelSet = pkLevelSet;
}

CTD::LevelSet* GetLevelSet()
{
    return s_pkLevelSet;
}

// clear movement flags
void ClearMovementFlags()
{

	memset( g_aiKeys, 0, CTD_KEYFLAG_ARRAY_NUM * sizeof( int ) );

}

// lock movement inputs for chatmember entity
void LockMovementInput( bool bLock )
{

	g_bLockMovementInut = bLock;

}

CTDInputListener::CTDInputListener( InputGroup *pkGroup ) : InputEntity( pkGroup )
{

	// setup key names
	m_strKeyRotUp				= KEY_ROTATE_UP;
	m_iKeyCodeRotUp				= 0;
	m_strKeyRotDown				= KEY_ROTATE_DOWN;
	m_iKeyCodeRotDown			= 0;
	m_strKeyRotLeft				= KEY_ROTATE_LEFT;
	m_iKeyCodeRotLeft			= 0;
	m_strKeyRotRight			= KEY_ROTATE_RIGHT;
	m_iKeyCodeRotRight			= 0;
	m_strKeyMoveForward			= KEY_MOVE_FORWARD;
	m_iKeyCodeMoveForward		= 0;
	m_strKeyMoveBackward		= KEY_MOVE_BACKWARD;
	m_iKeyCodeMoveBackward		= 0;
	m_strKeyMoveSideLeft		= KEY_MOVE_SIDE_LEFT;
	m_iKeyCodeMoveSideLeft		= 0;
	m_strKeyMoveSideRight		= KEY_MOVE_SIDE_RIGHT;
	m_iKeyCodeMoveSideRight		= 0;
	m_strKeyChangeMode			= KEY_CHANGE_MODE;
	m_iKeyCodeChangeMode		= 0;

	// bind key names to key codes
	if ( Framework::Get()->GetKeyCode( m_strKeyRotUp, m_iKeyCodeRotUp ) == false ) {

		neolog << LogLevel( WARNING ) << endl;
		neolog << LogLevel( WARNING ) << " *** CTD chat could not bind key '" << KEY_ROTATE_UP << "'" << endl;
		neolog << LogLevel( WARNING ) << endl;
	
	}

	if ( Framework::Get()->GetKeyCode( m_strKeyRotDown, m_iKeyCodeRotDown ) == false ) {

		neolog << LogLevel( WARNING ) << endl;
		neolog << LogLevel( WARNING ) << " *** CTD chat could not bind key '" << KEY_ROTATE_DOWN << "'" << endl;
		neolog << LogLevel( WARNING ) << endl;
	
	}

	if ( Framework::Get()->GetKeyCode( m_strKeyRotLeft, m_iKeyCodeRotLeft ) == false ) {

		neolog << LogLevel( WARNING ) << endl;
		neolog << LogLevel( WARNING ) << " *** CTD chat could not bind key '" << KEY_ROTATE_LEFT << "'" << endl;
		neolog << LogLevel( WARNING ) << endl;
	
	}

	if ( Framework::Get()->GetKeyCode( m_strKeyRotRight, m_iKeyCodeRotRight ) == false ) {

		neolog << LogLevel( WARNING ) << endl;
		neolog << LogLevel( WARNING ) << " *** CTD chat could not bind key '" << KEY_ROTATE_RIGHT << "'" << endl;
		neolog << LogLevel( WARNING ) << endl;
	
	}

	if ( Framework::Get()->GetKeyCode( m_strKeyMoveForward, m_iKeyCodeMoveForward ) == false ) {

		neolog << LogLevel( WARNING ) << endl;
		neolog << LogLevel( WARNING ) << " *** CTD chat could not bind key '" << KEY_MOVE_FORWARD << "'" << endl;
		neolog << LogLevel( WARNING ) << endl;
	
	}

	if ( Framework::Get()->GetKeyCode( m_strKeyMoveBackward, m_iKeyCodeMoveBackward ) == false ) {

		neolog << LogLevel( WARNING ) << endl;
		neolog << LogLevel( WARNING ) << " *** CTD chat could not bind key '" << KEY_MOVE_BACKWARD << "'" << endl;
		neolog << LogLevel( WARNING ) << endl;
	
	}

	if ( Framework::Get()->GetKeyCode( m_strKeyMoveSideLeft, m_iKeyCodeMoveSideLeft ) == false ) {

		neolog << LogLevel( WARNING ) << endl;
		neolog << LogLevel( WARNING ) << " *** CTD chat could not bind key '" << KEY_MOVE_SIDE_RIGHT << "'" << endl;
		neolog << LogLevel( WARNING ) << endl;
	
	}

	if ( Framework::Get()->GetKeyCode( m_strKeyMoveSideRight, m_iKeyCodeMoveSideRight ) == false ) {

		neolog << LogLevel( WARNING ) << endl;
		neolog << LogLevel( WARNING ) << " *** CTD chat could not bind key '" << KEY_MOVE_SIDE_RIGHT << "'" << endl;
		neolog << LogLevel( WARNING ) << endl;
	
	}

	if ( Framework::Get()->GetKeyCode( m_strKeyChangeMode, m_iKeyCodeChangeMode ) == false ) {

		neolog << LogLevel( WARNING ) << endl;
		neolog << LogLevel( WARNING ) << " *** CTD chat could not bind key '" << KEY_CHANGE_MODE << "'" << endl;
		neolog << LogLevel( WARNING ) << endl;
	
	}

}

// input listener
void CTDInputListener::Input( const InputEvent *pkEvent )
{

	if( ( ( pkEvent->m_iType == IE_KEYDOWN ) && ( pkEvent->m_aArgs[0].m_iData == KC_F2 ) ) ||
		( ( pkEvent->m_iType == IE_SYSEVENT ) && ( pkEvent->m_aArgs[0].m_iData == IE_SYSEVENT_KILL ) ) ) {

		Framework::Get()->ExitGame();

	}

	int	iKeyData = pkEvent->m_aArgs[0].m_iData;
	int	iFlag    = 0;

	// handle key down/up events
	if ( pkEvent->m_iType == IE_KEYDOWN ) {

		iFlag = 1;

	} else {

		if ( pkEvent->m_iType == IE_KEYUP ) {

			iFlag = 0;
		
		}
	
	}

	// if movement inputs are locked then do not check the movement input keys
	if ( g_bLockMovementInut == true ) {

		return;

	}

    if( pkEvent->m_iType == IE_MOUSEMOVE ) {
        
        static int s_iPosX = 0,  s_iLastPosX = 0;
        static int s_iPosY = 0,  s_iLastPosY = 0;

        s_iLastPosX = s_iPosX;
        s_iPosX = pkEvent->m_aArgs[0].m_iData;

        s_iLastPosY = s_iPosY;
        s_iPosY = pkEvent->m_aArgs[1].m_iData;

        if ( ( s_iPosX -  s_iLastPosX ) != 0 ) {
            if ( ( s_iPosX -  s_iLastPosX ) > 0 ) {		
                g_aiKeys[ MOUSE_RIGHT ] = 1;
            } else {            
                g_aiKeys[ MOUSE_LEFT ]  = 1;
            }
        } 
        if ( ( s_iPosY -  s_iLastPosY ) != 0 ) {
            if ( ( s_iPosY -  s_iLastPosY ) > 0 ) {
                g_aiKeys[ MOUSE_UP ]   = 1;
            } else {
                g_aiKeys[ MOUSE_DOWN ] = 1;
            }
        } 

    }

	// handle character control keys
	if ( iKeyData == m_iKeyCodeRotUp ) {

		g_aiKeys[ ROTATE_UP ] = iFlag;

	}

	if ( iKeyData == m_iKeyCodeRotDown ) {

		g_aiKeys[ ROTATE_DOWN ] = iFlag;

	}

	if ( iKeyData == m_iKeyCodeRotLeft ) {

		g_aiKeys[ ROTATE_LEFT ] = iFlag;

	}

	if ( iKeyData == m_iKeyCodeRotRight ) {

		g_aiKeys[ ROTATE_RIGHT ] = iFlag;

	}

	if ( iKeyData == m_iKeyCodeMoveForward ) {

		g_aiKeys[ MOVE_FORWARD ] = iFlag;

	}

	if ( iKeyData == m_iKeyCodeMoveBackward ) {

		g_aiKeys[ MOVE_BACKWARD ] = iFlag;

	}

	if ( iKeyData == m_iKeyCodeMoveSideLeft ) {

		g_aiKeys[ MOVE_SIDE_LEFT ] = iFlag;

	}

	if ( iKeyData == m_iKeyCodeMoveSideRight ) {

		g_aiKeys[ MOVE_SIDE_RIGHT ] = iFlag;

	}

}

// setup input listener
void CreateInputListener()
{

	CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Chat) setting up input listener." );

	g_pkInputGroup		= new InputGroup;
	g_pkInputListener	= new CTDInputListener( g_pkInputGroup );
	// set up input and input listener//
	g_pkInputGroup->AttachEntity( ( InputEntity* )NeoEngine::Core::Get()->GetConsole() );

}

//----------------------------------------------------------------//

float		g_fFOVAngle		= 60.0f;
float		g_fNearplane	= 1.0f;
float		g_fFarplane		= 10000.0f;
// create and setup a camera 
void CreateCamera()
{

	CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Chat) setting up camera." );

	// we would like to have a chat in this plugin
	Camera *pkCam = new Camera( "_chatCamera" );
	pkCam->SetRoom( Framework::Get()->GetCurrentLevelSet()->GetRoom() );
	Framework::Get()->GetCurrentLevelSet()->SetCamera( pkCam );
	//Set default view position
	pkCam->SetTranslation( Vector3d( 0.0f, 0.0f, 0.0f ) );

	// set fov and near/far distance
	Framework::Get()->GetRenderDevice()->SetPerspectiveProjection( g_fFOVAngle, g_fNearplane, g_fFarplane );

}
//------------------------------------------------------//

} // namespace CTD_IPluginChat

