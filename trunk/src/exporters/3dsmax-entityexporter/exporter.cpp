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
 # neoengine, 3ds max exporter for level entities
 #  this code is basing on NeoEngine's 3ds max exporter
 #
 #   date of creation:  12/26/2003
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/


#include "exporter.h"


#include <IGame.h>
#include <IGameObject.h>
#include <IGameProperty.h>
#include <IGameControl.h>
#include <IGameModifier.h>
#include <IConversionManager.h>
#include <IGameError.h>

using namespace std;

namespace NeoMaxExporter
{


HINSTANCE g_hInstance = 0;


class ExporterClassDesc : public ClassDesc2
{
	public:

		int                                           IsPublic() { return TRUE; }

		void                                         *Create( BOOL loading = FALSE ) { return new Exporter(); }
	
		const TCHAR                                  *ClassName() { return ENTITYEXPORTER_CLASSNAME; }

		SClass_ID                                     SuperClassID() { return SCENE_EXPORT_CLASS_ID; }

		Class_ID                                      ClassID() { return ENTITYNSCEEXPORTER_CLASSID; }

		const TCHAR                                  *Category() { return "NeoEngine"; }

		const TCHAR                                  *InternalName() { return _T( ENTITYEXPORTER_CLASSNAME ); } // returns fixed parsable name (scripter-visible name)
	
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





int Exporter::s_iMajorEntVersion = 1;
int Exporter::s_iMinorEntVersion = 0;


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
	return _T( "lvl" );
}


const TCHAR *Exporter::LongDesc()
{
	return _T( "Export to CTD level File" );
}
	

const TCHAR *Exporter::ShortDesc() 
{			
	return _T( "CTD level exporter" );
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
	return 100;
}


void Exporter::ShowAbout( HWND hWnd )
{			
	MessageBox( hWnd, "CTD level file exporter\nA. Botorabi (botorabi@gmx.net)\nReleased under GNU license", "About", MB_OK );
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

	// clean up entity list, etc.
	m_vkEntities.clear();
	m_vkPluginDeclarations.clear();
	// clear log string, etc.
	m_strExportLog = "";
	m_strTemplateCommonSection = "";

	Interface *pkCoreInterface = GetCOREInterface();

	ExporterErrorCallback kErrorCallback;

	SetErrorCallBack( &kErrorCallback );


	pkCoreInterface->ProgressStart( _T( "Exporting lvl.." ), TRUE, ProgressBar, 0 );
	
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
		string strName = pszName;

		strName = strName.substr( 0, strName.find_last_of( '.' ) );

	}


	//Force lowercase extension
	if( !strcmp( pszName + strlen( pszName ) - 3, "LVL" ) )
		memcpy( (void*)( pszName + strlen( pszName ) - 3 ), "lvl", 3 );

	//Write level file
	WriteProperties( (char*)pszName );

	m_pkIGameScene->ReleaseIGame();

	pkCoreInterface->ProgressEnd();

	return TRUE;
}


void Exporter::ExportNodeInfo( IGameNode *pkNode, int &iCurNode )
{
	TSTR strBuf;

	strBuf = TSTR( "Processing: " ) + TSTR( pkNode->GetName() );
	GetCOREInterface()->ProgressUpdate( (int)( ( (float)iCurNode++ / (float)m_pkIGameScene->GetTotalNodeCount() ) * 100.0f ), FALSE, strBuf.data() ); 

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

		// skip non-entity nodes
		if ( strName[0] != '$' ) {

			return;

		}

		string	strEntityName;
		// cut leading '$' character from entity name
		for ( unsigned int i = 1; i < strlen( ( char* )strName ); i++) {
			strEntityName += ( ( char* )strName )[i];
		}
		

		if( pkNode->GetMaterialIndex() != -1 )
		{
			// foo?
		}

		if( bIsXRef )
		{
			// bar?
		}

		// entities are defined using meshes
		if ( pkObj->GetIGameType() != IGameObject::IGAME_MESH ) {

			return;

		}

		// get user-defined properties
		INode *pkMaxNode = pkNode->GetMaxNode();
		TSTR	strProps;
		pkMaxNode->GetUserPropBuffer( strProps );
		char	*pBuff = ( char* )strProps;

		if ( pBuff && strcmp( pBuff, "" ) ) {

			ParseProperties( strEntityName.c_str(), pBuff, pkNode );

		}

	}

	pkNode->ReleaseIGameObject();
}

