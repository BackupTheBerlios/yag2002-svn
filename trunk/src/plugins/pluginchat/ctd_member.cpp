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
using namespace NeoChunkIO;

namespace CTD_IPluginChat {


// uncomment this define when the gui test is completed, use this mode only with standalone simulation
//#define GUI_TEST  1

// plugin global entity descriptor for chat member
CTDChatMemberDesc   g_pkChatMemberEntity_desc;
//-------------------------------------------//

// this is the global instance ctdprintf object
CTDPrintf           g_CTDPrintf;
CTDPrintf           g_CTDPrintfNwStats;
//-------------------------------------------//

// maximal step height which can be automatically climbed by player
#define CTD_MAX_STEPHEIGHT      0.7f
// gravity force
#define CTD_PLAYER_GRAVITY      0.98f

template< class InterpolatorT, class InterpolatorRotT >
CTDChatMember< InterpolatorT, InterpolatorRotT >::CTDChatMember()
{

    CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Chat) entity ' ChatMember ' created " );

    m_kInitPosition             = Vector3d( 0, 0, 0 );
    m_kInitRotation             = Vector3d( 0, 0, 0 );
    m_kDimensions               = Vector3d( 0.5f, 0.5f, 0.5f );
    m_fPositionalSpeed          = 5.0f;
    m_fAngularSpeed             = 1.0f;

    m_pkCamera                  = NULL;
    m_pkMesh                    = NULL;
    m_pkNetworkServer           = NULL;
    m_pkNetworkClient           = NULL;
    m_pkNetworkDevice           = NULL;
    m_pkChatMember              = NULL;
    m_pkEntityGui               = NULL;
    m_pkRoom                    = NULL;

    m_fPassedPosSendTime        = 0;
    m_fPassedRotSendTime        = 0;
    m_fAngularAcceleration      = 0;
    m_fPositionalAcceleration   = 0;

    m_bMoved                    = false;
    m_eMove                     = stateIdleMove;
    m_bRotated                  = false;
    m_eRotate                   = stateIdleRotate;
    m_eTranslationState         = stateTranslationIdle;
    m_eRotationState            = stateRotationIdle;


    // activate networking as client object, for remote client objects this type will be set by framework after integration into network session
    SetNetworkingType( CTD::stateCLIENTOBJECT );

    // set the entity ( node ) name
    //  for client objects it is essential that the entity name is set in constructor
    SetName( CTD_ENTITY_NAME_ChatMember );

}

template< class InterpolatorT, class InterpolatorRotT >
CTDChatMember< InterpolatorT, InterpolatorRotT >::~CTDChatMember()
{

    CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Chat) entity ' ChatMember ' destroyed " );

    // FIXME: remove the ifdefs after testing the chat box gui
#ifndef GUI_TEST

    if ( ( m_eGameMode == stateCLIENT ) && ( GetNetworkingType() == stateCLIENTOBJECT ) ) {

        tCTD_NM_SendChatMsg         kChatMsg;
        kChatMsg.m_usNetworkID      = GetNetworkID();
        kChatMsg.m_ucDataType       = CTD_NM_ChatMember_CHAT_MSG_REQUEST;
        string strMsgText( "<system> " + m_strPlayerName + " says goodby" ); 
        strcpy( kChatMsg.m_pcChatText, strMsgText.c_str() );

        bool bSuccess = m_pkNetworkDevice->SendClient(
            GetNetworkID(),
            ( char* )&kChatMsg, 
            sizeof( tCTD_NM_SendChatMsg ), 
            HIGH_PRIORITY, 
            RELIABLE_ORDERED, 
            1
            );

    }

#endif

}

