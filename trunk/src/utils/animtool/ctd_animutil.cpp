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
 # keyframe animation tool
 #
 #
 #   date of creation:  10/16/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include "base.h"
#include "ctd_animutil.h"


//#define PRINTF
#define PRINTF  printf

using namespace NeoEngine;
using namespace NeoChunkIO;
using namespace std;

KeyFrameAnimUtil::KeyFrameAnimUtil()
{

    m_pkAnimatedNode = NULL;
    m_pkNodeAnim     = NULL;
    m_uiBeginFrame   = 0;
    m_uiEndFrame     = 0;

}

KeyFrameAnimUtil::~KeyFrameAnimUtil()
{
    if ( m_pkAnimatedNode ) {

        delete m_pkAnimatedNode;

    }
}

bool KeyFrameAnimUtil::Load( NeoEngine::File* pkFile )
{
    if ( pkFile->Open( "", pkFile->GetName(), ios_base::in | ios_base::binary ) == false ) {

        return false;

    }
    return ReadKeyframes( pkFile );
}

bool KeyFrameAnimUtil::Load( const string &strFileName )
{
    auto_ptr< File > pkFile( new File() );
 
    if ( pkFile->Open( "", strFileName, ios_base::in | ios_base::binary ) == false ) {

        return false;

    }
    return ReadKeyframes( pkFile.get() );
}

bool KeyFrameAnimUtil::Store( File* pkFile, bool bBinaryOutput )
{
    if ( pkFile->Open( "", pkFile->GetName(), ios_base::out | ios_base::binary ) == false ) {

        return false;

    }
    return WriteKeyframes( pkFile, bBinaryOutput );
}

bool KeyFrameAnimUtil::Store( const std::string &strFileName, bool bBinaryOutput )
{
    auto_ptr< File > pkFile( new File() );
 
    if ( pkFile->Open( "", strFileName, ios_base::out | ios_base::binary ) == false ) {

        return false;

    }
    return WriteKeyframes( pkFile.get(), bBinaryOutput );
}

NeoEngine::AnimatedNode* KeyFrameAnimUtil::GetAnimatedNode()
{
    return m_pkAnimatedNode;
}

NeoEngine::NodeAnimation* KeyFrameAnimUtil::GetNodeAnimation()
{
    return m_pkNodeAnim;
}

unsigned int KeyFrameAnimUtil::GetNumKeyframes()
{
    assert( m_pkAnimatedNode && " no animation loaded " );
    return ( unsigned int )m_pkNodeAnim->m_vpkKeyframes.size();
}

void KeyFrameAnimUtil::SetAnimationName( const string& strName )
{
    assert( m_pkAnimatedNode && " no animation loaded " );
    m_pkNodeAnim->m_strName = strName;
}

const string& KeyFrameAnimUtil::GetAnimationName()
{
    assert( m_pkAnimatedNode && " no animation loaded " );
    return m_pkNodeAnim->m_strName;
}

void KeyFrameAnimUtil::SetBeginFrame( size_t uiBeginFrame ) 
{ 
    m_uiBeginFrame = uiBeginFrame; 
}
    
void KeyFrameAnimUtil::SetEndFrame( size_t uiEndFrame )
{ 
    m_uiEndFrame = uiEndFrame; 
}


void KeyFrameAnimUtil::SetAnimLength( float fLength ) 
{ 
    assert( m_pkAnimatedNode && " no animation loaded " );
    m_pkNodeAnim->m_fLength = fLength; 
}

float KeyFrameAnimUtil::GetAnimLength() 
{ 
    assert( m_pkAnimatedNode && " no animation loaded " );
    return m_pkNodeAnim->m_fLength;
}

unsigned int KeyFrameAnimUtil::GetAnimID()
{
    assert( m_pkAnimatedNode && " no animation loaded " );
    return m_pkNodeAnim->m_uiID;
}

