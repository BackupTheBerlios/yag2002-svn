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
 # neoengine, animated mesh class
 #
 # this class implements an keyframe animated mesh
 #
 #
 #   date of creation:  02/23/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>

#include "ctd_animatedmesh.h"
#include <ctd_printf.h>

#include <neochunkio/nodeanimation.h>
#include <neochunkio/scenenode.h>
#include <neochunkio/stdstring.h>
#include <neochunkio/float.h>
#include <neochunkio/integer.h>

using namespace std;
using namespace CTD;
using namespace NeoEngine;
using namespace NeoChunkIO;

namespace CTD_IPluginVisuals {


// plugin global entity descriptor for piston
CTDAnimMeshDesc g_pkAnimatedMeshEntity_desc;
//-------------------------------------------//



CTDAnimMesh::CTDAnimMesh()
{

    CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Visuals) entity ' AnimatedMesh ' created " );

    m_pkMeshEntity          = NULL;
    m_vOffsetPosition       = Vector3d( 0.0f, 0.0f, 0.0f );
    m_vOffsetRotation       = Vector3d( 0.0f, 0.0f, 0.0f );

}

CTDAnimMesh::~CTDAnimMesh()
{

    CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Visuals) entity ' AnimatedMesh ' destroyed " );

}

// init entity, this is called after entity parameter assignments at startup
void CTDAnimMesh::Initialize() 
{ 

    // set the entity ( node ) name
    SetName( CTD_ENTITY_NAME_AnimMesh );


    Quaternion kOffsetRot = Quaternion( EulerAngles( m_vOffsetRotation.x * PI / 180.0f, m_vOffsetRotation.y * PI / 180.0f, m_vOffsetRotation.z * PI / 180.0f ) );
    SetRotation( kOffsetRot );
    SetTranslation( m_vOffsetPosition );
    //------------------------------//

    // setup the key animation track
    //-----------------------------//
    m_pkAnimatedNode = ReadKeyframes( m_strAnimationFile );
    if ( m_pkAnimatedNode == NULL ) {

        CTDCONSOLE_PRINT( LogLevel( WARNING ), " (Plugin Visuals) entity ' AnimatedMesh::" + GetInstanceName() + " ': could not load animation data, entity deactivated! " );
        Deactivate();
        return;

    }
    //--------------------------------//

    
}

void CTDAnimMesh::PostInitialize() 
{

    m_pkMeshEntity = Framework::Get()->FindEntity( m_strMeshEntiyName, CTD_PLUGINNAME_VISUALS );
    if ( !m_pkMeshEntity ) {

        Deactivate();
        CTDCONSOLE_PRINT( LogLevel( WARNING ), " (Plugin Visuals) entity ' AnimatedMesh::" + GetInstanceName() + " ' could not find mesh entity, entity deactivated! " );
        return;

    }
    m_pkAnimatedNode->SetEntity( m_pkMeshEntity->GetEntity() );

}


void CTDAnimMesh::UpdateEntity( float fDeltaTime ) 
{ 

    m_pkAnimatedNode->Update( fDeltaTime );

    // update node's position and orientation
    Vector3d    kTranslation = m_pkAnimatedNode->GetTranslation();
    Quaternion  kOrienation  = m_pkAnimatedNode->GetRotation();

    SetTranslation( kTranslation );
    SetRotation( kOrienation );
    m_pkMeshEntity->SetTranslation( kTranslation );
    m_pkMeshEntity->SetRotation( kOrienation );

}   


bool CTDAnimMesh::Render( Frustum *pkFrustum, bool bForce ) 
{

     if( bForce ) {
         
         m_uiLastFrame = s_uiFrameCount; 
         return true; 
     } 

     if( !m_bActive || ( m_uiLastFrame >= s_uiFrameCount ) ) 
         return false; 

     m_uiLastFrame = s_uiFrameCount; 

     // render the mesh
     m_pkAnimatedNode->Render( pkFrustum, bForce );

     return true;
}


// read the keyframes stored in an ".nani" file
AnimatedNode* CTDAnimMesh::ReadKeyframes( string &strFileName )
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

        if( pkChunk->GetType() == ChunkType::ANIMATEDNODE /*ANIMATEDNODE NODEANIMATION*/ ) {

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

int CTDAnimMesh::Message(int iMsgId, void *pkMsgStruct) 
{ 
    return 0; 
}   

int CTDAnimMesh::ParameterDescription( int iParamIndex, ParameterDescriptor *pkDesc )
{

    int iParamCount = 5;

    if (pkDesc == NULL) {

        return iParamCount;
    }


    switch( iParamIndex ) 
    {

    case 0:

        pkDesc->SetName( "OffsetPosition" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_VECTOR3 );
        pkDesc->SetVar( &m_vOffsetPosition );
        
        break;

    case 1:

        pkDesc->SetName( "OffsetRotation" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_VECTOR3 );
        pkDesc->SetVar( &m_vOffsetRotation );
        
        break;

    case 2:

        pkDesc->SetName( "MeshEntity" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_STRING );
        pkDesc->SetVar( &m_strMeshEntiyName );
        
        break;

    case 3:

        pkDesc->SetName( "AnimationFile" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_STRING );
        pkDesc->SetVar( &m_strAnimationFile );

        break;

    default:
        
        return -1;
    
    }

    return iParamCount;

}

} // namespace CTD_IPluginVisuals 
