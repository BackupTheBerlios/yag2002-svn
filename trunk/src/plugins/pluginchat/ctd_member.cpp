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
 # neoengine, chat member class
 #
 # this class implements a chat member
 #
 #
 #   date of creation:  08/13/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include "base.h"
#include "ctd_member.h"
#include "ctd_setup.h"
#include <ctd_printf.h>
#include <ctd_settings.h>
#include "ctd_physics.h"

using namespace std;
using namespace CTD;
using namespace NeoEngine;

namespace CTD_IPluginChat {

// uncomment this to enable mouse input
#define TEST_DISABLE_MOUSE_INPUT    1

// uncomment this to avoid statistics output into screen
#define PRINT_STATISTICS            1

// plugin global entity descriptor for chat member
CTDChatMemberDesc   g_pkChatMemberEntity_desc;

//-------------------------------------------//

// this is the global instance ctdprintf object
CTDPrintf           g_CTDPrintf;
CTDPrintf           g_CTDPrintfNwStats;
//-------------------------------------------//

// maximal step height which can be automatically climbed by player
#define CTD_MAX_STEPHEIGHT          0.7f
// gravity force
#define CTD_PLAYER_GRAVITY          0.98f
// mouse input debounce timer
#define CTD_MOUSE_DEBOUNCE_TIME     0.2f


// animation command related stuff
//#####################################################
#define ANIM_CMD_MOVE_FORWARD		(unsigned int)0x1
#define ANIM_CMD_MOVE_BACKWARD		(unsigned int)0x2
#define ANIM_CMD_MOVE_RIGHT			(unsigned int)0x4
#define ANIM_CMD_MOVE_LEFT			(unsigned int)0x8
#define	ANIM_CMD_MOVE_STOP			(unsigned int)0x10

#define ANIM_CMD_MOVEMENT_MASK		(unsigned int)0x1F

#define ANIM_CMD_JUMP				(unsigned int)0x20
#define ANIM_CMD_ROTATE				(unsigned int)0x40
#define ANIM_CMD_LAND				(unsigned int)0x80

#define	SET_COMMAND(flag)		    (m_uiCmdFlag |= (flag))
#define CLEAR_COMMAND(flag)		    (m_uiCmdFlag &= ~(flag))
#define IS_COMMAND(flag)		    ((m_uiCmdFlag & (flag)) ? true:false)
#define WAS_LAST_COMMAND(flag)	    ((m_uiLastCmdFlag & (flag)) ? true:false)
#define UPDATE_LAST_COMMANDS()	    (m_uiLastCmdFlag = m_uiCmdFlag)
#define CLEAR_MOVE_COMMANDS()	    (m_uiCmdFlag &= ~ANIM_CMD_MOVEMENT_MASK)
//#####################################################

CTDChatMember::CTDChatMember()
{

    CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Chat) entity ' ChatMember ' created " );
    
    m_pkPlayerNetworking        = NULL;

    m_kInitPosition             = Vector3d( 0, 0, 0 );
    m_kInitRotation             = Vector3d( 0, 0, 0 );
    m_kDimensions               = Vector3d( 0.5f, 1.8f, 0.5f );
    m_fPositionalSpeed          = 5.0f;
    m_fAngularSpeed             = 1.0f;

    m_pkCamera                  = NULL;
    m_pkNetworkDevice           = NULL;
    m_pkEntityGui               = NULL;
    m_pkRoom                    = NULL;
    m_pkAnimMgr                 = NULL;

    m_fAngularAcceleration      = 0;
    m_fPositionalAcceleration   = 0;
    m_fMouseSensitivity         = 1.0f;
    m_fMouseDebounceTimer       = 0;

    m_bMoved                    = false;
    m_bRotated                  = false;
    m_bMouseRotated             = false;
    m_uiCmdFlag = m_uiLastCmdFlag   = 0;

    // set the entity ( node ) name
    //  for client objects it is essential that the entity name is set in constructor
    SetName( CTD_ENTITY_NAME_ChatMember );

    // get simulation mode
    m_eGameMode = Framework::Get()->GetGameMode();

}

