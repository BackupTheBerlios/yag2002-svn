/***************************************************************************
                      exporter.h  -  Main exporter class
                             -------------------
    begin                : Thu Jun 5 2003
    copyright            : (C) 2003 by Reality Rift Studios
    email                : mattias@realityrift.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 * This software is provided 'as-is', without any express or implied       *
 * warranty. In no event will the authors be held liable for any damages   *
 * arising from the use of this software.                                  *
 *                                                                         *
 * Permission is granted to anyone to use this software for any purpose,   *
 * including commercial applications, and to alter it and redistribute     *
 * it freely, subject to the following restrictions:                       *
 *                                                                         *
 *   1. The origin of this software must not be misrepresented; you must   *
 *      not claim that you wrote the original software. If you use this    *
 *      software in a product, an acknowledgment in the product            *
 *      documentation would be appreciated but is not required.            *
 *                                                                         *
 *   2. Altered source versions must be plainly marked as such, and must   *
 *      not be misrepresented as being the original software.              *
 *                                                                         *
 *   3. This notice may not be removed or altered from any source          *
 *      distribution.                                                      *
 *                                                                         *
 ***************************************************************************/

// extended by botorabi (01/03/2004) : support for exporting multi-textured meshes
//                                     -  base texture on diffuse map and uv channel 1
//                                     -  lightmap texture on self-illumination map and channel 3
//
// boto ( 11/03/ 2004 )              : further extension for supporting bump channel and chunk aliases
//                                     in order to export bump map channel 

//                                     1. use the bump map in your material and assign a bitmap with a valid image
//                                     2. give your material following name: $bumpshadername material name
//                                         e.g. $bump_spacular dirty_asphalt


#include "exporter.h"
#include "bone.h"
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

using namespace std;
using namespace NeoEngine;
using namespace NeoChunkIO;


#define MAP_CHANNEL_TEXTURE     1
#define MAP_CHANNEL_LIGHTMAP    3


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
    
        const TCHAR                                  *ClassName() { return NSCEEXPORTER_CLASSNAME; }

        SClass_ID                                     SuperClassID() { return SCENE_EXPORT_CLASS_ID; }

        Class_ID                                      ClassID() { return NSCEEXPORTER_CLASSID; }

        const TCHAR                                  *Category() { return "NeoEngine"; }

        const TCHAR                                  *InternalName() { return _T( NSCEEXPORTER_CLASSNAME ); } // returns fixed parsable name (scripter-visible name)
    
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
    return _T( "Export to NeoEngine Scene File ( CTD Lightmap support )" );
}
    

const TCHAR *Exporter::ShortDesc() 
{           
    return _T( "CTD NSCE exporter (lightmap support)" );
}


const TCHAR *Exporter::AuthorName()
{           
    return _T( "Mattias Jansson (mattias@realityrift.com), A. Botorabi (botorabi@gmx.net)" );
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
    MessageBox( hWnd, "CTD NSCE exporter, support for exporting lightmaps\nMattias Jansson (mattias@realityrift.com)\nA. Botorabi (botorabi@gmx.net)\n\nReleased under zlib/libpng license", "About", MB_OK );
}


BOOL Exporter::SupportsOptions(int ext, DWORD options)
{
    return TRUE;
}

