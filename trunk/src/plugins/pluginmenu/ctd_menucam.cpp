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

#include "base.h"
#include "ctd_menucam.h"
#include <ctd_animutil.h>

using namespace std;
using namespace CTD;
using namespace NeoEngine;
using namespace NeoChunkIO;


namespace CTD_IPluginMenu
{

// plugin global entity descriptor for menu camera
CTDMenuCameraDesc   g_pkCTDMenuCameraEntity_desc;
//-------------------------------------------//


CTDMenuCamera::CTDMenuCamera()
{

    CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Menu) entity ' MenuCamera ' created " );

    m_kPosition                 = Vector3d( 0, 0, 0 );
    m_kRotation                 = Vector3d( 0, 0, 0 );
    m_fFOVAngle                 = 60.0f;
    m_fNearplane                = 0.1f;
    m_fFarplane                 = 10000.0f;
    m_pkPathAnim                = NULL;
    m_eState                    = eIdle;
    m_bHasAnimation             = false;
    m_fAnimLength               = 0;
    m_iCheckLastKey             = 1;

    // set the entity ( node ) name
    SetName( CTD_ENTITY_NAME_MenuCamera );
    // set the instance name as the entity name in order to be able to search for this camera in other entities
    //  such as change group entity ( which controls the camera position / rotation )
    SetInstanceName( CTD_ENTITY_NAME_MenuCamera );

    // create camera light
    m_kLightColor                       = Vector3d( 0.9f, 0.9f, 0.9f );
    m_pkLight                           = new Light( Light::POINT, Light::CASTSHADOWS );
    m_pkLight->m_kAmbient               = Color( 0.0f, 0.0f, 0.0f );
    m_pkLight->m_kDiffuse               = Color( m_kLightColor.x, m_kLightColor.y, m_kLightColor.z );
    m_pkLight->m_kSpecular              = Color( 1.0f, 1.0f, 1.0f );    
    m_pkLight->m_fConstantAttenuation   = 0.01f;
    m_pkLight->m_fLinearAttenuation     = 0.01f;
    m_pkLight->m_fQuadraticAttenuation  = 0.001f;

}

CTDMenuCamera::~CTDMenuCamera()
{

    CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Menu) entity ' MenuCamera ' destroyed " );
    GetMenuLevelSet()->GetRoom()->DetachGlobalNode( this );

}

// init entity
void CTDMenuCamera::Initialize() 
{ 

    SetEntity( m_pkLight );

    // create a bounding volume for camera ( its is used by its light )
    AABB    kBB;
    kBB.SetDim( Vector3d( 5.0f, 5.0f, 5.0f ) );
    GetBoundingVolume()->Generate( &kBB );

    // set initial position and orientation
    SetTranslation( m_kPosition );
    Quaternion  kRot( EulerAngles( m_kRotation.x * PI / 180.0f, m_kRotation.y * PI / 180.0f, m_kRotation.z * PI / 180.0f ) );
    SetRotation( kRot );

    // attach camera node as global node into menu's room
    GetMenuLevelSet()->GetRoom()->AttachGlobalNode( this );

    // setup the menu entrance animation
    if ( m_strEntranceAnim.length() > 0 ) {

        // try to load the animation file
        KeyFrameAnimUtil    kKfUtil;        

        File* pkFile = NeoEngine::Core::Get()->GetFileManager()->GetByName( m_strEntranceAnim );
        if ( !pkFile || ( kKfUtil.Load( pkFile ) == false ) ) {

            CTDCONSOLE_PRINT( LogLevel( WARNING ), " (Plugin Menu) entity ' CTDMenuCamera::" + GetInstanceName() +
                " ', cannot find entrance animation file, using fix position." );

            m_bHasAnimation = false;

        }

        m_pkPathAnim = new AnimatedNode( *kKfUtil.GetAnimatedNode() );

        if ( m_pkPathAnim == NULL ) {

            CTDCONSOLE_PRINT( LogLevel( WARNING ), " (Plugin Menu) entity ' CTDMenuCamera::" + GetInstanceName() +
                " ', cannot get path animation, using fix position." );

            m_bHasAnimation = false;
        
        } else {

            m_bHasAnimation = true;

        }

    }

    // get menu's camera object created in framework
    m_pkCamera = GetMenuLevelSet()->GetCamera();
    assert( m_pkCamera );

    // set initial position and orientation to those of last key frame
    NodeAnimation* pkNodeAnim       = m_pkPathAnim->GetAnimation();
    NodeKeyframe *pkLastKeyframe    = pkNodeAnim->m_vpkKeyframes[ pkNodeAnim->m_vpkKeyframes.size() - 1 ];
    m_iCheckLastKey                 = ( int )pkNodeAnim->m_vpkKeyframes.size() - 1;

    SetTranslation( pkLastKeyframe->m_kTranslation );
    SetRotation( pkLastKeyframe->m_kRotation );

    // set initial camera position and orientation
    m_pkCamera->SetTranslation( GetTranslation() );
    m_pkCamera->SetRotation( GetRotation() );

    // assign camera light color
    m_pkLight->m_kDiffuse               = Color( m_kLightColor.x, m_kLightColor.y, m_kLightColor.z, 1.0f );

    // set fov and near/far distance
    Framework::Get()->GetRenderDevice()->SetPerspectiveProjection( m_fFOVAngle, m_fNearplane, m_fFarplane );

}