CTDChatMember::~CTDChatMember()
{

    CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Chat) entity ' ChatMember ' destroyed " );

    // networking instance of remote clients are deleted by object distribution system ( RakNet )
    if ( ( m_eGameMode == stateCLIENT ) && ( !m_pkPlayerNetworking->IsRemoteClient() ) ) {

        m_pkPlayerNetworking->PutChatText( "<system> " + m_strPlayerName + " says goodby" );
        delete m_pkPlayerNetworking;

    }

    if ( m_pkAnimMgr ) {
    
        delete m_pkAnimMgr;

    }

}


// init entity
void CTDChatMember::Initialize() 
{ 

    // get player name
    Framework::Get()->GetGameSettings()->GetValue( CTD_STOKEN_PLAYERNAME, m_strPlayerName );

    // get mouse sensitivity
    Framework::Get()->GetGameSettings()->GetValue( CTD_FTOKEN_MOUSE_SENSITIVITY, m_fMouseSensitivity );

    // store our room for later use ( e.g. for collision checks )
    m_pkRoom = Framework::Get()->GetCurrentLevelSet()->GetRoom();

    // set initial position and orientation
    SetTranslation( m_kInitPosition );
    m_kCurrentPosition   = m_kInitPosition;

    Quaternion  kRot( EulerAngles( m_kInitRotation.x * PI / 180.0f, m_kInitRotation.y * PI / 180.0f, m_kInitRotation.z * PI / 180.0f ) );
    SetRotation( kRot );
    m_kCurrentRotationQ  = kRot;
    m_kCurrentRotation   = Vector3d( m_kInitRotation.x * PI / 180.0f, m_kInitRotation.y * PI / 180.0f, m_kInitRotation.z * PI / 180.0f );

    // convert the angular speed from degree to radiant / second
    m_fAngularSpeed     = ( m_fAngularSpeed * PI / 180.0f );

    // setup the physics
    m_kPhysics.Initialize( this, Framework::Get()->GetCurrentLevelSet()->GetRoom(), CTD_MAX_STEPHEIGHT, CTD_PLAYER_GRAVITY );

    // setup networking
    if ( m_eGameMode != stateSTANDALONE ) {

        m_pkNetworkDevice   = Framework::Get()->GetNetworkDevice();
        assert ( m_pkNetworkDevice );

        if ( !m_pkPlayerNetworking ) {
            m_pkPlayerNetworking = new CTD_MemberNw( this );
            // publish the object in network
            m_pkPlayerNetworking->Publish(); 
        } 
        if ( ( m_eGameMode == stateCLIENT ) && ( !m_pkPlayerNetworking->IsRemoteClient() ) ) {

            m_pkPlayerNetworking->Initialize( m_kCurrentPosition, m_strPlayerName, m_strCal3dConfigFile );

        }

    } 

    // set the ChatMember as player, if this session is not started as server and this object is not a remote client ( ghost )
    switch ( m_eGameMode ) {

        case stateSTANDALONE:
            
            Framework::Get()->SetPlayer( this );            
            m_pkCamera = Framework::Get()->GetCurrentLevelSet()->GetCamera();
            assert( m_pkCamera );
            break;

        case stateCLIENT:

            if ( !m_pkPlayerNetworking->IsRemoteClient() ) {

                Framework::Get()->SetPlayer( this );            
                m_pkCamera = Framework::Get()->GetCurrentLevelSet()->GetCamera();
                assert( m_pkCamera );
           
            }
            break;

        default:;
    }

}
// post-init entity
void CTDChatMember::PostInitialize()
{

    // we need the gui only for non-server mode
    if ( m_eGameMode != stateSERVER ) {

        // get the gui entity, there must be only one single gui entity
        m_pkEntityGui = ( CTDGui* )Framework::Get()->FindEntity( CTD_ENTITY_NAME_Gui );
        assert( m_pkEntityGui && "*** CTDChatMember: no gui is defined!" );
        m_pkEntityGui->SetupGuiSystem();

    }

    if ( ( m_eGameMode == stateCLIENT ) && ( !m_pkPlayerNetworking->IsRemoteClient() ) ) {

        m_pkPlayerNetworking->PutChatText( "<system> " + m_strPlayerName + " says hello" );
    
    }

    if ( ( m_eGameMode == stateSTANDALONE ) || 
         ( ( m_eGameMode == stateCLIENT ) && ( !m_pkPlayerNetworking->IsRemoteClient() ) ) ) {

	    // create and initialize the animation manager
	    //-------------------------------------------------------------//
	    m_pkAnimMgr = new CTDCal3DAnim ( ( SceneNode* )this );

	    // initialize the character animation module
	    if ( !m_pkAnimMgr->Initialize( m_strCal3dConfigFile ) ) {

		    delete m_pkAnimMgr;
		    m_pkAnimMgr = NULL;
		    Deactivate();

		    CTDCONSOLE_PRINT( LogLevel( WARNING ), " (Plugin Chat) entity ' ChatMember ' deactivated " );

		    return;
	    }

    }

	//-------------------------------------------------------------//

    // set bounding volume
    BoundingVolume  *pkBBox = GetBoundingVolume();
    ( ( AABB *)pkBBox )->SetDim( Vector3d( m_kDimensions ) );


}

