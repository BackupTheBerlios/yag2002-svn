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


using namespace std;
using namespace CTD;
using namespace NeoEngine;
using namespace NeoChunkIO;


namespace CTD_IPluginMenu
{

// plugin global entity descriptor for menu camera
CTDMenuCameraDesc	g_pkCTDMenuCameraEntity_desc;
//-------------------------------------------//


CTDMenuCamera::CTDMenuCamera()
{

	CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Menu) entity ' MenuCamera ' created " );

	m_kPosition					= Vector3d( 0, 0, 0 );
	m_kRotation					= Vector3d( 0, 0, 0 );
	m_fFOVAngle					= 60.0f;
	m_fNearplane				= 0.1f;
	m_fFarplane					= 10000.0f;
	m_pkPathAnim				= NULL;
	m_fAnimBlend				= 0;
	m_eState					= eIdle;
	m_bHasAnimation				= false;
	m_fAnimLength				= 0;


	// set the entity ( node ) name
	SetName( CTD_ENTITY_NAME_MenuCamera );
	// set the instance name as the entity name in order to be able to search for this camera in other entities
	//  such as change group entity ( which controls the camera position / rotation )
	SetInstanceName( CTD_ENTITY_NAME_MenuCamera );

	// create camera light
	m_pkLight							= new Light( Light::POINT, Light::CASTSHADOWS );
	m_pkLight->m_kAmbient				= Color( 0.5f, 0.5f, 0.5f );
	m_pkLight->m_kDiffuse				= Color( 0.8f, 0.8f, 0.7f );
	m_pkLight->m_kSpecular				= Color( 0.9f, 0.9f, 0.9f );	
	m_pkLight->m_fConstantAttenuation	= 0.3f;
	m_pkLight->m_fLinearAttenuation		= 0.001f;
	m_pkLight->m_fQuadraticAttenuation	= 0.00001f;

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
	AABB	kBB;
	kBB.SetDim( Vector3d( 5.0f, 5.0f, 5.0f ) );
	GetBoundingVolume()->Generate( &kBB );

	// set initial position and orientation
	SetTranslation( m_kPosition );
	Quaternion	kRot( EulerAngles( m_kRotation.x * PI / 180.0f, m_kRotation.y * PI / 180.0f, m_kRotation.z * PI / 180.0f ) );
	SetRotation( kRot );

	// get menu's camera object created in framework
	m_pkCamera = GetMenuLevelSet()->GetCamera();
	assert( m_pkCamera );

	// set initial camera position and orientation
	m_pkCamera->SetTranslation( GetTranslation() );
	m_pkCamera->SetRotation( GetRotation() );

	// set fov and near/far distance
    Framework::Get()->GetRenderDevice()->SetPerspectiveProjection( m_fFOVAngle, m_fNearplane, m_fFarplane );


	// attach camera node as global node into menu's room
    GetMenuLevelSet()->GetRoom()->AttachGlobalNode( this );

	// setup the menu entrance animation
	if ( m_strEntranceAnim.length() > 0 ) {

		m_pkPathAnim = ReadKeyframes( m_strEntranceAnim );
		if ( m_pkPathAnim == NULL ) {

			CTDCONSOLE_PRINT( LogLevel( WARNING ), " (Plugin Menu) entity ' CTDMenuCamera::" + GetInstanceName() +
				" ', cannot find entrance animation file, using fix position." );

			m_bHasAnimation = false;
		
		} else {

			m_bHasAnimation = true;

		}


	}

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
			NodeAnimation* pkNodeAnim =	m_pkPathAnim->GetAnimation();
			m_fAnimBlend += fDeltaTime / pkNodeAnim->m_fLength;			