bool Exporter::ParseProperties( const char *pcEntityName, const char *pcBuffer, IGameNode *pkNode )
{

	Entity		kEntity;

	char	*pcBuff = (char*)pcBuffer;
	string	strPropLine;
	char	pcLineBuff[256];

	unsigned int uiBuffSize = strlen( pcBuffer );
	unsigned int uiBuffCnt = 0;

	char	pcParamName[256];
	char	pcParamType[256];
	char	pcParamValue[256];
	char	pcBuffName[32];
	char	pcBuffType[20];

	while ( uiBuffCnt < uiBuffSize ) {

		pcParamName[0]  = '\0';
		pcParamType[0]  = '\0';
		pcParamValue[0] = '\0';
		pcLineBuff[0]   = '\0';

		// copy one line into line buffer
		int i;
		for ( i = 0; ( pcBuff[uiBuffCnt] != '\n' ) && ( uiBuffCnt < uiBuffSize ); i++) {

			pcLineBuff[i] = pcBuff[ uiBuffCnt++ ];

		}

		// cut the dirt at end of property string!
		if ( uiBuffCnt >= uiBuffSize ) {

			break;

		}

		if ( pcLineBuff[i - 1] == 13 ) {
		
			pcLineBuff[i - 1] = '\0';

		}

		uiBuffCnt++; // plus 1 skipping carriage return

		int iScannedTokens = sscanf( pcLineBuff, "%s %s =", pcBuffName, pcBuffType );
		if ( iScannedTokens < 2 ) {

			m_strExportLog += " Error exporting parameter ' " + string( pcBuffName ) + " ' in entity ' " + string( pcEntityName ) + " '\n";
			continue;

		}

		// extract param strings
		//---------------------------------------------//

		// scann value field
		for ( i = 0; pcLineBuff[i] != '='; i++ );
		for (; pcLineBuff[i] != '<'; i++ );
		int j;
		for ( j = 0, i++; pcLineBuff[i] != '>'; i++, j++ ) {

			pcParamValue[j] = pcLineBuff[i];

		}
		pcParamValue[j] = '\0';

		// read the param name
		for ( i = 1; pcBuffName[i] != '>'; i++ ) {

			pcParamName[i - 1] = pcBuffName[i];

		}
		pcParamName[i - 1] = '\0';

		// read the param type
		for ( i = 1; pcBuffType[i] != ']'; i++ ) {

			pcParamType[i - 1] = pcBuffType[i];

		}
		pcParamType[i - 1] = '\0';

		// evaluate and add the param tripple into list
		//--------------------------------------------//
		Parameter	kParam;

		kParam.m_strName = pcParamName;
		kParam.m_strType = pcParamType;

		// check for value macro
		if ( pcParamValue[0] != '$' ) {
		
			kParam.m_strValue = pcParamValue;

		} else {

			string	strMacro = pcParamValue;

			if ( strMacro == "$POSITION" ) {

				INode	*pkMaxNode = pkNode->GetMaxNode();
				Matrix3 kMatrix = pkMaxNode->GetNodeTM( 0 );
				Point3	vPosition = kMatrix.GetTrans();
				char	strVal[64];
				sprintf( strVal, "%f %f %f", vPosition.x, vPosition.z, -vPosition.y );
				kParam.m_strValue = strVal;

			}
			else
			if ( strMacro == "$ROTATION" ) {

				// get rotation
				INode	*pkMaxNode	= pkNode->GetMaxNode();
				Matrix3 kMatrixTM	= pkMaxNode->GetNodeTM( 0 );
				Quat	kQuat( kMatrixTM );
				Point3	vAngles;
				QuatToEuler( kQuat, &vAngles.x);

				// convert angles to degree
				vAngles *= ( 180.0f / PI );

				// clamp too small angles to zero
				if ( !( fabsf( vAngles.x ) > 0.0001f || fabsf( vAngles.y ) > 0.0001f || fabsf( vAngles.z ) > 0.0001f ) ) {

					vAngles = Point3( 0.0f, 0.0f, 0.0f );
					
				}

				char	strVal[64];
				sprintf( strVal, "%f %f %f", vAngles[ 0 ], vAngles[ 2 ], vAngles[ 1 ] );
				kParam.m_strValue = strVal;

			}
			else
			if ( strMacro == "$DIMENSIONS" ) {

				// get bounding box
				Box3 kBBox;
				pkNode->GetIGameObject()->GetBoundingBox( kBBox );
				Point3 vDims = kBBox.pmax - kBBox.pmin;

				char	strVal[64];
				sprintf( strVal, "%f %f %f", vDims.x, vDims.z, vDims.y );
				kParam.m_strValue = strVal;

			}
		}
		kEntity.m_vkParameters.push_back( kParam );
			
	}

	// extract entity and instance name ( <Entity name>:<Instance name> )
	string			strEntityName;
	string			strInstanceName;
	unsigned int	uiStrCnt;
	// get entity name
	for ( uiStrCnt = 0; uiStrCnt < strlen( pcEntityName ); uiStrCnt++ ) {

		if ( pcEntityName[uiStrCnt] != ':' ) {
		
			strEntityName += pcEntityName[uiStrCnt];

		} else {

			break;

		}
	}
	// get instance name
	uiStrCnt++;
	while ( uiStrCnt < strlen( pcEntityName ) ) {

		strInstanceName += pcEntityName[uiStrCnt];
		uiStrCnt++;
	}

	kEntity.m_strName = strEntityName;
	kEntity.m_strInstanceName = strInstanceName;

	m_vkEntities.push_back( kEntity );

	return true;

}

