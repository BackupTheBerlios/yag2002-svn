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
 # neoengine, Cal3D animation module 
 #
 # this module manages and synchronizes the character animation
 #  using Cal3D
 #
 #
 #   date of creation:  01/07/2003
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include "base.h"
#include "ctd_cal3danim.h"
#include <ctd_printf.h>

using namespace std;
using namespace CTD;
using namespace NeoEngine;
using namespace NeoChunkIO;

CTDCal3DAnim::CTDCal3DAnim( SceneNode *pkNode )
{

	m_pkNode		= pkNode;

	m_pCalRenderer	= NULL;
	m_blendTime		= 0.3f;
	m_fScale		= 1.0f;

	m_IdAnimIdle	= -1;
	m_IdAnimWalk	= -1;
	m_IdAnimRun		= -1;
	m_IdAnimAttack	= -1;
	m_IdAnimJump	= -1;
	m_IdAnimLand	= -1;
	m_IdAnimFreeze	= -1;

	m_bUpdatePolygonBuffer = false;

}

CTDCal3DAnim::~CTDCal3DAnim()
{

	// destroy model instance
	m_calModel.destroy();

	// destroy core model instance
	m_calCoreModel.destroy();

}

bool CTDCal3DAnim::Initialize( const string &strCfgFile )
{ // this code is basing on cal3d's miniviewer

	string  strMsg;

	strMsg = "Cal3dAnim: loading configuration file: \n";
	strMsg += strCfgFile;
	CTDCONSOLE_PRINT( LogLevel( INFO ), strMsg );

	if ( ParseModelConfiguration( strCfgFile ) == false ) {

		strMsg = " Error loading configuration file. \n";
		CTDCONSOLE_PRINT( LogLevel( WARNING ), strMsg );
		return false;
	}

	// make one material thread for each material
	// NOTE: this is not the right way to do it, but this viewer can't do the right
	// mapping without further information on the model etc., so this is the only
	// thing we can do here.
	int materialId;
	for(materialId = 0; materialId < m_calCoreModel.getCoreMaterialCount(); materialId++)
	{
		// create the a material thread
		m_calCoreModel.createCoreMaterialThread(materialId);

		// initialize the material thread
		m_calCoreModel.setCoreMaterialId(materialId, 0, materialId);
	}

	// create the model instance from the loaded core model
	if(!m_calModel.create(&m_calCoreModel))
	{
		strMsg = CalError::getLastErrorDescription();
		CTDCONSOLE_PRINT( LogLevel( WARNING ), strMsg );
		return false;
	}
	
	m_pCalRenderer = m_calModel.getRenderer();

	// attach all meshes to the model
	int meshId;
	for(meshId = 0; meshId < m_calCoreModel.getCoreMeshCount(); meshId++)
	{
		m_calModel.attachMesh(meshId);
	}

	// set the material set of the whole model
	m_calModel.setMaterialSet(0);

	// set initial animation state
	if(m_calCoreModel.getCoreAnimationCount() > 0)
	{
		m_IdLastAnimation	 = m_IdAnimIdle;
		m_IdNewAnimation	 = m_IdLastAnimation;
		m_calModel.getMixer()->executeAction(m_IdNewAnimation, 0.0f, m_blendTime);

	}
	else
	{
		strMsg = "Cal3dAnim: No animations exist!.";
		CTDCONSOLE_PRINT( LogLevel( WARNING ), strMsg );
		return false;
	}


	if ( SetupGeometry() == false ) {

		return false;

	}

	// we're done
	strMsg = "Cal3dAnim: Initialization successfull.";
	CTDCONSOLE_PRINT( LogLevel( INFO ), strMsg );

	m_pkNode->SetScaling( m_fScale );

	return true;

}

//----------------------------------------------------------------------------//
// Parse the configuration file and load the whole model                      //
//----------------------------------------------------------------------------//

