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
 # 3dsmax exporter for keyframe animated meshes
 #
 # this code is basing on nsce exporter of Reality Rift Studios
 #  ( mattias@realityrift.com )
 #
 #
 #   date of creation:  02/23/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 #
 ################################################################*/

#include "exporter.h"
#include "mesh.h"
#include "texcoord.h"
#include "vertex.h"

#include <neoengine/nemath.h>
#include <neoengine/skin.h>
#include <neoengine/polygon.h>
#include <neoengine/vertex.h>
#include <neoengine/vertexdecl.h>
#include <neoengine/file.h>

#include <neochunkio/chunkio.h>
#include <neochunkio/core.h>
#include <neochunkio/mesh.h>
#include <neochunkio/polygonbuffer.h>
#include <neochunkio/vertexbuffer.h>
#include <neochunkio/skin.h>
#include <neochunkio/skinvertexbuffer.h>
#include <neochunkio/vector3d.h>
#include <neochunkio/quaternion.h>
#include <neochunkio/stdstring.h>
#include <neochunkio/integer.h>
#include <neochunkio/float.h>
#include <neochunkio/bool.h>
#include <neochunkio/factory.h>

#include <IGame.h>
#include <IGameObject.h>
#include <IGameProperty.h>
#include <IGameControl.h>
#include <IGameModifier.h>
#include <IConversionManager.h>
#include <IGameError.h>
#include <decomp.h>

using namespace std;
using namespace NeoEngine;
using namespace NeoChunkIO;


// this plugin exports every mesh in a separate neoengine scene and animation file
// this exporter supports also multi-textured meshes
// -  base texture on diffuse map and uv channel 1
// -  lightmap texture on self-illumination map and channel 3

#define MAP_CHANNEL_TEXTURE		1
#define MAP_CHANNEL_LIGHTMAP	3


namespace NeoMaxExporter
{

// register our new vertex type
VertexDeclaration NormalDiffuseLightmapTexVertex::s_kDecl( 4 );


//Helper method
Quaternion GMatrixToQuat( GMatrix &rkMatrix );


HINSTANCE g_hInstance = 0;

Quaternion g_kConvQuat( EulerAngles( -HALF_PI, 0.0f, 0.0f ) );
Matrix g_kConvMat( Quaternion( EulerAngles( -HALF_PI, 0.0f, 0.0f ) ), Vector3d::ZERO );

class ExporterClassDesc : public ClassDesc2
{
	public:

		int                                           IsPublic() { return TRUE; }

		void                                         *Create( BOOL loading = FALSE ) { return new Exporter(); }
	
		const TCHAR                                  *ClassName() { return KFANIMEXPORTER_CLASSNAME; }

		SClass_ID                                     SuperClassID() { return SCENE_EXPORT_CLASS_ID; }

		Class_ID                                      ClassID() { return KFANIMEXPORTER_CLASSID; }

		const TCHAR                                  *Category() { return "NeoEngine"; }

		const TCHAR                                  *InternalName() { return _T( KFANIMEXPORTER_CLASSNAME ); } // returns fixed parsable name (scripter-visible name)
	
		HINSTANCE                                     HInstance() { return g_hInstance; } // returns owning module handle

};



static ExporterClassDesc gs_ExporterDesc;

ClassDesc2* GetExporterDesc() { return &gs_ExporterDesc; }



// Dummy function for progress bar
DWORD WINAPI ProgressBar( LPVOID pArg )
{
	return 0;
}


//Error callback
class ExporterErrorCallback : public IGameErrorCallBack
{
	public:

