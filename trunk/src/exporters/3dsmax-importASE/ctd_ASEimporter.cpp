/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2002-2004, Ali Botorabi
 *  http://yag2002.sourceforge.net
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
 # 3DS Max -- ASE importer
 #
 # this is an ASE importer for 3d studio max 5.1
 # it supports up to two texture and uv channels
 #
 #
 #   date of creation:  03/01/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/



#include <IGame.h>
#include <IGameObject.h>
#include <IGameProperty.h>
#include <IGameControl.h>
#include <IGameModifier.h>
#include <IConversionManager.h>
#include <IGameError.h>

#include <stdmat.h>

#include "ctd_ASEimporter.h"
#include <libASE.h>

#include <string>

using namespace std;

HINSTANCE g_hInstance = 0;

// map channels
#define MAP_CHANNEL_TEXTURE		1
#define MAP_CHANNEL_LIGHTMAP	3


class ImporterClassDesc : public ClassDesc2
{
	public:

		int                                           IsPublic() { return TRUE; }

		void                                         *Create( BOOL loading = FALSE ) { return new Importer(); }
	
		const TCHAR                                  *ClassName() { return CTD_ASE_IMPORTER_CLASSNAME; }

		SClass_ID                                     SuperClassID() { return SCENE_IMPORT_CLASS_ID; }

		Class_ID                                      ClassID() { return CTD_ASE_IMPORTER_CLASSID; }

		const TCHAR                                  *Category() { return "Importer"; }

		const TCHAR                                  *InternalName() { return _T( CTD_ASE_IMPORTER_CLASSNAME ); } // returns fixed parsable name (scripter-visible name)
	
		HINSTANCE                                     HInstance() { return g_hInstance; } // returns owning module handle

};


static ImporterClassDesc gs_ImporterDesc;
ClassDesc2* GetImporterDesc() { return &gs_ImporterDesc; }

// importer version
int Importer::s_iMajorVersion = 1;
int Importer::s_iMinorVersion = 0;


// Dummy function for progress bar
DWORD WINAPI ProgressBar( LPVOID pArg )
{
	return 0;
}


//Error callback
class ImporterErrorCallback : public IGameErrorCallBack
{
	public:

		void ErrorProc( IGameError Error )
		{
			TCHAR *pszErr = GetLastIGameErrorText();
			DebugPrint( "ErrorCode = %d ErrorText = %s\n", Error, pszErr );
		}
};


Importer::Importer()
{

	m_pkImpInterface = NULL;

}


Importer::~Importer()
{
}


int Importer::ExtCount()
{
	return 1;
}


const TCHAR *Importer::Ext( int iExtID )
{
	return _T( "ase" );
}


const TCHAR *Importer::LongDesc()
{
	return _T( "CTD -- Import ASE including lightmaps" );
}
	

const TCHAR *Importer::ShortDesc() 
{			
	return _T( "CTD ASE Importer" );
}


const TCHAR *Importer::AuthorName()
{			
	return _T( "A. Botorabi (botorabi@gmx.net)" );
}


const TCHAR *Importer::CopyrightMessage() 
{	
	return _T( "" );
}

const TCHAR *Importer::OtherMessage1() 
{		
	return _T( "" );
}


const TCHAR *Importer::OtherMessage2() 
{		
	return _T( "" );
}


unsigned int Importer::Version()
{				
	return 100;
}


void Importer::ShowAbout( HWND hWnd )
{			
	MessageBox( hWnd, "Importer for ASE format including an additional texture coordinate channel ( lightmap )\nA. Botorabi (botorabi@gmx.net)\n\nReleased under GNU license", "About", MB_OK );
}


BOOL Importer::SupportsOptions(int ext, DWORD options)
{
	return TRUE;
}