bool CTDCal3DAnim::ParseModelConfiguration( const string& strFilename )
{ // this code is basing on cal3d's miniviewer

	string  strMsg;
    string  strFullPath;

    auto_ptr< File > pkFile( NeoEngine::Core::Get()->GetFileManager()->GetByName( strFilename ) );

    if ( !pkFile.get() ) {
		
        strMsg = "***Could not find model configuration file '" + strFilename + "'";
		CTDCONSOLE_PRINT( LogLevel( WARNING ), strMsg );

    }
    strFullPath = Framework::Get()->GetRootPath() + "/" + pkFile->GetPath();

	// open the model configuration file
    if( !pkFile->Open( "", strFilename, std::ios::in | std::ios::binary ) )
	{
		strMsg = "Failed to open model configuration file";
		CTDCONSOLE_PRINT( LogLevel( WARNING ), strMsg );
		return false;
	}

	// create a core model instance
	if(!m_calCoreModel.create("character"))
	{
		strMsg = CalError::getLastErrorDescription();
		CTDCONSOLE_PRINT( LogLevel( WARNING ), strMsg );
		return false;
	}

	// parse all lines from the model configuration file
	int line;
	for(line = 1; ; line++)
	{
		// read the next model configuration line
        char    pcBuffer[ 256 ];
		string  strBuffer;
        // end of file reached?
        if ( !pkFile->GetLine( pcBuffer, 255 ) ) {
            break;
        }

        strBuffer = pcBuffer;

		// check if an error happend while reading from the file
		if( !( *pkFile ) )
		{
			strMsg = "   Error while reading from the model configuration file";
			CTDCONSOLE_PRINT( LogLevel( WARNING ), strMsg );
			return false;
		}

		// find the first non-whitespace character
		string::size_type pos;
		pos = strBuffer.find_first_not_of( " \t" );

		// check for empty lines
		if( ( pos == string::npos ) || ( strBuffer[ pos ] == '\n' ) || ( strBuffer[ pos ] == '\r' ) || ( strBuffer[ pos ] == 0 ) ) continue;

		// check for comment lines
		if( strBuffer[ pos ] == '#' ) continue;

		// get the key
		string strKey;
		strKey = strBuffer.substr(pos, strBuffer.find_first_of(" =\t\n\r", pos) - pos);
		pos += strKey.size();

		// get the '=' character
		pos = strBuffer.find_first_not_of(" \t", pos);
		if((pos == string::npos) || (strBuffer[pos] != '='))
		{
			char buf[10];
			strMsg = "   Invalid Syntax at line ";
			strMsg += itoa(line, buf, 10);
			CTDCONSOLE_PRINT( LogLevel( WARNING ), strMsg );
			return false;
		}

		// find the first non-whitespace character after the '=' character
		pos = strBuffer.find_first_not_of(" \t", pos + 1);

		// get the data
		string strData;
		strData = strBuffer.substr(pos, strBuffer.find_first_of("\n\r", pos) - pos);
        string strLoadFileName = strFullPath + "/" + strData;

		// handle the model creation
		if(strKey == "scale")
		{
			// set rendering scale factor
			m_fScale = (float)atof(strData.c_str());
		}
		else if(strKey == "rotation")
		{
			float x, y, z;
			sscanf( strData.c_str(), "%f %f %f", &x, &y, &z );
			// set offset rotation
            m_kRotation = Quaternion( EulerAngles( x * PI / 180.0f, y * PI / 180.0f, z * PI / 180.0f ) );
		}
		else if(strKey == "position")
		{
			float x, y, z;
			sscanf( strData.c_str(), "%f %f %f", &x, &y, &z );
			// set offset position
			m_kPosition = Vector3d( x, y, z );
		}
		else if(strKey == "skeleton")
		{
			// load core skeleton
			//cout << "Loading skeleton '" << strData << "'..." << endl;
			if(!m_calCoreModel.loadCoreSkeleton( strLoadFileName ))
			{
				strMsg = CalError::getLastErrorDescription();
				CTDCONSOLE_PRINT( LogLevel( WARNING ), strMsg );
				return false;
			}
		}
		else if(strKey == "mesh")
		{
			// load core mesh
			//cout << "Loading mesh '" << strData << "'..." << endl;
			int	iMeshId =  m_calCoreModel.loadCoreMesh( strLoadFileName );
			if( iMeshId == -1 )
			{
				strMsg = CalError::getLastErrorDescription();
				CTDCONSOLE_PRINT( LogLevel( WARNING ), strMsg );
				return false;
			}
		}
		else if(strKey == "material")
		{

			CalCoreMaterial *pkCoreMaterial = new CalCoreMaterial;						
			string	*pstrMatName			= new string;
			*pstrMatName					= strData;

			// store the nmat file name
			pkCoreMaterial->setUserData( ( Cal::UserData )pstrMatName );
			m_calCoreModel.addCoreMaterial(  pkCoreMaterial );

		}

		////////////////////////////////////////////
		//          check for animations          //
		////////////////////////////////////////////
		else if(strKey == "animation_idle")
		{
			// load core animation
			m_IdAnimIdle = m_calCoreModel.loadCoreAnimation( strLoadFileName );
			if(m_IdAnimIdle == -1)
			{
				strMsg = CalError::getLastErrorDescription();
				strMsg += ": ";
				strMsg += strLoadFileName;
				CTDCONSOLE_PRINT( LogLevel( WARNING ), strMsg );
				return false;
			}
		}
		else if(strKey == "animation_walk")
		{
			// load core animation
			m_IdAnimWalk = m_calCoreModel.loadCoreAnimation( strLoadFileName );
			if(m_IdAnimWalk == -1)
			{
				strMsg = CalError::getLastErrorDescription();
				strMsg += ": ";
				strMsg += strLoadFileName;
				CTDCONSOLE_PRINT( LogLevel( WARNING ), strMsg );
				return false;
			}
		}
		else if(strKey == "animation_run")
		{
			// load core animation
			m_IdAnimRun = m_calCoreModel.loadCoreAnimation( strLoadFileName );
			if(m_IdAnimRun == -1)
			{
				strMsg = CalError::getLastErrorDescription();
				strMsg += ": ";
				strMsg += strLoadFileName;
				CTDCONSOLE_PRINT( LogLevel( WARNING ), strMsg );
				return false;
			}
		}
		else if(strKey == "animation_attack")
		{
			// load core animation
			m_IdAnimAttack = m_calCoreModel.loadCoreAnimation( strLoadFileName );
			if(m_IdAnimAttack == -1)
			{
				strMsg = CalError::getLastErrorDescription();
				strMsg += ": ";
				strMsg += strLoadFileName;
				CTDCONSOLE_PRINT( LogLevel( WARNING ), strMsg );
				return false;
			}
		}
		else if(strKey == "animation_jump")
		{
			// load core animation
			m_IdAnimJump = m_calCoreModel.loadCoreAnimation( strLoadFileName );
			if(m_IdAnimJump == -1)
			{
				strMsg = CalError::getLastErrorDescription();
				strMsg += ": ";
				strMsg += strLoadFileName;
				CTDCONSOLE_PRINT( LogLevel( WARNING ), strMsg );
				return false;
			}
		}
		else if(strKey == "animation_land")
		{
			// load core animation
			m_IdAnimLand = m_calCoreModel.loadCoreAnimation( strLoadFileName );
			if(m_IdAnimLand == -1)
			{
				strMsg = CalError::getLastErrorDescription();
				strMsg += ": ";
				strMsg += strLoadFileName;
				CTDCONSOLE_PRINT( LogLevel( WARNING ), strMsg );
				return false;
			}
		}
		else if(strKey == "animation_freeze")
		{
			// load core animation
			m_IdAnimFreeze = m_calCoreModel.loadCoreAnimation( strLoadFileName );
			if(m_IdAnimFreeze == -1)
			{
				strMsg = CalError::getLastErrorDescription();
				strMsg += ": ";
				strMsg += strLoadFileName;
				CTDCONSOLE_PRINT( LogLevel( WARNING ), strMsg );
				return false;
			}
		}
		/////////////////////////////////////////////

		else
		{
			// everything else triggers an error message, but is ignored
			char buf[10];
			strMsg = "   Invalid Syntax at line ";
			strMsg += itoa(line, buf, 10);
			CTDCONSOLE_PRINT( LogLevel( WARNING ), strMsg );
			return false;
		}
	}


	return true;
}

