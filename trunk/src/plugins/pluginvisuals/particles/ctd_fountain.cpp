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
 # neoengine, particle class
 #
 # this class implements a particle system as entity
 #
 #
 #   date of creation:  03/23/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>
#include "ctd_fountain.h"
#include <neochunkio/materiallib.h>
#include <neochunkio/material.h>
#include <neoengine/sprite.h>
#include <ctd_printf.h>

#include "particlelib/general.h"

using namespace std;
using namespace CTD;
using namespace NeoEngine;
using namespace NeoChunkIO;


namespace CTD_IPluginVisuals {


// plugin global entity descriptor for fountain particle system
CTDParticleFountainDesc g_pkParticleFountainEntity_desc;
//-------------------------------------------//


CTDParticleFountain::CTDParticleFountain()
{

	CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Visuals) entity ' Particle ' created " );

	m_kPosition				= Vector3d( 0, 0, 0 );
	m_kDimensions			= Vector3d( 2, 4, 2 );
	m_fSizeU				= 1.0f;
	m_fSizeV				= 1.0f;
	m_pkSprite				= NULL;

	m_bAutoActivate			= true;

	m_iPopulation			= 500;
	m_iRate					= 80;
	m_fLife					= 3.0f;
	m_fFadeOutFactor		= 0.0f;
	m_kColor				= Vector3d( 0.9f, 0.9f, 0.9f );
	m_kVelocity				= Vector3d( 0.0f, 0.0f, 8.0f );
	m_fVelocitySigma		= 0.3f;
	m_kGravity				= Vector3d( 0.0f, 0.0f, -9.8f );
	m_fBounceFriction		= 0.9f;
	m_fBounceResilience		= 0.35f;
	m_kSourcePosition		= Vector3d( 0.0f, 0.0f, 0.0f );
	m_kSourceRadius			= 0.4f;
	
}

CTDParticleFountain::~CTDParticleFountain()
{

	CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Visuals) entity ' Particle ' destroyed " );

}

// init entity
void CTDParticleFountain::Initialize() 
{ 

	// set the entity ( node ) name
	SetName( CTD_ENTITY_NAME_ParticleFountain );

	MaterialPtr		pkMaterial = NULL;
	// check for a valid texture file name
	if ( m_strMatFile.length() == 0 ) {
	
		CTDCONSOLE_PRINT( LogLevel( WARNING ), " (Plugin Visuals) entity ' Particle::" + GetInstanceName() +" ': missing material file name! " );

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

			CTDCONSOLE_PRINT( LogLevel( WARNING ), " (Plugin Visuals) entity ' Particle::" + GetInstanceName() +" ': could not find material file! " );

		}

		delete pkLib;

	}

	// create new sprite
	m_pkSprite = new Sprite( pkMaterial, m_fSizeU, m_fSizeV );

	// create bounding volume for particle entity
	AABB *pkBVol	= new AABB;
	pkBVol->SetDim( m_kDimensions );
	GetBoundingVolume()->Generate( pkBVol );

	// set position
	SetTranslation( m_kPosition );
	//------------------------------//

	// deactivate entity if auto activate is set to false
	if ( m_bAutoActivate == false ) {

		Deactivate();

	}

	// setup particle system
	//----------------------//

	// create a group
	CreateGroup( m_kPosition, m_iPopulation );
	// line: (a0-a2 first point, a3-a5 second point)
	pColorD( 1.0f, PDPoint, m_kColor.x, m_kColor.y, m_kColor.z );
	pSize( 1 );
	pStartingAge( m_fLife, sqrtf( m_fLife / 4.0f ) );
	pTimeStep( 0.0f );
	pVelocityD( PDBlob, m_kVelocity.x, m_kVelocity.z, m_kVelocity.y, m_fVelocitySigma );

	// create an action list which is executed in every update
	NewActionList();

		pCopyVertexB( false, true );
		pSource( m_iRate, PDDisc, m_kSourcePosition.x, m_kSourcePosition.z, m_kSourcePosition.y, 0,0,1, 0.05f, m_kSourceRadius );
		if ( m_fFadeOutFactor > 0.0f ) {
			pTargetColor( 0,0,0, 0, m_fFadeOutFactor );
		}

		pGravity( m_kGravity.x, m_kGravity.z, m_kGravity.y );
		pKillOld( m_fLife );
		// bouncing plane: (a0-a2 pos, a3-a5 normal vector)
		pBounce( m_fBounceFriction, m_fBounceResilience, 0, PDPlane, 0,0,0, 0,0,1 );
		pMove();
	
	EndActionList();


}

