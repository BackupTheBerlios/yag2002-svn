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
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  09/21/2004 boto       creation of InterpolatorBase and LinearInterpolator
 #
 ################################################################*/

#ifndef _CTD_INTERPOLATOR_H_
#define _CTD_INTERPOLATOR_H_


#include "base.h"


namespace CTD_IPluginChat {


//! Base class for all kinds of interpolators 
/**
* This is the base class for interpolators for X, Y, Z coordinates representing position or euler angles.
* Actually the most implementations also extrapolate!
*/
class InterpolatorBase
{

    public:                     

                                                    InterpolatorBase() { 
                                                                            m_fPassedTime               = 0.0f; 
                                                                            m_fMaxTime                  = 1.0f;
                                                                            m_fArrivalTime              = 0;
                                                                            m_bDestinatedInterpolation  = false; 
                                                                        }

        virtual                                     ~InterpolatorBase() {}

        //! Initialize the interpolator
        virtual                                     Initialize( 
                                                                const NeoEngine::Vector3d &kPoint,
                                                                const NeoEngine::Vector3d &kVelocity = NeoEngine::Vector3d( 0, 0, 0 ),
                                                                float fMaxTime = 1.0f
                                                               ) 
                                                    {
                                                        m_kCurrent                  = kPoint;
                                                        m_kLast                     = kPoint;
                                                        m_kDestination              = kPoint;
                                                        m_kVelocity                 = kVelocity;
                                                        m_fPassedTime               = 0;
                                                        m_fMaxTime                  = fMaxTime;
                                                    }



        //! Reset the interpolator. For position interpolation kInit is the initial position and for rotation it is the initial euler angles.
        virtual void                                Reset( const NeoEngine::Vector3d &kInit ) = 0;

        //! Add a new incoming update information for a maximal interpolation time of fMaxTime
        virtual void                                AddInterpolationPoint(  
                                                                            const NeoEngine::Vector3d &kNew, 
                                                                            const NeoEngine::Vector3d &kVelocity, 
                                                                            float fMaxTime = 1.0f 
                                                                          ) = 0;

        //! Add a destination for interpolation with given speed
        virtual void                                AddInterpolationDestination( const NeoEngine::Vector3d &kDestination, float fSpeed = 1.0f ) = 0; 

        //! Update the interpolation. 
        //!   This method accumulates the time. Call it in every entity update phase.
        virtual NeoEngine::Vector3d&                UpdateInterpolation( float fDeltaTime ) = 0;

        //! Get current interpolated point
        NeoEngine::Vector3d&                        GetCurrentPoint() { return m_kCurrent; }

    protected:

        //! Current point
        NeoEngine::Vector3d                         m_kCurrent;

        //! Last point got at upating
        NeoEngine::Vector3d                         m_kLast;

        //! Velocity
        NeoEngine::Vector3d                         m_kVelocity;

        //! Destination to interpolate to
        NeoEngine::Vector3d                         m_kDestination;

        //! Passed time since last update
        float                                       m_fPassedTime;

        //! Maximal interpolation time
        float                                       m_fMaxTime;

        //! Arrival time for destination interpolation
        float                                       m_fArrivalTime;

        //! Is set when an interpolation with a given destination point is to achieve
        bool                                        m_bDestinatedInterpolation;

};

//! Linear interpolator for non-accellerated movement
class LinearInterpolator: public InterpolatorBase
{

    public:

                                                    LinearInterpolator();

                                                    ~LinearInterpolator();

        void                                        Reset( const NeoEngine::Vector3d &kInit );

        void                                        AddInterpolationPoint(  
                                                                            const NeoEngine::Vector3d &kNew, 
                                                                            const NeoEngine::Vector3d &kVelocity, 
                                                                            float fMaxTime = 1.0f 
                                                                          );

        void                                        AddInterpolationDestination( const NeoEngine::Vector3d &kDestination, float fSpeed = 1.0f ); 

        NeoEngine::Vector3d&                        UpdateInterpolation( float fDeltaTime );


};

} // namespace CTD_IPluginChat

#endif //_CTD_INTERPOLATOR_H_
