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

#include <ctd_printf.h>

using namespace std;
using namespace CTD;
using namespace NeoEngine;
using namespace NeoChunkIO;

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
    m_fAnimBlend    = 0;
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

    m_pkPathAnim = ReadKeyframes( m_strPathAnim );
    if ( m_pkPathAnim == NULL ) {

        CTDCONSOLE_PRINT( LogLevel( WARNING ), " (Plugin Menu) entity ' MenuChangeGroupItem::" + GetInstanceName() +
            " ', cannot find animation file " );
        Deactivate();
        return;
    }

    // set initial position and orientation
    SetTranslation( m_kPosition );
    Quaternion  kRot( EulerAngles( m_kRotation.x * PI / 180.0f, m_kRotation.y * PI / 180.0f, m_kRotation.z * PI / 180.0f ) );
    SetRotation( kRot );

    // set the rotation offset
    m_kRotOffset    = Quaternion( EulerAngles( m_kRotationOffset.x * PI / 180.0f, m_kRotationOffset.y * PI / 180.0f, m_kRotationOffset.z * PI / 180.0f ) );

}

// post-init entity
void CTDMenuChangeGroupItem::PostInitialize()
{

    // set the initial position of item marker
    //  attention, you must use menu's framework for finding the camera entity
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
    m_fAnimBlend    = 0;
    m_eState        = eAnim;

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
            m_fAnimBlend += fDeltaTime / pkNodeAnim->m_fLength;         

            if ( m_fAnimBlend < 0.95f ) {

                m_pkPathAnim->Update( fDeltaTime );
                m_pkCamera->SetTranslation( m_pkPathAnim->GetTranslation() );
                m_pkCamera->SetRotation( m_kRotOffset * m_pkPathAnim->GetRotation() );

            } else {

                m_eState                        = eIdle;
                m_fAnimBlend                    = 0;
                pkNodeAnim->m_fCurTime          = 0;

                // set position and orientation to last key frame
                NodeKeyframe *pkLastKeyframe    = pkNodeAnim->m_vpkKeyframes[ pkNodeAnim->m_vpkKeyframes.size() - 1 ];
                m_pkCamera->SetTranslation( pkLastKeyframe->m_kTranslation );
                m_pkCamera->SetRotation( m_kRotOffset * pkLastKeyframe->m_kRotation );

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

// read the keyframes stored in an ".nani" file
AnimatedNode* CTDMenuChangeGroupItem::ReadKeyframes( const string &strFileName )
{

    File    *pkFile = NeoEngine::Core::Get()->GetFileManager()->GetByName( strFileName );
    if ( pkFile == NULL ) {

        pkFile = new File;

    }
    if ( pkFile->Open( "", strFileName, ios_base::in | ios_base::binary ) == false ) {

        CTDCONSOLE_PRINT( LogLevel( WARNING ), " (Plugin Visuals) entity ' AnimatedMesh ': could not find keyframe file! " );
        delete pkFile;
        return NULL;

    }

    char   szStr[5];
    string strMode;

    if( !pkFile->DetermineByteOrder( 0x494e414e ) )
    {
        neolog << LogLevel( ERROR ) << "*** Unable to load animation lib: Failed to determine byte order, possible corrupt file" << endl;
        delete pkFile;
        return NULL;
    }

    pkFile->Read( szStr, 4 ); szStr[4] = 0;

    if( string( szStr ) != "NANI" )
    {
        neolog << LogLevel( ERROR ) << "*** Unable to load animation lib: Invalid ID" << endl;
        delete pkFile;
        return NULL;
    }

    //Read mode string
    pkFile->Read( szStr, 4 );

    ChunkIO *pkIO = 0;

    if( string( szStr ) == "!bin" )
    {
        pkIO = new ChunkIO( ChunkIO::BINARY );
        pkFile->SetBinary( true );
    }
    else if( string( szStr ) == "!txt" )
    {
        pkIO = new ChunkIO( ChunkIO::ASCII );
        pkFile->SetBinary( false );
    }
    else
    {
        neolog << LogLevel( ERROR ) << "*** Unable to load animation lib: Unsupported chunk file mode [" << strMode << "]" << endl;
        delete pkFile;
        return NULL;
    }

    *pkFile >> pkIO->m_iMajorVersion >> pkIO->m_iMinorVersion;

    if( !( pkIO->m_iMajorVersion == ChunkIO::MAJORVERSIONREQUIRED ) || ( pkIO->m_iMinorVersion < ChunkIO::MINORVERSIONREQUIRED ) )
    {
        neolog << LogLevel( ERROR ) << "*** Unable to load animation lib: Invalid chunk format version " << pkIO->m_iMajorVersion << "." << pkIO->m_iMinorVersion << endl;
        delete pkFile;
        delete pkIO;
        return NULL;
    }


    Chunk               *pkChunk                = NULL;
    AnimatedNode        *pkAnimNode             = NULL;

    // read in the animation keyframes ( only the first set! )
    pkChunk = pkIO->ReadChunk( pkFile );
    if( pkChunk != NULL )
    {       
        string  strName;
        int     iID = -1;
        float   fLength = 0.0f;

        vector< Chunk* > vpkSubChunks = pkChunk->GetSubChunks();
        // pick the first chunk attributes: name, id, and length
        for ( unsigned int uiChunks = 0; uiChunks < 3; uiChunks++ ) {

            Chunk   *pkSChunk = vpkSubChunks[ uiChunks ];
            
            if ( ( pkSChunk->GetType() == ChunkType::STRING ) && ( pkSChunk->GetID() == "name" ) ) {

                strName = ( ( StringChunk* )pkSChunk )->m_strData;
                continue;

            }

            if ( ( pkSChunk->GetType() == ChunkType::INTEGER ) && ( pkSChunk->GetID() == "id" ) ) {

                iID = ( ( IntChunk* )pkSChunk )->m_iData;
                continue;

            }

            if ( ( pkSChunk->GetType() == ChunkType::FLOAT ) && ( pkSChunk->GetID() == "length" ) ) {


                fLength = ( ( FloatChunk* )pkSChunk )->m_fData;
                continue;

            }
        }

        if( pkChunk->GetType() == ChunkType::ANIMATEDNODE ) {

            SceneNodeChunk  *pkSceneNode = dynamic_cast< SceneNodeChunk* >( pkChunk );
            if( pkIO->ParseChunk( pkSceneNode, 0, 0 ) >= 0 ) {

                pkAnimNode = dynamic_cast< AnimatedNode* >(pkSceneNode->m_pkNode);

                // set animation parameters name and length
                if ( pkAnimNode ) {

                    pkAnimNode->SetAnimation( iID );
                    NodeAnimation * pkAnimTrack = pkAnimNode->GetAnimation( iID );
                    pkAnimTrack->m_fLength = fLength;
                    pkAnimTrack->m_strName = strName;

                }

            }
            
        }

    }

    if ( pkAnimNode == NULL ) {

        // error getting the animation and its keyframes
        CTDCONSOLE_PRINT( LogLevel( WARNING ), " (Plugin Visuals) entity ' AnimatedMesh ': no keyframes exist => no animation! " );

    }

    delete pkIO;
    delete pkFile;
    
    return pkAnimNode;

}

int CTDMenuChangeGroupItem::ParameterDescription( int iParamIndex, ParameterDescriptor *pkDesc )
{

    // get the param count of father class
    int iGeneralParamCount = CTDMenuItem::ParameterDescription( 0, NULL );
    int iParamCount = iGeneralParamCount + 3;

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

    case 2:
        pkDesc->SetName( "PathRotationOffset" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_VECTOR3 );
        pkDesc->SetVar( &m_kRotationOffset );
        
        break;

    default:
        return -1;
    }

    return iParamCount;

}

}
