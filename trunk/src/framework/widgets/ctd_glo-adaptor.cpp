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
 # neoengine, adaptor for gui system basing on GLO
 #
 #
 #   date of creation:  09/04/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_frbase.h>

#include "inputManager_neo.h"
#include "rendManager_neo.h"

#include <Utility/utlMacros.h>
#include <Utility/utlVector.h>
#include <Utility/utlPulseTypes.h>
#include <Utility/utlARGB.h>
#include <Utility/utlError.h>
#include <GILib/XML/xmlLoader.h>
#include <System/sysTimer.h>
#include <System/sysConfig.h>
#include <GUI/guiCWidget.h>
#include <GUI/guiCEdit.h>
#include <Render/rendCBase.h>
#include <Render/rendMesh.h>
#include <Main/mainGG.h>

#include "ctd_glo-adaptor.h"


using namespace NeoEngine;
using namespace NeoChunkIO;
using namespace std;

namespace CTD
{

// macro for printing messages into cmd console
#define  CTDCONSOLE_PRINT( level, msg )     neolog << ( level ) << ( msg ) << endl;


// helper class for wrapping neo texture
class NeoTexWrapper
{

    public:

                                            NeoTexWrapper() { m_fWidth = 0; m_fHeight = 0; m_pkMaterial = NULL; m_pkSprite = NULL; }

        NeoEngine::MaterialPtr              m_pkMaterial;

        NeoEngine::Sprite                   *m_pkSprite;

        float                               m_fWidth;
        float                               m_fHeight;

};

// helper class for wrapping neo font
class NeoFontWrapper
{

    public:

                                            NeoFontWrapper() { m_pkFont = NULL; }

