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
 # neoengine, entity for point light
 #
 #
 #   date of creation:  01/03/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>
#include "ctd_pointlight.h"
#include <ctd_printf.h>

#include <neoengine/sphere.h>


using namespace std;
using namespace CTD;
using namespace NeoEngine;

namespace CTD_IPluginVisuals {

// plugin global entity descriptor for point light
CTDPointLightDesc   g_pkPointLightEntity_desc;
//-------------------------------------------//

CTDPointLight::CTDPointLight()
{

    CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Visuals) entity ' PointLight ' created " );

    m_pkSprite          = NULL;
    m_pkSpriteFactory   = NULL;
    m_pkRoom            = NULL;

    m_fRadius           = 5.0f;

    // create light object
    m_pkLight                           = new Light( Light::POINT, Light::CASTSHADOWS );
    m_pkLight->m_kAmbient               = Color( 0.1f, 0.1f, 0.1f );
    m_pkLight->m_kDiffuse               = Color( 0.7f, 0.7f, 0.7f );
    m_pkLight->m_kSpecular              = Color( 0.9f, 0.9f, 0.9f );    
    m_pkLight->m_fConstantAttenuation   = 0.9f;
    m_pkLight->m_fLinearAttenuation     = 0.001f;
    m_pkLight->m_fQuadraticAttenuation  = 0.00001f;

}

CTDPointLight::~CTDPointLight()
{

    CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Visuals) entity ' PointLight ' destroyed " );
    if ( m_pkSprite != NULL ) {

        m_pkRoom->DetachNode( m_pkSprite );

    }

    m_pkRoom->DetachGlobalNode( this );

}


// init entity
void CTDPointLight::Initialize() 
{ 

    // set the entity ( node ) name
    SetName( CTD_ENTITY_NAME_PointLight );

    SetEntity( m_pkLight );

    // create a bbox for rendering ( only for debugging )
    Sphere kSphere;
    kSphere.m_fRadius = m_fRadius;
    GetBoundingVolume()->Generate( &kSphere );
    GetBoundingVolume()->SetTranslation( m_vPosition );

    SetTranslation( m_vPosition );
    
    // store the current level set room for later detaching light and sprite nodes
    m_pkRoom = Framework::Get()->GetCurrentLevelSet()->GetRoom();

    m_pkRoom->AttachGlobalNode( this );

}

void CTDPointLight::PostInitialize() 
{ 

    // assign sprite entity
    if ( m_strSprite.length() == 0 ) {
    
        CTDCONSOLE_PRINT( LogLevel( WARNING ), " (Plugin Visuals) entity ' PointLight::" + GetInstanceName() + " ' missing sprite entity parameter! " );

    }
        
    m_pkSpriteFactory   = Framework::Get()->FindEntity( m_strSprite, CTD_PLUGINNAME_VISUALS );

    if ( m_pkSpriteFactory == NULL ) {

        CTDCONSOLE_PRINT( LogLevel( WARNING ), " (Plugin Visuals) entity ' PointLight::" + GetInstanceName() + " ' cannot find sprite entity! " );

    } else {

        m_pkSprite = NULL;
        if ( m_pkSpriteFactory->Message( CTD_ENTITY_SPRITE_MSG_CREATE, &m_pkSprite ) == -1 ) {

            //unexpected return value
            return;

        }
    
        // set sprite position
        m_pkSprite->SetTranslation( GetTranslation() );
        // set sprite color
        Color   kColor( m_pkLight->m_kDiffuse );
        m_pkSprite->Message( CTD_ENTITY_SPRITE_MSG_SET_COLOR, &kColor );

    }

}
 
// render light
bool CTDPointLight::Render( Frustum *pkFrustum, bool bForce ) 
{

     m_pkLight->Render();

     return true;

}

int CTDPointLight::ParameterDescription( int iParamIndex, ParameterDescriptor *pkDesc )
{

    int iParamCount = 9;

    if (pkDesc == NULL) {

        return iParamCount;
    }

    switch( iParamIndex ) 
    {
    case 0:

        pkDesc->SetName( "Position" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_VECTOR3 );
        pkDesc->SetVar( &m_vPosition );
        
        break;

    case 1:

        pkDesc->SetName( "Radius" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_FLOAT );
        pkDesc->SetVar( &m_fRadius );
        
        break;

    case 2:

        pkDesc->SetName( "AmbientColor" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_VECTOR3 );
        pkDesc->SetVar( &m_pkLight->m_kAmbient );
        
        break;

    case 3:

        pkDesc->SetName( "DiffuseColor" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_VECTOR3 );
        pkDesc->SetVar( &m_pkLight->m_kDiffuse );
        
        break;

    case 4:

        pkDesc->SetName( "SpecularColor" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_VECTOR3 );
        pkDesc->SetVar( &m_pkLight->m_kSpecular );
        
        break;

    case 5:

        pkDesc->SetName( "AttenuationConstant" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_FLOAT );
        pkDesc->SetVar( &m_pkLight->m_fConstantAttenuation );
        
        break;

    case 6:

        pkDesc->SetName( "AttenuationLinear" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_FLOAT );
        pkDesc->SetVar( &m_pkLight->m_fLinearAttenuation );
        
        break;

    case 7:

        pkDesc->SetName( "AttenuationQuadratic" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_FLOAT );
        pkDesc->SetVar( &m_pkLight->m_fQuadraticAttenuation );
        
        break;

    case 8:

        pkDesc->SetName( "Sprite" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_STRING );
        pkDesc->SetVar( &m_strSprite );
        
        break;

    default:
        return -1;
    }

    return iParamCount;

}

} // namespace CTD_IPluginVisuals 