// init entity
template< class InterpolatorT, class InterpolatorRotT >
void CTDChatMember< InterpolatorT, InterpolatorRotT >::Initialize() 
{ 

    // get simulation mode
    m_eGameMode = Framework::Get()->GetGameMode();

    if ( m_eGameMode != stateSTANDALONE ) {

        m_pkNetworkDevice   = Framework::Get()->GetNetworkDevice();
        assert ( m_pkNetworkDevice );
        m_pkNetworkServer   = m_pkNetworkDevice->GetNetworkServer();
        m_pkNetworkClient   = m_pkNetworkDevice->GetNetworkClient();

    } 

    // set the ChatMember as player, if this session is not started as server and this object is not a ghost
    if ( ( m_eGameMode != stateSERVER ) && ( GetNetworkingType() != stateREMOTE_CLIENTOBJECT ) ) {

        Framework::Get()->SetPlayer( this );
        
        m_pkCamera = Framework::Get()->GetCurrentLevelSet()->GetCamera();
        assert( m_pkCamera );

    }

    // if we have a mesh then use its bounding volume dimensions
    if ( m_pkMesh != NULL ) {

        GetBoundingVolume()->Generate( m_pkMesh->GetBoundingVolume() );


    } else {

        // set bounding volume
        BoundingVolume  *pkBBox = GetBoundingVolume();
        ( ( AABB *)pkBBox )->SetDim( Vector3d( m_kDimensions ) );

    }

    // set initial position and orientation
    SetTranslation( m_kInitPosition );
    m_kActualPosition   = m_kInitPosition;
    m_kPositionInterpolator.Reset( m_kActualPosition );

    Quaternion  kRot( EulerAngles( m_kInitRotation.x * PI / 180.0f, m_kInitRotation.y * PI / 180.0f, m_kInitRotation.z * PI / 180.0f ) );
    SetRotation( kRot );
    m_kActualRotationQ  = kRot;
    m_kActualRotation   = Vector3d( m_kInitRotation.x * PI / 180.0f, m_kInitRotation.y * PI / 180.0f, m_kInitRotation.z * PI / 180.0f );
    m_kRotationInterpolator.Reset( m_kActualRotation );

    // convert the angular speed from degree to radiant / second
    m_fAngularSpeed     = ( m_fAngularSpeed * PI / 180.0f );

    // get player name
    Framework::Get()->GetGameSettings()->GetValue( CTD_STOKEN_PLAYERNAME, m_strPlayerName );

    // store our room for later use ( e.g. for collision checks )
    m_pkRoom = Framework::Get()->GetCurrentLevelSet()->GetRoom();

    // setup the physics
    m_kPhysics.Initialize( this, Framework::Get()->GetCurrentLevelSet()->GetRoom(), CTD_MAX_STEPHEIGHT, CTD_PLAYER_GRAVITY );

}
// post-init entity
template< class InterpolatorT, class InterpolatorRotT >
void CTDChatMember< InterpolatorT, InterpolatorRotT >::PostInitialize()
{

    // if we are a remote client object then we have to wait for getting initialized
    //  using init data sent by actual client, we deactivate ourself so long
    if ( GetNetworkingType() == stateREMOTE_CLIENTOBJECT ) {

        Deactivate();

    }

    // store the local chat member for cancelling chat message echos later
    m_pkChatMember  = Framework::Get()->GetPlayer();

#ifndef GUI_TEST
    if ( ( m_eGameMode == stateCLIENT ) && ( GetNetworkingType() == stateCLIENTOBJECT ) ) {

        tCTD_NM_SendChatMsg         kChatMsg;
        kChatMsg.m_usNetworkID      = GetNetworkID();
        kChatMsg.m_ucDataType       = CTD_NM_ChatMember_CHAT_MSG_REQUEST;
        string strMsgText( "<system> " + m_strPlayerName + " says hello" ); 
        strcpy( kChatMsg.m_pcChatText, strMsgText.c_str() );

        bool bSuccess = m_pkNetworkDevice->SendClient(
            GetNetworkID(),
            ( char* )&kChatMsg, 
            sizeof( tCTD_NM_SendChatMsg ), 
            HIGH_PRIORITY, 
            RELIABLE_ORDERED, 
            1
            );

    }
#endif


    // we need the gui only for non-server mode
    if (  m_eGameMode != stateSERVER ) {

        // get the gui entity, there must be only one single gui entity
        m_pkEntityGui = Framework::Get()->FindEntity( CTD_ENTITY_NAME_Gui );

    }
}

template< class InterpolatorT, class InterpolatorRotT >
void CTDChatMember< InterpolatorT, InterpolatorRotT >::ProcessInput( float fDeltaTime )
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

    if( g_aiKeys[ MOVE_FORWARD ] ) {

        m_kMove.z = -m_fPositionalSpeed;
        m_bMoved = true;

    } else {

        if( g_aiKeys[ MOVE_BACKWARD ] ) {

            m_kMove.z = m_fPositionalSpeed;
            m_bMoved = true;
        
        }
    
    }

    if( g_aiKeys[ MOVE_SIDE_RIGHT ] ) {

        m_kMove.x = m_fPositionalSpeed;
        m_bMoved = true;
        
    } else {

        if( g_aiKeys[ MOVE_SIDE_LEFT ] ) {
        
            m_kMove.x = -m_fPositionalSpeed;
            m_bMoved = true;
        
        }
    
    }

    // calculate position vector
    if ( m_bMoved == true ) {

        m_eMove = stateMoving;
        m_kMove *= fDeltaTime;

    } else {

        m_eMove = stateIdleMove;
        m_kMove.Reset();

    }

    // calculate rotation vector
    if ( m_bRotated == true ) {

        m_eRotate = stateRotating;
        m_kRotate *= fDeltaTime;

    } else {

        m_eRotate = stateIdleRotate;
        m_kRotate.Reset();

    }

    // evaluate the movement and set appropriate translation state
    switch( m_eTranslationState ) {

        case stateTranslationIdle:

            if ( m_eMove == stateMoving ) {
             
                m_eTranslationState = stateTranslationStartMoving;

            }
            break;

        case stateTranslationStartMoving:

            m_eTranslationState = stateTranslationMoving;
            break;

        case stateTranslationMoving:

            if ( m_eMove == stateIdleMove ) {

                m_eTranslationState = stateTranslationStopMoving;

            }
            break;

        case stateTranslationStopMoving:

            m_eTranslationState = stateTranslationIdle;
            break;

        default:
            CTDCONSOLE_PRINT( LogLevel( ERROR ), " (Plugin Chat) entity ' ChatMember ' invalid translation state " );

    }

    // evaluate the movement and set appropriate rotation state
    switch( m_eRotationState ) {

        case stateRotationIdle:

            if ( m_eRotate == stateRotating ) {
             
                m_eRotationState = stateRotationStartMoving;

            }
            break;

        case stateRotationStartMoving:

            m_eRotationState = stateRotationMoving;
            break;

        case stateRotationMoving:

            if ( m_eRotate == stateIdleRotate ) {

                m_eRotationState = stateRotationStopMoving;

            }
            break;

        case stateRotationStopMoving:

            m_eRotationState = stateRotationIdle;
            break;

        default:
            CTDCONSOLE_PRINT( LogLevel( ERROR ), " (Plugin Chat) entity ' ChatMember ' invalid rotation state " );

    }

}

