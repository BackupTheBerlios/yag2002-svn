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
 # neoengine, static mesh class
 #
 # this class implements an entity for placing meshes into a map
 #
 #
 #   date of creation:  12/14/2003
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>

#include "ctd_staticmesh.h"
#include <ctd_printf.h>

#include <neoengine/pixelbuffer.h>


using namespace std;
using namespace CTD;
using namespace NeoEngine;

namespace CTD_IPluginVisuals {

// plugin global entity descriptor for static mesh
CTDStaticMeshDesc g_pkStaticMeshEntity_desc;
//-------------------------------------------//


// helper class for automatic texture matrix generation
//  this can be used to achieve environmental mapping effect for the mesh
class TextureMatrixGenInvView : public TextureMatrixGen
{
	public:

		virtual void                            GenerateMatrix( Matrix *pkMatrix );

		virtual TextureMatrixGen               *Duplicate() { return new TextureMatrixGenInvView; }
};

void TextureMatrixGenInvView::GenerateMatrix( Matrix *pkMatrix )
{
	*pkMatrix = Framework::Get()->GetRenderDevice()->GetViewMatrix();

	// Reset translation
	pkMatrix->m_aafMatrix[0][3] = pkMatrix->m_aafMatrix[1][3] = pkMatrix->m_aafMatrix[2][3] = 0.0f;

	// Invert rotation
	pkMatrix->Transpose();
}


CTDStaticMesh::CTDStaticMesh()
{

	CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Visuals) entity ' StaticMesh ' created " );

	m_kPosition				= Vector3d( 0, 0, 0 );
	m_kRotation				= Vector3d( 0, 0, 0 );

	m_kAmientColor			= Vector3d( 0.2f, 0.2f, 0.2f );
	m_kDiffuseColor			= Vector3d( 0.5f, 0.5f, 0.5f );
	m_kSpecularColor		= Vector3d( 0.0f, 0.0f, 0.0f );
	m_kEmission				= Vector3d( 0.0f, 0.0f, 0.0f );
	m_fShininess			= 0.0f;

	m_fScale				= 1.0f;
	m_bThrowShadows			= false;
	m_pkMesh				= NULL;

	m_pkRenderDevice		= NULL;
	m_pkCubemap				= NULL;
	m_bEnvMapping			= false;

}

CTDStaticMesh::~CTDStaticMesh()
{

	CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Visuals) entity ' StaticMesh ' destroyed " );


}

