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
 # interpolators for smoothing position and rotation updates
 #
 # this class implements a chat member
 #
 #
 #   date of creation:  09/21/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include "base.h"
#include "ctd_interpolator.h"

using namespace std;
using namespace CTD;
using namespace NeoEngine;

namespace CTD_IPluginChat {

                                                        
LinearInterpolator::LinearInterpolator()
{
}


LinearInterpolator::~LinearInterpolator()
{
}

void LinearInterpolator::Reset( const Vector3d &kInit )
{

    m_kLast                     = kInit;
    m_kCurrent                  = kInit;
    m_fPassedTime               = 0;
    m_fMaxTime                  = 1.0f;
    m_fArrivalTime              = 0;
    m_bDestinatedInterpolation  = false;
    m_kMoveDir.Reset();
    m_kDestination.Reset();

}

void LinearInterpolator::AddInterpolationPoint( const Vector3d &kNew, const Vector3d &kMoveDir, float fMaxTime )
{

    m_kLast                     = m_kCurrent;
    m_kMoveDir                  = ( ( kMoveDir * 2.0f ) + ( ( kNew - m_kCurrent ) * 3.0f ) ) * 0.5f; // try to compensate the deviation via move direction
    m_fPassedTime               = 0.0f;
    m_fMaxTime                  = fMaxTime;
    m_bDestinatedInterpolation  = false;

}

void LinearInterpolator::AddInterpolationDestination( const NeoEngine::Vector3d &kDestination, float fSpeed )
{

    m_bDestinatedInterpolation  = true;
    m_kDestination              = kDestination;
    m_fPassedTime               = 0;
    m_kMoveDir                  = ( kDestination - m_kCurrent ) * fSpeed;
    m_fArrivalTime              = 1.0f / fSpeed;
    m_kLast                     = m_kCurrent;

}

Vector3d& LinearInterpolator::UpdateInterpolation( float fDeltaTime )
{

    // in destination interpolation we proof the passed time ( it serves as blend factor )
    if ( m_bDestinatedInterpolation == true ) {

        if ( m_fPassedTime > m_fArrivalTime ) {

            // returning the destination causes abrupt moves! 
            // as long the deviation to current point is not as high the current position is ok
            //return m_kDestination;

            return m_kCurrent;

        }

    // in timed interpolation we proof the maximal interpolation time
    } else {

        if ( m_fPassedTime > m_fMaxTime ) {

            //!TODO: figure out a good strategy for extrapolating

        }

    }

    m_fPassedTime += fDeltaTime;
    m_kCurrent = m_kLast + m_kMoveDir * m_fPassedTime;

    return m_kCurrent;

}


} // namespace CTD_IPluginChat