void CTDChatMember::ProcessInput( float fDeltaTime )
{

    m_bMoved   = false;
    m_bRotated = false;
    
    // Process pressed keys
    if( g_aiKeys[ ROTATE_UP ] ) {

        m_kRotate.x = -m_fAngularSpeed;
        m_bRotated = true;

    } else {

        if( g_aiKeys[ ROTATE_DOWN ] ) {

            m_kRotate.x = m_fAngularSpeed;
            m_bRotated = true;

        }

    }

    if( g_aiKeys[ ROTATE_LEFT ] ) {

        m_kRotate.y = m_fAngularSpeed;
        m_bRotated = true;

    } else {

        if( g_aiKeys[ ROTATE_RIGHT ] ) {

            m_kRotate.y = -m_fAngularSpeed;
            m_bRotated = true;

        }

    }

//! TODO remove this later
#ifndef TEST_DISABLE_MOUSE_INPUT
    // Process mouse movement  
    if( g_aiKeys[ MOUSE_UP ] ) {

        m_kRotate.x = -m_fAngularSpeed * m_fMouseSensitivity;
        m_bMouseRotated = true;

    } else {

        if( g_aiKeys[ MOUSE_DOWN ] ) {

            m_kRotate.x = m_fAngularSpeed * m_fMouseSensitivity;
            m_bMouseRotated = true;

        }

    }

    if( g_aiKeys[ MOUSE_LEFT ] ) {

        m_kRotate.y = m_fAngularSpeed * m_fMouseSensitivity;
        m_bMouseRotated = true;

    } else {

        if( g_aiKeys[ MOUSE_RIGHT ] ) {

            m_kRotate.y = -m_fAngularSpeed * m_fMouseSensitivity;
            m_bMouseRotated = true;

        }

    }
    // reset mouse movement flags
    if ( m_bMouseRotated ) {

        // debounce the mouse movement inputs, otherwise the rotations become jerky
        m_fMouseDebounceTimer += fDeltaTime;
        if ( m_fMouseDebounceTimer > CTD_MOUSE_DEBOUNCE_TIME ) {

            g_aiKeys[ MOUSE_DOWN ] = g_aiKeys[ MOUSE_UP ] = g_aiKeys[ MOUSE_LEFT ] = g_aiKeys[ MOUSE_RIGHT ] = 0;
            m_fMouseDebounceTimer = 0;

        }
    }

#endif //TEST_DISABLE_MOUSE_INPUT

    if( g_aiKeys[ MOVE_FORWARD ] ) {

    	SET_COMMAND( ANIM_CMD_MOVE_FORWARD );
        m_kMove.z = -m_fPositionalSpeed;
        m_bMoved = true;

    } else {

        if( g_aiKeys[ MOVE_BACKWARD ] ) {

           	SET_COMMAND( ANIM_CMD_MOVE_BACKWARD );
            m_kMove.z = m_fPositionalSpeed;
            m_bMoved = true;
        
        }
    
    }

    if( g_aiKeys[ MOVE_SIDE_RIGHT ] ) {

        SET_COMMAND( ANIM_CMD_MOVE_RIGHT );
        m_kMove.x = m_fPositionalSpeed;
        m_bMoved = true;
        
    } else {

        if( g_aiKeys[ MOVE_SIDE_LEFT ] ) {
        
            SET_COMMAND( ANIM_CMD_MOVE_LEFT );
            m_kMove.x = -m_fPositionalSpeed;
            m_bMoved = true;
        
        }
    
    }

    // calculate position vector
    if ( m_bMoved == true ) {

        m_kMove *= fDeltaTime;

    } else {

        SET_COMMAND( ANIM_CMD_MOVE_STOP );
        m_kMove.Reset();

    }

    // calculate rotation vector
    if ( m_bRotated || m_bMouseRotated ) {

       	SET_COMMAND( ANIM_CMD_ROTATE );
        m_kRotate *= fDeltaTime;

    } else {

        m_kRotate.Reset();

    }

}

