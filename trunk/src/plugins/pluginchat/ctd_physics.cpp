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
    m_fGroundOffset = 0.3f;
    m_fStepHeight   = 0.8f;
    m_fObjectHeight = 0.8f;
    m_bLastContact  = false;
    m_bHasContact   = false;

}


WalkPhysics::~WalkPhysics()
{
}

void WalkPhysics::Initialize( SceneNode *pkSceneNode, float fGravity )
{

    m_pkSceneNode   = pkSceneNode;
    m_pkBBOX        = static_cast< AABB* >( m_pkSceneNode->GetBoundingVolume() );
    m_pkRoom        = Framework::Get()->GetCurrentLevelSet()->GetRoom();
    m_fGravity      = fGravity;
    m_fObjectHeight = m_pkBBOX->GetRadius();

}

bool WalkPhysics::MoveBody( Vector3d &kPosition, const Vector3d &kMoveVector )
{

#ifdef _DEBUG
    // some printfs into screen
    char    strBuff[256];
#endif

    //!FIXME: this should be a parameter of this  method
    float fDeltaTime      = 0.03f;

    //------------------------------------------------------------------//

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

    // add ground offset
    kCurrPosition.y -= m_fGroundOffset;
    
    // move the bbox in xz axes
    m_pkBBOX->SetTranslation( kCurrPosition );

    // ckeck for collisions
    if ( m_bHasContact = m_pkRoom->Intersection( m_pkBBOX, &m_kContactSet ) ) {

        Vector3d kNormalXYZ;
        Vector3d kNormalXZ;
        Vector3d kNormalY;
        float    fY = 0;
        float    fDepthY  = 0.0f;
        float    fDepthXZ = 0.0f;
        vector< Contact* >::iterator    pkContact    = m_kContactSet.m_vpkContacts.begin();
        vector< Contact* >::iterator    pkContactEnd = m_kContactSet.m_vpkContacts.end();//MIN( m_kContactSet.m_vpkContacts.end(), pkContact + 10 );
        while ( pkContact != pkContactEnd ) {

            // collect and categorize collision normals
            Contact* pkCurrentContact = ( *pkContact );
            kNormalXYZ = pkCurrentContact->m_kNormal;
            if ( kNormalXYZ.y > 0.8f ) {

                kNormalY += kNormalXYZ;
                fY       = MAX( fY, pkCurrentContact->m_pkPoints[ 0 ].y );
                fDepthY  += pkCurrentContact->m_pfDepths[ 0 ];

            } else {

                if ( fabs( kNormalXYZ.y ) < 0.1f ) { // we don't want to add y-normals of -1 into xz normals!

                    kNormalXZ += kNormalXYZ;
                    fDepthXZ  += pkCurrentContact->m_pfDepths[ 0 ];

                }
            }

#ifdef _DEBUG
    // print out the contact points
    sprintf( strBuff, "contact point %0.3f %0.3f %0.3f, max Y %0.3f", 
        ( *pkContact )->m_pkPoints[ 0 ].x, 
        ( *pkContact )->m_pkPoints[ 0 ].y, 
        ( *pkContact )->m_pkPoints[ 0 ].z, fY );

    CTDPRINTF_PRINT( g_CTDPrintf, strBuff );
#endif

            pkContact++;

        }

        kNormalXZ.Normalize();
        kNormalY.Normalize();

        // adapt y coordinate
        if ( fDepthY > 0.1f && fDepthY < m_fStepHeight ) {

            kNormalY        *= fDepthY;
            kCurrPosition.y += kNormalY.y;
            m_bLastContact   = true;

        } 
        else
        // collide with walls
        if ( ( fabs( kNormalXZ.x ) > 0.8f ) || ( fabs( kNormalXZ.z ) > 0.8f ) ) {

            //kNormalXZ       *= fDepthXZ;
            //kNormalXZ.y     = 0;
            //kCurrPosition   += kNormalXZ;

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

    // remove ground offset
    kCurrPosition.y += m_fGroundOffset;

    // set new position
    kPosition.x    = kCurrPosition.x;
    kPosition.z    = kCurrPosition.z;
    kPosition.y    = ( kPosition.y + kCurrPosition.y ) * 0.5f; // smoothly adapt new height avoiding noise

   
#ifdef _DEBUG
    // print out the count of collision points
    sprintf( strBuff, "count of contacts %d  ", m_kContactSet.m_vpkContacts.size() );
    CTDPRINTF_PRINT( g_CTDPrintf, strBuff );

    sprintf( strBuff, "bbox dimensions %0.3f %0.3f %0.3f", m_pkBBOX->GetRealDim().x, m_pkBBOX->GetRealDim().y, m_pkBBOX->GetRealDim().z );
    CTDPRINTF_PRINT( g_CTDPrintf, strBuff );

#endif

    // clear the contact data
    m_kContactSet.Clear();
 
    return true;

}

} // namespace CTD_IPluginChat
