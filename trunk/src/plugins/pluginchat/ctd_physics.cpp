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
 # physics for moving the body
 #
 #
 #   date of creation:  10/28/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include "base.h"
#include "ctd_physics.h"
#include <ctd_printf.h>

using namespace std;
using namespace CTD;
using namespace NeoEngine;

namespace CTD_IPluginChat {

// this is the global instance ctdprintf object
extern CTDPrintf           g_CTDPrintf;


WalkPhysics::WalkPhysics()
{

    m_pkSceneNode   = NULL;
    m_pkBBOX        = NULL;
    m_pkRoom        = NULL;
    m_fGravity      = 0.98f;
    m_fStepHeight   = 0.5f;
    m_bLastContact  = false;
    m_bHasContact   = false;

}


WalkPhysics::~WalkPhysics()
{
}

void WalkPhysics::Initialize( SceneNode *pkSceneNode, Room *pkRoom, float fMaxStepHeight, float fGravity )
{

    m_pkSceneNode   = pkSceneNode;
    m_pkBBOX        = static_cast< AABB* >( m_pkSceneNode->GetBoundingVolume() );
    m_pkRoom        = pkRoom;
    m_fStepHeight   = fMaxStepHeight;
    m_fGravity      = fGravity;

}

bool WalkPhysics::MoveBody( Vector3d &kPosition, const Vector3d &kMoveVector, float fDeltaTime )
{

    Vector3d kCurrPosition = kPosition;
 
    // during falling avoid moving in x and z directions
    if ( m_bLastContact &&  m_bHasContact ) {

        kCurrPosition += Vector3d( kMoveVector.x, 0, kMoveVector.z );
    
    } else {

        if ( !m_bLastContact &&  m_bHasContact ) {
        
            m_kLastMoveVector = kMoveVector;
            kCurrPosition += Vector3d( m_kLastMoveVector.x, 0, m_kLastMoveVector.z );

        } else {
        
            if ( !m_bLastContact &&  !m_bHasContact ) {

                kCurrPosition += Vector3d( m_kLastMoveVector.x, 0, m_kLastMoveVector.z );

            }
        }
    }

    // store the last contact flag
    m_bLastContact = m_bHasContact;

    // move the bbox in xz axes
    m_pkBBOX->SetTranslation( kCurrPosition );

    // ckeck for collisions
    if ( m_bHasContact = m_pkRoom->Intersection( m_pkBBOX, &m_kContactSet ) ) {

        Vector3d kNormalXYZ;
        Vector3d kNormalXZ;
        float    fNormalY           = 0;
        float    fHighestCollision  = 0;
        float    fCountY            = 0;

        vector< Contact* >::iterator    pkContact    = m_kContactSet.m_vpkContacts.begin();
        vector< Contact* >::iterator    pkContactEnd = MIN( m_kContactSet.m_vpkContacts.end(), pkContact + 50 ); // limit count of considered contacts
        while ( pkContact != pkContactEnd ) {

            Contact*& pkCurrentContact = ( *pkContact );
            kNormalXYZ   = pkCurrentContact->m_kNormal;
            float fDepth = pkCurrentContact->m_pfDepths[ 0 ];

            // collect and categorize collision normals
            //-----------------------------------------

            if ( kNormalXYZ.y > 0.8f ) {

                fNormalY            += ( kNormalXYZ.y * fDepth );
                fHighestCollision   = MAX( fHighestCollision, fDepth );
                fCountY             += 1.0f;

            } else {

                // try to proof some plausibility in order to detect wall corners ( they make some headache, use some heuristics )

                // don't consider backfacing polygons
                Vector3d kDir = m_kLastPosition - ( kPosition + kMoveVector ); 
                bool bBackFacing = ( kDir * kNormalXYZ ) < 0;
                if  ( bBackFacing ) {

                    pkContact++;
                    continue;

                }

                float fPlausiOffset2 = kMoveVector.Len2();
                float fDepth2        = fDepth * fDepth;

                // if we stand and just rotate in the near of a wall then consider the bbox radius for plausi chek
                if (  fPlausiOffset2 < 0.0001f ) {

                    fPlausiOffset2 = ( m_pkBBOX->GetRealDim().x + m_pkBBOX->GetRealDim().z ) * 0.5f;
                    fPlausiOffset2 *= fPlausiOffset2;

                }

                // check the contact depth against out plausibility offset
                if ( fDepth2 < fPlausiOffset2 ) { 
                
                    kNormalXZ += ( kNormalXYZ * fDepth );
                
                }
            }

            pkContact++;

        }

        // average Y normals for a smooth height adaptation
        if ( fCountY > 0 ) {
            fNormalY *= ( 1.0f / fCountY );
        }

        // adapt y coordinate
        if ( fHighestCollision < m_fStepHeight ) {

            kCurrPosition.y += fNormalY;

        } else  {

            // collide with walls
            kNormalXZ.y     = 0;
            kCurrPosition   += kNormalXZ;

        }

    } 

    // check for falling
    if ( !m_bLastContact ) {

        // add gravity force
        m_kVelocity.y -= m_fGravity * fDeltaTime;

    } else {
        
        m_kVelocity.y = 0;

    }

    // apply velocity
    kCurrPosition  += m_kVelocity;

    // store last position for later use in next step
    m_kLastPosition = kPosition;

    // set new position
    kPosition       = ( kPosition + kCurrPosition ) * 0.5f; // smoothly adapt new position avoiding noise
   
#ifdef _DEBUG

    char    strBuff[256];
    // print out the count of collision points
    sprintf( strBuff, "count of contacts %d  ", m_kContactSet.m_vpkContacts.size() );
    CTDPRINTF_PRINT( g_CTDPrintf, strBuff );

#endif

    // clear the contact data
    m_kContactSet.Clear();
 
    return true;

}

} // namespace CTD_IPluginChat