void CTDChatMember::UpdateEntity( float fDeltaTime ) 
{ 

    // limit delta time
    if ( fDeltaTime > 0.06f ) {

        fDeltaTime = 0.06f;

    }

    // in server mode or stand alone we get input commands from keyboard
    //  in client mode we get the current state of remote ChatMember via networking
    switch ( m_eGameMode ) {

        case stateSTANDALONE:

            UpdateStandaloneObject( fDeltaTime );
            break;

        case stateSERVER:
            
            UpdateServerObject( fDeltaTime );
            break;

        case stateCLIENT:

            UpdateClientObject( fDeltaTime );
            break;
        
        default:

            // something must have been gone wrong!?
            break;
        
    }

    
}

void CTDChatMember::UpdateServerObject( float fDeltaTime ) 
{

    m_pkPlayerNetworking->Update();

}

void CTDChatMember::UpdateClientObject( float fDeltaTime ) 
{

    m_pkPlayerNetworking->Update();

    if ( !m_pkPlayerNetworking->IsRemoteClient() ) {
    
        // process new input
        ProcessInput( fDeltaTime );

        // handle rotation
        if ( IS_COMMAND( ANIM_CMD_ROTATE ) ) {

            m_kCurrentRotationQ      *= Quaternion( EulerAngles( m_kRotate.x, 0.0f, 0.0f ) );
            m_kCurrentRotationQ       = Quaternion( EulerAngles( 0.0f, m_kRotate.y, 0.0f ) ) * m_kCurrentRotationQ;

            SetRotation( m_kCurrentRotationQ );
            m_pkCamera->SetRotation( m_kCurrentRotationQ );

            CLEAR_COMMAND( ANIM_CMD_ROTATE );

        }

        switch ( m_uiCmdFlag & ANIM_CMD_MOVEMENT_MASK ) {

            case ANIM_CMD_MOVE_FORWARD:
            case ANIM_CMD_MOVE_BACKWARD:
            {
	            // set move animation
	            m_pkAnimMgr->Run();

                // calculate position depending on local inputs
                Vector3d kMoveVec        = m_kCurrentRotationQ * m_kMove;

                // move the body considering collisions
                m_kPhysics.MoveBody( m_kCurrentPosition, kMoveVec, fDeltaTime );

                m_pkCamera->SetTranslation( m_kCurrentPosition );

                // update position in network
                m_pkPlayerNetworking->UpdatePosition( 
                    m_kCurrentPosition.x, 
                    m_kCurrentPosition.y, 
                    m_kCurrentPosition.z 
                    );

            }
            break;

            default:;

        }
         
    } else {

        // get position
        m_pkPlayerNetworking->GetPosition( 
            m_kCurrentPosition.x, 
            m_kCurrentPosition.y, 
            m_kCurrentPosition.z 
            );

        //! TODO: get also rotation over network
        SetRotation( m_kCurrentRotationQ );
        SetTranslation( m_kCurrentPosition );

    }


#ifdef PRINT_STATISTICS
    // this is the code which is executed on client's own player
    if ( !m_pkPlayerNetworking->IsRemoteClient() ) {

        // some printfs into screen
        char    strBuff[256];
        // print out fps
        sprintf( strBuff, "FPS %.0f  ", Framework::Get()->GetFPS() );
        CTDPRINTF_PRINT( g_CTDPrintf, strBuff );
        sprintf( strBuff, "player name: %s  ", m_strPlayerName.c_str() );
        CTDPRINTF_PRINT( g_CTDPrintf, strBuff );
        sprintf( strBuff, "position: %f %f %f  ", m_kCurrentPosition.x, m_kCurrentPosition.y, m_kCurrentPosition.z );
        CTDPRINTF_PRINT( g_CTDPrintf, strBuff );
        sprintf( strBuff, "rotation: %f %f %f  ", m_kCurrentRotation.x, m_kCurrentRotation.y, m_kCurrentRotation.z );
        CTDPRINTF_PRINT( g_CTDPrintf, strBuff );

    }
#endif

}

