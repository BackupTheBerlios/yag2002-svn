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
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  03/01/2004 boto       creation of ASE importer
 #
 ################################################################*/


#ifndef __CTD_ASEIMPORTER_H
#define __CTD_ASEIMPORTER_H

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#include <max.h>

#ifdef USE_MAXMEM
# if (_MSC_VER >= 1300)
#   include "MAX_Mem.h"
# endif
#endif

#include <istdplug.h>
#include <iparamm2.h>
#include <iparamb2.h>
#include <stdmat.h>

#include <vector>
#include <string>

#include <libASE.h>


#define CTD_ASE_IMPORTER_CLASSNAME		"CTD ASE importer"
#define CTD_ASE_IMPORTER_CLASSID		Class_ID(0x70496808, 0x58e575af)
extern  HINSTANCE						g_hInstance;
ClassDesc2								*GetImporterDesc();


/*! External classes */
class IGameScene;

class Importer : public SceneImport
{
	public:

		/**
		*/
		                                                        Importer();
		/**
		*/
		virtual                                                ~Importer();


		/**
		* \param iExtID                                         Extension string number
		* \return                                               Extensions string
		*/
		const TCHAR                                            *Ext( int iExtID );

		/**
		* \return                                               Number of extensions supported
		*/
		int														ExtCount();

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
		* Import scene
		* \param pszName                                        File name
		* \param pImpInterface                                  Importing interface
		* \param pInterface                                     Interface
		* \param bSuppressPrompts                               Suppress prompts flag
		* \param ulOptions                                      Options
		* \return                                               TRUE if success, FALSE if error
		*/
		int                                                     DoImport( const TCHAR *pszName, ImpInterface *pImpInterface, Interface *pInterface, BOOL bSuppressPrompts = FALSE );

		/*! file major version */
		static int                                              s_iMajorVersion;

		/*! file minor version */
		static int                                              s_iMinorVersion;


	private:

		/*! IGame scene object */
		IGameScene                                             *m_pkIGameScene;

		/**
		* Pointer to Importer interface; this is used by function ImportNodeInfo
		*/
		ImpInterface											*m_pkImpInterface;

		/**
		* Material list setup after reading the ase file
		*/
		std::vector< StdMat* >									m_vkMaterials;

		/**
		* Import geometry read in ASE file
		* \param pkASEGeomObj                                   ASE object
		*/
		void                                                    ImportGeometry( ASE_GeomObject *pkASEGeomObj );

		/**
		* Setup materials
		*/
		void													SetupMaterials( ASE_Material *pkASEMaterial );

};

#endif // __CTD_ASEIMPORTER_H
