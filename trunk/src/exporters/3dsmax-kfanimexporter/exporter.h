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

#ifndef __NEOENGINE_3DSMAX_EXPORTER_H
#define __NEOENGINE_3DSMAX_EXPORTER_H

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#include <neoengine/nemath.h>

#include <max.h>
/*
#if (_MSC_VER >= 1300)
#  include "MAX_Mem.h"
#endif
*/
#include <istdplug.h>
#include <iparamm2.h>
#include <iparamb2.h>

#include <neoengine/polygon.h>
#include <neoengine/vertex.h>
#include <neochunkio/chunk.h>

#include <vector>
#include <string>

#include "vertex.h"
#include "mesh.h"

#define KFANIMEXPORTER_CLASSNAME         "CTD Keyframe Animation Exporter"
#define KFANIMEXPORTER_CLASSID           Class_ID(0x21b8134e, 0x65b93b28)


/*! External classes */
class IGameScene;
class IGameNode;
class IGameUVGen;


namespace NeoMaxExporter
{

/*! External classes */	
class MaxBone;

// new vertex type supporting base and lightmap textures
class NormalDiffuseLightmapTexVertex
{
	public:

		/*! Position */
		NeoEngine::Vector3d                           m_kPosition;

		/*! Normal */
		NeoEngine::Vector3d                           m_kNormal;

		/*! Base texture coordinate */
		float                                         m_afBaseTexCoord[2];

		/*! Lightmap texture coordinate */
		float                                         m_afLmTexCoord[2];


		/*! Vertex declaration */
		static NeoEngine::VertexDeclaration           s_kDecl;
};

class TextureLayers 
{
	public:

		std::string			m_strBaseTexture;
		std::string			m_strLightmapTexture;

};

extern  HINSTANCE                      g_hInstance;

ClassDesc2                            *GetExporterDesc();


class Exporter : public SceneExport
{
	public:

		/*! NSCE file major version */
		static int                                              s_iMajorNSCEVersion;

		/*! NSCE file minor version */
		static int                                              s_iMinorNSCEVersion;

		/*! NANI file major version */
		static int                                              s_iMajorNANIVersion;

		/*! NANI file minor version */
		static int                                              s_iMinorNANIVersion;

		/*! IGame scene object */
		IGameScene                                             *m_pkIGameScene;


		float                                                   m_fFramesPerSecond;
		int                                                     m_iTicksPerFrame;
		float                                                   m_fTicksPerSecond;

		int                                                     m_iStartFrame;
		int                                                     m_iEndFrame;

		
		/*! Max meshes */
		std::vector< MaxMesh* >									m_vpkMeshes;

		/*! Blueprints */
		std::vector< NeoChunkIO::Chunk* >                       m_vpkBlueprints;

		/*! Animations */
		std::vector< NeoChunkIO::Chunk* >                       m_vpkAnimations;

		/*! Vertices */
		std::vector< std::vector< MaxVertex* > >                m_vvpkMeshVertices;

		/*! Polygons */
		std::vector< std::vector< NeoEngine::Polygon   > >      m_vvkMeshPolygons;

		/*! Texture names */
		std::vector< TextureLayers >                            m_vstrTextures;

		/*! Texture matrices */
		std::vector< IGameUVGen* >                              m_vpkTexGen;

		/*! Lightmap Texture matrices */
		std::vector< IGameUVGen* >                              m_vpkLmTexGen;

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


		void													AddMesh( IGameNode *pkMesh );

};


};


#endif