void CTDChatMember::UpdateStandaloneObject( float fDeltaTime ) 
{

    // process new input
    ProcessInput( fDeltaTime );

    static Quaternion s_kCamRotation;

    // handle rotation
    if ( IS_COMMAND( ANIM_CMD_ROTATE ) ) {

        // calculate rotation depending on local inputs
        s_kCamRotation       *= Quaternion( EulerAngles( m_kRotate.x, 0.0f, 0.0f ) );
        s_kCamRotation       = Quaternion( EulerAngles( 0.0f, m_kRotate.y, 0.0f ) ) * s_kCamRotation;
        m_kCurrentRotationQ  = Quaternion( EulerAngles( 0.0f, m_kRotate.y, 0.0f ) ) * m_kCurrentRotationQ;

        SetRotation( m_kCurrentRotationQ );
        // set camera rotation and position
        m_pkCamera->SetRotation( s_kCamRotation );

        if (!WAS_LAST_COMMAND( ANIM_CMD_MOVE_FORWARD | ANIM_CMD_MOVE_BACKWARD ) ) {
            m_pkAnimMgr->Turn();
        } 

        CLEAR_COMMAND( ANIM_CMD_ROTATE );

    }

    // trigger animations
    switch ( m_uiCmdFlag & ANIM_CMD_MOVEMENT_MASK ) {
    
        case ANIM_CMD_MOVE_LEFT:

            m_pkAnimMgr->Run();
            break;

        case ANIM_CMD_MOVE_RIGHT:

            m_pkAnimMgr->Run();
            break;

        case ANIM_CMD_MOVE_FORWARD:

            m_pkAnimMgr->Run();
            break;

        case ANIM_CMD_MOVE_BACKWARD:
	        
            m_pkAnimMgr->WalkBack();
            break;

        case ANIM_CMD_MOVE_STOP:
        
 	        m_pkAnimMgr->Stop();        
            break;

        default:
            ;

    }

    Vector3d kMoveVec;

    if ( m_uiCmdFlag & ANIM_CMD_MOVEMENT_MASK ) {

        // calculate position depending on local inputs
        kMoveVec    = m_kCurrentRotationQ * m_kMove;

        SetTranslation( m_kCurrentPosition );

        // TODO: add camera offset as controllable parameter
        Vector3d    kOffset( 0, 1.0f, 10.0f );
        m_pkCamera->SetTranslation( m_kCurrentPosition + ( s_kCamRotation * kOffset ) );

        // reset movement flags
        CLEAR_COMMAND( ANIM_CMD_MOVEMENT_MASK );

    }

    // move the body considering collisions
    m_kPhysics.MoveBody( m_kCurrentPosition, kMoveVec, fDeltaTime );

    // update animation
	m_pkAnimMgr->UpdateAnim( fDeltaTime );


#ifdef PRINT_STATISTICS
    // this is the code which is executed on client's own player
    // some printfs into screen
    static char strBuff[256];
    // print out fps
    sprintf( strBuff, "FPS %.0f  ", Framework::Get()->GetFPS() );
    CTDPRINTF_PRINT( g_CTDPrintf, strBuff );
    sprintf( strBuff, "player name: %s  ", m_strPlayerName.c_str() );
    CTDPRINTF_PRINT( g_CTDPrintf, strBuff );
    sprintf( strBuff, "position: %f %f %f  ", m_kCurrentPosition.x, m_kCurrentPosition.y, m_kCurrentPosition.z );
    CTDPRINTF_PRINT( g_CTDPrintf, strBuff );
    sprintf( strBuff, "rotation: %f %f %f  ", m_kCurrentRotation.x, m_kCurrentRotation.y, m_kCurrentRotation.z );
    CTDPRINTF_PRINT( g_CTDPrintf, strBuff );
#endif

}

