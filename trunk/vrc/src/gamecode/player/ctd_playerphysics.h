/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2002-2005, Ali Botorabi
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
 # player physics
 #
 # this class implements the player physics
 #
 #
 #   date of creation:  03/01/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  03/01/2005 boto       creation of PlayerPhysics
 #
 ################################################################*/

#ifndef _CTD_PLAYERPHYSICS_H_
#define _CTD_PLAYERPHYSICS_H_

#include <ctd_main.h>

namespace CTD
{

#define ENTITY_NAME_PLPHYS  "PlayerPhysics"

class EnPlayer;

//! Player physics entity
class EnPlayerPhysics : public BaseEntity
{

    public:

                                                    EnPlayerPhysics();

        virtual                                     ~EnPlayerPhysics();

        /**
        * Set player association, player must call this in post-initialize phase
        * \param p_player                           Player instance
        */
        void                                        setPlayer( EnPlayer* p_player );

        /**
        * Destroy physics. This must be called during Player's destruction.
        */
        void                                        destroy();

        /**
        * Initializing function
        */
        void                                        initialize();

        /**
        * Post-Initializing function
        */
        void                                        postInitialize();

        /**
        * Update called by EnPlayer entity. Note: this is not the framework update method!
        * \param deltaTime                          Time passed since last update
        */
        void                                        update( float deltaTime );

        /**
        * Set force in x and y direction.
        */
        void                                        setForce( float x, float y );

        /**
        * Get angular force, it's used by EnPlayer to update rotation about up axis
        */
        float                                       getAngularForce();

        /**
        * Call this method to force the body to jump
        */
        void                                        jump();

        /**
        * Indicates whether we are on ground or in air
        */
        bool                                        onGround() { return !_isAirBorne; }

        //! Internal callbacks
        //-------------------------------------------------------------------------------------------------//
        //! Physics system call-back for body destruction
        static void                                 physicsBodyDestructor( const NewtonBody* p_body );

        //! Physics system call-back for body transformation
        static void                                 physicsSetTransform( const NewtonBody* p_body, const float* p_matrix );

        //! Physics system call-back for applying force to body
        static void                                 physicsApplyForceAndTorque( const NewtonBody* p_body );

        //! Physics system call-back for raycasting
        static float                                physicsRayCastPlacement( const NewtonBody* p_body, const float* p_normal, int collisionID, void* p_userData, float intersectParam );

        //! Handle collisions with level, called in collision callback function
        int                                         collideWithLevel( const NewtonMaterial* p_material, const NewtonContact* p_contact );

        //! Handle collisions with other entities, called in collision callback function
        int                                         collideWithOtherEntities( const NewtonMaterial* p_material, const NewtonContact* p_contact );
 
        //! Return player, used in static mehods
        EnPlayer*                                   getPlayer() { return _p_player; }

        //! Returns true if we are moving
        bool                                        isMoving() { return !_isStopped; }
        //-------------------------------------------------------------------------------------------------//

    protected:

        // find floor under player
        float                                       findFloor( NewtonWorld* world, const osg::Vec3f& p0, float maxDist );

        // entity attributes
        //--------------------------------------------------------//

        //! Dimentions of player: width / length / height
        osg::Vec3f                                  _dimensions;

        //! Step height
        float                                       _stepHeight;

        //! Max force for movement
        float                                       _linearForce;

        //! Max force for rotation
        float                                       _angularForce;

        //! Player's mass
        float                                       _mass;

        //! Force for movement
        osg::Vec3f                                  _force;

        //! Player's gravity, default is the Physics system gravity
        float                                       _gravity;

        //! Linear damping
        float                                       _linearDamping;

        //--------------------------------------------------------//

        EnPlayer*                                   _p_player;

        NewtonWorld*                                _p_world;

        NewtonBody*                                 _p_body;

        //! Contraint joint to keep the body up
        NewtonJoint*                                _upVectorJoint;

        //! Player height
        float                                       _playerHeight;

        //! Shows that there is no force for movement
        bool                                        _isStopped;

        //! Indicates whether we are on ground or in air
        bool                                        _isAirBorne;

        //! Climb contact
        osg::Vec3f                                  _climbContact;

        float                                       _climbForce;

        unsigned int                                _jumpTimer;

        float                                       _jumpForce;

        //! Body matrix
        osg::Matrixf                                _matrix;

};

inline void EnPlayerPhysics::setForce( float x, float y )
{
    _force._v[ 0 ] = x * _linearForce;
    _force._v[ 1 ] = y * _linearForce;
}

inline float EnPlayerPhysics::getAngularForce()
{
    return _angularForce;
}

//! Entity type definition used for type registry
class PlayerPhysicsEntityFactory : public BaseEntityFactory
{
    public:
                                                    PlayerPhysicsEntityFactory() : BaseEntityFactory(ENTITY_NAME_PLPHYS) {}

        virtual                                     ~PlayerPhysicsEntityFactory() {}

        Macro_CreateEntity( EnPlayerPhysics );
};


} // namespace CTD

#endif // _CTD_PLAYERPHYSICS_H_
