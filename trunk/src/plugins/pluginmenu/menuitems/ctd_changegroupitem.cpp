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
 # neoengine, item for changing to another menu section ( group )
 #
 #
 #   date of creation:  05/31/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>
#include "ctd_changegroupitem.h"
#include "../ctd_menucam.h"
#include <ctd_animutil.h>
#include <ctd_printf.h>

using namespace std;
using namespace CTD;
using namespace NeoEngine;

namespace CTD_IPluginMenu
{

// plugin global entity descriptor for change group item
CTDMenuChangeGroupItemDesc g_pkCTDMenuChangeGroupItemEntity_desc;
//-------------------------------------------//

extern CTDPrintf    g_CTDPrintfBuffered;

CTDMenuChangeGroupItem::CTDMenuChangeGroupItem()
{

    CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Menu) entity ' MenuChangeGroupItem ' created " );
    
    m_pkPathAnim    = NULL;
    m_iDestGroupID  = -1;
    m_iCheckLastKey = 1;
    m_eState        = eIdle;
    m_pkCamera      = NULL;
    m_fPosVar       = 0;

    // set item type
    CTDMenuItem::SetType( CTDMenuItem::eChangeGroup );

    // set the entity ( node ) name
    SetName( CTD_ENTITY_NAME_MenuChangeGroupItem );

}

CTDMenuChangeGroupItem::~CTDMenuChangeGroupItem()
{

    CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Menu) entity ' MenuChangeGroupItem ' destroyed " );

}

// init entity
void CTDMenuChangeGroupItem::Initialize() 
{ 

    // let father class initialize first
    CTDMenuItem::Initialize();


    if ( m_iDestGroupID == -1 ) {

        CTDCONSOLE_PRINT( LogLevel( WARNING ), " (Plugin Menu) entity ' MenuChangeGroupItem::" + GetInstanceName() + 
            " ', parameter 'ChangeToGroupID' is not set! deactivating entity " );
        Deactivate();
        return;

    }

    // try to load the animation file
    KeyFrameAnimUtil    kKfUtil;        

    File* pkFile = NeoEngine::Core::Get()->GetFileManager()->GetByName( m_strPathAnim );
    if ( !pkFile || ( kKfUtil.Load( pkFile ) == false ) ) {

        CTDCONSOLE_PRINT( LogLevel( WARNING ), " (Plugin Menu) entity ' MenuChangeGroupItem::" + GetInstanceName() +
            " ', cannot find animation file " );
        Deactivate();
        return;

    }

    m_pkPathAnim     = new AnimatedNode( *kKfUtil.GetAnimatedNode() );
    m_iCheckLastKey  = ( int )m_pkPathAnim->GetAnimation()->m_vpkKeyframes.size() - 1;

    // set initial position and orientation
    SetTranslation( m_kPosition );

	// set initial position and orientation
	SetTranslation( m_kPosition );
	Quaternion	kRot( EulerAngles( m_kRotation.x * PI / 180.0f, m_kRotation.y * PI / 180.0f, m_kRotation.z * PI / 180.0f ) );
	SetRotation( kRot );

}

// post-init entity
void CTDMenuChangeGroupItem::PostInitialize()
{

    m_pkCamera = Framework::Get()->FindEntity( CTD_ENTITY_NAME_MenuCamera );
    if ( m_pkCamera == NULL ) {

        CTDCONSOLE_PRINT( LogLevel( WARNING ), "CTDMenuChangeGroupItem::" + GetInstanceName() + " : could not find camera entity!" );
        Deactivate();
        return;

    } 

}

void CTDMenuChangeGroupItem::OnBeginFocus()
{

    CTDMenuItem::OnBeginFocus();
    m_eState        = eFocus;
    m_fPosVar       = 0;
    m_kCurrPosition = m_kPosition;

}

void CTDMenuChangeGroupItem::OnEndFocus()
{

    CTDMenuItem::OnEndFocus();
    SetTranslation( m_kPosition );

}

void CTDMenuChangeGroupItem::OnActivate()
{

    if ( IsActive() == false ) {

        CTDPRINTF_PRINT( g_CTDPrintfBuffered, "CTDMenuChangeGroupItem::" + GetInstanceName() + " warning, item is invalid! cannot change group" );
        return;
    }

    // begin the path animation
    m_eState        = eAnim;
    m_pkPathAnim->GetAnimation()->m_iNextKeyframe = 1;

}

void CTDMenuChangeGroupItem::UpdateEntity( float fDeltaTime ) 
{

    switch ( m_eState ) {

        case eIdle:
        {
        }
        break;

        case eFocus:
        {

            if ( IsFocused() == false ) {

                break;

            }
            m_fPosVar += fDeltaTime;
            if ( m_fPosVar > ( 2.0f * PI ) ) {

                m_fPosVar -= ( 2.0f * PI );

            }
            m_kCurrPosition = m_kPosition + Vector3d( 0, 0.3f * cosf( m_fPosVar ), 0 );
            SetTranslation( m_kCurrPosition );

        }
        break;

        case eAnim:
        {
            // as neoengine currently does not provide functions to check the current animation blending state so we have to do that manually!
            NodeAnimation* pkNodeAnim = m_pkPathAnim->GetAnimation();

            if ( pkNodeAnim->m_iNextKeyframe < m_iCheckLastKey ) {

                m_pkPathAnim->Update( fDeltaTime );
                m_pkCamera->SetTranslation( m_pkPathAnim->GetTranslation() );
                m_pkCamera->SetRotation( m_pkPathAnim->GetRotation() );

            } else {

                m_eState                        = eIdle;
                pkNodeAnim->m_fCurTime          = 0;

            }

        }
        break;

        default:
            break;

    }

}

bool CTDMenuChangeGroupItem::Render( Frustum *pkFrustum, bool bForce ) 
{

    return CTDMenuItem::Render( pkFrustum, bForce );

}

int CTDMenuChangeGroupItem::Message( int iMsgId, void *pkMsgStruct ) 
{ 

    // check wether we are initialized properly
    if ( IsActive() == false ) {

        return -1;

    }

    switch ( iMsgId ) {

        // this message retrieves the destination group id
        case CTD_CHANGEGROUP_GET_ID:
        {

            return m_iDestGroupID;

        }
        // this message returns 1 if destination position reached ( i.e. if the anim blend factor is 1 )
        case CTD_CHANGEGROUP_CAM_IN_POS:
        {

            if ( m_eState == eIdle ) {

                return 1;

            } else {

                return 0;

            }

        }
        break;
    
        default:
            break;

    }

    return 0; 

}   

int CTDMenuChangeGroupItem::ParameterDescription( int iParamIndex, ParameterDescriptor *pkDesc )
{

    // get the param count of father class
    int iGeneralParamCount = CTDMenuItem::ParameterDescription( 0, NULL );
    int iParamCount = iGeneralParamCount + 2;

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
        pkDesc->SetName( "ChangeToGroupID" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_INTEGER );
        pkDesc->SetVar( &m_iDestGroupID );
        
        break;

    case 1:
        pkDesc->SetName( "Path" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_STRING );
        pkDesc->SetVar( &m_strPathAnim );
        
        break;

    default:
        return -1;
    }

    return iParamCount;

}

}
