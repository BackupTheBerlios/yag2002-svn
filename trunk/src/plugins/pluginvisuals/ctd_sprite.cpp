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
 # neoengine, sprite class
 #
 # this class implements a simple sprite
 #
 #
 #   date of creation:  03/21/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>

#include "ctd_sprite.h"
#include <ctd_printf.h>

#include <neoengine/sprite.h>
#include <neoengine/callback.h>
#include <neochunkio/materiallib.h>
#include <neochunkio/material.h>

using namespace std;
using namespace CTD;
using namespace NeoEngine;
using namespace NeoChunkIO;

namespace CTD_IPluginVisuals {

// plugin global entity descriptor for static mesh
CTDSpriteDesc g_pkSpriteEntity_desc;
//-------------------------------------------//



CTDSprite::CTDSprite()
{

    CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Visuals) entity ' Sprite ' created " );

    m_kPosition             = Vector3d( 0, 0, 0 );
    m_pkSprite              = NULL;
    m_fSizeU                = 1.0f;
    m_fSizeV                = 1.0f;
    m_kColor                = Vector3d( 0.9f, 0.9f, 0.9f );

    m_bAutoActivate         = true;

}

CTDSprite::~CTDSprite()
{

    CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Visuals) entity ' Sprite ' destroyed " );


}

// init entity
void CTDSprite::Initialize() 
{ 

    // set the entity ( node ) name
    SetName( CTD_ENTITY_NAME_Sprite );

    MaterialPtr     pkMaterial = NULL;
    // check for a valid texture file name
    if ( m_strMatFile.length() == 0 ) {
    
        CTDCONSOLE_PRINT( LogLevel( WARNING ), " (Plugin Visuals) entity ' Sprite::" + GetInstanceName() +" ': missing material file name! " );

    } else {

        // create a material
        MaterialLibrary *pkLib = new MaterialLibrary( NeoEngine::Core::Get()->GetMaterialManager(), NULL );
        pkLib->Load( m_strMatFile );

        // we need only the first material in mat lib!
        if ( pkLib->m_vpkMaterials.size() > 0 ) {

            pkMaterial = pkLib->m_vpkMaterials[ 0 ];
            pkMaterial->m_kAmbient[ 0 ] = m_kColor.x;
            pkMaterial->m_kAmbient[ 1 ] = m_kColor.y;
            pkMaterial->m_kAmbient[ 2 ] = m_kColor.z;
            pkMaterial->m_kDiffuse[ 0 ] = m_kColor.x;
            pkMaterial->m_kDiffuse[ 1 ] = m_kColor.y;
            pkMaterial->m_kDiffuse[ 2 ] = m_kColor.z;

        } else {

            CTDCONSOLE_PRINT( LogLevel( WARNING ), " (Plugin Visuals) entity ' Sprite" + GetInstanceName() +" ': could not find material file! " );

        }

        delete pkLib;

    }


    // create new sprite
    m_pkSprite = new Sprite( pkMaterial, m_fSizeU, m_fSizeV );

    // create bounding volume for sprite entity
    AABB *pkBVol    = new AABB;
    float fMaxLen   = ( m_fSizeU > m_fSizeV ) ? m_fSizeU : m_fSizeV; 
    pkBVol->SetDim( Vector3d( fMaxLen / 2.0f, fMaxLen / 2.0f, fMaxLen / 2.0f ) );
    m_pkSprite->SetBoundingVolume( pkBVol );

    SetEntity( m_pkSprite );

    // adapt mesh's bounding volume considering the scaling
    pkBVol = ( AABB* )m_pkSprite->GetBoundingVolume();
    pkBVol->SetDim( pkBVol->GetDim() );
    GetBoundingVolume()->Generate( pkBVol );

    // set position
    SetTranslation( m_kPosition );
    //------------------------------//

    // add callback function into render device
    Framework::Get()->GetRenderDevice()->RegisterFrameCallback( FrameCallback::FRAMEEND, this, true );


    // deactivate entity if auto activate is set to false
    if ( m_bAutoActivate == false ) {

        Deactivate();

    }

}

// draw the sprite at end of rendering phase ( do not render with dynamic lighting! )
void CTDSprite::FrameEvent( FRAMECALLBACKTYPE eType, void *pData )
{

    m_pkSprite->Render();

}

int CTDSprite::Message( int iMsgId, void *pMsgStruct )
{

    switch ( iMsgId ) {

        case CTD_ENTITY_SPRITE_MSG_CREATE:

            {

                // create a copy of this sprite entity and attach it into room manager
                CTDSprite   *pkSprite     = new CTDSprite;
                pkSprite->m_fSizeU        = m_fSizeU;
                pkSprite->m_fSizeV        = m_fSizeV;
                pkSprite->m_kColor        = m_kColor;
                pkSprite->m_strMatFile    = m_strMatFile;
                pkSprite->m_bAutoActivate = true;
                pkSprite->Initialize();
                Framework::Get()->GetCurrentLevelSet()->GetRoom()->AttachNode( ( SceneNode* )pkSprite );

                // return the entity for positioning by requesting entity
                * ( ( void** )pMsgStruct )  = ( void* )pkSprite; 
                break;
            }

        // needs a valid Color pointer for color components rgb
        case CTD_ENTITY_SPRITE_MSG_SET_COLOR:

            {
                Color   *pkColor = ( Color* )pMsgStruct;                
                Material  *pkMaterial = m_pkSprite->m_pkMaterial;
                pkMaterial->m_kAmbient = *pkColor;
                pkMaterial->m_kDiffuse = *pkColor;

                break;
            }

        default:

            return -1;

    }

    return 0;

}

int CTDSprite::ParameterDescription( int iParamIndex, ParameterDescriptor *pkDesc )
{

    int iParaCount = 6;

    if (pkDesc == NULL) {

        return iParaCount;
    }

    switch( iParamIndex ) 
    {
    case 0:

        pkDesc->SetName( "Position" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_VECTOR3 );
        pkDesc->SetVar( &m_kPosition );
        
        break;

    case 1:

        pkDesc->SetName( "SizeU" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_FLOAT );
        pkDesc->SetVar( &m_fSizeU );
        
        break;

    case 2:

        pkDesc->SetName( "SizeV" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_FLOAT );
        pkDesc->SetVar( &m_fSizeV );
        
        break;

    case 3:

        pkDesc->SetName( "MaterialFile" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_STRING );
        pkDesc->SetVar( &m_strMatFile );
        
        break;

    case 4:

        pkDesc->SetName( "Color" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_VECTOR3 );
        pkDesc->SetVar( &m_kColor );
        
        break;

    case 5:

        pkDesc->SetName( "AutoActivate" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_BOOL );
        pkDesc->SetVar( &m_bAutoActivate );
        
        break;

    default:

        return -1;
    
    }

    return iParaCount;

}

} // namespace CTD_IPluginVisuals 
