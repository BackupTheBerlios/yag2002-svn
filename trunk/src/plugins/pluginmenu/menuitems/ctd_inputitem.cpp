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
 # neoengine, input item for 3d menu
 #
 #
 #   date of creation:  05/14/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>
#include "ctd_inputitem.h"
#include "../3dfonts/ctd_fontmgr.h"
#include <ctd_printf.h>

using namespace std;
using namespace CTD;
using namespace NeoEngine;

namespace CTD_IPluginMenu
{

// plugin global entity descriptor for input item
CTDMenuInputItemDesc g_pkCTDMenuInputItemEntity_desc;
//-------------------------------------------//

// gap between two 3d letters ( in % of letter width )
#define CTD_LETTER_GAP				0.05f
// default maximal characters for this input item
#define CTD_DEFAULT_MAX_CHARS		10

CTDMenuInputItem::CTDMenuInputItem()
{

	CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Menu) entity ' MenuInputItem ' created " );

	CTDMenuItem::m_kPosition			= Vector3d( 0, 0, 0 );
	CTDMenuItem::m_kRotation			= Vector3d( 0, 0, 0 );
	CTDMenuItem::m_pkMesh				= NULL;
	m_uiInputLetterIndex				= 0;
	m_fTextSize							= 0;
	m_fSpaceGap							= 0.1f;
	m_uiMaxChars						= CTD_DEFAULT_MAX_CHARS;
	m_strInputText						= "empty";
	m_fDistanceOnFocused				= CTD_DISTANCE_TO_CAMERA_WHEN_FOCUSED;

	// set item type
	CTDMenuItem::SetType( CTDMenuItem::eInput );

	// set the entity ( node ) name
	SetName( CTD_ENTITY_NAME_MenuInputItem );

}

CTDMenuInputItem::~CTDMenuInputItem()
{

	CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Menu) entity ' MenuInputItem ' destroyed " );

}

// init entity
void CTDMenuInputItem::Initialize() 
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
void CTDMenuInputItem::PostInitialize()
{
	
	CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Menu) entity ' MenuInputItem::" + GetInstanceName() + "', post-initializing ... " );
	CTDMenuItem::PostInitialize();

	// if something went wrong durin father class initialization then
	//  this object will be deactivated
	if ( IsActive() == false ) {

		return;

	}

	// try to find the 3d font manager
    BaseEntity	*pkFontManager = Framework::Get()->FindEntity( CTD_ENTITY_NAME_3DFontManager );
	if ( pkFontManager == NULL ) {

		CTDCONSOLE_PRINT( LogLevel( ERROR ), " (Plugin Menu) entity ' MenuInputItem::" + GetInstanceName() + "', cannot find 3d font manager. there must be single instance of it! deactivating enitiy " );
		Deactivate();
		return;
	}

	// get 3d font
	tCTD3DFont				k3DFont;
	k3DFont.m_pvpkLetters	= NULL;
	pkFontManager->Message( CTD_3DFONTMGR_GETFONT, &k3DFont );
	if ( k3DFont.m_pvpkLetters == NULL ) {

		CTDCONSOLE_PRINT( LogLevel( ERROR ), " (Plugin Menu) entity ' MenuInputItem::" + GetInstanceName() + "', internal error: invalid font list! deactivating enitiy " );
		Deactivate();
		return;

	}
	m_pvpkLetters = k3DFont.m_pvpkLetters;
	m_fSpaceGap	= k3DFont.m_fSpaceSize;

	// setup the initial text
	for ( unsigned int uiLetterCnt = 0; uiLetterCnt < m_strInputText.length(); uiLetterCnt++ ) {

		AddLetterTo3DText( GetLetterMeshNode( m_strInputText[ uiLetterCnt ] ) );

	}

}

void CTDMenuInputItem::OnBeginFocus() 
{

	if ( IsActive() == false ) {

		return;

	}

	// call father class' OnBeginFocus function to set some internal states
	CTDMenuItem::OnBeginFocus();

	m_fBlendFactor	= 0;
	m_eState		= eStartFocus;
	m_kDestPos		= ( Camera::GetActive()->GetTranslation() + Camera::GetActive()->GetWorldRotation() * Vector3d( 0, 0, -m_fDistanceOnFocused ) );

}

void CTDMenuInputItem::OnEndFocus() 
{

	if ( IsActive() == false ) {

		return;

	}

	// call father class' OnEndFocus function to set some internal states
	CTDMenuItem::OnEndFocus();

	m_fBlendFactor	= 0;
	m_eState		= eLostFocus;

}