// init entity
void CTDStaticMesh::Initialize() 
{ 

	// set the entity ( node ) name
	SetName( CTD_ENTITY_NAME );

	// activate this entity on startup only if the mesh is valid!
	if ( m_pkMesh ) {
	
		Activate();

	} else {

		Deactivate();
		CTDCONSOLE_PRINT( LogLevel( WARNING ), " (Plugin Visuals) entity ' StaticMesh ': missing mesh parameter, entity deactivated! " );
		return;
	}

	// set the mesh into this node
	SetEntity( m_pkMesh );

	// adapt mesh's bounding volume considering the scaling
	AABB *pkBVol = ( AABB* )m_pkMesh->GetBoundingVolume();
	pkBVol->SetDim( pkBVol->GetDim() * m_fScale );
	GetBoundingVolume()->Generate( pkBVol );

	// set initial scaling, position and orientation
	SetScaling( m_fScale );
	Quaternion	kRot( EulerAngles( m_kRotation.x * PI / 180.0f, m_kRotation.y * PI / 180.0f, m_kRotation.z * PI / 180.0f ) );
	SetRotation( kRot );
	SetTranslation( m_kPosition );
	//------------------------------//

	Color	kAmbient( m_kAmientColor.x, m_kAmientColor.y, m_kAmientColor.z, 1.0f );
	Color	kDiffuse( m_kDiffuseColor.x, m_kDiffuseColor.y, m_kDiffuseColor.z, 1.0f );
	Color	kSpecular( m_kSpecularColor.x, m_kSpecularColor.y, m_kSpecularColor.z, 1.0f );
	Color	kEmission( m_kEmission.x, m_kEmission.y, m_kEmission.z, 1.0f );

	m_pkRenderDevice	= Framework::Get()->GetRenderDevice();
	m_pkCubemapCamera = new Camera( "cubemap cam" );
	m_pkCubemapCamera->SetRoom( Framework::Get()->GetCurrentLevelSet()->GetRoom() );

	if ( m_bEnvMapping == true ) {

		if( !( m_pkCubemap = m_pkRenderDevice->CreatePixelBuffer( 32, 32, m_pkRenderDevice->GetBPP(), Texture::CUBEMAP ) ) ) {
		
			CTDCONSOLE_PRINT( LogLevel( WARNING ), " (Plugin Visuals) entity ' StaticMesh ': Unable to create environment cubemap pixelbuffer! " );

		}
	
	}
	
	// set shadow flag
	unsigned int uiNumMeshes = ( unsigned int )m_pkMesh->GetSubMeshes().size();
	for ( unsigned int uiMeshCnt = 0; uiMeshCnt < uiNumMeshes; uiMeshCnt++ ) {

		m_pkMesh->GetSubMeshes()[ uiMeshCnt ]->m_pkMaterial->m_bDynamicShadows = m_bThrowShadows;
		m_pkMesh->GetSubMeshes()[ uiMeshCnt ]->m_pkMaterial->m_kAmbient			= kAmbient;
		m_pkMesh->GetSubMeshes()[ uiMeshCnt ]->m_pkMaterial->m_kDiffuse			= kDiffuse;
		m_pkMesh->GetSubMeshes()[ uiMeshCnt ]->m_pkMaterial->m_kSpecular		= kSpecular;
		m_pkMesh->GetSubMeshes()[ uiMeshCnt ]->m_pkMaterial->m_kEmission		= kEmission;
		m_pkMesh->GetSubMeshes()[ uiMeshCnt ]->m_pkMaterial->m_fShininess		= m_fShininess;

		if ( ( m_bEnvMapping == true ) && ( m_pkCubemap != NULL ) ) {

			TextureLayer *pkLayer = new TextureLayer;

			pkLayer->m_pkTexture    = m_pkCubemap->GetTexture();
			pkLayer->m_eTexCoordGen = TextureLayer::REFLECTION;
			//pkLayer->m_uiUVAddress  = TextureLayer::CLAMP;
			pkLayer->m_uiUVLayer    = 1;

			if( Framework::Get()->GetRenderDevice()->GetCaps().IsSet( RenderCaps::BLENDMODE_FACTOR ) ) {

				pkLayer->m_kBlendMode.Set( BlendMode::DECALFACTOR );
				pkLayer->m_kBlendMode.m_fFactor = 0.6f;
			
			} else {

				pkLayer->m_kBlendMode.Set( BlendMode::MODULATE_2X );

			}

			pkLayer->m_vpkTexMatrixGen.push_back( new TextureMatrixGenInvView );

			m_pkMesh->GetSubMeshes()[ uiMeshCnt ]->m_pkMaterial->m_vpkTextureLayers.push_back( pkLayer );

		}

	}

	//m_pkMesh->CalculateTangents();

	if ( m_bEnvMapping == true ) {
	
		Framework::Get()->RegisterPreRenderEntity( this );

	}


}

void CTDStaticMesh::UpdateEntity( float fDeltaTime )
{

}


bool CTDStaticMesh::Render( Frustum *pkFrustum, bool bForce ) 
{

	 if( bForce ) {
		 
		 m_uiLastFrame = s_uiFrameCount; 
		 return true; 
	 } 
	 
	 if( !m_bActive || ( m_uiLastFrame >= s_uiFrameCount ) ) 
		 return false; 
	 
	 m_uiLastFrame = s_uiFrameCount; 

	 m_pkMesh->Render();

	 return true;
}