void CTDParticleFountain::UpdateEntity( float fDeltaTime )
{

	// update particle system
	UpdateParticles( fDeltaTime );

}

bool CTDParticleFountain::Render( Frustum *pkFrustum, bool bForce ) 
{

	if( bForce ) {

		m_uiLastFrame = s_uiFrameCount; 
		return true; 
	} 

	if( !m_bActive || ( m_uiLastFrame >= s_uiFrameCount ) ) 
		return false; 

	m_uiLastFrame = s_uiFrameCount; 

	// prepare a render primitive
	static RenderPrimitive  skPrimitive;
	static RenderPrimitive *spkPrimitive = &skPrimitive;	
	spkPrimitive->m_ePrimitive           = RenderPrimitive::TRIANGLESTRIP;
	spkPrimitive->m_pkVertexBuffer       = m_pkSprite->m_pkVertexBuffer;
	spkPrimitive->m_pkPolygonStripBuffer = m_pkSprite->m_pkPolygonBuffer;
	spkPrimitive->m_uiNumPrimitives      = spkPrimitive->m_pkPolygonStripBuffer->GetNumElements();
	spkPrimitive->m_pkMaterial           = m_pkSprite->m_pkMaterial;

	// render all particles
	RenderParticles( spkPrimitive );

	// reset pointers to allow ref counter to clean up
	spkPrimitive->m_pkMaterial           = 0;
	spkPrimitive->m_pkVertexBuffer       = 0;
	spkPrimitive->m_pkPolygonStripBuffer = 0;

	return true;

}

int CTDParticleFountain::Message( int iMsgId, void *pMsgStruct )
{

	return 0;

}

int	CTDParticleFountain::ParameterDescription( int iParamIndex, ParameterDescriptor *pkDesc )
{

	int iParaCount = 18;

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

		pkDesc->SetName( "Dimensions" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_VECTOR3 );
		pkDesc->SetVar( &m_kDimensions );
		
		break;

	case 2:

		pkDesc->SetName( "Population" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_INTEGER );
		pkDesc->SetVar( &m_iPopulation );
		
		break;

	case 3:

		pkDesc->SetName( "GenerationRate" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_INTEGER );
		pkDesc->SetVar( &m_iRate );
		
		break;

	case 4:

		pkDesc->SetName( "Life" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_FLOAT );
		pkDesc->SetVar( &m_fLife );
		
		break;

	case 5:

		pkDesc->SetName( "FadeOutFactor" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_FLOAT );
		pkDesc->SetVar( &m_fFadeOutFactor );
		
		break;

	case 6:

		pkDesc->SetName( "SizeU" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_FLOAT );
		pkDesc->SetVar( &m_fSizeU );
		
		break;

	case 7:

		pkDesc->SetName( "SizeV" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_FLOAT );
		pkDesc->SetVar( &m_fSizeV );
		
		break;

	case 8:

		pkDesc->SetName( "MaterialFile" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_STRING );
		pkDesc->SetVar( &m_strMatFile );
		
		break;

	case 9:

		pkDesc->SetName( "Color" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_VECTOR3 );
		pkDesc->SetVar( &m_kColor );
		
		break;

	case 10:

		pkDesc->SetName( "Velocity" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_VECTOR3 );
		pkDesc->SetVar( &m_kVelocity );
		
		break;

	case 11:

		pkDesc->SetName( "VelocitySigma" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_FLOAT );
		pkDesc->SetVar( &m_fVelocitySigma );
		
		break;

	case 12:

		pkDesc->SetName( "Gravity" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_VECTOR3 );
		pkDesc->SetVar( &m_kGravity );
		
		break;

	case 13:

		pkDesc->SetName( "BounceFriction" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_FLOAT );
		pkDesc->SetVar( &m_fBounceFriction );
		
		break;

	case 14:

		pkDesc->SetName( "BounceResilience" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_FLOAT );
		pkDesc->SetVar( &m_fBounceResilience );
		
		break;

	case 15:

		pkDesc->SetName( "SourcePosition" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_VECTOR3 );
		pkDesc->SetVar( &m_kSourcePosition );
		
		break;

	case 16:

		pkDesc->SetName( "SourceRadius" );
		pkDesc->SetType( ParameterDescriptor::CTD_PD_FLOAT );
		pkDesc->SetVar( &m_kSourceRadius );
		
		break;
	
	case 17:

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
