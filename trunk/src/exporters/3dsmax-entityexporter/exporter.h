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
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  12/26/2003 boto       creation of basic functionalities
 #
 #  12/27/2003 boto       added level template support
 #
 #  12/28/2003 boto       added statistics output
 #
 ################################################################*/


#ifndef __NEOENGINE_3DSMAX_ENTITYEXPORTER_H
#define __NEOENGINE_3DSMAX_ENTITYEXPORTER_H

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#include <max.h>

#if (_MSC_VER >= 1300)
#  include "MAX_Mem.h"
#endif

#include <istdplug.h>
#include <iparamm2.h>
#include <iparamb2.h>

#include <vector>
#include <string>


#define ENTITYEXPORTER_CLASSNAME			 "CTD Entity Exporter"
#define ENTITYNSCEEXPORTER_CLASSID           Class_ID(0x72733b0, 0x4fb82422)



/*! External classes */
class IGameScene;
class IGameNode;
class IGameUVGen;


namespace NeoMaxExporter
{

extern  HINSTANCE                      g_hInstance;

ClassDesc2                            *GetExporterDesc();

// one single parameter
class Parameter
{
	public:
																Parameter() {}
																~Parameter() {}

		std::string												m_strName;
		std::string												m_strType;
		std::string												m_strValue;
};
// entity parameters
class Entity
{
	public:

																Entity() { m_bExported = false; }
																~Entity() {}

		std::string												m_strName;
		std::string												m_strInstanceName;
		std::vector< Parameter >								m_vkParameters;

		bool													m_bExported;

};

// this class is used to setup valid plugins and their entities basing on template file
class PluginDeclaration
{

	public:
																PluginDeclaration() {};
																~PluginDeclaration() {};

		std::string												m_strName;
		std::vector< std::string >								m_vkEnities;

};

class Exporter : public SceneExport
{
	public:

		/*! NSCE file major version */
		static int                                              s_iMajorEntVersion;

		/*! NSCE file minor version */
		static int                                              s_iMinorEntVersion;

		/*! IGame scene object */
		IGameScene                                             *m_pkIGameScene;

	private:

		std::vector< Entity >									m_vkEntities;
		std::string												m_strExportLog;
		std::vector< PluginDeclaration >						m_vkPluginDeclarations;
		std::string												m_strTemplateCommonSection;
		bool													ParseProperties( const char *pcEntityName, const char *pcBuffer, IGameNode *pkNode );
		bool													ParseTemplateFile( const char *pcFileName );
		bool													WriteProperties( const char *pcFileName );
		/**
		* Format level file and return total count of parameters.
		*/
		unsigned int											FormatLevelFile( std::string &strBuffer );

		/**
		* return true if any entities in model are needed from given plugin.
		*/
		bool													ExistPluginEntities( const std::string &strPluginName );
		/**
		* Function for generating statistics.
		*/
		void													GenStatistics( std::string &strBuffer );


	public:

		float                                                   m_fFramesPerSecond;
		int                                                     m_iTicksPerFrame;
		float                                                   m_fTicksPerSecond;

		int                                                     m_iStartFrame;
		int                                                     m_iEndFrame;



		/**
		* \return                                               Number of extensions supported
		*/
		int                                                     ExtCount();

		/**
		* \param iExtID                                         Extension string number
		* \return                                               Extensions string
		*/
		const TCHAR                                            *Ext( int iExtID );

		/**
		* \return                                               Long ASCII description
		*/
		const TCHAR                                            *LongDesc();

		/**
		* \return                                               Short ASCII description
		*/
		const TCHAR                                            *ShortDesc();

		/**
		* \return                                               ASCII author name
		*/
		const TCHAR                                            *AuthorName();

		/**
		* \return                                               ASCII copyright message
		*/
		const TCHAR                                            *CopyrightMessage();

		/**
		* \return                                               Other ASCII message #1
		*/
		const TCHAR                                            *OtherMessage1();

		/**
		* \return                                               Other ASCII message #2
		*/
		const TCHAR                                            *OtherMessage2();

		/**
		* \return                                               Version number * 100 (i.e v3.01 = 301 )
		*/
		unsigned int                                            Version();

		/**
		* Show DLL's About box
		* \param hWnd                                           Parent window
		*/
		void                                                    ShowAbout( HWND hWnd );

		/**
		* Query if support options
		* \param iExtID                                         Extension number
		* \param ulOptions                                      Options
		* \return                                               TRUE if supported, FALSE if not
		*/
		BOOL                                                    SupportsOptions( int iExtID, unsigned long ulOptions );

		/**
		* Export scene
		* \param pszName                                        File name
		* \param pExpInterface                                  Exporting interface
		* \param pInterface                                     Interface
		* \param bSuppressPrompts                               Suppress prompts flag
		* \param ulOptions                                      Options
		* \return                                               TRUE if success, FALSE if error
		*/
		int                                                     DoExport( const TCHAR *pszName, ExpInterface *pExpInterface, Interface *pInterface, BOOL bSuppressPrompts = FALSE, unsigned long ulOptions = 0 );

		/**
		* Export node
		* \param pkNode                                         Node
		* \param iCurNode                                       Node counter
		*/
		void                                                    ExportNodeInfo( IGameNode *pkNode, int &iCurNode );

		/**
		*/
		                                                        Exporter();

		/**
		*/
		virtual                                                ~Exporter();
};


};


#endif // __NEOENGINE_3DSMAX_ENTITYEXPORTER_H