// render the cubemap
void CTDStaticMesh::PreRender()
{
	Deactivate();
	//m_pkLight->Deactivate();

	m_pkRenderDevice->SetPerspectiveProjection( 90.0f, 0.1f, 10.0f );

	m_pkCubemap->SetCubeMapFace( Texture::CUBEFACE_FRONT );

	m_pkRenderDevice->SetRenderTarget( m_pkCubemap->GetTargetID() );

	m_pkRenderDevice->SetCullMode( RenderDevice::CULLFRONT );

	m_pkRenderDevice->Clear( RenderDevice::COLORBUFFER | RenderDevice::ZBUFFER | RenderDevice::STENCILBUFFER, Color::BLACK, 1.0f, 0 );

	m_pkCubemapCamera->SetRotation( Quaternion::IDENTITY );
	m_pkCubemapCamera->SetTranslation( GetTranslation() );

	Matrix kFlipView = m_pkCubemapCamera->GetViewMatrix();

	kFlipView.m_aafMatrix[0][0] = -kFlipView.m_aafMatrix[0][0];
	kFlipView.m_aafMatrix[0][1] = -kFlipView.m_aafMatrix[0][1];
	kFlipView.m_aafMatrix[0][2] = -kFlipView.m_aafMatrix[0][2];
	kFlipView.m_aafMatrix[0][3] = -kFlipView.m_aafMatrix[0][3];

	m_pkRenderDevice->Begin( kFlipView );
	{
		m_pkCubemapCamera->Render();
	}
	m_pkRenderDevice->End();


	m_pkCubemap->SetCubeMapFace( Texture::CUBEFACE_BACK );

	m_pkRenderDevice->Clear( RenderDevice::COLORBUFFER | RenderDevice::ZBUFFER | RenderDevice::STENCILBUFFER, Color::BLACK, 1.0f, 0 );

	m_pkCubemapCamera->SetRotation( EulerAngles( 0.0f, PI, 0.0f ) );

	kFlipView = m_pkCubemapCamera->GetViewMatrix();

	kFlipView.m_aafMatrix[0][0] = -kFlipView.m_aafMatrix[0][0];
	kFlipView.m_aafMatrix[0][1] = -kFlipView.m_aafMatrix[0][1];
	kFlipView.m_aafMatrix[0][2] = -kFlipView.m_aafMatrix[0][2];
	kFlipView.m_aafMatrix[0][3] = -kFlipView.m_aafMatrix[0][3];

	m_pkRenderDevice->Begin( kFlipView );
	{
		m_pkCubemapCamera->Render();
	}
	m_pkRenderDevice->End();


	m_pkCubemap->SetCubeMapFace( Texture::CUBEFACE_RIGHT );

	m_pkRenderDevice->Clear( RenderDevice::COLORBUFFER | RenderDevice::ZBUFFER | RenderDevice::STENCILBUFFER, Color::BLACK, 1.0f, 0 );

	m_pkCubemapCamera->SetRotation( EulerAngles( 0.0f, -HALF_PI, 0.0f ) );

	kFlipView = m_pkCubemapCamera->GetViewMatrix();

	kFlipView.m_aafMatrix[0][0] = -kFlipView.m_aafMatrix[0][0];
	kFlipView.m_aafMatrix[0][1] = -kFlipView.m_aafMatrix[0][1];
	kFlipView.m_aafMatrix[0][2] = -kFlipView.m_aafMatrix[0][2];
	kFlipView.m_aafMatrix[0][3] = -kFlipView.m_aafMatrix[0][3];

	m_pkRenderDevice->Begin( kFlipView );
	{
		m_pkCubemapCamera->Render();
	}
	m_pkRenderDevice->End();


	m_pkCubemap->SetCubeMapFace( Texture::CUBEFACE_LEFT );

	m_pkRenderDevice->Clear( RenderDevice::COLORBUFFER | RenderDevice::ZBUFFER | RenderDevice::STENCILBUFFER, Color::BLACK, 1.0f, 0 );

	m_pkCubemapCamera->SetRotation( EulerAngles( 0.0f, HALF_PI, 0.0f ) );

	kFlipView = m_pkCubemapCamera->GetViewMatrix();

	kFlipView.m_aafMatrix[0][0] = -kFlipView.m_aafMatrix[0][0];
	kFlipView.m_aafMatrix[0][1] = -kFlipView.m_aafMatrix[0][1];
	kFlipView.m_aafMatrix[0][2] = -kFlipView.m_aafMatrix[0][2];
	kFlipView.m_aafMatrix[0][3] = -kFlipView.m_aafMatrix[0][3];

	m_pkRenderDevice->Begin( kFlipView );
	{
		m_pkCubemapCamera->Render();
	}
	m_pkRenderDevice->End();

	
	m_pkCubemap->SetCubeMapFace( Texture::CUBEFACE_UP );

	m_pkRenderDevice->Clear( RenderDevice::COLORBUFFER | RenderDevice::ZBUFFER | RenderDevice::STENCILBUFFER, Color::BLACK, 1.0f, 0 );

	m_pkCubemapCamera->SetRotation( EulerAngles( HALF_PI, 0.0f, 0.0f ) );
	m_pkCubemapCamera->Rotate( EulerAngles( 0.0f, 0.0f, PI ) );

	kFlipView = m_pkCubemapCamera->GetViewMatrix();

	kFlipView.m_aafMatrix[0][0] = -kFlipView.m_aafMatrix[0][0];
	kFlipView.m_aafMatrix[0][1] = -kFlipView.m_aafMatrix[0][1];
	kFlipView.m_aafMatrix[0][2] = -kFlipView.m_aafMatrix[0][2];
	kFlipView.m_aafMatrix[0][3] = -kFlipView.m_aafMatrix[0][3];

	m_pkRenderDevice->Begin( kFlipView );
	{
		m_pkCubemapCamera->Render();
	}
	m_pkRenderDevice->End();


	m_pkCubemap->SetCubeMapFace( Texture::CUBEFACE_DOWN );

	m_pkRenderDevice->Clear( RenderDevice::COLORBUFFER | RenderDevice::ZBUFFER | RenderDevice::STENCILBUFFER, Color::BLACK, 1.0f, 0 );

	m_pkCubemapCamera->SetRotation( EulerAngles( -HALF_PI, 0.0f, 0.0f ) );
	m_pkCubemapCamera->Rotate( EulerAngles( 0.0f, 0.0f, PI ) );

	kFlipView = m_pkCubemapCamera->GetViewMatrix();

	kFlipView.m_aafMatrix[0][0] = -kFlipView.m_aafMatrix[0][0];
	kFlipView.m_aafMatrix[0][1] = -kFlipView.m_aafMatrix[0][1];
	kFlipView.m_aafMatrix[0][2] = -kFlipView.m_aafMatrix[0][2];
	kFlipView.m_aafMatrix[0][3] = -kFlipView.m_aafMatrix[0][3];

	m_pkRenderDevice->Begin( kFlipView );
	{
		m_pkCubemapCamera->Render();
	}
	m_pkRenderDevice->End();


	m_pkRenderDevice->SetRenderTarget( RenderDevice::FRAMEBUFFER );

	m_pkRenderDevice->SetCullMode( RenderDevice::CULLBACK );


	Activate();
	//m_pkLight->Activate();

}