int Exporter::DoExport( const TCHAR *pszName, ExpInterface *pExpInterface, Interface *pInterface, BOOL bSuppressPrompts, unsigned long ulOptions )
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


    m_vpkBones.resize( m_pkIGameScene->GetTotalNodeCount() );

    vector< MaxBone* >::iterator ppkBone    = m_vpkBones.begin();
    vector< MaxBone* >::iterator ppkBoneEnd = m_vpkBones.end();

    for( ; ppkBone != ppkBoneEnd; ++ppkBone )
        *ppkBone = 0;


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


    //Compress node/bone array
    std::vector<int> viNodeMap( m_vpkBones.size(), -1 );

    {
        unsigned int uiBone;
        unsigned int uiCurBone = 0;
        int          iRootBone = -1;

        ppkBone    = m_vpkBones.begin();
        ppkBoneEnd = m_vpkBones.end();

        for( uiBone = 0; ppkBone != ppkBoneEnd; ++ppkBone, ++uiBone )
        {
            MaxBone *pkBone = *ppkBone;

            if( pkBone )
            {
                pkBone->m_iID = (int)uiCurBone;

                viNodeMap[ uiBone ] = uiCurBone++;

                if( uiBone != viNodeMap[ uiBone ] )
                    m_vpkBones[ viNodeMap[ uiBone ] ] = pkBone;

                if( ( pkBone->m_iParent < 0 ) && ( iRootBone < 0 ) )
                    iRootBone = viNodeMap[ uiBone ];
            }
        }

        //Resize bone vector
        m_vpkBones.resize( uiCurBone );

        //Modify parent indices and force single root joint
        ppkBone    = m_vpkBones.begin();
        ppkBoneEnd = m_vpkBones.end();

        for( uiBone = 0; ppkBone != ppkBoneEnd; ++ppkBone, ++uiBone )
        {
            MaxBone *pkBone = *ppkBone;

            assert( pkBone );
            assert( pkBone->m_iID == (int)uiBone );

            if( ( pkBone->m_iParent != -1 ) && ( viNodeMap[ pkBone->m_iParent ] >= 0 ) )
                pkBone->m_iParent = viNodeMap[ pkBone->m_iParent ];
            else
                pkBone->m_iParent = -1;
                
            if( pkBone->m_iParent < 0 )
            {
                if( (int)uiBone != iRootBone )
                    pkBone->m_iParent = iRootBone;
            }
        }
    }


    {
        string strName = pszName;

        strName = strName.substr( 0, strName.find_last_of( '.' ) );

        //Create blueprint chunk
        MeshChunk *pkMeshChunk = new MeshChunk;

        pkMeshChunk->AttachChunk( new StringChunk( strName, "name" ) );

        m_vpkBlueprints.push_back( pkMeshChunk );

        //Attach all submeshes
        vector< vector< MaxVertex* > >::iterator pvpkMeshVertices           = m_vvpkMeshVertices.begin();
        vector< vector< MaxVertex* > >::iterator pvpkMeshVerticesEnd        = m_vvpkMeshVertices.end();
        vector< vector< NeoEngine::Polygon > >::iterator pvkMeshPolygons    = m_vvkMeshPolygons.begin();
        vector< MaterialDefinition >::iterator pkMatDef                     = m_vMaterialDefinitions.begin();
        vector< IGameUVGen* >::iterator ppkTexGen                           = m_vpkTexGen.begin();
        vector< IGameUVGen* >::iterator ppkLmTexGen                         = m_vpkLmTexGen.begin();

        bool bSkeleton = false;

        for( ; pvpkMeshVertices != pvpkMeshVerticesEnd; ++pvpkMeshVertices, ++pvkMeshPolygons, ++pkMatDef, ++ppkTexGen, ++ppkLmTexGen )
        {
            if( !pvkMeshPolygons->size() )
                continue;

            bool bSkin      = (*pvpkMeshVertices)[0]->m_pkSkinVertex ? true : false;
            bool bTexCoords = (*pvpkMeshVertices)[0]->m_bUV;
            bool bLmTexCoords = (*pvpkMeshVertices)[0]->m_bLmUV;

            if( bSkin )
                bSkeleton = true;

            Chunk *pkSubMeshChunk = ChunkFactory::CreateChunk( bSkin ? ChunkType::SKELETALSUBMESH : ChunkType::SUBMESH, "" );
            pkMeshChunk->AttachChunk( pkSubMeshChunk );


            PolygonBufferPtr pkPolygonBuffer     = new PolygonBuffer( Buffer::STATIC | Buffer::READPRIORITIZED, pvkMeshPolygons->size() );

            PolygonBufferChunk *pkPolygonChunk   = new PolygonBufferChunk( pkPolygonBuffer, "", true );

            pkSubMeshChunk->AttachChunk( pkPolygonChunk );

            pkPolygonBuffer->Lock( Buffer::WRITE );

            NeoEngine::Polygon *pkPolygon = pkPolygonBuffer->GetPolygon();

            vector< NeoEngine::Polygon >::iterator pkSrcPolygon    = pvkMeshPolygons->begin();
            vector< NeoEngine::Polygon >::iterator pkSrcPolygonEnd = pvkMeshPolygons->end();

            for( ; pkSrcPolygon != pkSrcPolygonEnd; ++pkSrcPolygon, ++pkPolygon )
                *pkPolygon = *pkSrcPolygon;

            pkPolygonBuffer->Unlock();

            VertexBufferPtr pkVertexBuffer;
            if ( bSkin == true ) {

                // here a refinement could differ between vertex color vertex and base textured vertex!
                pkVertexBuffer = new VertexBuffer( Buffer::DYNAMIC | Buffer::READPRIORITIZED | Buffer::NORENDER, pvpkMeshVertices->size(), &NormalTexVertex::s_kDecl );
            
            } else {

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

            }

            VertexBufferChunk *pkVertexChunk = new VertexBufferChunk( pkVertexBuffer, "", true );

            pkVertexBuffer->Lock( Buffer::WRITE );

            unsigned char *pucVertex = (unsigned char*)pkVertexBuffer->GetVertex();

            vector< MaxVertex* >::iterator ppkSrcVertex    = pvpkMeshVertices->begin();
            vector< MaxVertex* >::iterator ppkSrcVertexEnd = pvpkMeshVertices->end();

            for( ; ppkSrcVertex != ppkSrcVertexEnd; ++ppkSrcVertex, pucVertex += pkVertexBuffer->GetVertexSize() )
            {
                NormalVertex *pkVertex = (NormalVertex*)pucVertex;
                pkVertex->m_kPosition = g_kConvMat * Vector3d( (*ppkSrcVertex)->m_kCoord.x, (*ppkSrcVertex)->m_kCoord.z, -(*ppkSrcVertex)->m_kCoord.y );
                pkVertex->m_kNormal   = g_kConvMat * Vector3d( (*ppkSrcVertex)->m_kNormal.x, (*ppkSrcVertex)->m_kNormal.z, -(*ppkSrcVertex)->m_kNormal.y );;

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

            
            if( bSkin )
            {
                SkinChunk *pkSkinChunk = new SkinChunk;

                pkSkinChunk->AttachChunk( pkVertexChunk );

                pkSubMeshChunk->AttachChunk( pkSkinChunk );


                //Add weighting data
                SkinVertex *pkSkinBuffer = new SkinVertex[ pvpkMeshVertices->size() ];

                SkinVertexBufferChunk *pkSkinBufferChunk = new SkinVertexBufferChunk( pkSkinBuffer, pvpkMeshVertices->size(), "" );

                pkSkinBufferChunk->AttachChunk( new IntChunk( (int)pvpkMeshVertices->size(), "num" ) );

                ppkSrcVertex    = pvpkMeshVertices->begin();
                ppkSrcVertexEnd = pvpkMeshVertices->end();

                SkinVertex *pkDst = pkSkinBuffer;

                for( ; ppkSrcVertex != ppkSrcVertexEnd; ++ppkSrcVertex, ++pkDst )
                {
                    SkinVertex *pkSrc = (*ppkSrcVertex)->m_pkSkinVertex;

                    pkDst->m_iNumInfluences = pkSrc->m_iNumInfluences;
                    pkDst->m_pkInfluences   = new BoneInfluence[ pkSrc->m_iNumInfluences ];

                    for( int iInfluence = 0; iInfluence < pkSrc->m_iNumInfluences; ++iInfluence )
                    {
                        //Modify bone indices in weights with node remapping vector
                        pkDst->m_pkInfluences[ iInfluence ].m_iBoneID = viNodeMap[ pkSrc->m_pkInfluences[ iInfluence ].m_iBoneID ];
                        pkDst->m_pkInfluences[ iInfluence ].m_fWeight = pkSrc->m_pkInfluences[ iInfluence ].m_fWeight;
                        pkDst->m_pkInfluences[ iInfluence ].m_kOffset = g_kConvMat * pkSrc->m_pkInfluences[ iInfluence ].m_kOffset;
                    }
                }

                pkSkinChunk->AttachChunk( pkSkinBufferChunk );
            }
            else
                pkSubMeshChunk->AttachChunk( pkVertexChunk );

            // create material chunk
            //-----------------------------------------------------------------------------------------//

            Chunk *pkMatChunk = ChunkFactory::CreateChunk( ChunkType::MATERIAL, "" );
            pkSubMeshChunk->AttachChunk( pkMatChunk );

            pkMatChunk->AttachChunk( new StringChunk( pkMatDef->m_strMaterialName, "name" ) );

            // check for material definition type
            if ( pkMatDef->m_eDefinitionType == MaterialDefinition::eFile ) {
    
                StringChunk *pkAlias = new StringChunk( string( "tex_base string " ) + pkMatDef->m_strBaseTexture, "defalias" );
                pkMatChunk->AttachChunk( pkAlias );

                if ( pkMatDef->m_strLightmapTexture.length() ) {

                    StringChunk *pkAlias = new StringChunk( string( "tex_lm string " ) + pkMatDef->m_strLightmapTexture, "defalias" );
                    pkMatChunk->AttachChunk( pkAlias );
                
                }
                if ( pkMatDef->m_strHeightmapTexture.length() ) {

                    StringChunk *pkAlias = new StringChunk( string( "tex_height string " ) + pkMatDef->m_strHeightmapTexture, "defalias" );
                    pkMatChunk->AttachChunk( pkAlias );
                
                }

                pkMatChunk->AttachChunk( new StringChunk( pkMatDef->m_strMaterialFileName, "file" ) );

            } else {

                bool bBaseAndLightmapTexture = pkMatDef->m_strBaseTexture.length() && pkMatDef->m_strLightmapTexture.length();
                if( pkMatDef->m_strBaseTexture.length() )
                {
                    Chunk *pkTexLayerChunk = ChunkFactory::CreateChunk( ChunkType::TEXTURELAYER, "" );
                    pkMatChunk->AttachChunk( pkTexLayerChunk );

                    pkTexLayerChunk->AttachChunk( new StringChunk( pkMatDef->m_strBaseTexture, "texture" ) );

                    if ( bBaseAndLightmapTexture == true ) {

                        pkTexLayerChunk->AttachChunk( new StringChunk( "one zero", "blend" ) );

                    }

                }

                if( pkMatDef->m_strLightmapTexture.length() )
                {
                    Chunk *pkTexLayerChunk = ChunkFactory::CreateChunk( ChunkType::TEXTURELAYER, "" );
                    pkMatChunk->AttachChunk( pkTexLayerChunk );

                    pkTexLayerChunk->AttachChunk( new StringChunk( pkMatDef->m_strLightmapTexture, "texture" ) );

                    if ( bBaseAndLightmapTexture == true ) {

                        pkTexLayerChunk->AttachChunk( new StringChunk( "destcolor zero", "blend" ) );
                        pkTexLayerChunk->AttachChunk( new IntChunk( 1,  "uv" ) );

                    }
                }
            }
        }

        if( bSkeleton )
        {
            //Create skeleton
            Chunk *pkSkeletonChunk = ChunkFactory::CreateChunk( ChunkType::SKELETON, "" );
            pkMeshChunk->AttachChunk( pkSkeletonChunk );

            vector< MaxBone* >::iterator ppkBone    = m_vpkBones.begin();
            vector< MaxBone* >::iterator ppkBoneEnd = m_vpkBones.end();

            for( ; ppkBone != ppkBoneEnd; ++ppkBone )
            {
                Chunk *pkBoneChunk = ChunkFactory::CreateChunk( ChunkType::BONE, "" );
                pkSkeletonChunk->AttachChunk( pkBoneChunk );

                pkBoneChunk->AttachChunk( new IntChunk( (*ppkBone)->m_iID, "id" ) );
                pkBoneChunk->AttachChunk( new IntChunk( (*ppkBone)->m_iParent, "parent" ) );

                if( (*ppkBone)->m_strName.length() )
                    pkBoneChunk->AttachChunk( new StringChunk( (*ppkBone)->m_strName, "name" ) );

                Quaternion kQuat = GMatrixToQuat( (*ppkBone)->m_kLocalTM );

                Vector3d kPos( (*ppkBone)->m_kLocalTM.GetRow( 3 )[0], (*ppkBone)->m_kLocalTM.GetRow( 3 )[1], (*ppkBone)->m_kLocalTM.GetRow( 3 )[2] );

                pkBoneChunk->AttachChunk( new VectorChunk( g_kConvMat * kPos, "pos" ) );
                pkBoneChunk->AttachChunk( new QuaternionChunk( g_kConvQuat * kQuat, "rot" ) );
            }


            Chunk *pkSkelAnimChunk = ChunkFactory::CreateChunk( ChunkType::SKELETONANIMATION, "" );

            float fStartTime = (float)m_iStartFrame / m_fFramesPerSecond;
            float fEndTime   = (float)m_iEndFrame   / m_fFramesPerSecond;
            float fLength    = fEndTime - fStartTime;
            int   iNumFrames = m_iEndFrame - m_iStartFrame;

            pkSkelAnimChunk->AttachChunk( new StringChunk( strName, "name" ) );
            pkSkelAnimChunk->AttachChunk( new IntChunk( 0, "id" ) );
            pkSkelAnimChunk->AttachChunk( new FloatChunk( fLength, "length" ) );

            m_vpkAnimations.push_back( pkSkelAnimChunk );

            ppkBone    = m_vpkBones.begin();
            ppkBoneEnd = m_vpkBones.end();

            for( ; ppkBone != ppkBoneEnd; ++ppkBone )
            {
                Chunk *pkChannelChunk = ChunkFactory::CreateChunk( ChunkType::NODEANIMATION, "" );

                pkSkelAnimChunk->AttachChunk( pkChannelChunk );

                for( int iFrame = 0; iFrame < iNumFrames; ++iFrame )
                {
                    GMatrix kLocalMat = (*ppkBone)->m_pkNode->GetLocalTM( iFrame * m_iTicksPerFrame );

                    Quaternion kQuat = GMatrixToQuat( kLocalMat );

                    Vector3d kPos( kLocalMat.GetRow( 3 )[0], kLocalMat.GetRow( 3 )[1], kLocalMat.GetRow( 3 )[2] );

                    Chunk *pkFrameChunk = ChunkFactory::CreateChunk( ChunkType::NODEKEYFRAME, "" );
    
                    pkChannelChunk->AttachChunk( pkFrameChunk );
        
                    pkFrameChunk->AttachChunk( new VectorChunk( g_kConvMat * kPos, "pos" ) );
                    pkFrameChunk->AttachChunk( new QuaternionChunk( g_kConvQuat * kQuat, "rot" ) );
                    pkFrameChunk->AttachChunk( new FloatChunk( (float)iFrame / (float)iNumFrames, "time" ) );
                }
            }

            ppkBone    = m_vpkBones.begin();
            ppkBoneEnd = m_vpkBones.end();

            for( ; ppkBone != ppkBoneEnd; ++ppkBone )
                delete( *ppkBone );

            m_vpkBones.clear();
        }
    }


    //Force lowercase extension
    if( !strcmp( pszName + strlen( pszName ) - 4, "NSCE" ) )
        memcpy( (void*)( pszName + strlen( pszName ) - 4 ), "nsce", 4 );

    //Write scene file
    {
        File *pkOutFile = new File( "", pszName, ios_base::out | ios_base::binary, true );

        if( !pkOutFile->IsValid() )
        {
            MessageBox( 0, "ERROR: Unable to open out file!", "nsce exporter", MB_OK );
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


        vector< Chunk* >::iterator ppkBlueprint    = m_vpkBlueprints.begin();
        vector< Chunk* >::iterator ppkBlueprintEnd = m_vpkBlueprints.end();

        for( ; ppkBlueprint != ppkBlueprintEnd; ++ppkBlueprint )
        {
            pkIO->WriteChunk( (*ppkBlueprint), pkOutFile );
            delete (*ppkBlueprint);
        }

        m_vpkBlueprints.clear();


        delete pkIO;
        delete pkOutFile;
    }

    //Write animation file
    if( m_vpkAnimations.size() )
    {
        memcpy( (void*)( pszName + strlen( pszName ) - 4 ), "nani", 4 );

        File *pkOutFile = new File( "", pszName, ios_base::out | ios_base::binary, true );

        if( !pkOutFile->IsValid() )
        {
            MessageBox( 0, "ERROR: Unable to open out file!", "nsce exporter", MB_OK );
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


        vector< Chunk* >::iterator ppkAnimation    = m_vpkAnimations.begin();
        vector< Chunk* >::iterator ppkAnimationEnd = m_vpkAnimations.end();

        for( ; ppkAnimation != ppkAnimationEnd; ++ppkAnimation )
        {
            pkIO->WriteChunk( (*ppkAnimation), pkOutFile );
            delete (*ppkAnimation);
        }

        m_vpkAnimations.clear();


        delete pkIO;
        delete pkOutFile;
    }


    m_pkIGameScene->ReleaseIGame();

    pkCoreInterface->ProgressEnd();


    ppkBone    = m_vpkBones.begin();
    ppkBoneEnd = m_vpkBones.end();

    for( ; ppkBone != ppkBoneEnd; ++ppkBone )
        delete( *ppkBone );

    m_vpkBones.clear();

    NeoChunkIO::Core::Get()->Shutdown();

    NeoEngine::Core::Get()->Shutdown();


    return TRUE;
}


void Exporter::ExportNodeInfo( IGameNode *pkNode, int &iCurNode )
{
    TSTR strBuf;

    strBuf = TSTR( "Processing: " ) + TSTR( pkNode->GetName() );
    GetCOREInterface()->ProgressUpdate( (int)( ( (float)iCurNode++ / (float)m_pkIGameScene->GetTotalNodeCount() ) * 100.0f ), FALSE, strBuf.data() ); 

        // skip entity objects
    if ( pkNode->GetName()[0] == '$' ) {

        return;

    }

    if( pkNode->IsGroupOwner() )
    {
        // safe to ignore?
    }
    else
    {
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
                    MessageBox( 0, "ERROR: Unable to initialize mesh data!", "nsce exporter", MB_OK );
                    break;
                }

                if( ( pkSkin = pkMesh->GetIGameSkin() ) )
                {
                    if( pkMesh->GetNumberOfVerts() != pkSkin->GetNumOfSkinnedVerts() )
                    {
                        MessageBox( 0, "ERROR: Mesh vertices and skin vertices does not match!", "nsce exporter", MB_OK );
                        break;
                    }
                }


                Vector3d    *pkCoords           = 0;
                MaxTexCoord *pkTexCoords        = 0;
                MaxTexCoord *pkLightmapCoords   = 0;
                Vector3d    *pkNormals          = 0;
                SkinVertex  *pkSkinVerts        = 0;

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

                pkTexCoords         = ( iTexVerts > 0 ) ? new MaxTexCoord[ iTexVerts ] : 0;
                pkLightmapCoords    = ( iLmVerts > 0 ) ? new MaxTexCoord[ iLmVerts ] : 0;

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
                    Point3 VertPoint = pkMesh->GetVertex( iVert );

                    pkCoords[ iVert ].x = VertPoint.x;
                    pkCoords[ iVert ].y = VertPoint.y;
                    pkCoords[ iVert ].z = VertPoint.z;
                }

                if( pkSkin )
                {
                    pkSkinVerts = ( iVerts > 0 ) ? new SkinVertex[ iVerts ] : 0;

                    for( iVert = 0; iVert < iVerts; ++iVert )
                    {
                        pkSkinVerts[ iVert ].m_iNumInfluences = pkSkin->GetNumberOfBones( iVert );
                        pkSkinVerts[ iVert ].m_pkInfluences   = new BoneInfluence[ pkSkinVerts[ iVert ].m_iNumInfluences ];

                        for( int iBone = 0; iBone < pkSkinVerts[ iVert ].m_iNumInfluences; ++iBone )
                        {
                            IGameNode *pkBone = pkSkin->GetIGameBone( iVert, iBone );

                            //Add bone
                            AddBone( pkBone );

                            GMatrix BindTM;
                            GMatrix InvBindTM;
                            Point3  Offset;
                                
                            BindTM = pkBone->GetWorldTM();
                            //pSkin->GetInitBoneTM( pSkin->GetBone( iVert, iBone ), BindTM );

                            pkSkinVerts[ iVert ].m_pkInfluences[ iBone ].m_fWeight = pkSkin->GetWeight( iVert, iBone );
                            pkSkinVerts[ iVert ].m_pkInfluences[ iBone ].m_iBoneID = pkBone->GetNodeID() - 1;
                                
                            Offset[0] = pkCoords[ iVert ].x - BindTM.GetRow( 3 )[0];
                            Offset[1] = pkCoords[ iVert ].y - BindTM.GetRow( 3 )[1];
                            Offset[2] = pkCoords[ iVert ].z - BindTM.GetRow( 3 )[2];

                            InvBindTM.SetColumn( 0, BindTM.GetRow( 0 ) );
                            InvBindTM.SetColumn( 1, BindTM.GetRow( 1 ) );
                            InvBindTM.SetColumn( 2, BindTM.GetRow( 2 ) );
                            InvBindTM.SetColumn( 3, Point4( 0.0f, 0.0f, 0.0f, 1.0f ) );

                            Offset = Offset * InvBindTM;

                            pkSkinVerts[ iVert ].m_pkInfluences[ iBone ].m_kOffset.x = Offset.x;
                            pkSkinVerts[ iVert ].m_pkInfluences[ iBone ].m_kOffset.y = Offset.y;
                            pkSkinVerts[ iVert ].m_pkInfluences[ iBone ].m_kOffset.z = Offset.z;
                        }
                    }


                    unsigned int uiBone = 0;

                    vector< MaxBone* >::iterator ppkBone    = m_vpkBones.begin();
                    vector< MaxBone* >::iterator ppkBoneEnd = m_vpkBones.end();

                    for( ; ppkBone != ppkBoneEnd; ++ppkBone, ++uiBone )
                    {
                        MaxBone *pkBone = *ppkBone;
                            
                        if( pkBone && ( pkBone->m_iParent >= 0 ) )
                        {
                            MaxBone *pkParentBone = m_vpkBones[ pkBone->m_iParent ];

                            if( !pkParentBone )
                            {
                                char szStr[256];

                                _snprintf( szStr, 256, "ERROR: Bone [%d] has null parent bone ptr at index [%d]", uiBone, pkBone->m_iParent );

                                MessageBox( 0, szStr, "nsce exporter", MB_OK );

                                //Add default bone
                                pkParentBone = new MaxBone;

                                pkParentBone->m_iID       = pkBone->m_iParent;
                                pkParentBone->m_iParent   = -1;
                                pkParentBone->m_strName   = "__missing_parent";

                                pkParentBone->m_kWorldTM.SetColumn( 0, Point4( 1.0f, 0.0f, 0.0f, 0.0f ) );
                                pkParentBone->m_kWorldTM.SetColumn( 1, Point4( 0.0f, 1.0f, 0.0f, 0.0f ) );
                                pkParentBone->m_kWorldTM.SetColumn( 2, Point4( 0.0f, 0.0f, 1.0f, 0.0f ) );
                                pkParentBone->m_kWorldTM.SetColumn( 3, Point4( 0.0f, 0.0f, 0.0f, 1.0f ) );

                                m_vpkBones[ pkBone->m_iParent ] = pkParentBone;
                            }
                        }
                    }
                }



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

                    vector< MaterialDefinition >::iterator pkMatDef      = m_vMaterialDefinitions.begin();
                    vector< MaterialDefinition >::iterator pkMatDefEnd   = m_vMaterialDefinitions.end();

                    vector< vector< MaxVertex*   > >::iterator pvpkMeshVertices = m_vvpkMeshVertices.begin();
                    vector< vector< NeoEngine::Polygon > >::iterator pvkMeshPolygons  = m_vvkMeshPolygons.begin();

                    string strBaseTexture, strLmTexture, strBumpTexture;
                    int iSlash;
                    int iLength;

                    IGameMaterial *pkMat = pkMesh->GetMaterialFromFace( pkFace );
                    IGameTextureMap *pkTexture     = NULL;
                    IGameTextureMap *pkBaseTexture = NULL;
                    IGameTextureMap *pkLmTexture   = NULL;
                    BitmapTex       *pkBumpTexture = NULL;;

                    // botorabi: if no material found then skip this face
                    if (pkMat == NULL) {

                        //MessageBox(NULL, "Missing material for mesh!", "Warning", MB_OK);
                        continue;

                    }
                    // check all texture maps for this material
                    for ( int iTexNum = 0; iTexNum < pkMat->GetNumberOfTextureMaps(); iTexNum++ ) {
                        
                        pkTexture = pkMat->GetIGameTextureMap( iTexNum );
                        int iMapSlot = pkTexture->GetStdMapSlot();
 
                        // check for diffuse map ( we use this as base map )
                        if ( iMapSlot == ID_DI ) {

                            pkBaseTexture = pkTexture;

                        } else {
                        
                            // check for self-illumination map ( we use this as lightmap )
                            if ( iMapSlot == ID_SI ) {

                                pkLmTexture = pkTexture;

                            } 
                            // for some stupid reason it is not possible to retrieve the bump channel here, see below!
                            //else {

                            //    // check for bump map
                            //    if ( iMapSlot == ID_BU ) {

                            //        pkBumpTexture = pkTexture;

                            //    }

                            //}

                        }

                    }

                    // check for bump map channel
                    Mtl         *pkMaxMat        = pkMat->GetMaxMaterial();
                    Texmap      *pkBumpMap       = pkMaxMat->GetSubTexmap( ID_BU );
                    if( pkBumpMap->ClassID() == Class_ID(BMTEX_CLASS_ID, 0 ) ) {
                        pkBumpTexture = ( BitmapTex* )pkBumpMap;
                    }

                    string strMaterialName = pkMat->GetMaterialName();
                       
                    // get base texture name
                    if( pkBaseTexture && pkBaseTexture->GetBitmapFileName() )
                    {

                        TCHAR   *pcBmpFileName = pkBaseTexture->GetBitmapFileName();

                        // this seems to be a problem: sometimes the the function "GetBitmapFileName" returns an id instead of texture file name!?
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

                        TCHAR   *pcBmpFileName = pkLmTexture->GetBitmapFileName();

                        // this seems to be a problem: sometimes the the function "GetBitmapFileName" returns an id instead of texture file name!?
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

                    // get bumptmap texture name
                    if( pkBumpTexture && pkBumpTexture->GetMapName() )
                    {

                        TCHAR   *pcBmpFileName = pkBumpTexture->GetMapName();

                        // this seems to be a problem: sometimes the the function "GetMapName" returns an id instead of texture file name!?
                        if ( ( ( int )pcBmpFileName ) != TEXMAP_CLASS_ID ) {

                            strBumpTexture = pkBumpTexture->GetMapName();

                            if( ( ( iSlash = strBumpTexture.find_last_of( '\\' ) ) == string::npos ) && ( ( iSlash = strBumpTexture.find_last_of( '/' ) ) == string::npos ) )
                                iSlash = 0;
                            else
                                ++iSlash;

                            strBumpTexture = strBumpTexture.substr( iSlash, ( ( ( iLength = strBumpTexture.find_last_of( '.' ) ) != string::npos ) && ( iLength > iSlash ) ) ? ( iLength - iSlash ) : string::npos );

                        } else {

                            strBumpTexture = "";

                        }
                    }

                    for( ; pkMatDef != pkMatDefEnd; ++pkMatDef, ++pvpkMeshVertices, ++pvkMeshPolygons ) {

                        if( ( pkMatDef->m_strBaseTexture == strBaseTexture ) && ( pkMatDef->m_strLightmapTexture == strLmTexture ) ) {

                            break;

                        }
                    }
                    if( pkMatDef == pkMatDefEnd )
                    {
                        //Add new mesh
                        m_vvpkMeshVertices.push_back( vector< MaxVertex* >() );
                        m_vvkMeshPolygons.push_back(  vector< NeoEngine::Polygon >() );

                        MaterialDefinition   kMaterialDef;

                        kMaterialDef.m_strMaterialName     = strMaterialName;
                        // check for file type material
                        if ( strMaterialName[ 0 ] == '$' ) {
                         
                            // extract the material name and its material file name
                            //  deconding following format: $MaterialFileName MaterialName
                            char pcMatName[ 128 ];
                            char pcMatFileName[ 128 ];
                            sscanf( strMaterialName.c_str(), "%s %s", pcMatFileName, pcMatName );

                            kMaterialDef.m_strMaterialName = pcMatName;
                            kMaterialDef.m_strMaterialFileName = pcMatFileName;
                            kMaterialDef.m_strMaterialFileName.erase( 0, 1 ); // strip the leading character "$"

                            kMaterialDef.m_eDefinitionType = MaterialDefinition::eFile;

                        }
                        kMaterialDef.m_strBaseTexture      = strBaseTexture;
                        kMaterialDef.m_strLightmapTexture  = strLmTexture;
                        kMaterialDef.m_strHeightmapTexture = strBumpTexture;
                        m_vMaterialDefinitions.push_back( kMaterialDef );

                        // note: we assume that the bump channel has the same uv as base channel!
                        m_vpkTexGen.push_back( pkTexture ? pkTexture->GetIGameUVGen() : 0 );
                        m_vpkLmTexGen.push_back( pkLmTexture ? pkLmTexture->GetIGameUVGen() : 0 );

                        pvpkMeshVertices = m_vvpkMeshVertices.end() - 1;
                        pvkMeshPolygons  = m_vvkMeshPolygons.end() - 1;
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
                            pkVertex->m_bUV     = true;

                        }

                        if( pkLightmapCoords ) {

                            pkVertex->m_kLmUV   = pkLightmapCoords[ aiLmUV[ iIndex ] ];
                            pkVertex->m_bLmUV   = true;

                        }

                        unsigned int uiVertex = 0;

                        for( ; ppkVertex != ppkVertexEnd; ++ppkVertex, ++uiVertex )
                            if( **ppkVertex == *pkVertex )
                                break;

                        if( ppkVertex == ppkVertexEnd )
                        {
                            if( pkSkin )
                            {
                                pkVertex->m_pkSkinVertex = new SkinVertex;

                                pkVertex->m_pkSkinVertex->m_iNumInfluences = pkSkinVerts[ aiIndex[ iIndex ] ].m_iNumInfluences;
                                pkVertex->m_pkSkinVertex->m_pkInfluences   = new BoneInfluence[ pkVertex->m_pkSkinVertex->m_iNumInfluences ];

                                for( int iInfluence = 0; iInfluence < pkVertex->m_pkSkinVertex->m_iNumInfluences; ++iInfluence )
                                {
                                    pkVertex->m_pkSkinVertex->m_pkInfluences[ iInfluence ].m_fWeight = pkSkinVerts[ aiIndex[ iIndex ] ].m_pkInfluences[ iInfluence ].m_fWeight;
                                    pkVertex->m_pkSkinVertex->m_pkInfluences[ iInfluence ].m_iBoneID = pkSkinVerts[ aiIndex[ iIndex ] ].m_pkInfluences[ iInfluence ].m_iBoneID;
                                    pkVertex->m_pkSkinVertex->m_pkInfluences[ iInfluence ].m_kOffset = pkSkinVerts[ aiIndex[ iIndex ] ].m_pkInfluences[ iInfluence ].m_kOffset;
                                }
                            }

                            pvpkMeshVertices->push_back( pkVertex );
                        }
                        else
                            delete pkVertex;

                        kPolygon.v[ iIndex ] = uiVertex;
                    }

                    pvkMeshPolygons->push_back( kPolygon );
                }

                delete [] pkSkinVerts;
                delete [] pkCoords;
                delete [] pkTexCoords;

                break;
            }

            case IGameObject::IGAME_HELPER:
            {
//              IGameSupportObject * hO = (IGameSupportObject*)obj;
//              IPropertyContainer * cc = hO->GetIPropertyContainer();
//              IGameMesh * hm = hO->GetMeshObject();
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
                //MessageBox( 0, "WARNING: Unknown node type", "md5mesh Export", MB_OK );
                break;
            }
        }

        /*
        if(exportModifiers)
        {
            int numMod = obj->GetNumModifiers();
            if(numMod > 0)
            {
                CComPtr <IXMLDOMNode> mod;
                CreateXMLNode(pXMLDoc,parent,_T("Modifiers"),&mod);
                TSTR Buf;
                buf.printf("%d",numMod);
                AddXMLAttribute(mod,_T("count"),buf.data());

                for(int i=0;i<numMod;i++)
                {
                    IGameModifier * m = obj->GetIGameModifier(i);
                    DumpModifiers(mod,m);
                }
            }
        }
        */
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