// write out level file
bool Exporter::WriteProperties( const char *pcFileName )
{

	FILE	*pkFile;
	FILE	*pkLogFile;

	if ( !( pkFile = fopen( pcFileName, "w+" ) ) ) {

		return false;

	}
		
	unsigned int uiTotParamCnt = 0;

	char pcTemplateFileName[256];
	strcpy( pcTemplateFileName, pcFileName );
	strcat( pcTemplateFileName, ".template" );

	if ( ParseTemplateFile( pcTemplateFileName ) == false ) {

		MessageBox( NULL, "Could not find a template file!\nLevel file will be written unformatted.", "Warning", MB_OK );

		for ( unsigned int i = 0; i < m_vkEntities.size(); i++ ) {

			fprintf( pkFile, "<Entity Name=\"%s\" InstanceName=\"%s\">\n", m_vkEntities[i].m_strName.c_str(), m_vkEntities[i].m_strInstanceName.c_str() );

			m_vkEntities[i].m_bExported = true;

			for ( unsigned int j = 0; j < m_vkEntities[i].m_vkParameters.size(); j++) {

				fprintf( pkFile, "  <Parameter Name=\"%s\" Type=\"%s\" Value=\"%s\" />\n", 
					m_vkEntities[i].m_vkParameters[j].m_strName.c_str(),  
					m_vkEntities[i].m_vkParameters[j].m_strType.c_str(),
					m_vkEntities[i].m_vkParameters[j].m_strValue.c_str() );

				uiTotParamCnt++;

			}

			fprintf( pkFile, "</Entity>\n" );

		}

	} else {

		string	strFormattedLevelFile;
		uiTotParamCnt = FormatLevelFile( strFormattedLevelFile );
		fprintf( pkFile, "%s", strFormattedLevelFile.c_str() );

	}

	fclose( pkFile );

	// write into export log file
	char pcBuff[10];
	m_strExportLog += "\n\n";
	m_strExportLog += "------------------------------------\n";
	// count exported entities
	unsigned int uiTotEntityCnt = 0;
	for( unsigned int i = 0; i < m_vkEntities.size(); i++ ) {
		if ( m_vkEntities[i].m_bExported == true ) uiTotEntityCnt++;
	}
	m_strExportLog += "Num of exported entities: " + string( itoa( uiTotEntityCnt, pcBuff, 10 ) ) + "\n";
	m_strExportLog += "Total Num of exported parameters: " + string( itoa( uiTotParamCnt, pcBuff, 10 ) ) + "\n";
	m_strExportLog += "\n\n";

	// gather some statistics
	GenStatistics( m_strExportLog );

	char pcLogFileName[256];
	strcpy( pcLogFileName, pcFileName );
	strcat( pcLogFileName, ".3ds_export_log" );
	if ( ( pkLogFile = fopen( pcLogFileName, "w+" ) ) ) {

		fprintf( pkLogFile, "%s", m_strExportLog.c_str() );
		fclose( pkLogFile );
	}


	
	return true;
}

// parse and evaluate the template file
//----------------------------------------------------------------------------------------//

#define CTD_TEMPLATE_PLUGIN_DELR	"PluginDeclaration"
#define CTD_TEMPLATE_COMMON_SEC		"CommonSection"

#define SAFE_BUFF_INCR				{ uiBufferCnt++; if ( uiBufferCnt > uiBufferSize ) { m_strExportLog += " Error parsing template file: file size exceeded during parsing. Check the syntax.!\n"; return false; } }
#define SKIP_CR						while ( strBuffer[ uiBufferCnt ] == 10 ) { SAFE_BUFF_INCR };
#define SKIP_WHITESPACE_CR			while ( ( strBuffer[ uiBufferCnt ] == 10 ) || ( strBuffer[ uiBufferCnt ] == ' ' ) ) { SAFE_BUFF_INCR };

