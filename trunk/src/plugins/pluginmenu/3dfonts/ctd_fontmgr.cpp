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
 # neoengine, 3d fonts
 #
 # this class implements a 3d font manager
 #
 #
 #   date of creation:  05/24/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>
#include "ctd_fontmgr.h"


using namespace std;
using namespace CTD;
using namespace NeoEngine;
using namespace NeoChunkIO;

namespace CTD_IPluginMenu
{

// plugin global entity descriptor for 3d font manager
CTD3DFontMgrDesc g_pkCTD3DFontMgrEntity_desc;

//-------------------------------------------//
bool	s_bSingleInstanceCreated		= false;
#define CTD_FONTCFG_DEFAULT_FILENAME	"3dfont.cfg"
#define CTD_MESHFILE_PREFIX				".nsce"

// command strings; these can be used in font config file
#define CTD_FONTCFG_CMD_FONTSIZE		"!fontsize"
#define CTD_FONTCFG_CMD_SPACESIZE		"!spacesize"


CTDLetter::CTDLetter()
{

	m_pkMeshNode	= NULL;

}
		
CTDLetter::~CTDLetter()
{

}

//--------------------------------------------//


CTD3DFontMgr::CTD3DFontMgr()
{

	CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Menu) entity ' 3DFontManager ' created " );

	m_bReady		= false;
	m_strFontsCfg	= CTD_FONTCFG_DEFAULT_FILENAME;
	m_fSpaceSize	= 0.3f;

	// set the entity ( node ) name
	SetName( CTD_ENTITY_NAME_3DFontManager );

}

CTD3DFontMgr::~CTD3DFontMgr()
{

	CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Menu) entity ' 3DFontManager ' destroyed " );

	// delete letters
	size_t uiLetthers = m_vpkLetters.size();
	for ( size_t uiLetterCnt = 0; uiLetterCnt < uiLetthers; uiLetterCnt++ ) {

		delete m_vpkLetters[ uiLetterCnt ];
	}

}

// init entity
void CTD3DFontMgr::Initialize() 
{ 

	CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Menu) entity ' 3DFontManager ' initializing ... " );

	if ( s_bSingleInstanceCreated == true ) {

		CTDCONSOLE_PRINT( LogLevel( WARNING ), " (Plugin Menu) entity ' 3DFontManager ', instance already exsits, skipp initialization of a new instance!" );
		// erase the instance name so this entity cannot be found later in search functions
		SetName( "" );

	}


	if ( SetupFontManager( m_strFontsCfg ) == false ) {

		CTDCONSOLE_PRINT( LogLevel( ERROR ), " *** (Plugin Menu) entity ' 3DFontManager ', could not setup font manager! " );

	}


	// this entity does not need to be active
	Deactivate();

	s_bSingleInstanceCreated	= true;
	m_bReady					= true;

	// set the instance name as the entity name so other entities can find this singleton entity 
	SetInstanceName( CTD_ENTITY_NAME_3DFontManager );

}