void KeyFrameAnimUtil::SetAnimID( unsigned int uiID )
{
    assert( m_pkAnimatedNode && " no animation loaded " );
    m_pkNodeAnim->m_uiID = uiID;
}

bool KeyFrameAnimUtil::ReadKeyframes( File* pkFile )
{
    char   szStr[5];
    string strMode;

    if( !pkFile->DetermineByteOrder( 0x494e414e ) )
    {
        PRINTF( "*** Unable to load animation lib: Failed to determine byte order, possible corrupt file\n" );
        delete pkFile;
        return false;
    }

    pkFile->Read( szStr, 4 ); szStr[4] = 0;

    if( string( szStr ) != "NANI" )
    {
        PRINTF( "*** Unable to load animation lib: Invalid ID\n" );
        delete pkFile;
        return false;
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
        PRINTF( "*** Unable to load animation lib: Unsupported chunk file mode %s", strMode.c_str() );
        return false;
    }

    *pkFile >> pkIO->m_iMajorVersion >> pkIO->m_iMinorVersion;

    if( !( pkIO->m_iMajorVersion == ChunkIO::MAJORVERSIONREQUIRED ) || ( pkIO->m_iMinorVersion < ChunkIO::MINORVERSIONREQUIRED ) )
    {
        PRINTF( "*** Unable to load animation lib: Invalid chunk format version %d.%d", pkIO->m_iMajorVersion, pkIO->m_iMinorVersion );
        delete pkIO;
        return false;
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

    delete pkIO;
    
    m_pkAnimatedNode = pkAnimNode;
    m_pkNodeAnim     = pkAnimNode->GetAnimation();
    m_uiEndFrame     = ( unsigned int )m_pkNodeAnim->m_vpkKeyframes.size();

    return true;
}

bool KeyFrameAnimUtil::WriteKeyframes( File* pkFile, bool bBinaryOutput )
{
    // write header
    if( bBinaryOutput ) {

        pkFile->SetBinary( true );
        pkFile->Write( "NANI", 4 );
        pkFile->Write( "!bin", 4 );
        *pkFile << ChunkIO::MAJORVERSIONREQUIRED << ChunkIO::MINORVERSIONREQUIRED;
    } else  {
    
        pkFile->SetBinary( false );
        *pkFile << "NANI" << "!txt" << " " << ChunkIO::MAJORVERSIONREQUIRED << " " << ChunkIO::MINORVERSIONREQUIRED << endl;

    }

    ChunkIO *pkIO   = new ChunkIO;
    pkIO->m_eMode   = bBinaryOutput ? ChunkIO::BINARY : ChunkIO::ASCII;

    // write out the chunks
    Chunk *pkAnimChunk = ChunkFactory::CreateChunk( ChunkType::ANIMATEDNODE, "" );

    pkAnimChunk->AttachChunk( new StringChunk( pkFile->GetName(), "name" ) );
    pkAnimChunk->AttachChunk( new IntChunk( 0, "id" ) );
    pkAnimChunk->AttachChunk( new FloatChunk( m_pkNodeAnim->m_fLength, "length" ) );

    Chunk *pkChannelChunk = ChunkFactory::CreateChunk( ChunkType::NODEANIMATION, "" );
    pkAnimChunk->AttachChunk( pkChannelChunk );

    NodeKeyframe *pkKeyframe = NULL;
    size_t       uiFrameNum  = m_pkNodeAnim->m_vpkKeyframes.size();
    for( size_t uiCnt = 0; uiCnt < uiFrameNum; uiCnt++ )
    {

        pkKeyframe = m_pkNodeAnim->m_vpkKeyframes[ uiCnt ];

        Chunk *pkFrameChunk = ChunkFactory::CreateChunk( ChunkType::NODEKEYFRAME, "" );
        pkChannelChunk->AttachChunk( pkFrameChunk );

        pkFrameChunk->AttachChunk( new VectorChunk( pkKeyframe->m_kTranslation, "pos" ) );
        pkFrameChunk->AttachChunk( new QuaternionChunk( pkKeyframe->m_kRotation, "rot" ) );
        pkFrameChunk->AttachChunk( new FloatChunk( (float)uiCnt / (float)uiFrameNum, "time" ) );

    }

    pkIO->WriteChunk( pkAnimChunk, pkFile );

    delete pkAnimChunk;
    delete pkIO;

    return true;
}


//--------------------------------------------------------------------------------------------------------------------------//
// keyframe manipulation commands
//--------------------------------------------------------------------------------------------------------------------------//

void KeyFrameAnimUtil::CmdDeleteFrames( size_t uiBegin, size_t uiEnd )
{
    assert( m_pkAnimatedNode && " no animation loaded " );
    
    std::vector< NodeKeyframe* >::iterator    pkBeginFrame = m_pkNodeAnim->m_vpkKeyframes.begin();  pkBeginFrame += uiBegin;
    std::vector< NodeKeyframe* >::iterator    pkEndFrame   = m_pkNodeAnim->m_vpkKeyframes.begin();  pkEndFrame   += uiEnd;
    m_pkNodeAnim->m_vpkKeyframes.erase( pkBeginFrame, pkEndFrame );
}


void KeyFrameAnimUtil::CmdRotate( const Vector3d& kRotationAxis, float fAngle )
{
    assert( m_pkAnimatedNode && " no animation loaded " );

    Vector3d kRotEuler   = kRotationAxis * fAngle;
    kRotEuler            = Vector3d( kRotEuler.x * PI / 180.0f, kRotEuler.y * PI / 180.0f, kRotEuler.z * PI / 180.0f );
    Quaternion kRotation = Quaternion( EulerAngles( kRotEuler ) );

    for( size_t uiCnt = m_uiBeginFrame; uiCnt < m_uiEndFrame; uiCnt++ ) {

        NodeKeyframe *pkKeyframe = m_pkNodeAnim->m_vpkKeyframes[ uiCnt ];
        pkKeyframe->m_kRotation  = pkKeyframe->m_kRotation * kRotation;

    }
}

void KeyFrameAnimUtil::CmdSetRotation( const Vector3d& kRotation )
{
    assert( m_pkAnimatedNode && " no animation loaded " );

    Vector3d kRotEuler   =  Vector3d( kRotation.x * PI / 180.0f, kRotation.y * PI / 180.0f, kRotation.z * PI / 180.0f );
    Quaternion kRot      = Quaternion( EulerAngles( kRotEuler ) );

    for( size_t uiCnt = m_uiBeginFrame; uiCnt < m_uiEndFrame; uiCnt++ ) {

        NodeKeyframe *pkKeyframe = m_pkNodeAnim->m_vpkKeyframes[ uiCnt ];
        pkKeyframe->m_kRotation  = kRot;

    }
}

void KeyFrameAnimUtil::CmdTranslate( const Vector3d& kTranslate )
{
    assert( m_pkAnimatedNode && " no animation loaded " );

    for( size_t uiCnt = m_uiBeginFrame; uiCnt < m_uiEndFrame; uiCnt++ ) {

        NodeKeyframe *pkKeyframe    = m_pkNodeAnim->m_vpkKeyframes[ uiCnt ];
        pkKeyframe->m_kTranslation  = pkKeyframe->m_kTranslation + kTranslate;

    }
}

void KeyFrameAnimUtil::CmdSetTranslation( const Vector3d& kTranslation )
{
    assert( m_pkAnimatedNode && " no animation loaded " );

    for( size_t uiCnt = m_uiBeginFrame; uiCnt < m_uiEndFrame; uiCnt++ ) {

        NodeKeyframe *pkKeyframe    = m_pkNodeAnim->m_vpkKeyframes[ uiCnt ];
        pkKeyframe->m_kTranslation  = kTranslation;

    }
}