template< class InterpolatorT, class InterpolatorRotT >
void CTDChatMember< InterpolatorT, InterpolatorRotT >::UpdateEntity( float fDeltaTime ) 
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

            break;

        case stateCLIENT:

            if ( GetNetworkingType() == stateREMOTE_CLIENTOBJECT ) {                    

                UpdateRemoteClientObject( fDeltaTime );

            } else {

                UpdateClientObject( fDeltaTime );

            }
            break;
        
        default:

            // something must have been gone wrong!?
            break;
        
    }


    //-------------------------//

    // this is the code which is executed on client's own player
    if (  Framework::Get()->GetPlayer() == this ) {

        // some printfs into screen
        char    strBuff[256];
        // print out fps
        sprintf( strBuff, "FPS %.0f  ", Framework::Get()->GetFPS() );
        CTDPRINTF_PRINT( g_CTDPrintf, strBuff );
        sprintf( strBuff, "player name: %s  ", m_strPlayerName.c_str() );
        CTDPRINTF_PRINT( g_CTDPrintf, strBuff );
        sprintf( strBuff, "position: %f %f %f  ", m_kActualPosition.x, m_kActualPosition.y, m_kActualPosition.z );
        CTDPRINTF_PRINT( g_CTDPrintf, strBuff );
        sprintf( strBuff, "rotation: %f %f %f  ", m_kActualRotation.x, m_kActualRotation.y, m_kActualRotation.z );
        CTDPRINTF_PRINT( g_CTDPrintf, strBuff );

    }
    
}