bool Exporter::ParseTemplateFile( const char *pcFileName )
{

	FILE	*pkFile;

	if ( !( pkFile = fopen( pcFileName, "r" ) ) ) {

		m_strExportLog += "Error opening template file '" + string( pcFileName ) + " '\n";
		return false;

	}

	m_strExportLog += "Parsing template file ...\n";

	string			strBuffer;
	string			strToken;
	char			cBuffer;
	unsigned int	uiBufferCnt = 0;
	unsigned int	uiBufferSize;

	// read in the template file
	while ( fread( &cBuffer, sizeof( char ), 1, pkFile ) == 1 ) {

		strBuffer += cBuffer;

	}

	uiBufferSize = strBuffer.length();

	// process template

	// skip comments
	while ( strBuffer[ uiBufferCnt ] != '@' ) {

		SAFE_BUFF_INCR;

	}

	// read token
	SAFE_BUFF_INCR;
	while ( ( strBuffer[ uiBufferCnt ] != 10 ) && ( strBuffer[ uiBufferCnt ] != ' ' ) ) {

		strToken += strBuffer[ uiBufferCnt ];
		SAFE_BUFF_INCR;

	}

	// parse plugin declaration section
	if ( strToken != CTD_TEMPLATE_PLUGIN_DELR ) {

		char pcBuff[10];
		m_strExportLog += " Error in template file: could not find plugin declarations ( buffer position ( " + string( itoa ( uiBufferCnt, pcBuff, 10 ) ) + " )\n";
		return false;

	}
	// read plugin declarations
	while ( strBuffer[ uiBufferCnt ] != '@' ) {
	
		string				strPlugin;
		PluginDeclaration	kPluginDeclr;


		SKIP_WHITESPACE_CR;

		while ( strBuffer[ uiBufferCnt ] != 10 ) {

			strPlugin += strBuffer[ uiBufferCnt ];
			SAFE_BUFF_INCR;
		}

		SKIP_WHITESPACE_CR;
		if ( strBuffer[ uiBufferCnt ] != '[' ) {

			char pcBuff[10];
			m_strExportLog += " Error in template file: could not find plugin entities ( buffer position ( " + string( itoa ( uiBufferCnt, pcBuff, 10 ) ) + " )\n";
			return false;

		}
		SAFE_BUFF_INCR;

		// read in entities
		while ( strBuffer[ uiBufferCnt ] != ']' ) {

			string			strEntity;

			SKIP_CR;
			while ( strBuffer[ uiBufferCnt ] != 10 ) {

				strEntity += strBuffer[ uiBufferCnt ];
				SAFE_BUFF_INCR;

			}

			kPluginDeclr.m_vkEnities.push_back( strEntity );
			SKIP_CR;
		}
		SAFE_BUFF_INCR;
		SKIP_WHITESPACE_CR;

		kPluginDeclr.m_strName = strPlugin;
		m_vkPluginDeclarations.push_back( kPluginDeclr );

	}
	SAFE_BUFF_INCR;
	
	// read common section
	while ( strBuffer[ uiBufferCnt ] != '@' ) {
		SAFE_BUFF_INCR
	}
	SAFE_BUFF_INCR;

	// read token
	strToken = "";
	while ( ( strBuffer[ uiBufferCnt ] != 10 ) && ( strBuffer[ uiBufferCnt ] != ' ' ) ) {

		strToken += strBuffer[ uiBufferCnt ];
		SAFE_BUFF_INCR;

	}
	if ( strToken != CTD_TEMPLATE_COMMON_SEC ) {

		char pcBuff[10];
		m_strExportLog += " Error in template file: could not find common section ( buffer position ( " + string( itoa ( uiBufferCnt, pcBuff, 10 ) ) + " )\n";
		return false;

	}
	while ( strBuffer[ uiBufferCnt ] != '@' ) {

		m_strTemplateCommonSection	+= strBuffer[ uiBufferCnt ];
		SAFE_BUFF_INCR;

	}

	fclose( pkFile );

	m_strExportLog += "Template file successfully parsed.\n";
	
	return true;

}