void Exporter::AddBone( IGameNode *pkBone )
{
    if( !pkBone )
        return;

    int iBoneID = pkBone->GetNodeID() - 1;

    if( iBoneID >= (int)m_vpkBones.size() )
    {
        unsigned int uiOldSize = m_vpkBones.size();

        m_vpkBones.resize( iBoneID + 1 );

        vector< MaxBone* >::iterator ppkBone    = m_vpkBones.begin() + uiOldSize;
        vector< MaxBone* >::iterator ppkBoneEnd = m_vpkBones.end();

        for( ; ppkBone != ppkBoneEnd; ++ppkBone )
            *ppkBone = 0;
    }
    else if( m_vpkBones[ iBoneID ] )
        return;

    MaxBone *pkMaxBone  = new MaxBone;

    pkMaxBone->m_iID      = iBoneID;
    pkMaxBone->m_strName  = pkBone->GetName();
    pkMaxBone->m_iParent  = pkBone->GetNodeParent() ? pkBone->GetNodeParent()->GetNodeID() - 1 : -1;
    pkMaxBone->m_kWorldTM = pkBone->GetWorldTM();
    pkMaxBone->m_kLocalTM = pkBone->GetLocalTM();
    pkMaxBone->m_pkNode   = pkBone;

    m_vpkBones[ iBoneID ] = pkMaxBone;

    AddBone( pkBone->GetNodeParent() );
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