template< class InterpolatorT, class InterpolatorRotT >
void CTDChatMember< InterpolatorT, InterpolatorRotT >::UpdateClientObject( float fDeltaTime ) 
{

    // process new input
    ProcessInput( fDeltaTime );

    // update sending timers
    m_fPassedPosSendTime    += fDeltaTime;
    m_fPassedRotSendTime    += fDeltaTime;

    // calculate actual rotation depending on local inputs
    //-------------------------------------------------------------------------------------------------------//
    // first store last rotation for calculating the velocity on send
    if ( m_eRotationState != stateRotationIdle ) {

        m_kLastRotation     = m_kActualRotation;
        m_kActualRotation   += m_kRotate;

        m_kActualRotationQ  = Quaternion( EulerAngles( m_kActualRotation.x, 0.0f, 0.0f ) );
        m_kActualRotationQ  = Quaternion( EulerAngles( 0.0f, m_kActualRotation.y, 0.0f ) ) * m_kActualRotationQ;
    
    }
    //-------------------------------------------------------------------------------------------------------//

    // calculate actual position depending on local inputs
    //-------------------------------------------------------------------------------------------------------//
    // first store last position for calculating the velocity on send
    if ( m_eTranslationState != stateTranslationIdle ) {

        m_kLastPosition     = m_kActualPosition;
        Vector3d kMoveVec   = m_kActualRotationQ * m_kMove;        
        m_kActualPosition   += kMoveVec;

    }
    //-------------------------------------------------------------------------------------------------------//

    // send requesting new position information to server, start and stop moving events are sent immediately
    if ( ( m_eTranslationState == stateTranslationStartMoving ) || 
         ( m_eTranslationState == stateTranslationStopMoving )  ||
         ( m_eTranslationState == stateTranslationMoving ) && ( m_fPassedPosSendTime > CTD_ChatMember_POS_UPDATE_PERIODE ) ) {

        // calculate velocity
        Vector3d kVelocity = ( m_kActualPosition - m_kLastPosition ) * ( 1.0f / fDeltaTime );

        // fill and send the message structure
        //------------------------------------------------------------------//

        static  tCTD_NM_ChatMemberPosition  skNetMsgPos; // is used to distribute the current position

        unsigned short usNetworkID          = GetNetworkID();
        skNetMsgPos.m_ucDataType            = CTD_NM_ChatMember_DT_POSITION_REQUEST;

        // on stopping we correct our actual position to interpolated instead of vice versa!
        //  thus we avoid tedious ( and visually inconvenient ) deviation corrections when stop moving
        if ( m_eTranslationState == stateTranslationStopMoving ) {

            m_kActualPosition = m_kPositionInterpolator.GetCurrentPoint();
            kVelocity.Reset();
            m_kPositionInterpolator.AddInterpolationPoint( m_kActualPosition, kVelocity, 0 );

        }
        skNetMsgPos.m_fPosX                 = m_kActualPosition.x;
        skNetMsgPos.m_fPosY                 = m_kActualPosition.y;
        skNetMsgPos.m_fPosZ                 = m_kActualPosition.z;

        skNetMsgPos.m_fVelX                 = kVelocity.x;
        skNetMsgPos.m_fVelY                 = kVelocity.y;
        skNetMsgPos.m_fVelZ                 = kVelocity.z;

        skNetMsgPos.m_stateTranslation      = ( unsigned char )m_eTranslationState;

        // send the message
        bool bSuccess = m_pkNetworkDevice->SendClient(                  
                                                        usNetworkID,
                                                        ( char* )&skNetMsgPos, 
                                                        sizeof( tCTD_NM_ChatMemberPosition ), 
                                                        HIGH_PRIORITY, 
                                                        RELIABLE_ORDERED, 
                                                        0
                                                      );              

        m_fPassedPosSendTime    = 0;

    }

    // send requesting new rotation information to server
    if ( ( m_eRotationState == stateRotationStartMoving ) || 
         ( m_eRotationState == stateRotationStopMoving )  ||
         ( m_eRotationState == stateRotationMoving ) && ( m_fPassedRotSendTime > CTD_ChatMember_ROT_UPDATE_PERIODE ) ) {

        // calculate velocity
        Vector3d kVelocity = ( m_kActualRotation - m_kLastRotation ) * ( 1.0f / fDeltaTime );

        // fill and send the message structure
        //------------------------------------------------------------------//

        static  tCTD_NM_ChatMemberRotation  skNetMsgRot; // is used to distribute the current rotation

        unsigned short usNetworkID          = GetNetworkID();
        skNetMsgRot.m_ucDataType            = CTD_NM_ChatMember_DT_ROTATION_REQUEST;

        // on stopping we correct our actual rotation to interpolated instead of vice versa!
        //  thus we avoid tedious ( and visually inconvenient ) deviation corrections when stop moving
        if ( m_eRotationState == stateRotationStopMoving ) {

            m_kActualRotation = m_kRotationInterpolator.GetCurrentPoint();
            kVelocity.Reset();
            m_kRotationInterpolator.AddInterpolationPoint( m_kActualRotation, kVelocity, 0 );

        }
        skNetMsgRot.m_fRotX                 = m_kActualRotation.x;
        skNetMsgRot.m_fRotY                 = m_kActualRotation.y;

        skNetMsgRot.m_fVelX                 = kVelocity.x;
        skNetMsgRot.m_fVelY                 = kVelocity.y;

        skNetMsgRot.m_stateRotation         = ( unsigned char )m_eRotationState;

        // send the message now
        bool bSuccess = m_pkNetworkDevice->SendClient(                  
                                                        usNetworkID,
                                                        ( char* )&skNetMsgRot, 
                                                        sizeof( tCTD_NM_ChatMemberRotation ), 
                                                        HIGH_PRIORITY, 
                                                        RELIABLE_ORDERED, 
                                                        0
                                                      );

        m_fPassedRotSendTime    = 0;

    }

    // interpolate position
    m_kInterpolatedPosition = m_kPositionInterpolator.UpdateInterpolation( fDeltaTime );
    SetTranslation( m_kInterpolatedPosition );

    // interpolate rotation
    m_kInterpolatedRotation = m_kRotationInterpolator.UpdateInterpolation( fDeltaTime );
    SetRotation( Quaternion( EulerAngles( m_kInterpolatedRotation ) ) );

    // update camera
    m_pkCamera->SetTranslation( GetTranslation() );
    m_pkCamera->SetRotation( GetRotation() );
   
    
    // some debug prints
    //------------------------------------------------------------------//
#ifdef _DEBUG
     // calculate the position deviation 
    float fPosDiff2 = ( m_kInterpolatedPosition - m_kActualPosition ).Len2();
    // calculate the rotation deviation 
    float fRotDiff2 = ( m_kInterpolatedRotation - m_kActualRotation ).Len2();

    char    pcBuff[256];
    int iMyPing = m_pkNetworkDevice->GetNetworkClient()->GetAveragePing();
    sprintf( pcBuff, "average ping: %d ms", iMyPing );
    CTDPRINTF_PRINT( g_CTDPrintfNwStats, pcBuff );

    unsigned long ulRelPacketWait = m_pkNetworkDevice->GetNetworkClient()->GetUnacknowledgedSentPacketListSize();
    sprintf( pcBuff, "reliable packets waiting for send: %d ", ulRelPacketWait );
    CTDPRINTF_PRINT( g_CTDPrintfNwStats, pcBuff );

    unsigned long ulSendQueueBuffersize = m_pkNetworkDevice->GetNetworkClient()->GetPacketOutputBufferSize();
    sprintf( pcBuff, "packets waiting for send: %d ", ulSendQueueBuffersize );
    CTDPRINTF_PRINT( g_CTDPrintfNwStats, pcBuff );

    unsigned long ulBytesSentPerSecond = m_pkNetworkDevice->GetNetworkClient()->GetBytesSentPerSecond();
    sprintf( pcBuff, "bytes sent per second: %d ", ulBytesSentPerSecond );
    CTDPRINTF_PRINT( g_CTDPrintfNwStats, pcBuff );

    unsigned long ulBytesReceivedPerSecond = m_pkNetworkDevice->GetNetworkClient()->GetBytesReceivedPerSecond();
    sprintf( pcBuff, "bytes received per second: %d ", ulBytesReceivedPerSecond );
    CTDPRINTF_PRINT( g_CTDPrintfNwStats, pcBuff );

    sprintf( pcBuff, "interpolated position diff2: %f", fPosDiff2 );
    CTDPRINTF_PRINT( g_CTDPrintfNwStats, pcBuff );

    sprintf( pcBuff, "interpolated rotation diff2: %f", fRotDiff2 );
    CTDPRINTF_PRINT( g_CTDPrintfNwStats, pcBuff );

    sprintf( pcBuff, "avarage position packet update time: %f", m_kAvaragePositionUpdatePeriod.GetAvarageUpdatePeriod() );
    CTDPRINTF_PRINT( g_CTDPrintfNwStats, pcBuff );
    sprintf( pcBuff, "avarage rotation packet update time: %f", m_kAvarageRotationUpdatePeriod.GetAvarageUpdatePeriod() );
    CTDPRINTF_PRINT( g_CTDPrintfNwStats, pcBuff );
#endif
    //------------------------------------------------------------------//


}

