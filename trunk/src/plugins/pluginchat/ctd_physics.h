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
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  10/28/2004 boto       creation of WalkPhysics
 #
 ################################################################*/

#ifndef _CTD_PHYSICS_H_
#define _CTD_PHYSICS_H_


#include "base.h"


namespace CTD_IPluginChat {


//! Physics for walking
/**
* This class implements simple physics basing on collision detections in order to let a rigid body walk ( it uses scene node's AABB ).
* The walk physics uses some heuristics to accellerate the calculations. It also handles automatic stairway climbing.
*/
class WalkPhysics
{

    public:                     

                                                    WalkPhysics();

                                                    ~WalkPhysics();

        /**
        * Initialize the physics
        * \param pkSceneNode                        Scene node which bbox is used for collision tests
        * \param pkRoom                             The room which contains the scene node
        * \param fMaxStepHeight                     Maximal step height which can be automatically climbed
        * \param fGravity                           Gravity force used for falling
        */
        void                                        Initialize( 
                                                                NeoEngine::SceneNode    *pkSceneNode, 
                                                                NeoEngine::Room         *pkRoom, 
                                                                float                   fMaxStepHeight = 0.5f,
                                                                float                   fGravity       = 0.98f 
                                                               );

        /**
        * Change the room where scene node resides
        * \param pkRoom                             The new room which contains the scene node
        */
        void                                        ChangeRoom( NeoEngine::Room *pkRoom )
                                                    {
                                                        m_pkRoom = pkRoom;
                                                    }

        /**
        * Move the body from position pkCurrentPosition to ( pkCurrentPosition + pkMoveVector ) considering all effects of possible collisions
        * \param kPosition                          Current position of object, after calling this method it contains the new position.
        * \param kMoveVector                        The vector describing where to move the object
        * \param fDeltaTime                         Time past since last update
        * \return                                   true if a collision occured, otherwise false
        */
        bool                                        MoveBody( NeoEngine::Vector3d &kPosition, const NeoEngine::Vector3d &kMoveVector, float fDeltaTime );

        /**
        * Shows whether body is on ground.
        * \return                                   true if body is on ground.
        */
        bool                                        IsOnGround();

    protected:

        //! Room containing the level geometry and objects
        NeoEngine::Room                             *m_pkRoom;

        //! Scene node
        NeoEngine::SceneNode                        *m_pkSceneNode;

        //! Scene node's bounding box
        NeoEngine::AABB                             *m_pkBBOX;

        //! Contact information about collisions
        NeoEngine::ContactSet                       m_kContactSet;

        //! Gravity
        float                                       m_fGravity;

        //! Maximal height of a step which can be automatically climbed
        float                                       m_fStepHeight;

        //! Flag indicating that during last update a collision occured
        bool                                        m_bLastContact;
        
        //! Flag indicating that during current update a collision occured
        bool                                        m_bHasContact;

        //! Last calculated position
        NeoEngine::Vector3d                         m_kLastPosition;

        //! Current velocity
        NeoEngine::Vector3d                         m_kVelocity;

};

} // namespace CTD_IPluginChat

#endif //_CTD_PHYSICS_H_