			if ( m_fAnimBlend < 0.95f ) {

				m_pkPathAnim->Update( fDeltaTime );
				SetTranslation( m_pkPathAnim->GetTranslation() );
				SetRotation( m_pkPathAnim->GetRotation() );

			} else {

				m_eState						= eIdle;
				m_fAnimBlend					= 0;
				pkNodeAnim->m_fCurTime			= 0;

				// set position and orientation to last key frame
				NodeKeyframe *pkLastKeyframe	= pkNodeAnim->m_vpkKeyframes[ pkNodeAnim->m_vpkKeyframes.size() - 1 ];
				SetTranslation( pkLastKeyframe->m_kTranslation );
				SetRotation( pkLastKeyframe->m_kRotation );

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

int	CTDMenuCamera::Message( int iMsgId, void *pMsgStruct ) 
{ 

	switch ( iMsgId ) {

		// handle pausing ( e.g. when entering the menu )
		case CTD_ENTITY_ENTER_MENU:

			// reset menu's fov and near/far distance
            Framework::Get()->GetRenderDevice()->SetPerspectiveProjection( m_fFOVAngle, m_fNearplane, m_fFarplane );

			if ( m_bHasAnimation == true ) {

				m_eState	= eAnim;

			} else {

				m_eState	= eIdle;
			
			}
			break;

		case CTD_ENTITY_EXIT_MENU:

			break;

		default:
			break;

	}

	return 0; 
}	


// read the keyframes stored in an ".nani" file
AnimatedNode* CTDMenuCamera::ReadKeyframes( const string &strFileName )
{

    File	*pkFile = NeoEngine::Core::Get()->GetFileManager()->GetByName( strFileName );
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
		return NULL;
	}

    pkFile->Read( szStr, 4 ); szStr[4] = 0;

	if( string( szStr ) != "NANI" )
	{
		neolog << LogLevel( ERROR ) << "*** Unable to load animation lib: Invalid ID" << endl;
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
		return NULL;
	}

	*pkFile >> pkIO->m_iMajorVersion >> pkIO->m_iMinorVersion;

	if( !( pkIO->m_iMajorVersion == ChunkIO::MAJORVERSIONREQUIRED ) || ( pkIO->m_iMinorVersion < ChunkIO::MINORVERSIONREQUIRED ) )
	{
		neolog << LogLevel( ERROR ) << "*** Unable to load animation lib: Invalid chunk format version " << pkIO->m_iMajorVersion << "." << pkIO->m_iMinorVersion << endl;
		return NULL;
	}


	Chunk				*pkChunk				= NULL;
	AnimatedNode		*pkAnimNode				= NULL;

	// read in the animation keyframes ( only the first set! )
	pkChunk = pkIO->ReadChunk( pkFile );
	if( pkChunk != NULL )
	{		
		string	strName;
		int		iID = -1;
		float	fLength = 0.0f;

		vector< Chunk* > vpkSubChunks = pkChunk->GetSubChunks();
		// pick the first chunk attributes: name, id, and length
		for ( unsigned int uiChunks = 0; uiChunks < 3; uiChunks++ ) {

			Chunk	*pkSChunk = vpkSubChunks[ uiChunks ];
			
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

			SceneNodeChunk	*pkSceneNode = dynamic_cast< SceneNodeChunk* >( pkChunk );
			if( pkIO->ParseChunk( pkSceneNode, 0, 0 ) >= 0 ) {

				pkAnimNode = dynamic_cast< AnimatedNode* >(pkSceneNode->m_pkNode);

				// set animation parameters name and length
				if ( pkAnimNode ) {

					pkAnimNode->SetAnimation( iID );
					NodeAnimation * pkAnimTrack = pkAnimNode->GetAnimation( iID );
					pkAnimTrack->m_fLength  = fLength;
					pkAnimTrack->m_strName  = strName;
					m_fAnimLength			= fLength;

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

int	CTDMenuCamera::ParameterDescription( int iParamIndex, ParameterDescriptor *pkDesc )
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
		pkDesc->SetVar( &m_kPosition );
		
		break;

	case 1:
		pkDesc->SetName( "EntranceAnim" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_STRING );
		pkDesc->SetVar( &m_strEntranceAnim );
		
		break;

	case 2:
		pkDesc->SetName( "Rotation" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_VECTOR3 );
		pkDesc->SetVar( &m_kRotation );
		
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


	default:
		return -1;
	}

	return iParamCount;

}

}