template< class InterpolatorT, class InterpolatorRotT >
void CTDChatMember< InterpolatorT, InterpolatorRotT >::UpdateRemoteClientObject( float fDeltaTime ) 
{

    // interpolate position
    m_kInterpolatedPosition = m_kPositionInterpolator.UpdateInterpolation( fDeltaTime );
    SetTranslation( m_kInterpolatedPosition );


    // interpolate rotation
    m_kInterpolatedRotation = m_kRotationInterpolator.UpdateInterpolation( fDeltaTime );
    SetRotation( Quaternion( EulerAngles( m_kInterpolatedRotation ) ) );

}

template< class InterpolatorT, class InterpolatorRotT >
void CTDChatMember< InterpolatorT, InterpolatorRotT >::UpdateStandaloneObject( float fDeltaTime ) 
{

    // process new input
    ProcessInput( fDeltaTime );

    // calculate rotation depending on local inputs
    m_kActualRotationQ      *= Quaternion( EulerAngles( m_kRotate.x, 0.0f, 0.0f ) );
    m_kActualRotationQ       = Quaternion( EulerAngles( 0.0f, m_kRotate.y, 0.0f ) ) * m_kActualRotationQ;

    // calculate position depending on local inputs
    Vector3d kMoveVec        = m_kActualRotationQ * m_kMove;

    // move the body considering collisions
    m_kPhysics.MoveBody( m_kActualPosition, kMoveVec, fDeltaTime );

    SetRotation( m_kActualRotationQ );
    SetTranslation( m_kActualPosition );

    m_pkCamera->SetRotation( m_kActualRotationQ );
    m_pkCamera->SetTranslation( m_kActualPosition );

}

template< class InterpolatorT, class InterpolatorRotT >
bool CTDChatMember< InterpolatorT, InterpolatorRotT >::Render( Frustum *pkFrustum, bool bForce ) 
{

     if( bForce ) {
         
         m_uiLastFrame = s_uiFrameCount; 
         return true; 
     } 

     if( !m_bActive || ( m_uiLastFrame >= s_uiFrameCount ) ) 
         return false; 

     m_uiLastFrame = s_uiFrameCount; 


    // skip drawing ourself, we are visible only to other players
    if ( GetNetworkingType() == stateREMOTE_CLIENTOBJECT ) {

        // render the mesh
        m_pkMesh->Render( pkFrustum, bForce );

    }

#ifdef _DEBUG
    GetBoundingVolume()->RenderOutlines( Color::BLUE );
#endif

    return true;

}

// incomming client object network messages
template< class InterpolatorT, class InterpolatorRotT >
void CTDChatMember< InterpolatorT, InterpolatorRotT >::NetworkMessage( int iMsgId, void *pkMsgStruct )
{


    switch ( iMsgId ) {
        
        // network messages sent by other entities in network
        case CTD_NM_NETWORK_ENTITY:
        {

            ProcessEntityMessage( pkMsgStruct );

        }
        break;

        // network system messages sent by framework
        case CTD_NM_NETWORK_SYSTEM:
        {
        
            ProcessSystemMessage( pkMsgStruct );

        }
        break;

        case CTD_NM_SYSTEM_NOTIFY_CONNECTION_LOST:
        {
        }
        break;

        case CTD_NM_SYSTEM_NOTIFY_CONNECTION_RESUMED:
        {
        }
        break;

        case CTD_NM_SYSTEM_NOTIFY_DISCONNECTED:
        {
        }
        break;

        default:
            // unexpected message type!
            ;

    }

}