int	Importer::DoImport( const TCHAR *pszName, ImpInterface *pImpInterface, Interface *pInterface, BOOL bSuppressPrompts )
{

	// setup various things 
	m_pkImpInterface = pImpInterface;
	Interface *pkCoreInterface = GetCOREInterface();
	ImporterErrorCallback kErrorCallback;
	SetErrorCallBack( &kErrorCallback );
	pkCoreInterface->ProgressStart( _T( "Importing ASE file.." ), TRUE, ProgressBar, 0 );	
	m_pkIGameScene = GetIGameInterface();
	IGameConversionManager *pkConvManager = GetConversionManager();
	pkConvManager->SetCoordSystem( IGameConversionManager::IGAME_MAX );
	m_pkIGameScene->InitialiseIGame( false );
	m_pkIGameScene->SetStaticFrame( 0 );

	// read the ase file
	ASE_Scene	*pkAseScene = NULL;
	pkAseScene = ASE_loadFilename( ( char* )pszName );

	if ( pkAseScene == NULL ) {

		MessageBox( NULL, "Invalid ASE format", "Error reading file", MB_OK );
		m_pkIGameScene->ReleaseIGame();
		pkCoreInterface->ProgressEnd();
		return FALSE;

	}

	// clear the material list
	m_vkMaterials.clear();
	// we assume that all materials are defined as submaterial of a big main material
	ASE_Material *pkMaterial = pkAseScene->materials;
	// create materials read from ase file
	SetupMaterials( pkMaterial );

	// create geometry objects
	for( int iAseObjs = 0; iAseObjs < pkAseScene->objectCount; ++iAseObjs ) {		

		ASE_GeomObject *pkGeomObj = &pkAseScene->objs[ iAseObjs ];
		ImportGeometry( pkGeomObj );
	
	}

	// release resources
	ASE_freeScene( pkAseScene );
	m_pkIGameScene->ReleaseIGame();
	pkCoreInterface->ProgressEnd();

	return TRUE;

}