void CTDMenuInputItem::OnInput( CTDMenuControl::CTDCtrlKeys eCtrlKey, unsigned int iKeyData, char cKeyData ) 
{

	if ( IsActive() == false ) {

		return;

	}

	if ( ( iKeyData >= KC_A ) && ( iKeyData <= KC_Z ) ) {

		if ( eCtrlKey == CTDMenuControl::eSHIFT ) {
		
			m_uiInputLetterIndex = iKeyData - KC_A + 'A';

		} else {

			m_uiInputLetterIndex = iKeyData - KC_A + 'a';

		}

		m_bInputDispatched = false;

	} else {

		if ( iKeyData == KC_SPACE ) {

			m_uiInputLetterIndex = KC_SPACE;
			m_bInputDispatched = false;
		
		} else {

			if ( iKeyData == KC_BACKSPACE ) {

				m_uiInputLetterIndex = KC_BACKSPACE;
				m_bInputDispatched = false;

			} else { // handle special characters as !%& etc.

				if ( ( iKeyData >= KC_DOT ) && ( iKeyData <= KC_AT ) ) {

					m_uiInputLetterIndex = cKeyData;
					m_bInputDispatched = false;

				}

			}

		}

	} 

}

void CTDMenuInputItem::UpdateEntity( float fDeltaTime ) 
{ 
	
	switch ( m_eState ){

		case eIdle:

			break;

		case eStartFocus:
		{
			m_kCurrentPos = m_kCurrentPos + ( m_kDestPos - m_kCurrentPos ) * m_fBlendFactor;

			if ( m_fBlendFactor > 1.0f ) {

				m_fBlendFactor	= 0;
				m_eState		= eEditing;

			}
			m_fBlendFactor += fDeltaTime;

			SetTranslation( m_kCurrentPos );
		}
		break;

		case eEditing:
		{

			if ( m_bInputDispatched == false ) {
				
				if ( m_uiInputLetterIndex == KC_BACKSPACE ) {

					DelLetter();
					m_uiInputLetterIndex = 0;
					break;

				}

				// space is defined by a NULL as mesh pointer
				if ( ( char )m_uiInputLetterIndex == KC_SPACE ) {

					AddLetterTo3DText( NULL );

				} else {

					SceneNode	*pkLetterNode = GetLetterMeshNode( ( char )m_uiInputLetterIndex );

					// skip unsupported letters
					if ( pkLetterNode != NULL ) {

						// add and format new letter to 3d text
						AddLetterTo3DText( pkLetterNode );

					}

				}

				m_uiInputLetterIndex = 0;

			}
		
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

	m_bInputDispatched = true;

}

bool CTDMenuInputItem::Render( Frustum *pkFrustum, bool bForce ) 
{

	m_kRotationMatrix;
	if ( m_eState != eIdle ) {

		// z-align the rotation to camera
		Vector3d kZAxis = Camera::GetActive()->GetWorldTranslation() - GetTranslation(); 
		kZAxis.Normalize();
		Vector3d kXAxis = Vector3d( 0, 1, 0 ) % kZAxis;
		kXAxis.Normalize();
		Vector3d kYAxis = kZAxis % kXAxis;
		kYAxis.Normalize();
		m_kRotationMatrix[0][0] = kXAxis[0];
		m_kRotationMatrix[1][0] = kXAxis[1];
		m_kRotationMatrix[2][0] = kXAxis[2];
		m_kRotationMatrix[3][0] = 0.0f;
		m_kRotationMatrix[0][1] = kYAxis[0];
		m_kRotationMatrix[1][1] = kYAxis[1];
		m_kRotationMatrix[2][1] = kYAxis[2];
		m_kRotationMatrix[3][1] = 0.0f;
		m_kRotationMatrix[0][2] = kZAxis[0];
		m_kRotationMatrix[1][2] = kZAxis[1];
		m_kRotationMatrix[2][2] = kZAxis[2];
		m_kRotationMatrix[3][2] = 0.0f;
		m_kRotationMatrix[0][3] = 0.0f;
		m_kRotationMatrix[1][3] = 0.0f;
		m_kRotationMatrix[2][3] = 0.0f;
		m_kRotationMatrix[3][3] = 1.0f;
		SetRotation( Quaternion( m_kRotationMatrix ) );

	}

	// first render the father mesh
	CTDMenuItem::Render( pkFrustum, bForce );

	// now render the letters
	//----------------------------------------------------------//
	SceneNode				*pkMeshNode;
	size_t					uiLetters = m_vpkFormatted3DText.size();

	Vector3d				kPosition = GetTranslation();
	Vector3d				kLetterPosition;

	for ( size_t uiLetterCnt = 0; uiLetterCnt < uiLetters; uiLetterCnt++ ) {

		pkMeshNode			= m_vpkFormatted3DText[ uiLetterCnt ]->m_pkMeshNode;
		kLetterPosition		= m_vpkFormatted3DText[ uiLetterCnt ]->m_kPositionOffset;

		// NULL defines a space character
		if ( pkMeshNode == NULL ) {

		} else {

			pkMeshNode->SetRotation( m_kRotationMatrix );
			pkMeshNode->SetTranslation( kPosition + m_kRotationMatrix * kLetterPosition );
			pkMeshNode->Render( NULL, true ); // force rendering of the same letter several times!

		}

	}

	return true;

}


// given a letter this functions returns the corresponding 3d mesh
SceneNode* CTDMenuInputItem::GetLetterMeshNode( const char &cLetter )
{

	SceneNode		*pkNode = NULL;

	size_t uiLetters = m_pvpkLetters->size();
	for ( size_t uiLetterCnt = 0; uiLetterCnt < uiLetters; uiLetterCnt++ ) {

		if ( ( *m_pvpkLetters )[ uiLetterCnt ]->m_cLetter == cLetter ) {

			pkNode = ( *m_pvpkLetters )[ uiLetterCnt ]->m_pkMeshNode;

			break;
		}

	}

	return pkNode;

}

// add and format a new letter to 3d text
void CTDMenuInputItem::AddLetterTo3DText( NeoEngine::SceneNode* pkLetterNode )
{

	size_t uiLetters = m_vpkFormatted3DText.size();

	if ( uiLetters >= m_uiMaxChars ) {

		return;

	}

	CTD3DInputText	*pkTextNode = new CTD3DInputText;
	pkTextNode->m_pkMeshNode	= pkLetterNode;

	// a letter node of NULL means a space
	if ( pkLetterNode ) {
		
		pkTextNode->m_fLetterWidth = ( ( AABB* )pkLetterNode->GetBoundingVolume() )->GetDim().x * ( 2.0f + CTD_LETTER_GAP );

	} else {

		pkTextNode->m_fLetterWidth = m_fSpaceGap;

	}

	// recalculate the text size
	m_fTextSize += pkTextNode->m_fLetterWidth;

	m_vpkFormatted3DText.push_back( pkTextNode );

	// format the text
	Format3DText();


}

void CTDMenuInputItem::DelLetter()
{

	size_t uiLetters = m_vpkFormatted3DText.size();
	if ( uiLetters < 1 ) {

		return;

	}

	CTD3DInputText	*pkTextNode = m_vpkFormatted3DText[ uiLetters - 1 ];

	// recalculate the text size
	m_fTextSize -= pkTextNode->m_fLetterWidth;

	// delete the last letter mesh in 3d text
	m_vpkFormatted3DText.pop_back();
	delete pkTextNode;

	// format the text
	Format3DText();

}

// middle-align the text
void CTDMenuInputItem::Format3DText()
{
	
	float	fPositioning = -( m_fTextSize * 0.5f );

	// format all existing letters ( middle aligned )
	size_t uiLetters = m_vpkFormatted3DText.size();
	for ( size_t uiLetterCnt = 0; uiLetterCnt < uiLetters; uiLetterCnt++ ) {

		float	&rfCurrOffset = m_vpkFormatted3DText[ uiLetterCnt ]->m_kPositionOffset.x;
		float	&rfCurrWidth  = m_vpkFormatted3DText[ uiLetterCnt ]->m_fLetterWidth;
		fPositioning	+= rfCurrWidth;
		rfCurrOffset	= fPositioning - rfCurrWidth * 0.5f;

	}

}

int	CTDMenuInputItem::Message( int iMsgId, void *pkMsgStruct ) 
{ 

	// check wether we are initialized properly
	if ( IsActive() == false ) {

		return -1;

	}

	switch ( iMsgId ) {

		// set the input string ( e.g. after loading the game settings )
		//  a void-pointer casted std string in pMsgStruct is expected
		case CTD_INPUTITEM_SET_STRING:
		{

			string	*pkString	= ( string* )pkMsgStruct;
			m_strInputText		= *pkString;

		}
		break;
	
		// get the input string
		//  a void-pointer casted std string in pMsgStruct is expected
		case CTD_INPUTITEM_GET_STRING:
		{

			*( ( string* )pkMsgStruct ) = m_strInputText;

		}
		break;

		default:
			break;

	}

	return 0; 

}	


int	CTDMenuInputItem::ParameterDescription( int iParamIndex, ParameterDescriptor *pkDesc )
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
		pkDesc->SetName( "DistOnFocus" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_FLOAT );
		pkDesc->SetVar( &m_fDistanceOnFocused );
		
		break;

	case 1:
		pkDesc->SetName( "MaxChars" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_INTEGER );
		pkDesc->SetVar( &m_uiMaxChars );
		
		break;

	default:
		return -1;
	}

	return iParamCount;

}

}
