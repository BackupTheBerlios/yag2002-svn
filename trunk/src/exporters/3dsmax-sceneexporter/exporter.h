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

#ifndef __NEOENGINE_3DSMAX_EXPORTER_H
#define __NEOENGINE_3DSMAX_EXPORTER_H

#ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>

#include <neoengine/nemath.h>

#include <max.h>

#if (_MSC_VER >= 1300)
#  include "MAX_Mem.h"
#endif

#include <istdplug.h>
#include <iparamm2.h>
#include <iparamb2.h>

#include <neoengine/polygon.h>
#include <neoengine/vertex.h>
#include <neochunkio/chunk.h>

#include <vector>
#include <string>

#include "vertex.h"

#define NSCEEXPORTER_CLASSNAME         "CTD NSCEExporter ( Lightmaps support )"
#define NSCEEXPORTER_CLASSID           Class_ID(0x3cfa620c, 0x4c5a7191)


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

//! Material definition
class MaterialDefinition 
{
    public:

                                    MaterialDefinition() { m_eDefinitionType = eInline; }

        //! Material name
        std::string                 m_strMaterialName;

        //! Material file name for those materials which are not inlined
        std::string                 m_strMaterialFileName;

        //! Base texture
        std::string                 m_strBaseTexture;

        //! Lightmap texture
        std::string                 m_strLightmapTexture;

        //! Heighttmap texture
        std::string                 m_strHeightmapTexture;

        //! Inline material definition ( inside of nsce file ) or extern material lib definition
        enum { eInline, eFile }     m_eDefinitionType;

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


        /*! Bones */
        std::vector< MaxBone* >                                 m_vpkBones;

        /*! Blueprints */
        std::vector< NeoChunkIO::Chunk* >                       m_vpkBlueprints;

        /*! Animations */
        std::vector< NeoChunkIO::Chunk* >                       m_vpkAnimations;

        /*! Vertices */
        std::vector< std::vector< MaxVertex* > >                m_vvpkMeshVertices;

        /*! Polygons */
        std::vector< std::vector< NeoEngine::Polygon   > >      m_vvkMeshPolygons;

        /*! Material definitions */
        std::vector< MaterialDefinition >                       m_vMaterialDefinitions;

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
        * Add a bone to skeletal hierarchy
        * \param pkBone                                         Bone object
        */
        void                                                    AddBone( IGameNode *pkBone );

        /**
        */
                                                                Exporter();

        /**
        */
        virtual                                                ~Exporter();
};


};


#endif