// import geometry
void Importer::ImportGeometry( ASE_GeomObject *pkASEGeomObj )
{

	ImpNode	*pkMaxNode = m_pkImpInterface->CreateNode();

	if ( !pkMaxNode ) {

		// error creating a new node!
		return;

	}

	pkMaxNode->SetName( pkASEGeomObj->name );

	// set node's transformation matrix
	INode	*pkNode = pkMaxNode->GetINode();
	Matrix3	kMatrix;
	kMatrix.IdentityMatrix();
	Point3 kRow0( 
		pkASEGeomObj->transform.matrix[ 0 ][ 0 ], 
		pkASEGeomObj->transform.matrix[ 0 ][ 1 ],
		pkASEGeomObj->transform.matrix[ 0 ][ 2 ]
		);
	Point3 kRow1( 
		pkASEGeomObj->transform.matrix[ 1 ][ 0 ], 
		pkASEGeomObj->transform.matrix[ 1 ][ 1 ],
		pkASEGeomObj->transform.matrix[ 1 ][ 2 ]
		);
	Point3 kRow2( 
		pkASEGeomObj->transform.matrix[ 2 ][ 0 ], 
		pkASEGeomObj->transform.matrix[ 2 ][ 1 ],
		pkASEGeomObj->transform.matrix[ 2 ][ 2 ]
		);
	Point3 kRow3( 0.0f, 0.0f, 0.0f );
	kMatrix.Set( kRow0, kRow1, kRow2, kRow3 );
	Point3	kPosition( pkASEGeomObj->transform.pos.x, pkASEGeomObj->transform.pos.y, pkASEGeomObj->transform.pos.z );
	kMatrix.SetTrans( kPosition );

	pkNode->SetNodeTM( 0, kMatrix );

	// build the inverse matrix to transform the mesh vertices into object space
	Matrix3 kInvMatrix = kMatrix;
	kInvMatrix.Invert();

	// create object
	TriObject *pkObject = (TriObject *)m_pkImpInterface->Create( GEOMOBJECT_CLASS_ID,Class_ID( EDITTRIOBJ_CLASS_ID,0 ) );
	pkMaxNode->Reference( pkObject );

	// set up the mesh vertices, etc.
	Mesh			*pkMesh			= &pkObject->mesh;
	ASE_Vector3D	*pkVerts		= pkASEGeomObj->mesh.vertices;
	ASE_Vector3D	*pkNormals		= pkASEGeomObj->mesh.vertex_normals;
	ASE_Face		*pkFaces		= pkASEGeomObj->mesh.faces;
	ASE_Vector3D	*pkBaseTVerts	= pkASEGeomObj->mesh.texture_coordinates;
	ASE_Vector3D	*pkLmTVerts		= pkASEGeomObj->mesh.texture_second_coordinates;

	pkMesh->setNumFaces( pkASEGeomObj->mesh.faceCount );
	pkMesh->setNumVerts( pkASEGeomObj->mesh.vertexCount );

	// if a second map channel exists then create an additional map for the mesh
	if ( pkLmTVerts ) {

		pkMesh->setNumMaps( 2, TRUE ); // we support maximap 2 maps: 1. base channel, 2. lightmap
		pkMesh->setMapSupport( MAP_CHANNEL_LIGHTMAP, TRUE );

		// we assume that there are equal counts of base and lm textured faces / vertices
		pkMesh->setNumMapVerts( MAP_CHANNEL_LIGHTMAP, pkASEGeomObj->mesh.vertexCount );
		pkMesh->setNumMapFaces( MAP_CHANNEL_LIGHTMAP, pkASEGeomObj->mesh.faceCount );

	}

	pkMesh->setNumTVFaces( pkASEGeomObj->mesh.faceCount );
	pkMesh->setNumTVerts( pkASEGeomObj->mesh.textureCoordinateCount );

	// set vertices and their normals ( if they exist )
	for ( int iNumVerts = 0; iNumVerts < pkASEGeomObj->mesh.vertexCount; iNumVerts++ ) {

		// transform the mesh vertices considering the transformation matrix
		Point3 kTransVert( pkVerts[ iNumVerts ].x, pkVerts[ iNumVerts ].y, pkVerts[ iNumVerts ].z );
		kTransVert = kInvMatrix * kTransVert;

		pkMesh->setVert( iNumVerts, kTransVert );

		// set base texture coords
		pkMesh->setTVert( iNumVerts, pkBaseTVerts[ iNumVerts ].x, pkBaseTVerts[ iNumVerts ].y, 0 );

		if ( pkNormals ) {

			Point3	kNormal( pkNormals[ iNumVerts ].x, pkNormals[ iNumVerts ].y, pkNormals[ iNumVerts ].z );
			pkMesh->setNormal( iNumVerts, kNormal );

		}

		// set lightmap texture coords, if they exist
		if ( pkLmTVerts ) {
		
			UVVert	kUVVert;
			kUVVert.x = pkLmTVerts[ iNumVerts ].x; kUVVert.y = pkLmTVerts[ iNumVerts ].y; kUVVert.z = 0; 
			pkMesh->setMapVert( MAP_CHANNEL_LIGHTMAP, iNumVerts, kUVVert );

		}

	}

	// set faces
	for ( int iNumFaces = 0; iNumFaces < pkASEGeomObj->mesh.faceCount; iNumFaces++ ) {

		ASE_Face		*pkFace		= &pkASEGeomObj->mesh.faces[ iNumFaces ];

		pkMesh->faces[ iNumFaces ].v[ 0 ] = pkFace->vertex[ 0 ];
		pkMesh->faces[ iNumFaces ].v[ 1 ] = pkFace->vertex[ 1 ];
		pkMesh->faces[ iNumFaces ].v[ 2 ] = pkFace->vertex[ 2 ];
		pkMesh->tvFace[ iNumFaces ].t[ 0 ] = pkFace->texture_coordinates[ 0 ];
		pkMesh->tvFace[ iNumFaces ].t[ 1 ] = pkFace->texture_coordinates[ 1 ];
		pkMesh->tvFace[ iNumFaces ].t[ 2 ] = pkFace->texture_coordinates[ 2 ];

		pkMesh->faces[ iNumFaces ].setSmGroup( pkFace->smoothing );
		pkMesh->faces[ iNumFaces ].setMatID( pkFace->mtlid );
		pkMesh->faces[ iNumFaces ].flags=EDGE_ALL;

		// set faces for second texture map ( lightmap )
		if ( pkLmTVerts ) {

			TVFace	kTVFace;
			kTVFace.t[ 0 ] = pkFace->texture_second_coordinates[ 0 ];
			kTVFace.t[ 1 ] = pkFace->texture_second_coordinates[ 1 ];
			kTVFace.t[ 2 ] = pkFace->texture_second_coordinates[ 2 ];

			TVFace *pkMapFace = pkMesh->mapFaces( MAP_CHANNEL_LIGHTMAP );
			if ( pkMapFace ) {
			
				pkMapFace[ iNumFaces ] = kTVFace;

			}

		}

		// set material, check for valid material id first!
		if ( pkFace->mtlid <= ( int )m_vkMaterials.size() ) {

			StdMat	*pkMat = m_vkMaterials[ pkFace->mtlid ];
			pkMaxNode->GetINode()->SetMtl( pkMat );

		} 

	}

	pkMesh->InvalidateGeomCache();
	pkMesh->InvalidateTopologyCache();
	pkMesh->buildNormals();
	m_pkImpInterface->AddNodeToScene( pkMaxNode );

}