/////////////////////////////////////////////////
//            setup meshes and materials       //
/////////////////////////////////////////////////

bool CTDCal3DAnim::SetupGeometry() 
{

	//update the cal3d model to get valid mesh data
	m_calModel.update( 0.0f );
	// note that the polygon buffers are created with highest vertex count, hence set the highest lod
	m_calModel.setLodLevel( 1.0f );

	// go to render mode so we can access geometry data
	if( m_pCalRenderer->beginRendering() == false ) {

		return false;
	}


	// get the number of meshes
	int iMeshCount = m_pCalRenderer->getMeshCount();

	Mesh			*pkMesh		   = NULL;
	MeshEntity		*pkMeshEntity  = NULL;
	SceneNode		*pkSceneNode   = NULL;
	
	// get all meshes of the model
	for( int iMeshId = 0; iMeshId < iMeshCount; iMeshId++ ) {

		// setup a new mesh
		pkMesh          = new Mesh;

		m_vpkMeshes.push_back( pkMesh );
		pkMesh->SetName( "" );

		// get the number of submeshes
		int iSubmeshCount = m_pCalRenderer->getSubmeshCount( iMeshId );

		// get all submeshes of the mesh
		for( int iSubmeshId = 0; iSubmeshId < iSubmeshCount; iSubmeshId++ ) {

			// select mesh and submesh for further data access
			if( m_pCalRenderer->selectMeshSubmesh( iMeshId, iSubmeshId  ) ) {

				int				iVertexCount;
				int				iFaceCount;
				iVertexCount	= m_pCalRenderer->getVertexCount();
				iFaceCount		= m_pCalRenderer->getFaceCount();

				if ( ( iVertexCount <= 0 ) || ( iFaceCount <= 0 ) ) {

					string strMsg = " invalid mesh, skipping!";
					CTDCONSOLE_PRINT( LogLevel( WARNING ), strMsg );
					continue;

				}

				// create a new submesh
				SubMesh			*pkSubMesh = new SubMesh;

				// setup vertex buffer
                VertexBufferPtr pkVertexBuffer		= Framework::Get()->GetRenderDevice()->CreateVertexBuffer( Buffer::DYNAMIC, iVertexCount, &NormalTexVertex::s_kDecl );
				pkVertexBuffer->Lock( Buffer::WRITE );
				NormalTexVertex *pkVertex = (NormalTexVertex*)pkVertexBuffer->GetVertex();
				m_pCalRenderer->getNeoVerticesNormalsAndTexCoords( pkVertex );                
				pkVertexBuffer->Unlock();
				pkSubMesh->SetVertexBuffer( pkVertexBuffer );

				// setup polygon buffer
				PolygonBufferPtr pkPolygonBuffer	= Framework::Get()->GetRenderDevice()->CreatePolygonBuffer( Buffer::DYNAMIC, iFaceCount );

				pkPolygonBuffer->Lock( Buffer::WRITE );
				Polygon *pkPolygon = pkPolygonBuffer->GetPolygon();
				m_pCalRenderer->getNeoFaces( pkPolygon );
				pkPolygonBuffer->Unlock();

				pkSubMesh->SetPolygonBuffer( pkPolygonBuffer );

				// create material
				CalCoreMaterial *pkCoreMaterial;
				int iMatId = m_pCalRenderer->m_pSelectedSubmesh->getCoreMaterialId();
				pkCoreMaterial = m_calCoreModel.getCoreMaterial( iMatId );

				if ( pkCoreMaterial == NULL ) {

					string strMsg = " error creating material. check the cfg file";
					CTDCONSOLE_PRINT( LogLevel( WARNING ), strMsg );

				} else {

					// get stored material name
					string	*pstrCoreMaterialFileName = ( string* )pkCoreMaterial->getUserData();

					if ( pstrCoreMaterialFileName != NULL ) {

						// create a material and attach it into submesh
                        MaterialLibrary *pkLib = new MaterialLibrary( NeoEngine::Core::Get()->GetMaterialManager(), NULL );
						pkLib->Load( *pstrCoreMaterialFileName );

						// we need only the first material in mat lib!
						if ( pkLib->m_vpkMaterials.size() > 0 ) {

							pkSubMesh->m_pkMaterial = pkLib->m_vpkMaterials[ 0 ];

						} else {

							string strMsg = " could not find material file ' " + ( *pstrCoreMaterialFileName ) + " '";
							CTDCONSOLE_PRINT( LogLevel( WARNING ), strMsg );

							pkSubMesh->m_pkMaterial = NULL;
						}

						delete pkLib;

					}
				}

				// add submesh to scene node's mesh
				pkMesh->m_vpkSubMeshes.push_back( pkSubMesh );

			}

			// we do not need to attach every mesh in a scene node and attach it into main character node
			//  this would be interesting for rag dolls where every part of character can have an own dynamic
			//pkSceneNode		= new SceneNode;
			//pkMeshEntity	= new MeshEntity( pkMesh );
			//pkSceneNode->GetBoundingVolume()->Generate( mesh's bbox );
			//pkSceneNode->SetTranslation( mesh's position );
			//pkSceneNode->SetEntity( pkMeshEntity );
			//// attach new scene node to main character node
			//m_pkNode->AttachNode( pkSceneNode );

		}

	}


	m_pCalRenderer->endRendering();


	// delete material file names in user data
	for ( int iMatId = 0; iMatId <  m_calCoreModel.getCoreMaterialCount(); iMatId++ ) {

		CalCoreMaterial *pkCoreMaterial = m_calCoreModel.getCoreMaterial( iMatId );
		string			*pstrCoreMaterialFileName = ( string* )pkCoreMaterial->getUserData();
		delete			pstrCoreMaterialFileName;
		pkCoreMaterial->setUserData( NULL );
	}

	return true;
}