		void ErrorProc( IGameError Error )
		{
			TCHAR *pszErr = GetLastIGameErrorText();
			DebugPrint( "ErrorCode = %d ErrorText = %s\n", Error, pszErr );
		}
};





int Exporter::s_iMajorNSCEVersion = 0;
int Exporter::s_iMinorNSCEVersion = 30;
int Exporter::s_iMajorNANIVersion = 0;
int Exporter::s_iMinorNANIVersion = 30;



Exporter::Exporter()
{
}


Exporter::~Exporter()
{
}


int Exporter::ExtCount()
{
	return 1;
}


const TCHAR *Exporter::Ext( int iExtID )
{
	return _T( "nsce" );
}


const TCHAR *Exporter::LongDesc()
{
	return _T( "Export to NeoEngine Mesh and animation files" );
}
	

const TCHAR *Exporter::ShortDesc() 
{			
	return _T( "CTD Keyframe animation exporter" );
}


const TCHAR *Exporter::AuthorName()
{			
	return _T( "A. Botorabi (botorabi@gmx.net)" );
}


const TCHAR *Exporter::CopyrightMessage() 
{	
	return _T( "" );
}

const TCHAR *Exporter::OtherMessage1() 
{		
	return _T( "" );
}


const TCHAR *Exporter::OtherMessage2() 
{		
	return _T( "" );
}


unsigned int Exporter::Version()
{				
	return 101;
}


void Exporter::ShowAbout( HWND hWnd )
{			
	MessageBox( hWnd, "CTD Exporter for NeoEngine Mesh and animation files,\nA. Botorabi (botorabi@gmx.net)\n\nReleased under zlib/libpng license", "About", MB_OK );
}


BOOL Exporter::SupportsOptions(int ext, DWORD options)
{
	return TRUE;
}

int	Exporter::DoExport( const TCHAR *pszName, ExpInterface *pExpInterface, Interface *pInterface, BOOL bSuppressPrompts, unsigned long ulOptions )
{
	if( !bSuppressPrompts )
	{
		//Show options dialog
		//...
	}


	NeoEngine::Core::Get()->Initialize( 0, 0 );

	NeoChunkIO::Core::Get()->Initialize();

	// setup our new vertex type
	//*****************************************************************************************//
	NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[0].m_uiType   = VertexElement::FLOAT3;
	NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[0].m_uiUsage  = VertexElement::POSITION;
	NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[0].m_uiOffset = 0;

	NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[1].m_uiType   = VertexElement::FLOAT3;
	NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[1].m_uiUsage  = VertexElement::NORMAL;
	NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[1].m_uiOffset = 12;

	NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[2].m_uiType   = VertexElement::FLOAT2;
	NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[2].m_uiUsage  = VertexElement::TEXCOORD;
	NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[2].m_uiIndex  = 0;
	NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[2].m_uiOffset = 24;

	NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[3].m_uiType   = VertexElement::FLOAT2;
	NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[3].m_uiUsage  = VertexElement::TEXCOORD;
	NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[3].m_uiIndex  = 1;
	NormalDiffuseLightmapTexVertex::s_kDecl.m_pkElements[3].m_uiOffset = 32;
	//*****************************************************************************************//


	Interface *pkCoreInterface = GetCOREInterface();


	ExporterErrorCallback kErrorCallback;

	SetErrorCallBack( &kErrorCallback );



	pkCoreInterface->ProgressStart( _T( "Exporting nsce.." ), TRUE, ProgressBar, 0 );
	
	m_pkIGameScene = GetIGameInterface();


	IGameConversionManager *pkConvManager = GetConversionManager();

	pkConvManager->SetCoordSystem( IGameConversionManager::IGAME_MAX );

	//Always export all
	m_pkIGameScene->InitialiseIGame( false );

	m_pkIGameScene->SetStaticFrame( 0 );
	

	TSTR strPath, strFile, strExt;

	SplitFilename( TSTR( pszName ), &strPath, &strFile, &strExt );

	int iNodeCounter = 0;


	m_fFramesPerSecond = (float)GetFrameRate();
	m_iTicksPerFrame   = m_pkIGameScene->GetSceneTicks();
	m_fTicksPerSecond  = (float)m_iTicksPerFrame * m_fFramesPerSecond;

	m_iStartFrame      = m_pkIGameScene->GetSceneStartTime() / m_iTicksPerFrame;
	m_iEndFrame        = m_pkIGameScene->GetSceneEndTime()   / m_iTicksPerFrame;

	for( int iNode = 0; iNode < m_pkIGameScene->GetTopLevelNodeCount(); ++iNode )
	{
		IGameNode *pkGameNode = m_pkIGameScene->GetTopLevelNode( iNode );
		
		if( pkGameNode->IsTarget() )
		{
			++iNodeCounter;
			continue;
		}

		ExportNodeInfo( pkGameNode, iNodeCounter );
	}

	{

		vector< vector< MaxVertex* > >::iterator pvpkMeshVertices    		= m_vvpkMeshVertices.begin();
		vector< vector< MaxVertex* > >::iterator pvpkMeshVerticesEnd 		= m_vvpkMeshVertices.end();
		vector< vector< NeoEngine::Polygon > >::iterator pvkMeshPolygons	= m_vvkMeshPolygons.begin();
		vector< TextureLayers        >::iterator pstrTexture         		= m_vstrTextures.begin();
		vector< IGameUVGen*          >::iterator ppkTexGen           		= m_vpkTexGen.begin();
		vector< IGameUVGen*          >::iterator ppkLmTexGen         		= m_vpkLmTexGen.begin();

		vector< MaxMesh* >::iterator ppkMesh    = m_vpkMeshes.begin();
		vector< MaxMesh* >::iterator ppkMeshEnd = m_vpkMeshes.end();

		for( ; pvpkMeshVertices != pvpkMeshVerticesEnd; ++pvpkMeshVertices, ++pvkMeshPolygons, ++pstrTexture, ++ppkTexGen, ++ppkLmTexGen, ++ppkMesh )
		{

			if( !pvkMeshPolygons->size() ) {

				continue;

			}

			// correct the mesh name to get the appropriate file name for it;
			string	strTmpName = ( *ppkMesh )->m_pkNode->GetName();
			string	strMeshName;

			//  eliminate non-alphanumeric characters in mesh name
			for ( unsigned int uiChars = 0; uiChars <  strTmpName.length(); uiChars++ ) {

				if ( IsCharAlphaNumeric( strTmpName[ uiChars ] ) == TRUE ) {

					strMeshName += strTmpName[ uiChars ];

				}
			}

			//Create blueprint chunk
			MeshChunk *pkMeshChunk = new MeshChunk;
			pkMeshChunk->AttachChunk( new StringChunk( strMeshName, "name" ) );
			m_vpkBlueprints.push_back( pkMeshChunk );

			bool bTexCoords = (*pvpkMeshVertices)[0]->m_bUV;
			bool bLmTexCoords = (*pvpkMeshVertices)[0]->m_bLmUV;

			Chunk *pkSubMeshChunk = ChunkFactory::CreateChunk( ChunkType::SUBMESH, "" );
			pkMeshChunk->AttachChunk( pkSubMeshChunk );


			PolygonBufferPtr pkPolygonBuffer     = new PolygonBuffer( Buffer::STATIC | Buffer::READPRIORITIZED, pvkMeshPolygons->size() );

			PolygonBufferChunk *pkPolygonChunk   = new PolygonBufferChunk( pkPolygonBuffer, "", true );

			pkSubMeshChunk->AttachChunk( pkPolygonChunk );

			pkPolygonBuffer->Lock( Buffer::WRITE );

			NeoEngine::Polygon *pkPolygon = pkPolygonBuffer->GetPolygon();

			vector< NeoEngine::Polygon >::iterator pkSrcPolygon    = pvkMeshPolygons->begin();
			vector< NeoEngine::Polygon >::iterator pkSrcPolygonEnd = pvkMeshPolygons->end();

			for( ; pkSrcPolygon != pkSrcPolygonEnd; ++pkSrcPolygon, ++pkPolygon ) {

				*pkPolygon = *pkSrcPolygon;

			}

			pkPolygonBuffer->Unlock();

			VertexBufferPtr pkVertexBuffer;

			if ( ( bTexCoords == true ) && ( bLmTexCoords == false ) ) {

				// vertex with only base texture
				pkVertexBuffer = new VertexBuffer( Buffer::STATIC | Buffer::READPRIORITIZED, pvpkMeshVertices->size(), &NormalTexVertex::s_kDecl );

			} else {

				// vertex with base and self-illumination ( lightmap ) texture 
				if ( ( bTexCoords == true ) && ( bLmTexCoords == true ) ) {

					pkVertexBuffer = new VertexBuffer( Buffer::STATIC | Buffer::READPRIORITIZED, pvpkMeshVertices->size(), &NormalDiffuseLightmapTexVertex::s_kDecl );

				} else {

					// vertex color vertex
					pkVertexBuffer = new VertexBuffer( Buffer::STATIC | Buffer::READPRIORITIZED, pvpkMeshVertices->size(), &NormalVertex::s_kDecl );

				}

			}

			VertexBufferChunk *pkVertexChunk = new VertexBufferChunk( pkVertexBuffer, "", true );

			pkVertexBuffer->Lock( Buffer::WRITE );

			unsigned char *pucVertex = (unsigned char*)pkVertexBuffer->GetVertex();

			vector< MaxVertex* >::iterator ppkSrcVertex    = pvpkMeshVertices->begin();
			vector< MaxVertex* >::iterator ppkSrcVertexEnd = pvpkMeshVertices->end();

			for( ; ppkSrcVertex != ppkSrcVertexEnd; ++ppkSrcVertex, pucVertex += pkVertexBuffer->GetVertexSize() )
			{
				NormalVertex *pkVertex = (NormalVertex*)pucVertex;

				pkVertex->m_kPosition = g_kConvMat * (*ppkSrcVertex)->m_kCoord;
				pkVertex->m_kNormal   = g_kConvMat * (*ppkSrcVertex)->m_kNormal;


				if( bTexCoords )
				{
					float *pfUV = (float*)( pucVertex + 24 );

					if( *ppkTexGen )
					{
						Point3 uv( (*ppkSrcVertex)->m_kUV.u, (*ppkSrcVertex)->m_kUV.v, 1.0f );

						uv = uv * (*ppkTexGen)->GetUVTransform();

						pfUV[0] = uv.x;
						pfUV[1] = uv.y;
					
					} else {

						pfUV[0] = (*ppkSrcVertex)->m_kUV.u;
						pfUV[1] = (*ppkSrcVertex)->m_kUV.v;
					}

					pfUV[1] = 1.0f - pfUV[1];
				}

				if( bLmTexCoords )
				{
					float *pfUV = (float*)( pucVertex + 32 );

					if( *ppkLmTexGen )
					{
						Point3 uv( (*ppkSrcVertex)->m_kLmUV.u, (*ppkSrcVertex)->m_kLmUV.v, 1.0f );

						uv = uv * (*ppkLmTexGen)->GetUVTransform();

						pfUV[0] = uv.x;
						pfUV[1] = uv.y;
					
					} else {

						pfUV[0] = (*ppkSrcVertex)->m_kLmUV.u;
						pfUV[1] = (*ppkSrcVertex)->m_kLmUV.v;
					}

					pfUV[1] = 1.0f - pfUV[1];
				}
			
			
			}

			pkVertexBuffer->Unlock();
			pkSubMeshChunk->AttachChunk( pkVertexChunk );

			Chunk *pkMatChunk = ChunkFactory::CreateChunk( ChunkType::MATERIAL, "" );
			pkSubMeshChunk->AttachChunk( pkMatChunk );

			bool bBaseAndLightmapTexture = pstrTexture->m_strBaseTexture.length() && pstrTexture->m_strLightmapTexture.length();
			if( pstrTexture->m_strBaseTexture.length() )
			{
				Chunk *pkTexLayerChunk = ChunkFactory::CreateChunk( ChunkType::TEXTURELAYER, "" );
				pkMatChunk->AttachChunk( pkTexLayerChunk );

				pkTexLayerChunk->AttachChunk( new StringChunk( pstrTexture->m_strBaseTexture, "texture" ) );
				
				if ( bBaseAndLightmapTexture == true ) {
				
					pkTexLayerChunk->AttachChunk( new StringChunk( "one zero", "blend" ) );
				
				}

			}

			if( pstrTexture->m_strLightmapTexture.length() )
			{
				Chunk *pkTexLayerChunk = ChunkFactory::CreateChunk( ChunkType::TEXTURELAYER, "" );
				pkMatChunk->AttachChunk( pkTexLayerChunk );

				pkTexLayerChunk->AttachChunk( new StringChunk( pstrTexture->m_strLightmapTexture, "texture" ) );

				if ( bBaseAndLightmapTexture == true ) {
				
					pkTexLayerChunk->AttachChunk( new StringChunk( "destcolor zero", "blend" ) );
					pkTexLayerChunk->AttachChunk( new IntChunk( 1,  "uv" ) );
				
				}
			}
		}


		float fStartTime = (float)m_iStartFrame / m_fFramesPerSecond;
		float fEndTime   = (float)m_iEndFrame   / m_fFramesPerSecond;
		float fLength    = fEndTime - fStartTime;
		int   iNumFrames = m_iEndFrame - m_iStartFrame;

		ppkMesh    = m_vpkMeshes.begin();
		ppkMeshEnd = m_vpkMeshes.end();

		// create an animation for every mesh
		for( ; ppkMesh != ppkMeshEnd; ++ppkMesh )
		{

			// correct the mesh name to get the appropriate file name for it;
			string	strTmpFileName = ( *ppkMesh )->m_pkNode->GetName();
			string	strAnimFileName;

			//  eliminate non-alphanumeric characters in mesh name
			for ( unsigned int uiChars = 0; uiChars <  strTmpFileName.length(); uiChars++ ) {

				if ( IsCharAlphaNumeric( strTmpFileName[ uiChars ] ) == TRUE ) {

					strAnimFileName += strTmpFileName[ uiChars ];

				}
			}			

			Chunk *pkAnimChunk = ChunkFactory::CreateChunk( ChunkType::ANIMATEDNODE, "" );

			pkAnimChunk->AttachChunk( new StringChunk( strAnimFileName, "name" ) );
			pkAnimChunk->AttachChunk( new IntChunk( 0, "id" ) );
			pkAnimChunk->AttachChunk( new FloatChunk( fLength, "length" ) );
			m_vpkAnimations.push_back( pkAnimChunk );

			Chunk *pkChannelChunk = ChunkFactory::CreateChunk( ChunkType::NODEANIMATION, "" );
			pkAnimChunk->AttachChunk( pkChannelChunk );

			for( int iFrame = 0; iFrame < iNumFrames; ++iFrame )
			{

				// FIXME: boto 07/27/2004
				//        after trying a while to solve the problem with wrong exported orientation i let the orientation 
				//         as it is extracted from local matrix and handle orientation corrections in game code!
				GMatrix		kLocalMat = (*ppkMesh)->m_pkNode->GetLocalTM( iFrame * m_iTicksPerFrame );

				AffineParts	kParts;
				Matrix3		kDecompMat = kLocalMat.ExtractMatrix3();
				decomp_affine( kDecompMat, &kParts );

				Quaternion	kQuat( kParts.q.x, kParts.q.y, kParts.q.z, kParts.q.w );

				Vector3d kPos( kLocalMat.GetRow( 3 )[0], kLocalMat.GetRow( 3 )[1], kLocalMat.GetRow( 3 )[2] );

				Chunk *pkFrameChunk = ChunkFactory::CreateChunk( ChunkType::NODEKEYFRAME, "" );

				pkChannelChunk->AttachChunk( pkFrameChunk );

				pkFrameChunk->AttachChunk( new VectorChunk( g_kConvMat * kPos, "pos" ) );
				pkFrameChunk->AttachChunk( new QuaternionChunk( kQuat, "rot" ) );
				pkFrameChunk->AttachChunk( new FloatChunk( (float)iFrame / (float)iNumFrames, "time" ) );
			}

			// free up mesh object
			delete( *ppkMesh );
		}
		m_vpkMeshes.clear();

	}


	// create a seperate mesh file for every animated mesh
	vector< Chunk* >::iterator ppkBlueprint    = m_vpkBlueprints.begin();
	vector< Chunk* >::iterator ppkBlueprintEnd = m_vpkBlueprints.end();
	for( ; ppkBlueprint != ppkBlueprintEnd; ++ppkBlueprint )
	{

		// get chunk name
		Chunk	*pkNameChunk = ( *ppkBlueprint )->FindChunk( "name", ChunkType::STRING );
		string strMeshFileName = ( ( StringChunk* )pkNameChunk )->m_strData;
		strMeshFileName += ".nsce";

		File *pkOutFile = new File( "", strMeshFileName, ios_base::out | ios_base::binary, true );

		if( !pkOutFile->IsValid() )
		{
			MessageBox( 0, "ERROR: Unable to create mesh file!", strMeshFileName.c_str(), MB_OK );
			return FALSE;
		}

		//Write ID and version
		ChunkIO::CHUNKIOMODE eMode = ChunkIO::ASCII;
		
		if( eMode == ChunkIO::BINARY )
		{
			pkOutFile->SetBinary( true );
			pkOutFile->Write( "NSCE", 4 );
			pkOutFile->Write( "!bin", 4 );

			*pkOutFile << s_iMajorNSCEVersion << s_iMinorNSCEVersion;
		}
		else if( eMode == ChunkIO::ASCII )
		{
			pkOutFile->SetBinary( false );

			*pkOutFile << "NSCE" << "!txt" << " " << s_iMajorNSCEVersion << " " << s_iMinorNSCEVersion << endl;
		}

		ChunkIO *pkIO = new ChunkIO;
		pkIO->m_eMode   = eMode;
		pkIO->WriteChunk( (*ppkBlueprint), pkOutFile );

		delete (*ppkBlueprint);
		delete pkIO;
		delete pkOutFile;
	}		
	m_vpkBlueprints.clear();


	//Write animation file
	vector< Chunk* >::iterator ppkAnimation    = m_vpkAnimations.begin();
	vector< Chunk* >::iterator ppkAnimationEnd = m_vpkAnimations.end();

	// create for every animation an own nani file with mesh name as file name
	for( ; ppkAnimation != ppkAnimationEnd; ++ppkAnimation ) {

		// get chunk name
		Chunk	*pkNameChunk = ( *ppkAnimation )->FindChunk( "name", ChunkType::STRING );
		string strNaniName = ( ( StringChunk* )pkNameChunk )->m_strData;
		strNaniName += ".nani";

		File *pkOutFile = new File( "", strNaniName, ios_base::out | ios_base::binary, true );

		if( !pkOutFile->IsValid() )
		{
			MessageBox( 0, "ERROR: Unable to create animation file!", strNaniName.c_str(), MB_OK );
			return FALSE;
		}

		//Write ID and version
		ChunkIO::CHUNKIOMODE eMode = ChunkIO::ASCII;
		
		if( eMode == ChunkIO::BINARY )
		{
			pkOutFile->SetBinary( true );

			pkOutFile->Write( "NANI", 4 );
			pkOutFile->Write( "!bin", 4 );

			*pkOutFile << s_iMajorNANIVersion << s_iMinorNANIVersion;
		}
		else if( eMode == ChunkIO::ASCII )
		{
			pkOutFile->SetBinary( false );

			*pkOutFile << "NANI" << "!txt" << " " << s_iMajorNANIVersion << " " << s_iMinorNANIVersion << endl;
		}

		ChunkIO *pkIO = new ChunkIO;
		pkIO->m_eMode   = eMode;
		pkIO->WriteChunk( (*ppkAnimation), pkOutFile );

		delete (*ppkAnimation);
		delete pkIO;
		delete pkOutFile;
	}
	m_vpkAnimations.clear();

	m_pkIGameScene->ReleaseIGame();
	pkCoreInterface->ProgressEnd();

	NeoChunkIO::Core::Get()->Shutdown();
	NeoEngine::Core::Get()->Shutdown();

	return TRUE;
}


void Exporter::ExportNodeInfo( IGameNode *pkNode, int &iCurNode )
{
	TSTR strBuf;

	strBuf = TSTR( "Processing: " ) + TSTR( pkNode->GetName() );
	GetCOREInterface()->ProgressUpdate( (int)( ( (float)iCurNode++ / (float)m_pkIGameScene->GetTotalNodeCount() ) * 100.0f ), FALSE, strBuf.data() ); 

	// skip non-keyframe animated meshes
	if ( pkNode->GetName()[0] != '~' ) {

		return;

	}

	if( pkNode->IsGroupOwner() )
	{
		// safe to ignore?
	}
	else
	{


		// reset the mesh into zero translation and rotation before exporting it
		//  as the animation track holds this information
		Matrix3 kTM;
		Matrix3 kOriginalTM;

		kTM.IdentityMatrix();

		INode	*pkMaxNode = pkNode->GetMaxNode();
		kOriginalTM = pkMaxNode->GetNodeTM( 0 );

		pkMaxNode->SetNodeTM( 0, kTM );
		//----------------------------------------------------------------------//


		TSTR                  strName   = pkNode->GetName();
		int                   iID       = pkNode->GetNodeID();
		IGameNode            *pkParent  = pkNode->GetNodeParent();
		GMatrix               kWorldTM  = pkNode->GetWorldTM();
		ULONG                 ulHandle  = pkNode->GetMaxNode()->GetHandle();
		IGameObject          *pkObj     = pkNode->GetIGameObject();
		IGameObject::MaxType  eMaxType  = pkObj->GetMaxType();
		bool                  bIsXRef   = pkObj->IsObjectXRef();


		if( pkNode->GetMaterialIndex() != -1 )
		{
			// foo?
		}

		if( bIsXRef )
		{
			// bar?
		}

		switch( pkObj->GetIGameType() )
		{
			case IGameObject::IGAME_BONE:
			{
				break;
			}

			case IGameObject::IGAME_MESH:
			{
				IGameMesh *pkMesh = (IGameMesh*)pkObj;
				IGameSkin *pkSkin = 0;

				if( !pkMesh->InitializeData() )
				{
					MessageBox( 0, "ERROR: Unable to initialize mesh data!", "CTD keyframe animation exporter", MB_OK );
					break;
				}

				if( ( pkSkin = pkMesh->GetIGameSkin() ) )
				{
					MessageBox( 0, "ERROR: no support for skinned meshes!", "CTD keyframe animation exporter", MB_OK );
					break;
				}

				// add the mesh into internal list for later animation creation
				AddMesh( pkNode );

				Vector3d    *pkCoords			= 0;
				MaxTexCoord *pkTexCoords		= 0;
				MaxTexCoord *pkLightmapCoords	= 0;
				Vector3d    *pkNormals			= 0;

				int iVert;
				int iVerts = 0;

				// botorabi (11/27/2003) : get all vertices in channel 'lightmap'
				int iLmVerts  = pkMesh->GetNumberOfMapVerts( MAP_CHANNEL_LIGHTMAP );
				int iTexVerts = pkMesh->GetNumberOfMapVerts( MAP_CHANNEL_TEXTURE );
				bool bLightmapTexture = true;

				// check wether this mesh contains lightmap texture
				if ( iLmVerts == 0 ) {

					bLightmapTexture = false;

				}

				pkTexCoords			= ( iTexVerts > 0 ) ? new MaxTexCoord[ iTexVerts ] : 0;
				pkLightmapCoords	= ( iLmVerts > 0 ) ? new MaxTexCoord[ iLmVerts ] : 0;

				// botorabi (11/28/2003): get all map vertices
				for( iVert = 0; iVert < iTexVerts; ++iVert )
				{

					Point2 kUVPoint = pkMesh->GetTexVertex( iVert );

					pkTexCoords[ iVert ].u = kUVPoint.x;
					pkTexCoords[ iVert ].v = kUVPoint.y;

					if ( bLightmapTexture == true ) {
					
						Point2 kLmUVPoint = pkMesh->GetMapVertex( MAP_CHANNEL_LIGHTMAP, iVert );
						pkLightmapCoords[ iVert ].u = kLmUVPoint.x;
						pkLightmapCoords[ iVert ].v = kLmUVPoint.y;

					} 

				}

				iVerts = pkMesh->GetNumberOfNormals();

				pkNormals = ( iVerts > 0 ) ? new Vector3d[ iVerts ] : 0;

				for( iVert = 0; iVert < iVerts; ++iVert )
				{
					Point3 Normal = pkMesh->GetNormal( iVert );

					pkNormals[ iVert ].x = Normal.x;
					pkNormals[ iVert ].y = Normal.y;
					pkNormals[ iVert ].z = Normal.z;
				}

				iVerts = pkMesh->GetNumberOfVerts();

				pkCoords = ( iVerts > 0 ) ? new Vector3d[ iVerts ] : 0;

				for( iVert = 0; iVert < iVerts; ++iVert )
				{
					Point3  VertPoint	= pkMesh->GetVertex( iVert );

					pkCoords[ iVert ].x = VertPoint.x;
					pkCoords[ iVert ].y = VertPoint.y;
					pkCoords[ iVert ].z = VertPoint.z;
				}

				// create vertex and polygon lists for every mesh
				vector< vector< MaxVertex* > >						vvpkMeshVertices;
				vector< vector< NeoEngine::Polygon > >				vvkMeshPolygons;
				vector< vector< MaxVertex*   > >::iterator			pvpkMeshVertices			= vvpkMeshVertices.begin();
				vector< vector< NeoEngine::Polygon > >::iterator	pvkMeshPolygons				= vvkMeshPolygons.begin();
				vector< vector< MaxVertex*   > >::iterator			pvpkExportMeshVertices		= m_vvpkMeshVertices.begin();
				vector< vector< NeoEngine::Polygon > >::iterator	pvkExportMeshPolygons		= m_vvkMeshPolygons.begin();
				
				vector< TextureLayers >					vstrTextures;
				vector< TextureLayers >::iterator		pstrTexture    = vstrTextures.begin();
				vector< TextureLayers >::iterator		pstrTextureEnd = vstrTextures.end();

				vector< IGameUVGen* >					vpkTexGen;
				vector< IGameUVGen* >					vpkLmTexGen;
				vector< IGameUVGen* >::iterator			ppkTexGen      = vpkTexGen.begin();
				vector< IGameUVGen* >::iterator			ppkLmTexGen    = vpkLmTexGen.begin();

				//Add triangle and vertex data
				int iFaces = pkMesh->GetNumberOfFaces();

				for( int iFace = 0; iFace < iFaces; ++iFace )
				{
					FaceEx *pkFace = pkMesh->GetFace( iFace );

					int aiIndex[3]  = { pkFace->vert[0],     pkFace->vert[1],     pkFace->vert[2]     };
					int aiNormal[3] = { pkFace->norm[0],     pkFace->norm[1],     pkFace->norm[2]     };
					int aiUV[3]     = { pkFace->texCoord[0], pkFace->texCoord[1], pkFace->texCoord[2] };

					// botorabi (11/28/2003) : store uv coords for lightmap
					DWORD piIndex[3] = {0};
					bool bHasLightmap = pkMesh->GetMapFaceIndex( MAP_CHANNEL_LIGHTMAP, iFace, piIndex );
					int aiLmUV[3]   = { piIndex[0], piIndex[1], piIndex[2] };					

					vector< TextureLayers >::iterator pstrTexture      = vstrTextures.begin();
					vector< TextureLayers >::iterator pstrTextureEnd   = vstrTextures.end();

					string strBaseTexture, strLmTexture;
					int iSlash;
					int iLength;

					IGameMaterial *pkMat = pkMesh->GetMaterialFromFace( pkFace );
					IGameTextureMap *pkTexture     = NULL;
					IGameTextureMap *pkBaseTexture = NULL;
					IGameTextureMap *pkLmTexture   = NULL;

					// botorabi: if no material found then skip this face
					if (pkMat == NULL) {

						//MessageBox(NULL, "Missing material for mesh!", "Warning", MB_OK);
						continue;

					}

					// check all texture maps for this material
					for ( int iTexNum = 0; iTexNum < pkMat->GetNumberOfTextureMaps(); iTexNum++ ) {
						
						pkTexture = pkMat->GetIGameTextureMap( iTexNum );

						// check for diffuse map ( we use this as base map )
						if ( pkTexture->GetStdMapSlot() == ID_DI ) {

							pkBaseTexture = pkTexture;

						} else {
						
							// check for self-illumination map ( we use this as lightmap )
							if ( pkTexture->GetStdMapSlot() == ID_SI ) {

								pkLmTexture = pkTexture;

							}

						}

					}
						
					// get base texture name
					if( pkBaseTexture && pkBaseTexture->GetBitmapFileName() )
					{

						TCHAR	*pcBmpFileName = pkBaseTexture->GetBitmapFileName();

						// this seems to be a problem: sometimes the function "GetBitmapFileName" returns an id instead of texture file name!?
						if ( ( ( int )pcBmpFileName ) != TEXMAP_CLASS_ID ) {

							strBaseTexture = pkBaseTexture->GetBitmapFileName();

							if( ( ( iSlash = strBaseTexture.find_last_of( '\\' ) ) == string::npos ) && ( ( iSlash = strBaseTexture.find_last_of( '/' ) ) == string::npos ) )
								iSlash = 0;
							else
								++iSlash;

							strBaseTexture = strBaseTexture.substr( iSlash, ( ( ( iLength = strBaseTexture.find_last_of( '.' ) ) != string::npos ) && ( iLength > iSlash ) ) ? ( iLength - iSlash ) : string::npos );

						} else {

							strBaseTexture = "";

						}
					}

					// get lightmap texture name
					if( pkLmTexture && pkLmTexture->GetBitmapFileName() )
					{

						TCHAR	*pcBmpFileName = pkLmTexture->GetBitmapFileName();

						// this seems to be a problem: sometimes the function "GetBitmapFileName" returns an id instead of texture file name!?
						if ( ( ( int )pcBmpFileName ) != TEXMAP_CLASS_ID ) {

							strLmTexture = pkLmTexture->GetBitmapFileName();

							if( ( ( iSlash = strLmTexture.find_last_of( '\\' ) ) == string::npos ) && ( ( iSlash = strLmTexture.find_last_of( '/' ) ) == string::npos ) )
								iSlash = 0;
							else
								++iSlash;

							strLmTexture = strLmTexture.substr( iSlash, ( ( ( iLength = strLmTexture.find_last_of( '.' ) ) != string::npos ) && ( iLength > iSlash ) ) ? ( iLength - iSlash ) : string::npos );

						} else {

							strLmTexture = "";

						}
					}

					for( ; pstrTexture != pstrTextureEnd; ++pstrTexture, ++pvpkMeshVertices, ++pvkMeshPolygons ) {

						if( ( pstrTexture->m_strBaseTexture == strBaseTexture ) && ( pstrTexture->m_strLightmapTexture == strLmTexture ) ) {

							break;

						}
					}

					if( pstrTexture == pstrTextureEnd )
					{
						//Add new mesh
						vector< MaxVertex* >			vpMaxVert;
						vector< NeoEngine::Polygon >	vvPolygon;

						m_vvpkMeshVertices.push_back( vpMaxVert );						
						m_vvkMeshPolygons.push_back(  vvPolygon );

						vvpkMeshVertices.push_back( vpMaxVert );
						vvkMeshPolygons.push_back(  vvPolygon );

						TextureLayers	kTexLayer;
						kTexLayer.m_strBaseTexture = strBaseTexture;
						kTexLayer.m_strLightmapTexture = strLmTexture;

						m_vstrTextures.push_back( kTexLayer );
						vstrTextures.push_back( kTexLayer );

						m_vpkTexGen.push_back( pkTexture ? pkTexture->GetIGameUVGen() : 0 );
						m_vpkLmTexGen.push_back( pkLmTexture ? pkLmTexture->GetIGameUVGen() : 0 );

						vpkTexGen.push_back( pkTexture ? pkTexture->GetIGameUVGen() : 0 );
						vpkLmTexGen.push_back( pkLmTexture ? pkLmTexture->GetIGameUVGen() : 0 );

						pvpkExportMeshVertices = m_vvpkMeshVertices.end() - 1;
						pvkExportMeshPolygons  = m_vvkMeshPolygons.end() - 1;

						pvpkMeshVertices = vvpkMeshVertices.end() - 1;
						pvkMeshPolygons  = vvkMeshPolygons.end() - 1;

					}

					//Add triangle
					NeoEngine::Polygon kPolygon;

					for( int iIndex = 0; iIndex < 3; ++iIndex )
					{
						vector< MaxVertex* >::iterator ppkVertex    = pvpkMeshVertices->begin();
						vector< MaxVertex* >::iterator ppkVertexEnd = pvpkMeshVertices->end();

						MaxVertex *pkVertex = new MaxVertex;

						pkVertex->m_kCoord      = pkCoords[ aiIndex[ iIndex ] ];

						if( pkNormals )
							pkVertex->m_kNormal = pkNormals[ aiNormal[ iIndex ] ];

						if( pkTexCoords ) {

							pkVertex->m_kUV     = pkTexCoords[ aiUV[ iIndex ] ];
							pkVertex->m_bUV		= true;

						}

						if( pkLightmapCoords ) {

							pkVertex->m_kLmUV   = pkLightmapCoords[ aiLmUV[ iIndex ] ];
							pkVertex->m_bLmUV	= true;

						}

						unsigned int uiVertex = 0;

						for( ; ppkVertex != ppkVertexEnd; ++ppkVertex, ++uiVertex )
							if( **ppkVertex == *pkVertex )
								break;

						if( ppkVertex == ppkVertexEnd )
						{

							pvpkMeshVertices->push_back( pkVertex );

							pvpkExportMeshVertices->push_back( pkVertex );

						}
						else
							delete pkVertex;

						kPolygon.v[ iIndex ] = uiVertex;
					}

					pvkMeshPolygons->push_back( kPolygon );

					pvkExportMeshPolygons->push_back( kPolygon );

				}

				delete [] pkCoords;
				delete [] pkTexCoords;

				break;
			}

			case IGameObject::IGAME_HELPER:
			{
				break;
			}

			case IGameObject::IGAME_LIGHT:
			{ 

				break;
			}

			case IGameObject::IGAME_CAMERA:
			{
				break;
			}

			case IGameObject::IGAME_SPLINE:
			{
				break;
			}

			case IGameObject::IGAME_IKCHAIN:
			{
				break;
			}

			case IGameObject::IGAME_UNKNOWN:
			default:
			{
				//MessageBox( 0, "WARNING: Unknown node type", "CTD Exporter", MB_OK );
				break;
			}
		}


		// reset the old transformation matrix of the mesh
		pkMaxNode->SetNodeTM( 0, kOriginalTM );

	}

	for( int iChild = 0; iChild < pkNode->GetChildCount(); ++iChild )
	{
		IGameNode *pkChild = pkNode->GetNodeChild( iChild );
		
		if( pkChild->IsTarget() )
		{
			++iCurNode;
			continue;
		}

		ExportNodeInfo( pkChild, iCurNode );
	}

	pkNode->ReleaseIGameObject();
}

void Exporter::AddMesh( IGameNode *pkMesh )
{
	if( !pkMesh )
		return;

	MaxMesh *pkMaxMesh  = new MaxMesh;

	pkMaxMesh->m_strName  = pkMesh->GetName();
	pkMaxMesh->m_kWorldTM = pkMesh->GetWorldTM();
	pkMaxMesh->m_kLocalTM = pkMesh->GetLocalTM();
	pkMaxMesh->m_pkNode   = pkMesh;

	m_vpkMeshes.push_back( pkMaxMesh );
}

Quaternion GMatrixToQuat( GMatrix &rkMatrix )
{
	// gamasutra again...
	float  tr, s, q[4];
	int    i, j, k;

	int nxt[3] = { 1, 2, 0 };

	Quaternion kQuat;

	tr = rkMatrix[0][0] + rkMatrix[1][1] + rkMatrix[2][2];
	
	// check the diagonal
	if( tr > 0.0f )
	{
		s = sqrtf( tr + 1.0f );
		kQuat.qw = s * 0.5f;
		s = 0.5f / s;
		kQuat.qx = ( rkMatrix[2][1] - rkMatrix[1][2] ) * s;
		kQuat.qy = ( rkMatrix[0][2] - rkMatrix[2][0] ) * s;
		kQuat.qz = ( rkMatrix[1][0] - rkMatrix[0][1] ) * s;
	}
	else
	{
		// diagonal is negative
		i = 0;
		if( rkMatrix[1][1] > rkMatrix[0][0] ) i = 1;
		if( rkMatrix[2][2] > rkMatrix[i][i] ) i = 2;
		j = nxt[i];
		k = nxt[j];
		
		s = sqrtf( ( rkMatrix[i][i] - ( rkMatrix[j][j] + rkMatrix[k][k] ) ) + 1.0f );
		
		q[i] = s * 0.5f;
		
		if( s != 0.0f ) s = 0.5f / s;
		
		q[3] = ( rkMatrix[k][j] - rkMatrix[j][k] ) * s;
		q[j] = ( rkMatrix[j][i] + rkMatrix[i][j] ) * s;
		q[k] = ( rkMatrix[k][i] + rkMatrix[i][k] ) * s;
		
		kQuat.qx = q[0];
		kQuat.qy = q[1];
		kQuat.qz = q[2];
		kQuat.qw = q[3];
	}

	kQuat.Normalize();

	return kQuat;
}


};