void CTDChatMember::PostChatText( const std::string& strText )
{
    
    m_pkEntityGui->AppendText( strText );

}

bool CTDChatMember::Render( Frustum *pkFrustum, bool bForce ) 
{

     if( bForce ) {
         
         m_uiLastFrame = s_uiFrameCount; 
         return true; 
     } 

     if( !m_bActive || ( m_uiLastFrame >= s_uiFrameCount ) ) 
         return false; 

     m_uiLastFrame = s_uiFrameCount; 

     if ( m_pkAnimMgr ) {
     
         m_pkAnimMgr->Render();

     }

    GetBoundingVolume()->RenderOutlines( Color::BLUE );

    return true;

}

int CTDChatMember::Message( int iMsgId, void *pMsgStruct ) 
{ 

    switch ( iMsgId ) {

        // handle pausing and entering the menu
        case CTD_ENTITY_ENTER_MENU:
        case CTD_ENTITY_PAUSING_BEGIN:

            CTDPRINTF_DISABLE( g_CTDPrintf );
            CTDPRINTF_FLUSH( g_CTDPrintf );
            break;

        case CTD_ENTITY_EXIT_MENU:
        case CTD_ENTITY_PAUSING_END:

            CTDPRINTF_ENABLE( g_CTDPrintf );
            break;

        // this message is sent by gui entity and signalized a new text has been edited by local chat member
        case CTD_ENTITY_GUI_SEND_TEXT:
        {

            wchar_t *pwcText = ( wchar_t* )pMsgStruct;
            char    pcText[ 256 ];
            wcstombs(  pcText, pwcText, 256 );
            m_pkPlayerNetworking->PutChatText( pcText );

        }
        break;

         
        //case CTD_NM_SYSTEM_NOTIFY_CONNECTION_LOST:
        //{

        //    // server broatcasts an object destruction notification on connection lost
        //    if ( m_eGameMode == stateSERVER ) {
        //        PlayerID *pkPlayerID = ( PlayerID* )pMsgStruct;
        //        if ( ( *pkPlayerID ) == m_pkPlayerNetworking->GetClientOwnerID() != 0 ) {
        //            m_pkPlayerNetworking->DestroyObjectOnNetwork();
        //        }
        //    }
        //}
        //break;

        default:
            break;

    }

    return 0; 
}   

int CTDChatMember::ParameterDescription( int iParamIndex, ParameterDescriptor *pkDesc )
{

    int iParamCount = 6;

    if (pkDesc == NULL) {

        return iParamCount;
    }

    switch( iParamIndex ) 
    {
    case 0:
        pkDesc->SetName( "Position" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_VECTOR3 );
        pkDesc->SetVar( &m_kInitPosition );
        
        break;

    case 1:
        pkDesc->SetName( "Rotation" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_VECTOR3 );
        pkDesc->SetVar( &m_kInitRotation );
        
        break;

    case 2:
        pkDesc->SetName( "Dimensions" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_VECTOR3 );
        pkDesc->SetVar( &m_kDimensions );
        
        break;

    case 3:
        pkDesc->SetName( "PositionalSpeed" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_FLOAT );
        pkDesc->SetVar( &m_fPositionalSpeed );
        break;

    case 4:
        pkDesc->SetName( "AngularSpeed" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_FLOAT );
        pkDesc->SetVar( &m_fAngularSpeed );
        break;

    case 5:
        pkDesc->SetName( "AnimationConfig" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_STRING );
        pkDesc->SetVar( &m_strCal3dConfigFile );
        break;

    default:
        return -1;
    }

    return iParamCount;

}

}