template< class InterpolatorT, class InterpolatorRotT >
void CTDChatMember< InterpolatorT, InterpolatorRotT >::ProcessSystemMessage( void *pkMsgStruct )
{

    tCTD_NM_InitRemoteChatMember    *pkSystemMsg;
    pkSystemMsg = ( tCTD_NM_InitRemoteChatMember* )pkMsgStruct;

    switch ( pkSystemMsg->m_ucDataType ) {

        // this notification is sent by framework to a client object forcing it to send its 
        //  initialization data to all existing clients
        case CTD_NM_CMD_SEND_INITIALIZION_DATA:
        {
            // this message can only be received by client objects on server
            assert( m_eGameMode == stateCLIENT );
            assert( GetNetworkingType() == stateCLIENTOBJECT );
            SendInitDataToRemoteClients( ( tCTD_NM_InitRemoteChatMember* )pkSystemMsg );

            CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Chat) entity ' ChatMember ' sending initialization data " );

        }
        break;

        // this notification is sent by a new connected client to all new created remote clients
        //  the initialization data must be handled by receivers
        case CTD_NM_CMD_RECEIVE_INITIALIZION_DATA:
        {

            // this message can only be received by a new created remote client on a client and by server's remote client representing this object
            tCTD_NM_InitRemoteChatMember    *pkInitData = ( tCTD_NM_InitRemoteChatMember* )pkSystemMsg;

            // extract initialization data
            //############################//

            m_kActualPosition = Vector3d(  pkInitData->m_fPosX, pkInitData->m_fPosY, pkInitData->m_fPosZ );
            m_kActualRotation = Vector3d(  pkInitData->m_fRotX, pkInitData->m_fRotY, pkInitData->m_fRotZ );

            // this initialization is also called for remote clients on server
            //  if so then skip the rest of entity initialization
            if ( m_eGameMode == stateSERVER ) {

                break;

            }

            // create mesh
            //----------------------------------------------------------------------//
            string      strMeshName = pkInitData->m_pcMeshFile;
            Scene      kScene;
            if ( kScene.Load( strMeshName ) == false ) {

                CTDCONSOLE_PRINT( LogLevel( WARNING ), " warning: (Plugin Chat) entity ' ChatMember ' remote object's mesh file could not be opened " ); 
                return;

            }

            m_pkMesh = new MeshEntity( kScene.GetMeshes()[0] );
            m_pkMesh->GenerateBoundingVolume();
            m_pkMesh->GetMesh()->SetName( strMeshName );

            //----------------------------------------------------------------------//

            if ( m_pkMesh ) {

                SetEntity( m_pkMesh );

            }

            // set initial rotation and position
            SetTranslation( m_kActualPosition );
            SetRotation( Quaternion( EulerAngles( m_kActualRotation ) ) );

            m_kPositionInterpolator.Initialize( m_kActualPosition );
            m_kRotationInterpolator.Initialize( m_kActualRotation );

            // setup the physics
            m_kPhysics.Initialize( this, Framework::Get()->GetCurrentLevelSet()->GetRoom(), CTD_MAX_STEPHEIGHT, CTD_PLAYER_GRAVITY );

            // now we can get activated
            Activate();

        }
        break;

        // unknown message
        default:

            CTDCONSOLE_PRINT( LogLevel( WARNING ), " *** warning: (Plugin Chat) entity ' ChatMember ' received unknown system message type! " );

            break;

    }

}

