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

}


WalkPhysics::~WalkPhysics()
{
}

void WalkPhysics::Initialize( SceneNode *pkSceneNode, float fGravity )
{

    m_pkSceneNode = pkSceneNode;
    m_pkBBOX      = static_cast< AABB* >( m_pkSceneNode->GetBoundingVolume() );
    m_pkRoom      = Framework::Get()->GetCurrentLevelSet()->GetRoom();
    m_fGravity    = fGravity;

}

bool WalkPhysics::MoveBody( Vector3d &kPosition, const Vector3d &kMoveVector )
{

 //	static		Vector3d	skForce;
	//static		Vector3d	skDestPos;
	//static		Vector3d	skVelocity;
	//static		Vector3d	skCurrPos;
	//static		float		sfLimDt;
	//static		Contact  	skContSet;

	//// limit dt
	//if ( fDt > 0.1f) {

	//	sfLimDt = 0.1f;
	//
	//} else {
	//
	//	sfLimDt = fDt;

	//}

	//skCurrPos		= GetTranslation();
	//skVelocity		= m_vVelocity + skForce * sfLimDt;
	//skForce.Reset();
	//skDestPos		= skCurrPos + ( skVelocity + kWalkVec ) * sfLimDt;
	//skDestPos.y		-= m_fGroundOffset;
	//m_bLastContact	= m_bHasContact;
	//skContSet.m_vpkContacts.clear();
	//m_bHasContact	= CheckCollision( skDestPos, &skContSet );

	//if ( m_bHasContact == true ) {

	//	unsigned int uiConSize = skContSet.m_vpkContacts.size();
	//	for ( unsigned int uiConCnt = 0; uiConCnt < uiConSize; uiConCnt++ ) {

	//		skForce += skContSet.m_vpkContacts[ uiConCnt ]->m_kNormal * skContSet.m_vpkContacts[ uiConCnt ]->m_pfDepths[ 0 ] * 500.0f;

	//	}

	//	if ( m_bLastContact == true ) {

	//		m_vVelocity.Reset();

	//	}

	//} else {

	//	skForce.y += m_fGravity * m_fMass;	// if on air, add gravity

	//}

	//skDestPos.y += m_fGroundOffset;

 //   return s_pkDestinationPosition = skDestPos;


#ifdef _DEBUG
    // some printfs into screen
    char    strBuff[256];
#endif

    //!FIXME: this should be a parameter of this  method
    float fDeltaTime      = 0.03f;

    //!FIXME: move these variables to class attributes
    float m_fGroundOffset = 0.3f;
    float m_fStepHeight   = 2.5f;
    float m_fObjectHeight = m_pkBBOX->GetRadius();
    static bool m_bLastContact = false;
    static bool m_bHasContact = false;
    static Vector3d m_kLastMoveVector;
    static Vector3d m_kVelocity;

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

    // subtract an offset from y component thus ignoring small unevenesses on ground
    kCurrPosition.y -= m_fGroundOffset;
    
    // move the bbox
    m_pkBBOX->SetTranslation( kCurrPosition );
    
    // ckeck for collisions
    if ( ( m_bHasContact = m_pkRoom->Intersection( m_pkBBOX, &m_kContactSet ) ) ) {

        Vector3d kNormal;
        float    fY = 0;
        float    fDepth = 0.0f;
        vector< Contact* >::iterator    pkContact    = m_kContactSet.m_vpkContacts.begin();
        vector< Contact* >::iterator    pkContactEnd = m_kContactSet.m_vpkContacts.end();//MIN( m_kContactSet.m_vpkContacts.end(), pkContact + 10 );
        while ( pkContact != pkContactEnd ) {

            Contact* pkCurrentContact = ( *pkContact );
            kNormal += pkCurrentContact->m_kNormal;             
            fDepth  += pkCurrentContact->m_pfDepths[ 0 ];
            fY      = MAX( fY, pkCurrentContact->m_pkPoints[ 0 ].y );

#ifdef _DEBUG
    // print out the contact points
    sprintf( strBuff, "contact point %0.3f %0.3f %0.3f, max Y %0.3f", ( *pkContact )->m_pkPoints[ 0 ].x, ( *pkContact )->m_pkPoints[ 0 ].y, ( *pkContact )->m_pkPoints[ 0 ].z, fY );
    CTDPRINTF_PRINT( g_CTDPrintf, strBuff );
#endif

            pkContact++;

        }

        kNormal.Normalize();
        // check for ground collisions
        if ( kNormal.y > 0.8f ) {

            kNormal       *= fDepth;
            kCurrPosition.y += kNormal.y;

        } /*else*/ {

            // automatic height correction ( elevation ) when we get a positive gradient or for step stairs
            if ( ( ( fabs( fY ) > 0 ) && ( ( fY - kCurrPosition.y ) > 0 ) && ( ( fY - kCurrPosition.y ) < m_fStepHeight ) ) ) {

                kCurrPosition.y = fY + m_fObjectHeight;// + m_fGroundOffset;

            } else {

                // collide with walls
                if ( ( fabs( kNormal.x ) > 0.01f ) || ( fabs( kNormal.z ) > 0.01f ) ) {

                    kNormal       *= fDepth;
                    kNormal.y     = 0;
                    kCurrPosition += kNormal;

                }
            }
        }        
    }
    if ( !m_bLastContact ) {

        // add gravity force
        m_kVelocity.y += m_fGravity * fDeltaTime;
        kCurrPosition.y -= m_kVelocity.y;

    } else {
        
        m_kVelocity.y = 0;

    }

    // remove the y offset
    kCurrPosition.y += m_fGroundOffset;
    kPosition       = kCurrPosition;

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
