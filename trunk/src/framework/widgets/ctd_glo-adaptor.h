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
 # neoengine, gui system basing on GLO
 #
 # adaptor for GLO
 #
 #
 #   date of creation:  09/04/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_GLO_ADAPTOR_H_
#define _CTD_GLO_ADAPTOR_H_

#include <ctd_frbase.h>

#include "inputManager_neo.h"
#include <vector>

namespace CTD
{
    
// global acii character buffer for text fields. this is used for conversion of unicode text into ascii text
#define         CTD_GLO_MAX_STRINGBUFFER_SIZE   4096

// some predeclarations
class NeoTexWrapper;
class Widgets; 

//! Adaptor class for GLO gui system
/**
* This class adapts the GLO gui system to CTD framework.
*/
class GLOAdaptor
{

    friend class Widgets;

    protected:

        /**
        * Screen width
        */
        static int                                      m_iScreenWidth;

        /**
        * Screen Height
        */
        static int                                      m_iScreenHeight;

        /**
        * Screen width in float
        */
        static float                                    m_fScreenWidth;

        /**
        * Screen Height in float
        */
        static float                                    m_fScreenHeight;

        /**
        * Buffer for converting unicode to ascii code
        */
        static char                                     m_pcASCII[ CTD_GLO_MAX_STRINGBUFFER_SIZE ];

        /**
        * List of all created textures in gui system
        */
        static std::vector< NeoTexWrapper* >            m_vpkTextures;

        /**
        * This flag shows whether the adaptor is already initialized
        */
        bool                                            m_bInitialized;

    public:

        // public GLO interface functions, don't call these functions! They are called by GLO.
        //*************************************************************************************//
        static bool                                     Init( int a_Value );

        static bool                                     ShutDown( int a_Value );

        static int                                      Pulse();


    protected:

                                                        GLOAdaptor();

                                                        ~GLOAdaptor();

        /**
        * Initialize the GLO gui system. This is called by Widget core
        */
        void                                            Initialize();

        /**
        * Shutdown the GLO gui system. This is called by Widget core
        */
        void                                            Shutdown();

        /**
        * Update the GLO gui system. This is called by Widget core
        */
        void                                            Update();

        /**
        * Load gui definitions from given xml file
        */
        bool                                            LoadXmlFile( const std::string& strFileName ) ;

        /**
        * Return the inputManager instance
        */
        inputManager_Neo*                               GetInputManager() { return g_pkInputManager; }

        // GLO interface functions
        //*************************************************************************************//

        static bool                                     PC_PrintError(const char* a_Stmt, ...);

        static bool                                     XML_ImportFromFile(char* a_pFileName);

        static bool                                     GetScreenSize(int* a_SizeX,  int* a_SizeY);

        static void                                     PC_TimeTick(void);
        
        static float                                    PC_GetTimeDiffF(void);
        
        static int                                      PC_GetTimeDiff(void);
        
        static int                                      PC_GetTimeCUrrent(void);
        
        static bool                                     GetTextureSize(int a_pData, float* a_SizeX, float* a_SizeY);
        
        static bool                                     SetTextureSize(int a_pData, float a_SizeX, float a_SizeY);

        static bool                                     CreateTexture( int* a_pData, char* a_pName, int a_flags, int a_FilterFlags = NeoEngine::Texture::NOMIPMAPS );
        
        static bool                                     CreateMeshTexture(int* a_pData, char* a_pName, int a_flags);
        
        static bool                                     CreateUnfilteredTexture(int* a_pData, char* a_pName, int a_flags);
        
        static bool                                     DestroyTexture(int a_pData);
        
        static bool                                     DrawSprite(int a_pData, float a_Position[3], float a_Rotation[4], int a_Color);
        
        static bool                                     GetMeshSize(int a_pData, float* a_SizeX, float* a_SizeY);

        static bool                                     SetMeshSize(int a_pData, float a_SizeX, float a_SizeY);
        
        static bool                                     CreateMesh(int* a_pData, char* a_pName, int a_flags);
        
        static bool                                     DestroyMesh(int a_pData);
        
        static bool                                     DrawMesh(int a_pData, float a_Position[3], float a_Rotation[4], int a_Color);
        
        static bool                                     CreateFont (int* a_pData, char* a_pName, int a_flags);
        
        static bool                                     DestroyFont(int a_pData);

        static bool                                     GetFontSize (int a_pData, void* a_pText, int a_NumChars, float* a_SizeX, float* a_SizeY);
        
        static bool                                     DrawFont(int a_pData, void* a_pText, int a_Length, float a_Position[3], float a_Size[2], int a_Color, int a_Flags);


};

} // namespace CTD

#endif //_CTD_GLO_ADAPTOR_H_