template< class InterpolatorT, class InterpolatorRotT >
void CTDChatMember< InterpolatorT, InterpolatorRotT >::ProcessEntityMessage( void *pkMsgStruct )
{

    tCTD_NM_ChatMemberTemplate  *pkNetMsg;
    pkNetMsg = ( tCTD_NM_ChatMemberTemplate* )pkMsgStruct;

    unsigned short usNetworkID = GetNetworkID();

    // if this assertion occurs then something went wrong in framework's message distribution system 
    assert( usNetworkID == pkNetMsg->m_usNetworkID );

    switch ( pkNetMsg->m_ucDataType ) {


        // server-specific network messages
        //---------------------------------------------------------------------------------//

        // incomming chat text from clients to server
        case CTD_NM_ChatMember_CHAT_MSG_REQUEST:
        {

            assert( ( m_eGameMode == stateSERVER ) && " a non-server node got server-specific packet 'CTD_NM_ChatMember_CHAT_MSG_REQUEST'!" );

            // forward the message to all clients

            static tCTD_NM_SendChatMsg  *pkDistributeChatMsg;
            pkDistributeChatMsg = ( tCTD_NM_SendChatMsg* )pkMsgStruct;
    
            // limit the text
            pkDistributeChatMsg->m_pcChatText[ 255 ]    = 0;
            pkDistributeChatMsg->m_ucDataType           = CTD_NM_ChatMember_CHAT_MSG;

            bool bSuccess = m_pkNetworkDevice->SendServer( 
                usNetworkID,
                ( char* )pkDistributeChatMsg, 
                sizeof( tCTD_NM_SendChatMsg ), 
                HIGH_PRIORITY, 
                RELIABLE_SEQUENCED, 
                0, 
                UNASSIGNED_PLAYER_ID, 
                true, 
                false
                );

        }
        break;
    
        // receive new incomming chat message from a client
        case CTD_NM_ChatMember_CHAT_MSG:
        {

            if ( m_eGameMode == stateSERVER ) {
                
                CTDCONSOLE_PRINT( LogLevel( WARNING ), " *** warning: (Plugin Chat) entity ' ChatMember ' a non-server node got server-specific packet 'CTD_NM_ChatMember_CHAT_MSG_REQUEST'! " );
                break;

            }

            // cancel echos
            if ( pkNetMsg->m_usNetworkID == m_pkChatMember->GetNetworkID() ) {

                break;

            }

            // we collect all client and remote client messages
            static tCTD_NM_SendChatMsg  *pkChatMsg;
            pkChatMsg = ( tCTD_NM_SendChatMsg* )pkMsgStruct;
    
            // limit the text
            pkChatMsg->m_pcChatText[ 255 ]  = 0;

            // print out the message using the gui system
            m_pkEntityGui->Message( CTD_ENTITY_CHATMEMBER_ADD_MSG, pkChatMsg->m_pcChatText );

        }
        break;
    
        // the server can make some plausibility checks here before send this information to requesting client
        case CTD_NM_ChatMember_DT_POSITION_REQUEST: // sent by clients, received by server
        {

            assert( ( m_eGameMode == stateSERVER ) && " a non-server node got server-specific packet 'CTD_NM_ChatMember_DT_POSITION_REQUEST'!" );

            // distribute the updated position to all clients
            static tCTD_NM_ChatMemberPosition *pkDistributePosMsg;
            pkDistributePosMsg = ( tCTD_NM_ChatMemberPosition* )pkNetMsg;
            pkDistributePosMsg->m_ucDataType = CTD_NM_ChatMember_DT_POSITION;

            bool bSuccess = m_pkNetworkDevice->SendServer( 
                usNetworkID,
                ( char* )pkDistributePosMsg, 
                sizeof( tCTD_NM_ChatMemberPosition ), 
                HIGH_PRIORITY, 
                UNRELIABLE_SEQUENCED, 
                0, 
                UNASSIGNED_PLAYER_ID, 
                true, 
                false
                );


            // set the updated position
            m_kActualPosition.x = pkDistributePosMsg->m_fPosX;
            m_kActualPosition.y = pkDistributePosMsg->m_fPosY;
            m_kActualPosition.z = pkDistributePosMsg->m_fPosZ;
            SetTranslation( m_kActualPosition );

        }
        break;

        case CTD_NM_ChatMember_DT_ROTATION_REQUEST: // sent by clients, received by server
        {

            assert( ( m_eGameMode == stateSERVER ) && " a non-server node got server-specific packet 'CTD_NM_ChatMember_DT_ROTATION_REQUEST'!" );

            // distribute the updated rotation to all clients
            static tCTD_NM_ChatMemberRotation *pkDistributeRotMsg;
            pkDistributeRotMsg = ( tCTD_NM_ChatMemberRotation* )pkNetMsg;
            pkDistributeRotMsg->m_ucDataType = CTD_NM_ChatMember_DT_ROTATION;

            bool bSuccess = m_pkNetworkDevice->SendServer( 
                usNetworkID,
                ( char* )pkDistributeRotMsg, 
                sizeof( tCTD_NM_ChatMemberRotation ), 
                HIGH_PRIORITY, 
                UNRELIABLE_SEQUENCED, 
                0, 
                UNASSIGNED_PLAYER_ID, 
                true, 
                false
                );

            m_kActualRotation.x = pkDistributeRotMsg->m_fRotX;
            m_kActualRotation.y = pkDistributeRotMsg->m_fRotY;
            SetRotation( Quaternion( EulerAngles( m_kActualRotation ) ) );

        }
        break;

        case CTD_NM_ChatMember_DT_POSITION: // sent by server received by clients 
        {

            assert( ( m_eGameMode == stateCLIENT ) && " a non-client node got client-specific packet 'CTD_NM_ChatMember_DT_POSITION'!" );

            static tCTD_NM_ChatMemberPosition *pkReceivedPosMsg;
            pkReceivedPosMsg = ( tCTD_NM_ChatMemberPosition* )pkNetMsg;

            Vector3d                kNewPosition, kVelocity;

            kNewPosition.x          = pkReceivedPosMsg->m_fPosX;
            kNewPosition.y          = pkReceivedPosMsg->m_fPosY;
            kNewPosition.z          = pkReceivedPosMsg->m_fPosZ;

            kVelocity.x             = pkReceivedPosMsg->m_fVelX;
            kVelocity.y             = pkReceivedPosMsg->m_fVelY;
            kVelocity.z             = pkReceivedPosMsg->m_fVelZ;

            m_eTranslationState     = ( _translationState )( pkReceivedPosMsg->m_stateTranslation );

            switch( m_eTranslationState ) {

                case stateTranslationStartMoving:
                {
                    // add a new point into interpolator
                    m_kPositionInterpolator.AddInterpolationPoint( kNewPosition, kVelocity, CTD_ChatMember_POS_UPDATE_PERIODE );
                    m_kAvaragePositionUpdatePeriod.Reset();
                }
                break;

                case stateTranslationStopMoving:
                {
                   // add a hard destination to interpolate to with a speed of 10 units / second
                    //  this is used when client stops translating
                    m_kPositionInterpolator.AddInterpolationDestination( kNewPosition, 10.0f );
                }
                break;

                case stateTranslationMoving:
                {
                    // determine the avarage position update period, this is used for calculation of maximal interpolation time below
                    m_kAvaragePositionUpdatePeriod.AddUpdate();
                    float fAvarageUpdatePeriod = m_kAvaragePositionUpdatePeriod.GetAvarageUpdatePeriod();
                    // add a new point into interpolator, this interpolation time should be adaptive ( depends on ping time, latency etc. )
                    m_kPositionInterpolator.AddInterpolationPoint( kNewPosition, kVelocity, fAvarageUpdatePeriod + 0.5f ); // we add a tollerance of 0.5 sec
                }
                break;

                default:
                    assert( NULL && "unhandled movement state" );

            }
        
        }
        break;

        case CTD_NM_ChatMember_DT_ROTATION: // sent by server received by clients 
        {

            assert( ( m_eGameMode == stateCLIENT ) && " a non-client node got client-specific packet 'CTD_NM_ChatMember_DT_POSITION'!" );

            static tCTD_NM_ChatMemberRotation *pkReceivedRotMsg;
            pkReceivedRotMsg = ( tCTD_NM_ChatMemberRotation* )pkNetMsg;

            Vector3d                kNewRotation, kVelocity;

            kNewRotation.x          = pkReceivedRotMsg->m_fRotX;
            kNewRotation.y          = pkReceivedRotMsg->m_fRotY;

            kVelocity.x             = pkReceivedRotMsg->m_fVelX;
            kVelocity.y             = pkReceivedRotMsg->m_fVelY;

            m_eRotationState        = ( _rotationState )( pkReceivedRotMsg->m_stateRotation );

            switch( m_eRotationState ) {

                case stateRotationStartMoving:
                {
                    // add a new point into interpolator
                    m_kRotationInterpolator.AddInterpolationPoint( kNewRotation, kVelocity, CTD_ChatMember_ROT_UPDATE_PERIODE );
                    m_kAvarageRotationUpdatePeriod.Reset();
                }
                break;

                case stateRotationStopMoving:
                {
                   // add a hard destination to interpolate to with a speed of 10 units / second
                    //  this is used when client stops rotating
                    m_kRotationInterpolator.AddInterpolationDestination( kNewRotation, 10.0f );
                }
                break;

                case stateRotationMoving:
                {
                    // determine the avarage rotation update period, this is used for calculation of maximal interpolation time below
                    m_kAvarageRotationUpdatePeriod.AddUpdate();
                    float fAvarageUpdatePeriod = m_kAvarageRotationUpdatePeriod.GetAvarageUpdatePeriod();
                    // add a new point into interpolator, this interpolation time should be adaptive ( depends on ping time, latency etc. )
                    m_kRotationInterpolator.AddInterpolationPoint( kNewRotation, kVelocity, fAvarageUpdatePeriod + 0.5f ); // we add a tollerance of 0.5 sec
                }
                break;

                default:
                    assert( NULL && "unhandled movement state" );

            }
        
        }        
        break;
        //---------------------------------------------------------------------------------//

        // unknown message
        default:

            CTDCONSOLE_PRINT( LogLevel( WARNING ), " *** warning: (Plugin Chat) entity ' ChatMember ' received unknown network client object message type! " );

            break;

    }

}