void CTDMenuCamera::UpdateEntity( float fDeltaTime ) 
{ 

    switch ( m_eState ) {

        case eIdle:

            break;

        // play the entrance animation
        case eAnim:
        {
                
            // as neoengine currently does not provide functions to check the current animation blending state so we have to do that manually!
            NodeAnimation* pkNodeAnim = m_pkPathAnim->GetAnimation();

            if ( pkNodeAnim->m_iNextKeyframe < m_iCheckLastKey ) {

                m_pkPathAnim->Update( fDeltaTime );
                SetTranslation( m_pkPathAnim->GetTranslation() );
                SetRotation( m_pkPathAnim->GetRotation() );

            } else {

                m_eState                        = eIdle;
                pkNodeAnim->m_fCurTime          = 0;

            }
        }
        break;

        default:
            ;

    }

    // update camera position and orientation
    m_pkCamera->SetTranslation( GetTranslation() );
    m_pkCamera->SetRotation( GetRotation() );

}

// render light
bool CTDMenuCamera::Render( Frustum *pkFrustum, bool bForce ) 
{

     m_pkLight->Render();
     return true;

}

int CTDMenuCamera::Message( int iMsgId, void *pMsgStruct ) 
{ 

    switch ( iMsgId ) {

        // handle pausing ( e.g. when entering the menu )
        case CTD_ENTITY_ENTER_MENU:

            // reset menu's fov and near/far distance
            Framework::Get()->GetRenderDevice()->SetPerspectiveProjection( m_fFOVAngle, m_fNearplane, m_fFarplane );

            if ( m_bHasAnimation == true ) {

                // begin the path animation
                m_eState    = eAnim;
                m_pkPathAnim->GetAnimation()->m_iNextKeyframe = 1;
                m_pkPathAnim->GetAnimation()->m_fCurTime      = 0;

            } else {

                m_eState    = eIdle;
            
            }
            break;

        case CTD_ENTITY_EXIT_MENU:

            break;

        default:
            break;

    }

    return 0; 
}   

int CTDMenuCamera::ParameterDescription( int iParamIndex, ParameterDescriptor *pkDesc )
{

    int iParamCount = 7;

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
        pkDesc->SetName( "EntranceAnim" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_STRING );
        pkDesc->SetVar( &m_strEntranceAnim );
        
        break;

    case 3:
        pkDesc->SetName( "FOV angle" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_FLOAT );
        pkDesc->SetVar( &m_fFOVAngle );
        break;

    case 4:
        pkDesc->SetName( "Nearplane" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_FLOAT );
        pkDesc->SetVar( &m_fNearplane );
        break;

    case 5:
        pkDesc->SetName( "Farplane" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_FLOAT );
        pkDesc->SetVar( &m_fFarplane );
        break;

    case 6:
        pkDesc->SetName( "HeadLightColor" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_VECTOR3 );
        pkDesc->SetVar( &m_kLightColor );
        
        break;

    default:
        return -1;
    }

    return iParamCount;

}

}