        NeoEngine::FontPtr                  m_pkFont;

};


// global instances
//------------------------------------------------------------------------------------------//
utlPulseUser< GLOAdaptor >                  g_pkPulse;
inputManager_Neo                            *g_pkInputManager;
rendManager_Neo                             *g_pkRenderManager;
NeoEngine::InputGroup                       *g_pkInputGroup;
//------------------------------------------------------------------------------------------//

// declare static member variables
int                                         GLOAdaptor::m_iScreenWidth;
int                                         GLOAdaptor::m_iScreenHeight;
float                                       GLOAdaptor::m_fScreenWidth;
float                                       GLOAdaptor::m_fScreenHeight;
char                                        GLOAdaptor::m_pcASCII[ CTD_GLO_MAX_STRINGBUFFER_SIZE ];
std::vector< NeoTexWrapper* >               GLOAdaptor::m_vpkTextures;


// adaptor's ctor
GLOAdaptor::GLOAdaptor()
{

    // proof: this class must be instantiated only once
    static unsigned int s_uiNumInstances = 0;
    s_uiNumInstances++;
    assert ( ( s_uiNumInstances == 1 ) && " GLOAdaptor can only be instantiated ones!" );
    //---------------------------------------------------------------------------------//

    m_iScreenWidth      = 0;
    m_iScreenHeight     = 0;
    m_fScreenWidth      = 0;
    m_fScreenHeight     = 0;
    m_pcASCII[ 0 ]      = 0;

}

void GLOAdaptor::Initialize()
{

    if ( m_bInitialized == true ) {

        return;

    }

    m_bInitialized      = true;

    // init the GLO gui system
    // create and setup the input manager
    g_pkInputGroup      = new InputGroup;
    g_pkInputManager    = new inputManager_Neo( g_pkInputGroup );
    g_pkInputGroup->AttachEntity( ( InputEntity* )NeoEngine::Core::Get()->GetConsole() );
    g_pkInputManager->Configure  ( NULL );

    g_pkRenderManager   = new rendManager_Neo();

    g_pkPulse.Construct( this, PS_RENDER_BEGIN, PG_GAME, "CTDGuiSystem" );

    g_pkRenderManager->Init( PT_SYSTEM );

    GG_Init();

}

void GLOAdaptor::Shutdown()
{

    if ( m_bInitialized == false ) {

        return;

    }

    // shutdown the GLO gui system
    GG_ShutDown();

}

void GLOAdaptor::Update()
{

    assert( ( m_bInitialized == true ) && " GLOApdator is not initialized, it cannot be updated." );

    // update the GLO gui system
    GG_Pulse();

}

// adaptor's dtor
GLOAdaptor::~GLOAdaptor()
{
}

// GLO interface funtions
//##################################################################################//

bool GLOAdaptor::Init( int a_Value )
{
    CTDCONSOLE_PRINT( LogLevel( INFO ), "GuiSystem: initializing the gui system" );

    if ( a_Value == PT_SYSTEM ) {

        // configure the gui system
        gg_SysAppConfig.pf_GetScreenSize      = GLOAdaptor::GetScreenSize;
        gg_SysAppConfig.pf_ImportScreen       = GLOAdaptor::XML_ImportFromFile;
        gg_SysAppConfig.pf_PrintError         = GLOAdaptor::PC_PrintError;

        gg_SysTimerConfig.pf_TimeTick         = GLOAdaptor::PC_TimeTick;
        gg_SysTimerConfig.pf_GetTimeDiffF     = GLOAdaptor::PC_GetTimeDiffF;
        gg_SysTimerConfig.pf_GetTimeDiff      = GLOAdaptor::PC_GetTimeDiff;
        gg_SysTimerConfig.pf_GetTimeCurrent   = GLOAdaptor::PC_GetTimeCUrrent;

        gg_SysMeshConfig.pf_GetMeshSize       = GLOAdaptor::GetMeshSize;
        gg_SysMeshConfig.pf_SetMeshSize       = GLOAdaptor::SetMeshSize;
        gg_SysMeshConfig.pf_CreateMesh        = GLOAdaptor::CreateMesh;
        gg_SysMeshConfig.pf_DestroyMesh       = GLOAdaptor::DestroyMesh;
        gg_SysMeshConfig.pf_DrawMesh          = GLOAdaptor::DrawMesh;

        gg_SysSpriteConfig.pf_DrawSprite      = GLOAdaptor::DrawSprite;

        gg_SysTextureConfig.pf_SetTextureSize = GLOAdaptor::SetTextureSize;
        gg_SysTextureConfig.pf_GetTextureSize = GLOAdaptor::GetTextureSize;
        gg_SysTextureConfig.pf_CreateTexture  = GLOAdaptor::CreateUnfilteredTexture;
        gg_SysTextureConfig.pf_DestroyTexture = GLOAdaptor::DestroyTexture;

        gg_SysFontConfig.pf_GetFontSize       = GLOAdaptor::GetFontSize;
        gg_SysFontConfig.pf_CreateFont        = GLOAdaptor::CreateFont;
        gg_SysFontConfig.pf_DestroyFont       = GLOAdaptor::DestroyFont;
        gg_SysFontConfig.pf_DrawFont          = GLOAdaptor::DrawFont;

        // calculate screen size related variables
        m_iScreenHeight = NeoEngine::Core::Get()->GetRenderDevice()->GetHeight();
        m_iScreenWidth  = NeoEngine::Core::Get()->GetRenderDevice()->GetWidth();
        m_fScreenHeight = ( float )m_iScreenHeight;
        m_fScreenWidth  = ( float )m_iScreenWidth;

    }

    return true;

}

bool GLOAdaptor::ShutDown( int a_Value )
{

    delete g_pkInputManager;
    delete g_pkInputGroup;

    return true;
}

int GLOAdaptor::Pulse()
{

    return 0;

}


bool GLOAdaptor::LoadXmlFile( const string& strFileName ) 
{

    // load the gui definitions from file
    return gg_SysAppConfig.pf_ImportScreen( ( char* )strFileName.c_str() );

}

bool GLOAdaptor::PC_PrintError( const char* a_Stmt, ... )
{

#ifdef _DEBUG

    char        pcTextBuffer[ 512 ];
    va_list     pkArguments;
    va_start( pkArguments, a_Stmt );
    vsprintf( pcTextBuffer, a_Stmt, pkArguments );
    va_end( pkArguments );

    CTDCONSOLE_PRINT( LogLevel( ERROR ), "GuiSystem-Msg: " + string( pcTextBuffer ) );

#endif

    return true;

}

bool GLOAdaptor::XML_ImportFromFile( char* a_pFileName )
{
    
    FileManager *pkFileManager = NeoEngine::Core::Get()->GetFileManager();
    File *pkFile = pkFileManager->GetByName( a_pFileName );
    if ( pkFile->Open() ) {

        CTDCONSOLE_PRINT( LogLevel( INFO ), "GuiSystem: parsing xml file " + string( a_pFileName ) );

        int     iFileSize = pkFile->GetSize();
        void    *pBuffer;
        pBuffer = new char[ iFileSize ];
        pkFile->Read( pBuffer, iFileSize );
        XML_ImportGG( pBuffer, iFileSize ); 

        delete []pBuffer;

    } else {

        CTDCONSOLE_PRINT( LogLevel( INFO ), "GuiSystem: could not open xml file " + string( a_pFileName ) );

        return false;

    }

    delete pkFile;
    
    return true;
}

bool GLOAdaptor::GetScreenSize( int* a_SizeX, int* a_SizeY )
{    

    *a_SizeX  = m_iScreenWidth;
    *a_SizeY  = m_iScreenHeight;

    return true;

};

/**************************************************
Timers
**************************************************/
void GLOAdaptor::PC_TimeTick() 
{
    sysTimer::p().Pulse();                
}

float GLOAdaptor::PC_GetTimeDiffF()
{
    return sysTimer::p().GetTimeDiffF();  
}

int GLOAdaptor::PC_GetTimeDiff()
{
    return sysTimer::p().GetTimeDiff();   
}

int GLOAdaptor::PC_GetTimeCUrrent()
{
    return sysTimer::p().GetTimeCurrent();
}

/**************************************************
Adapters - Textures
**************************************************/
bool GLOAdaptor::CreateTexture( int* a_pData, char* a_pName, int a_flags, int a_FilterFlags )
{

    string strFileName = File::ExtractBaseFileName( a_pName );

    TexturePtr pkTex  = NeoEngine::Core::Get()->GetRenderDevice()->LoadTexture( strFileName, Texture::TEX2D, Texture::DEFAULT, a_FilterFlags );

    // could we create a texture?
    if ( !pkTex ) {

        return false;

    }

    MaterialPtr  pkMaterial = new Material( a_pName, 0 );
    pkMaterial->m_kAmbient.Set( 1.0f, 1.0f, 1.0f, 1.0f );
    pkMaterial->m_kDiffuse.Set( 1.0f, 1.0f, 1.0f, 1.0f );
    pkMaterial->m_fShininess = 1.0f;
    pkMaterial->m_uiMaxLights = 0;
    pkMaterial->m_kBlendMode.Set( BlendMode::DECAL );   // decal means BlendMode::SRC_SRCALPHA | BlendMode::DEST_ONEMINUSSRCALPHA
    pkMaterial->m_kZBufferMode.Set( ZBufferMode::ALWAYSNOWRITE );
    pkMaterial->m_pkTexture = pkTex;

    NeoTexWrapper *pkTexWrapper = new NeoTexWrapper;
    pkTexWrapper->m_pkMaterial  = pkMaterial;
    pkTexWrapper->m_fWidth      = pkTex->GetOriginalWidth()  / m_fScreenWidth;
    pkTexWrapper->m_fHeight     = pkTex->GetOriginalHeight() / m_fScreenHeight;

    // we use neo sprite to setup and resizing primitives for 2d drawing
    pkTexWrapper->m_pkSprite    = new Sprite( pkTexWrapper->m_pkMaterial, pkTexWrapper->m_fWidth, pkTexWrapper->m_fHeight );

    // decrement smart pointers by one
    pkTex                       = NULL;
    pkMaterial                  = NULL;

    *a_pData = ( int )pkTexWrapper;

    m_vpkTextures.push_back( pkTexWrapper );

    return true;

}

bool GLOAdaptor::GetTextureSize( int a_pData, float* a_SizeX, float* a_SizeY )
{

    NeoTexWrapper *pkTexWrapper = ( NeoTexWrapper* )a_pData;

    if ( !pkTexWrapper ) {
        
        return false;

    }
    
    *a_SizeX = ( float )pkTexWrapper->m_fWidth;
    *a_SizeY = ( float )pkTexWrapper->m_fHeight;

    return true;

}

bool GLOAdaptor::SetTextureSize( int a_pData, float a_SizeX, float a_SizeY )
{

    NeoTexWrapper *pkTexWrapper = ( NeoTexWrapper* )a_pData;

    if ( !pkTexWrapper ) {
        
        return false;

    }

    pkTexWrapper->m_fWidth  = a_SizeX;
    pkTexWrapper->m_fHeight = a_SizeY;

    return true;
}

bool GLOAdaptor::CreateMeshTexture( int* a_pData, char* a_pName, int a_flags )
{

    return CreateTexture( a_pData, a_pName, a_flags, Texture::NOMIPMAPS );

}

bool GLOAdaptor::CreateUnfilteredTexture( int* a_pData, char* a_pName, int a_flags )
{
    
    return CreateTexture( a_pData, a_pName, a_flags, Texture::NOMIPMAPS );

}

bool GLOAdaptor::DestroyTexture( int a_pData )
{

    if (a_pData == 0) {

        return false;

    }

    NeoTexWrapper *pkTexWrapper = ( NeoTexWrapper* )a_pData;

    CTDCONSOLE_PRINT( LogLevel( INFO ), "GuiSystem: destroying texture '" + pkTexWrapper->m_pkSprite->m_pkMaterial->GetName() + "'" );

    // textures are always bound to sprites, they get destroyed when sprites are destroyed.
    delete pkTexWrapper->m_pkSprite;

    return true;

}

bool GLOAdaptor::DrawSprite( int a_pData, float a_Position[3], float a_Rotation[4], int a_Color )
{

    NeoTexWrapper *pkTexWrapper = ( NeoTexWrapper* )a_pData;

    if ( !pkTexWrapper ) {
        
        return false;

    }

    float fSpriteHeight = pkTexWrapper->m_fHeight; // m_fScreenHeight;
    float fSpriteWidth  = pkTexWrapper->m_fWidth;  // m_fScreenWidth;
    
    // render the gui element
    //----------------------------------------------//

    static RenderPrimitive  skPrimitive;
    static RenderPrimitive *spkPrimitive  = &skPrimitive;

    float fPosX = a_Position[ 0 ];
    float fPosY = a_Position[ 1 ];
    Vector3d kPosition( fPosX + fSpriteWidth * 0.5f, fPosY + 0.5f * fSpriteHeight, 0.0f );
    
    // setup rotation matrix
    spkPrimitive->m_kModelMatrix.SetRotation( Quaternion( 0, 1, 0, 0 ) );

    // FIXME: avoid this flip rotation by creating appropriate uv maps on sprite!
    Quaternion kRotFlip( EulerAngles( 0, 0, PI ) );
    spkPrimitive->m_kModelMatrix = kRotFlip * spkPrimitive->m_kModelMatrix;

    // if a rotation is needed then do it now
    //  there is a problem with rotating elements, they are skewed, the following if-then is a workaround for this problem
    if ( ( a_Rotation[ 1 ] != 0 ) || ( a_Rotation[ 2 ] != 0 ) || ( a_Rotation[ 3 ] != 1 ) ) 
    {

        // apply the gui element rotation
        Quaternion kRotQuat( a_Rotation[ 0 ], a_Rotation[ 1 ], a_Rotation[ 2 ], a_Rotation[ 3 ] );
        spkPrimitive->m_kModelMatrix = kRotQuat * spkPrimitive->m_kModelMatrix;
        pkTexWrapper->m_pkSprite->SetSize( /*( fOrthoRation ) **/ pkTexWrapper->m_fHeight /*/ m_fScreenHeight*/, pkTexWrapper->m_fWidth/* / m_fScreenWidth*/, true );

    } else {

        pkTexWrapper->m_pkSprite->SetSize( pkTexWrapper->m_fWidth /*/ m_fScreenWidth*/, pkTexWrapper->m_fHeight /*/ m_fScreenHeight*/, true );

    }

    // set translation
    spkPrimitive->m_kModelMatrix.SetTranslation( Vector3d( kPosition.x, kPosition.y, 0 ) );

    spkPrimitive->m_ePrimitive           = RenderPrimitive::TRIANGLESTRIP;
    spkPrimitive->m_pkVertexBuffer       = pkTexWrapper->m_pkSprite->m_pkVertexBuffer;
    spkPrimitive->m_pkPolygonStripBuffer = pkTexWrapper->m_pkSprite->m_pkPolygonBuffer;
    spkPrimitive->m_uiNumPrimitives      = spkPrimitive->m_pkPolygonStripBuffer->GetNumElements();
    spkPrimitive->m_pkMaterial           = pkTexWrapper->m_pkMaterial;

    if ( a_Color != -1 ) {
    
        utlARGB  *pkColor = ( utlARGB* ) &a_Color;
        float fBlue  = pkColor->m_C.blue  / 255.0f;
        float fGreen = pkColor->m_C.green / 255.0f;
        float fRed   = pkColor->m_C.red   / 255.0f;
        float fAlpha = pkColor->m_C.alpha / 255.0f;
        spkPrimitive->m_pkMaterial->m_kAmbient.Set( fBlue, fGreen, fRed, fAlpha );
    
    } else {

        spkPrimitive->m_pkMaterial->m_kAmbient.Set( Color::WHITE );

    }

    NeoEngine::Core::Get()->GetRenderDevice()->Render( *spkPrimitive );

    // reset smart pointers
    spkPrimitive->m_pkMaterial           = 0;
    spkPrimitive->m_pkVertexBuffer       = 0;
    spkPrimitive->m_pkPolygonStripBuffer = 0;

    return true;
}


/**************************************************
Adapters - Mesh's
**************************************************/
bool GLOAdaptor::CreateMesh( int* a_pData, char* a_pName, int a_flags )
{

    if (a_pData == 0)
        return false;

    CTDCONSOLE_PRINT( LogLevel( INFO ), "GuiSystem: creating mesh " + string( a_pName ) );

    Scene *pkScene = new Scene;
    if ( pkScene->Load( a_pName ) == false ) {

        neolog << LogLevel( WARNING ) << " GuiSystem: mesh '" << a_pName << "' could not be loaded " << endl; 

        delete pkScene;
        return false;

    }


    SceneNode  *pkNode = new SceneNode( a_pName );
    MeshEntity *pkMesh = new MeshEntity( pkScene->GetMeshes()[0] );
    pkMesh->GenerateBoundingVolume();
    pkMesh->GetMesh()->SetName( a_pName );
    pkNode->SetEntity( pkMesh );
        
    *a_pData = ( int )pkMesh;

    delete pkScene;

    return true;

}

bool GLOAdaptor::GetMeshSize( int a_pData, float* a_SizeX, float* a_SizeY )
{
    if (a_pData == 0)
        return false;

    MeshEntity *pkMesh = ( MeshEntity* )a_pData;
    AABB* pkAABB = ( AABB* )pkMesh->GetBoundingVolume();
    *a_SizeX  = pkAABB->GetDim().x;
    *a_SizeY  = pkAABB->GetDim().y;

    return true;
}


bool GLOAdaptor::SetMeshSize( int a_pData, float a_SizeX, float a_SizeY )
{
    if (a_pData == 0)
        return false;

    MeshEntity *pkMesh = ( MeshEntity* )a_pData;
    AABB* pkAABB = ( AABB* )pkMesh->GetBoundingVolume();
    pkAABB->SetDim( Vector3d( a_SizeX, a_SizeY, pkAABB->GetDim().z ) );

    return true;

}

bool GLOAdaptor::DestroyMesh( int a_pData )
{

    MeshEntity* pkMesh = ( MeshEntity* )a_pData;

    if ( !pkMesh ) {

        return false;

    }

    delete pkMesh->GetSceneNode();

    return true;

}

bool GLOAdaptor::DrawMesh( int a_pData, float a_Position[3], float a_Rotation[4], int a_Color )
{
    if (a_pData == 0)
        return false;

    Vector3d    kProjPosition( a_Position[ 0 ] / ( float )m_iScreenWidth, a_Position[ 1 ] / ( float )m_iScreenWidth, 1.0f / a_Position[ 2 ] ); 
    Vector3d    kPosition( Camera::GetActive()->GetWorldRotation() * kProjPosition );

    Quaternion  kRotation( a_Rotation[ 0 ], a_Rotation[ 1 ], a_Rotation[ 2 ], a_Rotation[ 3 ] );

    MeshEntity* pkMesh = ( MeshEntity* )a_pData;
    pkMesh->GetSceneNode()->SetTranslation( kPosition );
    pkMesh->GetSceneNode()->SetRotation( kRotation );
    pkMesh->GetSceneNode()->Rotate( Quaternion( 0, 0, 1, 0 ) );

    pkMesh->Render( NULL, true );

    return true;
}

/*************************************************
Fonts
*************************************************/
bool GLOAdaptor::CreateFont ( int* a_pData, char* a_pName, int a_flags )
{

    CTDCONSOLE_PRINT( LogLevel( INFO ), "GuiSystem: creating font " + string( a_pName ) );

    NeoFontWrapper *pkFontWrapper =  new NeoFontWrapper;

    if ( string( a_pName ) == "fixedsys" ) {

        pkFontWrapper->m_pkFont = NeoEngine::Core::Get()->GetConsole()->GetDefaultFont();
        *a_pData = ( int )pkFontWrapper;

        return true;

    }

    string strFontName = File::ExtractBaseFileName( a_pName );
    NeoEngine::FontManager  *pkFontManager = NeoEngine::Core::Get()->GetFontManager();
    pkFontWrapper->m_pkFont = pkFontManager->CreateFont( strFontName, true );
    
    if ( !pkFontWrapper->m_pkFont ) {

        CTDCONSOLE_PRINT( LogLevel( WARNING ), "GuiSystem: cannot create font" + strFontName );
        delete pkFontWrapper;
        return false;

    }

    int iClipWidth  = NeoEngine::Core::Get()->GetRenderDevice()->GetWidth();
    int iClipHeight = NeoEngine::Core::Get()->GetRenderDevice()->GetHeight();
    pkFontWrapper->m_pkFont->SetClipBox( 0, 0, iClipWidth, iClipHeight );
    pkFontWrapper->m_pkFont->SetHorizontalAlign( Font::LEFT );
    pkFontWrapper->m_pkFont->SetLineWrap( Font::NOWRAP );

    *a_pData = ( int )pkFontWrapper;

    return true;

}

bool GLOAdaptor::DestroyFont( int a_pData )
{

    if ( a_pData == 0 ) {

        return false;

    }

    NeoFontWrapper *pkFontWrapper = ( NeoFontWrapper* )a_pData;

    CTDCONSOLE_PRINT( LogLevel( INFO ), "GuiSystem: destroying font" );

    // the default delete operator calls also FontPtr's destructor
    delete pkFontWrapper;

    return true;

}

bool GLOAdaptor::GetFontSize ( int a_pData, void* a_pText, int a_NumChars, float* a_SizeX, float* a_SizeY )
{

    if (a_pText == 0) {

        *a_SizeX = 0;
        *a_SizeY = 0;
        return true;
        
    }

    NeoFontWrapper *pkFontWrapper = ( NeoFontWrapper* )a_pData;

    // use windows routine to convert unicode to ascii
    // FIXME: search for a multi-platform routine
    WideCharToMultiByte( CP_ACP, 0, ( wchar_t* )a_pText, -1, m_pcASCII, 1024, NULL, NULL );

    m_pcASCII[ a_NumChars ] = 0;

    *a_SizeX = ( float )pkFontWrapper->m_pkFont->GetStringLength( m_pcASCII )    / m_fScreenWidth;
    *a_SizeY = ( float )pkFontWrapper->m_pkFont->GetStringHeight( m_pcASCII, 0 ) / m_fScreenHeight;

    return true;

}

bool GLOAdaptor::DrawFont( int a_pData, void* a_pText, int a_Length, float a_Position[3], float a_Size[2], int a_Color, int a_Flags )
{

    if ( a_Length == 0 ) { 
        
        return false;

    }

    NeoFontWrapper *pkFontWrapper = ( NeoFontWrapper* )a_pData;

    // this flag is provided in xml file and defines a word-wrap for the given text
    if ( a_Flags == 16 ) {

        pkFontWrapper->m_pkFont->SetLineWrap( Font::WORDWRAP );

    } else {

        pkFontWrapper->m_pkFont->SetLineWrap( Font::NOWRAP );

    }

    int iClipX = ( int )( ( a_Position[ 0 ] + a_Size[ 0 ] ) * m_fScreenWidth );
    int iClipY = ( int )( ( a_Position[ 1 ] + a_Size[ 1 ] ) * m_fScreenHeight );

    pkFontWrapper->m_pkFont->SetClipBox( 0, 0, iClipX, iClipY );

    // set font color
    static Color s_kColor;
    if ( a_Color != -1 ) {
    
        utlARGB  *pkColor = ( utlARGB* ) &a_Color;
        float fBlue  = pkColor->m_C.blue  / 255.0f;
        float fGreen = pkColor->m_C.green / 255.0f;
        float fRed   = pkColor->m_C.red   / 255.0f;
        float fAlpha = pkColor->m_C.alpha / 255.0f;
        s_kColor.Set( fBlue, fGreen, fRed, fAlpha );
    
    } else {

        s_kColor.Set( Color::WHITE );

    }
    pkFontWrapper->m_pkFont->SetColor( s_kColor );

    // use windows routine to convert unicode to ascii
    // FIXME: search for a multi-platform routine
    static char s_pcASCII[ CTD_GLO_MAX_STRINGBUFFER_SIZE ];
    WideCharToMultiByte( CP_ACP, 0, ( wchar_t* )a_pText, -1, m_pcASCII, a_Length, NULL, NULL );
    m_pcASCII[ a_Length ] = 0; // terminate the string

    pkFontWrapper->m_pkFont->Printf( a_Position[ 0 ], a_Position[ 1 ], m_pcASCII );

    return true;

}

} // namespace CTD