// setup font manager
bool CTD3DFontMgr::SetupFontManager( std::string &strCfgFileName )
{

	CTDCONSOLE_PRINT( LogLevel( INFO ), "   loading fonts ... " );
		
	string	strLogMsg;

	// we search for the font config file
    File*   pkFile = NeoEngine::Core::Get()->GetFileManager()->GetByName( strCfgFileName );
	
	if ( !pkFile || ( pkFile->Open( "", strCfgFileName, ios_base::in | ios_base::binary ) == false ) ) {

		strLogMsg = "  *** error: could not find or open font configuration file '" + strCfgFileName + "'";
		CTDCONSOLE_PRINT( LogLevel( ERROR ) , strLogMsg );
		return false;

	}

	// read in the config file for further processing
	unsigned int uiFileSize = pkFile->GetSize();
	char	*pcInitialBuffer = new char[ uiFileSize ];
	if ( !pkFile->Read( pcInitialBuffer, uiFileSize ) ) {

		strLogMsg = "   *** error reading key configuration file '" + strCfgFileName + "'";
		CTDCONSOLE_PRINT( LogLevel( ERROR ), strLogMsg );
		delete pcInitialBuffer;
        delete pkFile;
		return false;

	}

	char			pcLineBuffer[256];
	char			pcLetter[64];
	char			pcMeshFile[64];
	char			pcCommand[64];
	char			pcCmdValue[64];
	char			*pcBuffer = pcInitialBuffer;
	bool			bReadLine = true;

	float			fFontSize = 1.0f;

	while ( bReadLine == true ) {

		// copy one line into line buffer
		unsigned int uiC;
		for ( uiC = 0; ( pcLineBuffer[ uiC ] = *pcBuffer++ ) != '\n'; uiC++ ) {

			// save check for end of buffer
			if ( ( unsigned int )( pcBuffer - pcInitialBuffer ) > uiFileSize ) {
			
				bReadLine = false;
				break;

			}
						
		}
		// terminate the line buffer string
		pcLineBuffer[ uiC ] = 0;

		// skip comments; take care, there is also the character '#' to be configured.
		//  comments being with # and a following blank
		if ( ( ( pcLineBuffer[0] == '#') && ( pcLineBuffer[1] == ' ') ) || ( pcLineBuffer[0] == '\n' ) ) {
		
			continue;

		}
	
		// process commands
		if ( pcLineBuffer[0] == '!' ) { // take care; there is also the character '!' to be configured.
		

			if ( sscanf( pcLineBuffer, "%s = %s", pcCommand, pcCmdValue ) != 2 ) {

				strLogMsg = "   *** warning: invalid command in key configuration file: '" + string( pcLineBuffer ) + "'! skipping";
				CTDCONSOLE_PRINT( LogLevel( WARNING ), strLogMsg );
				continue;

			} else {

				if ( strcmp( pcCommand, CTD_FONTCFG_CMD_FONTSIZE ) == 0 ) {

					fFontSize = ( float )atof( pcCmdValue );
					continue;

				} else {

					if ( strcmp( pcCommand, CTD_FONTCFG_CMD_SPACESIZE ) == 0 ) {

						m_fSpaceSize	= ( float )atof( pcCmdValue );
						continue;
					}

				}

			}

		}
		// skip invalid line formats
		if ( sscanf( pcLineBuffer, "%s = %s", pcLetter, pcMeshFile ) != 2 ) {

			continue;

		}

		if ( strlen( pcLetter ) > 1 ) {

			strLogMsg = "   *** warning: invalid entry in key configuration file: '" + string( pcLineBuffer ) + "', use only one letter on left side! skipping";
			CTDCONSOLE_PRINT( LogLevel( WARNING ), strLogMsg );
			continue;
		}
		
		// create a new letter in letter list
		CTDLetter	*pkLetter = new CTDLetter;
		pkLetter->m_cLetter = pcLetter[ 0 ];
		pkLetter->m_strMeshFile = pcMeshFile + string( CTD_MESHFILE_PREFIX );
		
		m_vpkLetters.push_back( pkLetter );

	}

	delete pcInitialBuffer;


	// now load all letter meshes
	size_t uiLetthers = m_vpkLetters.size();
	for ( size_t uiLetterCnt = 0; uiLetterCnt < uiLetthers; uiLetterCnt++ ) {

		Scene kScene;
		if ( kScene.Load( m_vpkLetters[ uiLetterCnt ]->m_strMeshFile ) == false ) {

			strLogMsg = "   *** warning: could not load letter mesh '" +  m_vpkLetters[ uiLetterCnt ]->m_strMeshFile + "' for '" + m_vpkLetters[ uiLetterCnt ]->m_cLetter + "', skipping";
			CTDCONSOLE_PRINT( LogLevel( WARNING ), strLogMsg );
			continue;

		}

		MeshEntity	*pkMesh = new MeshEntity( kScene.GetMeshes()[0] );

		// enable dynamic shadows for letter meshes
		//pkMesh->GetSubMeshes()[ 0 ]->m_pkMaterial->m_bDynamicShadows = true;

		// setup bounding box
		pkMesh->GenerateBoundingVolume();
		SceneNode	*pkNode = new SceneNode;
		pkNode->SetEntity( pkMesh );
		pkNode->SetScaling( fFontSize );
		// resize bounding volume
		BoundingVolume	*pkBB = pkMesh->GetBoundingVolume();
		( ( AABB* )pkBB )->SetDim( ( ( AABB* )pkBB )->GetDim() * fFontSize );
		pkNode->SetBoundingVolume( pkBB );

		m_vpkLetters[ uiLetterCnt ]->m_pkMeshNode = pkNode;

	}

    delete pkFile;
	return true;

}

int	CTD3DFontMgr::Message( int iMsgId, void *pMsgStruct ) 
{ 

	if ( m_bReady == false ) {

		return -1;

	}

	switch ( iMsgId ) {

		// retrieve the letter list
		case CTD_3DFONTMGR_GETFONT:
		{

			tCTD3DFont	*pkFont		= ( tCTD3DFont* )pMsgStruct;
			pkFont->m_pvpkLetters	= &m_vpkLetters;
			pkFont->m_fSpaceSize	= m_fSpaceSize;

		}
		break;
	
		default:
			break;

	}

	return 0; 
}	


int	CTD3DFontMgr::ParameterDescription( int iParamIndex, ParameterDescriptor *pkDesc )
{

	int iParamCount = 1;

	if (pkDesc == NULL) {

		return iParamCount;
	}

	switch( iParamIndex ) 
	{
	case 0:
		pkDesc->SetName( "ConfigFile" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_STRING );
		pkDesc->SetVar( &m_strFontsCfg );
		
		break;

	default:
		return -1;
	}

	return iParamCount;

}

}