void CTDCal3DAnim::SetLod( const float &fLod )
{

	// check for valid lod
	if ( ( fLod < 0.0f ) || ( fLod > 1.0f ) ) {
		return;
	}

	// set lod
	m_calModel.setLodLevel( fLod );

	// force polygon buffer updating during rendering!
	m_bUpdatePolygonBuffer = true;

}

// render character
void CTDCal3DAnim::Render()
{

	//! TODO: some models have inverted normals, consider this in material!
	//  but temorarily we deactivate the whole backface culling
	Framework::Get()->GetRenderDevice()->SetCullMode( RenderDevice::CULLNONE );

	// go to render mode
	if( m_pCalRenderer->beginRendering() == false ) {

		return;
	}

	// update buffers and render geometry
	static RenderPrimitive  skPrimitive;
	static RenderPrimitive *spkPrimitive = &skPrimitive;

    //! TODO: shift offsetting stuff into preprocessing, instead of doing it during run-time
	spkPrimitive->m_kModelMatrix    = m_pkNode->GetWorldTransform() * m_kRotation;
	spkPrimitive->m_kInvModelMatrix = m_pkNode->GetInverseWorldTransform() * m_kRotation;
    spkPrimitive->m_kModelMatrix.SetTranslation( m_pkNode->GetTranslation() + m_kPosition );

	unsigned int uiMeshCount = ( unsigned int )m_vpkMeshes.size();
	unsigned int uiSubMeshCount = 0;

	// go through all meshes and their submeshes, update their buffers and render them
	for ( unsigned int uiMeshCnt = 0; uiMeshCnt < uiMeshCount; uiMeshCnt++ ) {

		uiSubMeshCount = ( unsigned int )m_vpkMeshes[ uiMeshCnt ]->m_vpkSubMeshes.size();

		for ( unsigned int uiSubMeshCnt = 0; uiSubMeshCnt < uiSubMeshCount; uiSubMeshCnt++ ) {

			// select mesh and submesh for further data access
			if( m_pCalRenderer->selectMeshSubmesh( uiMeshCnt, uiSubMeshCnt  ) ) {

				SubMesh *pkSubMesh = m_vpkMeshes[ uiMeshCnt ]->m_vpkSubMeshes[ uiSubMeshCnt ];

				// update polygon buffer if necessary ( if lod changed )
				if ( m_bUpdatePolygonBuffer == true ) {

					PolygonBufferPtr pkPolygonBuffer = pkSubMesh->GetPolygonBuffer();
					pkPolygonBuffer->Lock( Buffer::WRITE );
					Polygon *pkPolygon = pkPolygonBuffer->GetPolygon();
					int iNumFaces = m_pCalRenderer->getNeoFaces( pkPolygon );

					// set current number of polygons
					pkPolygonBuffer->SetNumElements( iNumFaces );
					
					pkPolygonBuffer->Unlock();

				}

				// update vertex buffer
				VertexBufferPtr	pkVertexBuffer = pkSubMesh->GetVertexBuffer();
				pkVertexBuffer->Lock( Buffer::WRITE );
				NormalTexVertex  *pkVerBuffer = ( NormalTexVertex* )pkVertexBuffer->GetVertex( 0 );
				int iNumVertices = m_pCalRenderer->getNeoVerticesNormalsAndTexCoords( pkVerBuffer );

				// et current number of polygon vertices
				//  this can change if lod changes!
				pkVertexBuffer->SetNumElements( iNumVertices );

				pkVertexBuffer->Unlock();

				pkSubMesh->Render( spkPrimitive, NULL );

			}
		}
	}
	
	// reset poly update request
	if ( m_bUpdatePolygonBuffer == true ) {

		m_bUpdatePolygonBuffer = false;

	}


	//Reset pointers to allow reference counter to clean up
	spkPrimitive->m_pkVertexBuffer       = 0;
	spkPrimitive->m_pkPolygonBuffer      = 0;
	spkPrimitive->m_pkPolygonStripBuffer = 0;
	spkPrimitive->m_pkMaterial           = 0;

	m_pCalRenderer->endRendering();

}