int	CTDStaticMesh::ParameterDescription( int iParamIndex, ParameterDescriptor *pkDesc )
{

	int iParamCount = 11;

	if (pkDesc == NULL) {

		return iParamCount;
	}

	switch( iParamIndex ) 
	{
	case 0:

		pkDesc->SetName( "Position" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_VECTOR3 );
		pkDesc->SetVar( &m_kPosition );
		
		break;

	case 1:

		pkDesc->SetName( "Rotation" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_VECTOR3 );
		pkDesc->SetVar( &m_kRotation );
		
		break;

	case 2:

		pkDesc->SetName( "Scale" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_FLOAT );
		pkDesc->SetVar( &m_fScale );
		
		break;

	case 3:

		pkDesc->SetName( "Mesh" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_STATIC_MESH );
		pkDesc->SetVar( &m_pkMesh );
		
		break;

	case 4:

		pkDesc->SetName( "ThrowShadow" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_BOOL );
		pkDesc->SetVar( &m_bThrowShadows );
		
		break;

	case 5:

		pkDesc->SetName( "AmientColor" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_VECTOR3 );
		pkDesc->SetVar( &m_kAmientColor );
		
		break;

	case 6:

		pkDesc->SetName( "DiffuseColor" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_VECTOR3 );
		pkDesc->SetVar( &m_kDiffuseColor );
		
		break;

	case 7:

		pkDesc->SetName( "SpecularColor" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_VECTOR3 );
		pkDesc->SetVar( &m_kSpecularColor );
		
		break;
	
	case 8:

		pkDesc->SetName( "Emission" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_VECTOR3 );
		pkDesc->SetVar( &m_kEmission );
		
		break;
	
	case 9:

		pkDesc->SetName( "Shininess" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_FLOAT );
		pkDesc->SetVar( &m_fShininess );
		
		break;

	case 10:

		pkDesc->SetName( "EnvMapping" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_BOOL );
		pkDesc->SetVar( &m_bEnvMapping );
		
		break;

	default:

		return -1;
	
	}

	return iParamCount;

}

} // namespace CTD_IPluginVisuals 
