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
 # neoengine, base particle class
 #
 # this class implements common functions of a particle system
 #  it uses the library of David K. McAllister
 #
 #
 #   date of creation:  03/23/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>

#include "ctd_baseparticle.h"
#include "particlelib/general.h"

using namespace NeoEngine;
using namespace CTD;


namespace CTD_IPluginVisuals {


CTDBaseParticle::CTDBaseParticle()
{

    m_hParticleHandle       = -1;
    m_hActionHandle         = -1;

}

CTDBaseParticle::~CTDBaseParticle()
{

    if ( m_hParticleHandle != -1 ) {

        pDeleteParticleGroups( m_hParticleHandle, 1 );
    
    }

}

int CTDBaseParticle::NewActionList()
{

    m_hActionHandle = pGenActionLists( 1 );
    pNewActionList( m_hActionHandle );

    return m_hActionHandle;

}

void CTDBaseParticle::EndActionList()
{

    pEndActionList();

}

// botorabi ( 03/29/2004) : added neoengine's priminitve for rendering
int CTDBaseParticle::CreateGroup( const Vector3d &kOrigin, int iPopulation )
{

    m_kOrigin           = kOrigin;
    m_hParticleHandle   = pGenParticleGroups( 1, iPopulation ); 
    pCurrentGroup( m_hParticleHandle );

    return m_hParticleHandle;

}

void CTDBaseParticle::UpdateParticles( float fDeltaTime )
{

    // limit delta time to avoid crazy flying particles around
    float fLimDeltaTime = fDeltaTime;
    if ( fDeltaTime > 0.1f ) {

        fLimDeltaTime = 0.1f;

    }

    pCurrentGroup( m_hParticleHandle );
    pTimeStep( fLimDeltaTime );
    pCallActionList( m_hActionHandle );

}

void CTDBaseParticle::RenderParticles( RenderPrimitive *pkPrimitive )
{

     // setup the particle group
     //-------------------------//
    pCurrentGroup( m_hParticleHandle );
    _ParticleState &rkPS = _GetPState();
    // get a pointer to the particles in gp memory
    ParticleGroup *pkPG = rkPS.pgrp;
    if( pkPG == NULL ) {

        return; // ERROR
    
    }
    if( pkPG->p_count < 0 ) {

        return;

    }
     //-------------------------//

    static Camera *pkCamera;
    pkCamera = Camera::GetActive();
    static Quaternion kQuat;
    kQuat = pkCamera ? pkCamera->GetWorldRotation() : Quaternion::IDENTITY;


    // render all particles in group
    for( int iPartCnt = 0; iPartCnt < pkPG->p_count; iPartCnt++ ) {

        Particle &rkParticle = pkPG->list[ iPartCnt ];

        Vector3d kTrans( rkParticle.pos.x, rkParticle.pos.z, rkParticle.pos.y );

        pkPrimitive->m_kModelMatrix.Set( kQuat, m_kOrigin + kTrans );

        // i have still a problem with fasing out the particles; as they would share all the same material thus cannot have induvidual colors.
        //  the particle system must be extended later!
        //Color kColor( rkParticle.color.x, rkParticle.color.y, rkParticle.color.z, 1 );        
        //pkPrimitive->m_pkMaterial->m_kAmbient = kColor;
        //pkPrimitive->m_pkMaterial->m_kDiffuse = kColor;

        Framework::Get()->GetRenderDevice()->Render( *pkPrimitive, 0 );

    }

}

} // namespace CTD_IPluginVisuals