////////////////////////////////////////////////////
//              animation commands                //
////////////////////////////////////////////////////

// update animation state
void CTDCal3DAnim::UpdateAnim(const float &fElapsedTime)
{
	m_fElapsedTime = fElapsedTime;
	m_calModel.update(m_fElapsedTime);
}


void CTDCal3DAnim::Idle()
{
	// are we just starting this animation?	
	if ( m_IdNewAnimation != m_IdAnimIdle ) { 

		m_IdLastAnimation = m_IdNewAnimation; // store current animation to clear its cycle
		m_IdNewAnimation = m_IdAnimIdle;

		// blend the current animation to new animation sequence
		m_calModel.getMixer()->blendCycle(m_IdNewAnimation, 1.0f, m_blendTime);
		m_calModel.getMixer()->clearCycle(m_IdLastAnimation, m_blendTime);

	}
}

void CTDCal3DAnim::Walk()
{
	// are we just starting this animation?	
	if ( (m_IdNewAnimation != m_IdAnimWalk) && (m_IdNewAnimation != m_IdAnimJump) ) { 

		m_IdLastAnimation = m_IdNewAnimation; // store current animation to clear its cycle
		m_IdNewAnimation = m_IdAnimWalk;

		// blend the current animation to new animation sequence
		m_calModel.getMixer()->blendCycle(m_IdNewAnimation, 1.0f, m_blendTime);
		m_calModel.getMixer()->clearCycle(m_IdLastAnimation, m_blendTime);
	}
}
void CTDCal3DAnim::WalkBack()
{
	// are we just starting this animation?	
	if ( (m_IdNewAnimation != m_IdAnimWalk) && (m_IdNewAnimation != m_IdAnimJump) ) { 

		m_IdLastAnimation = m_IdNewAnimation; // store current animation to clear its cycle
		m_IdNewAnimation = m_IdAnimWalk;

		// blend the current animation to new animation sequence
		m_calModel.getMixer()->blendCycle(m_IdNewAnimation, 1.0f, m_blendTime);
		m_calModel.getMixer()->clearCycle(m_IdLastAnimation, m_blendTime);
	}
}