template< class InterpolatorT, class InterpolatorRotT >
void CTDChatMember< InterpolatorT, InterpolatorRotT >::SendInitDataToRemoteClients( tCTD_NM_InitRemoteChatMember *pkInitData )
{

    // fill in the init data for remote clients
    
    // set the right packet size
    pkInitData->m_lDataLength = sizeof( tCTD_NM_InitRemoteChatMember );
    // set mesh file name
    if ( m_pkMesh && ( m_pkMesh->GetMesh()->GetName().length() > 0 ) ) {

        strcpy( pkInitData->m_pcMeshFile, m_pkMesh->GetMesh()->GetName().c_str() );

    } else {

        strcpy( pkInitData->m_pcMeshFile, "No Mesh defined!" );

    }

    // set current position
    pkInitData->m_fPosX         = m_kActualPosition.x;
    pkInitData->m_fPosY         = m_kActualPosition.y;
    pkInitData->m_fPosZ         = m_kActualPosition.z;

    // set current rotation
    pkInitData->m_fRotX         = m_kActualRotation.x;
    pkInitData->m_fRotY         = m_kActualRotation.y;
    pkInitData->m_fRotZ         = m_kActualRotation.z;

}

template< class InterpolatorT, class InterpolatorRotT >
int CTDChatMember< InterpolatorT, InterpolatorRotT >::Message( int iMsgId, void *pMsgStruct ) 
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
            // send the text over network to other chat members
            static tCTD_NM_SendChatMsg  kChatMsg;

            kChatMsg.m_usNetworkID      = GetNetworkID();
            kChatMsg.m_ucDataType       = CTD_NM_ChatMember_CHAT_MSG_REQUEST;

            wchar_t *pcText = ( wchar_t* )pMsgStruct;
            // copy and convert the wide character sting to multi-byte string in send buffer
            wcstombs(  kChatMsg.m_pcChatText, pcText, 256 );

#ifndef GUI_TEST

            bool bSuccess = m_pkNetworkDevice->SendClient(
                kChatMsg.m_usNetworkID,
                ( char* )&kChatMsg, 
                sizeof( tCTD_NM_SendChatMsg ), 
                HIGH_PRIORITY, 
                RELIABLE_ORDERED, 
                1
                );

#endif

        }
        break;

        default:
            break;

    }

    return 0; 
}   


template< class InterpolatorT, class InterpolatorRotT >
int CTDChatMember< InterpolatorT, InterpolatorRotT >::ParameterDescription( int iParamIndex, ParameterDescriptor *pkDesc )
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