// format the level file and store it in strBuffer
unsigned int Exporter::FormatLevelFile( std::string &strBuffer )
{

	unsigned int uiTotParamCnt = 0;

	// append common section
	strBuffer += m_strTemplateCommonSection;

	// go through all plugins and entities and format those given in model
	for ( unsigned int uiPDecl = 0; uiPDecl < m_vkPluginDeclarations.size(); uiPDecl++ ) {

		if ( ExistPluginEntities( m_vkPluginDeclarations[ uiPDecl ].m_strName ) == false ) {

			continue;

		}

		strBuffer += "  <Module Name=\"" + m_vkPluginDeclarations[ uiPDecl ].m_strName + "\">\n";  

		for ( unsigned int uiEnt = 0; uiEnt < m_vkPluginDeclarations[ uiPDecl ].m_vkEnities.size(); uiEnt++ ) {


			string	strEntityName = m_vkPluginDeclarations[ uiPDecl ].m_vkEnities[ uiEnt ];

			for ( unsigned int i = 0; i < m_vkEntities.size(); i++ ) {

				bool	bEntityFound = true;

				if ( m_vkEntities[i].m_strName == strEntityName ) {

					bEntityFound = true;
					m_vkEntities[i].m_bExported = true;

					strBuffer += "\n    <Entity Name=\"" + strEntityName + "\" InstanceName=\"" + m_vkEntities[i].m_strInstanceName + "\">\n\n";

					for ( unsigned int j = 0; j < m_vkEntities[i].m_vkParameters.size(); j++) {

						strBuffer += "      <Parameter Name=\"" + 
							m_vkEntities[i].m_vkParameters[j].m_strName +
							"\" Type=\"" +
							m_vkEntities[i].m_vkParameters[j].m_strType +
							"\" Value=\"" + 
							m_vkEntities[i].m_vkParameters[j].m_strValue +
							"\" />\n";

						uiTotParamCnt++;

					}
				
					strBuffer += "\n    </Entity>\n";

				}

				if ( bEntityFound == false ) {

					m_strExportLog += " Warning: Could not find entity ' " + m_vkEntities[i].m_strName + " ' in template file; skipping";
				}
			}
		}

		strBuffer += "\n  </Module>\n\n";  

	}

	strBuffer += "</Level>\n";

	return uiTotParamCnt;
}

// search for entities of given plugin in model 
bool Exporter::ExistPluginEntities( const string &strPluginName )
{

	unsigned int uiPDecl;
	// first find plugin in declaration list
	for ( uiPDecl = 0; uiPDecl < m_vkPluginDeclarations.size(); uiPDecl++ ) {

		if ( m_vkPluginDeclarations[ uiPDecl ].m_strName == strPluginName ) {

			break;
		}

	}

	// this should not happen in normal situation, though checking for it is more safe.
	if ( uiPDecl == m_vkPluginDeclarations.size() ) {

		return false;
	}

	// now check wether any of its entities are placed in model
	for ( unsigned int uiEnt = 0; uiEnt < m_vkPluginDeclarations[ uiPDecl ].m_vkEnities.size(); uiEnt++ ) {

		string	strEntityName = m_vkPluginDeclarations[ uiPDecl ].m_vkEnities[ uiEnt ];

		for ( unsigned int i = 0; i < m_vkEntities.size(); i++ ) {				

			if ( m_vkEntities[i].m_strName == strEntityName ) {

				return true;
			}
		}
	}

	return false;
}

void Exporter::GenStatistics( std::string &strBuffer )
{

	strBuffer += "export statistics \n";
	strBuffer += "----------------- \n";

	// exported entities
	for ( unsigned int uiPDecl = 0; uiPDecl < m_vkPluginDeclarations.size(); uiPDecl++ ) {

		strBuffer += "\n plugin: " + m_vkPluginDeclarations[ uiPDecl ].m_strName + "\n";

		for ( unsigned int uiEnt = 0; uiEnt < m_vkPluginDeclarations[ uiPDecl ].m_vkEnities.size(); uiEnt++ ) {

			unsigned int uiEntCnt = 0;

			string	strEntityName = m_vkPluginDeclarations[ uiPDecl ].m_vkEnities[ uiEnt ];

			for ( unsigned int i = 0; i < m_vkEntities.size(); i++ ) {				

				if ( m_vkEntities[i].m_strName == strEntityName ) {

					uiEntCnt++;

				}

			}

			char	pcBuff[10];
			strBuffer += "   entity: " + strEntityName;
			strBuffer += "\t ( " + string( itoa( uiEntCnt, pcBuff, 10 ) )  + " )\n";

		}
	}

	strBuffer += "----------------- \n\n";

	// not exported entities
	strBuffer += "following entities are not exported as they were not declared in template file: \n";
	strBuffer += "------------------------------------------------------------------------------- \n";

	for ( unsigned int i = 0; i < m_vkEntities.size(); i++ ) {				

		if ( m_vkEntities[i].m_bExported == false ) {

			strBuffer += m_vkEntities[i].m_strName + "\n";

		}

	}

	strBuffer += "------------------------------------------------------------------------------- \n";

}

};