void CTDCal3DAnim::Run()
{
	// are we just starting this animation?	
	if ( (m_IdNewAnimation != m_IdAnimRun) && (m_IdNewAnimation != m_IdAnimJump) ) { 

		m_IdLastAnimation = m_IdNewAnimation; // store current animation to clear its cycle
		m_IdNewAnimation = m_IdAnimRun;

		// blend the current animation to new animation sequence
		m_calModel.getMixer()->blendCycle(m_IdNewAnimation, 1.0f, m_blendTime);
		m_calModel.getMixer()->clearCycle(m_IdLastAnimation, m_blendTime);
	}
}

void CTDCal3DAnim::Jump()
{
	// are we just starting this animation?	
	if ( m_IdNewAnimation != m_IdAnimJump ) { 

		m_IdLastAnimation = m_IdNewAnimation; // store current animation to clear its cycle
		m_IdNewAnimation = m_IdAnimJump;

		// blend the current animation to new animation sequence
		m_calModel.getMixer()->executeAction(m_IdNewAnimation, m_calCoreModel.getCoreAnimation(m_IdNewAnimation)->getDuration() - m_blendTime, m_blendTime);
		m_calModel.getMixer()->clearCycle(m_IdLastAnimation, m_blendTime);
	}
}

void CTDCal3DAnim::Landing()
{
	// are we just starting this animation?	
	if ( m_IdNewAnimation != m_IdAnimLand ) { 

		m_IdLastAnimation = m_IdNewAnimation; // store current animation to clear its cycle
		m_IdNewAnimation = m_IdAnimLand;

		// blend the current animation to new animation sequence
		m_calModel.getMixer()->executeAction(m_IdNewAnimation, m_calCoreModel.getCoreAnimation(m_IdNewAnimation)->getDuration() - m_blendTime, m_blendTime);
		m_calModel.getMixer()->clearCycle(m_IdLastAnimation, m_blendTime);
		m_fLandingTimer = m_calCoreModel.getCoreAnimation(m_IdNewAnimation)->getDuration();
	}
}

bool CTDCal3DAnim::IsLanded()
{
	if (m_fLandingTimer < 0.0f) {
	
		return true;

	} 

	m_fLandingTimer -= m_fElapsedTime;
	return false;
}

void CTDCal3DAnim::Stop()
{
	Idle();
}

void CTDCal3DAnim::Turn()
{
    Walk();
}

void CTDCal3DAnim::Look(const float &angleX, const float &angleY)
{

}


//void CTDCal3DAnim::Attack()
//{
//
//}
