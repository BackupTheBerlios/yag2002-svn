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
*/
class WalkPhysics
{

    public:                     

                                                    WalkPhysics();

                                                    ~WalkPhysics();

        //! Initialize the physics
        void                                        Initialize( NeoEngine::SceneNode *pkSceneNode, float fGravity = 0.98f );

        /**
        * Move the body from position pkCurrentPosition to ( pkCurrentPosition + pkMoveVector ) considering all effects of possible collisions
        * \param kPosition                          Current position of object, after calling this method it contains the new position.
        * \param kMoveVector                        The vector describing where to move the object
        * \return                                   true if a collision occured, otherwise false
        */
        bool                                        MoveBody( NeoEngine::Vector3d &kPosition, const NeoEngine::Vector3d &kMoveVector );

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


};

} // namespace CTD_IPluginChat

#endif //_CTD_PHYSICS_H_