// create materials
void Importer::SetupMaterials( ASE_Material *pkASEMaterial )
{

	// we expect all materials in as submaterials in one single big scene material
	int iNumSubMats = pkASEMaterial->numsubmaterials;
	ASE_Material  *pkMainMaterial = pkASEMaterial->submaterials;
	ASE_Material  *pkCurrentMaterial = NULL;

	// create max material
	MultiMtl *pkMultiMat = ( MultiMtl* )m_pkImpInterface->Create( MATERIAL_CLASS_ID, Class_ID( MULTI_CLASS_ID, 0 ) );

	pkMultiMat->SetName( TSTR( _T( "SceneMaterials" ) ) );
	pkMultiMat->SetNumSubMtls( iNumSubMats );

	for ( int iSubMats = 0; iSubMats < iNumSubMats; iSubMats++ ) {

		pkCurrentMaterial = &pkMainMaterial[ iSubMats ];

		// skip materials without diffuse map ( but the mat list must have a dummy material due to referencing face indices )
		if ( pkCurrentMaterial->diffuseMaterialMap.image_path == NULL ) {

			StdMat *pkDummyMat = ( StdMat* )m_pkImpInterface->Create( MATERIAL_CLASS_ID, Class_ID( DMTL_CLASS_ID, 0 ) );
			pkDummyMat->SetName( TSTR( _T( "NoTexture" ) ) );
			pkMultiMat->SetSubMtl( iSubMats, pkDummyMat);
			m_vkMaterials.push_back( pkDummyMat );
			continue;

		}		

		StdMat		*pkMat;
		BitmapTex	*pkBitmap;

		// set both texture maps
		//-------------------------------------------------------//

		// setup base texture		
		pkMat = ( StdMat* )m_pkImpInterface->Create( MATERIAL_CLASS_ID, Class_ID( DMTL_CLASS_ID, 0 ) );
		pkMat->SetName( TSTR( _T( pkCurrentMaterial->name ) ) );
		pkMat->SetAmbient( Color( pkCurrentMaterial->ambient.r, pkCurrentMaterial->ambient.g, pkCurrentMaterial->ambient.b) , 0 );
		pkMat->SetDiffuse( Color( pkCurrentMaterial->diffuse.r, pkCurrentMaterial->diffuse.g, pkCurrentMaterial->diffuse.b ),0 );
		pkMat->SetSpecular( Color( pkCurrentMaterial->specular.r, pkCurrentMaterial->specular.g, pkCurrentMaterial->specular.b ),0 );
		pkMat->SetShininess( pkCurrentMaterial->shine, 0 );
		pkMat->SetShinStr( pkCurrentMaterial->shinestrength, 0 );
		pkMat->SetTwoSided( FALSE );
		pkMat->SetSelfIllum( 0, 0 );
		pkMat->SetTexmapAmt( ID_DI, 1, 0 );
		pkMat->EnableMap( ID_DI, TRUE );
		pkBitmap = ( BitmapTex* )m_pkImpInterface->Create( TEXMAP_CLASS_ID, Class_ID( BMTEX_CLASS_ID, 0 ) );
		pkMat->SetSubTexmap( ID_DI, pkBitmap );
		pkBitmap->SetMapName( pkCurrentMaterial->diffuseMaterialMap.image_path );

		// setup lightmap texture
		pkMat->SetTexmapAmt( ID_SI, 1, 0 );
		pkMat->EnableMap( ID_SI, TRUE );
		pkBitmap = ( BitmapTex* )m_pkImpInterface->Create( TEXMAP_CLASS_ID, Class_ID( BMTEX_CLASS_ID, 0 ) );
		pkMat->SetSubTexmap( ID_SI, pkBitmap );
		pkBitmap->SetMapName( pkCurrentMaterial->self_illumMaterialMap.image_path );
		StdUVGen *pkUVWGen = pkBitmap->GetUVGen();
		// set the lightmap uv channel
		pkUVWGen->SetMapChannel( MAP_CHANNEL_LIGHTMAP );
		//-------------------------------------------------------//


		// add the new submaterial material to main material
		pkMultiMat->SetSubMtl( iSubMats, pkMat);

		// add it to material list
		m_vkMaterials.push_back( pkMat );

	}
	
}
