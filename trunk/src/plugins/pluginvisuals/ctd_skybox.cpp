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
 # neoengine, skybox class
 #
 # this class implements an entity a skybox
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         ali.botorabi@daimlerchrysler.com
 #
 #   date of creation:  02/13/2004
 #
 ################################################################*/


#include <base.h>

#include "ctd_skybox.h"

#include <ctd_printf.h>

#include <neoengine/skybox.h>

using namespace std;
using namespace CTD;
using namespace NeoEngine;


namespace CTD_IPluginVisuals {


// plugin global entity descriptor for sky box
 CTDSkyBoxDesc g_pkSkyBoxEntity_desc;
//-------------------------------------------//



CTDSkyBox::CTDSkyBox()
{

	CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Visuals) entity ' SkyBox ' created " );

	m_pkSkyBox = NULL;

}

CTDSkyBox::~CTDSkyBox()
{

	CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Visuals) entity ' SkyBox ' destroyed " );

	// check for valid initialization
	if ( IsActive() == true ) {

		Framework::Get()->GetCurrentLevelSet()->GetRoom()->DetachGlobalNode( this );

	}

}

// init entity
void CTDSkyBox::Initialize() 
{ 

	// set the entity ( node ) name
	SetName( CTD_ENTITY_NAME_SkyBox );

	// check wether all textures are valid
	if ( 
		!m_strTextureUP.length()    || 
		!m_strTextureDown.length()  ||
		!m_strTextureLeft.length()  ||
		!m_strTextureRight.length() ||
		!m_strTextureFront.length() ||
		!m_strTextureBack.length()
		) {
		
		Deactivate();
		CTDCONSOLE_PRINT( LogLevel( WARNING ), " (Plugin Visuals) entity ' SkyBox ': missing texture parameter (s), entity deactivated! " );
		return;
	}

	string              astrName[] = { m_strTextureUP, m_strTextureDown, m_strTextureLeft, m_strTextureRight, m_strTextureFront, m_strTextureBack };
	SkyBox::SKYBOXPLANE aePlane[]  = { SkyBox::UP, SkyBox::DOWN, SkyBox::LEFT, SkyBox::RIGHT, SkyBox::FRONT, SkyBox::BACK };

	m_pkSkyBox = new SkyBox;

	// load textures and assign them to skybox
	for( int i = 0; i < 6; ++i )
	{

		MaterialPtr pkMat = new Material( "skydome_" + astrName[i] );

		pkMat->m_pkTexture			= Framework::Get()->GetRenderDevice()->LoadTexture( astrName[i] );
		pkMat->m_uiUVAddress		= TextureLayer::CLAMP;

		m_pkSkyBox->SetMaterial( aePlane[i], pkMat );

	}
		

	// set the skybox into this node and attach it to room manager as global node
	SetEntity( m_pkSkyBox );
	
	Framework::Get()->GetCurrentLevelSet()->GetRoom()->AttachGlobalNode( this );

}

bool CTDSkyBox::Render( Frustum *pkFrustum, bool bForce ) 
{

	 if( bForce ) {
		 
		 m_uiLastFrame = s_uiFrameCount; 
		 return true; 
	 } 
	 
	 if( !m_bActive || ( m_uiLastFrame >= s_uiFrameCount ) ) 
		 return false; 
	 
	 m_uiLastFrame = s_uiFrameCount; 

	 m_pkSkyBox->Render();

	 return true;
}

int	CTDSkyBox::ParameterDescription( int iParamIndex, ParameterDescriptor *pkDesc )
{

	int iParamCount = 6;

	if (pkDesc == NULL) {

		return iParamCount;
	}

	switch( iParamIndex ) 
	{
	case 0:

		pkDesc->SetName( "TextureUp" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_STRING );
		pkDesc->SetVar( &m_strTextureUP );
		
		break;

	case 1:

		pkDesc->SetName( "TextureDown" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_STRING );
		pkDesc->SetVar( &m_strTextureDown );
		
		break;

	case 2:

		pkDesc->SetName( "TextureLeft" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_STRING );
		pkDesc->SetVar( &m_strTextureLeft );
		
		break;

	case 3:

		pkDesc->SetName( "TextureRight" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_STRING );
		pkDesc->SetVar( &m_strTextureRight );
		
		break;

	case 4:

		pkDesc->SetName( "TextureFront" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_STRING );
		pkDesc->SetVar( &m_strTextureFront );
		
		break;

	case 5:

		pkDesc->SetName( "TextureBack" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_STRING );
		pkDesc->SetVar( &m_strTextureBack );
		
		break;

	default:

		return -1;

	}

	return iParamCount;

}

} // namespace CTD_IPluginVisuals
