/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2007, A. Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU Lesser General Public 
 *  License version 2.1 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public 
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

class BasePlayerImplementation;

//! Player physics entity
class EnPlayerPhysics : public BaseEntity
{

    public:

                                                    EnPlayerPhysics();

        virtual                                     ~EnPlayerPhysics();

        /**
        * Set player's implementation , player entity must call this in post-initialize phase.
        * \param p_player                           Player instance
        */
        void                                        setPlayer( BasePlayerImplementation* p_playerimpl );

        /**
        * Initializing function
        */
        void                                        initialize();

        /**
        * Post-Initializing function
        */
        void                                        postInitialize();

        /**
        * This entity needs no transformation node.
        */
        const bool                                  isTransformable() const { return false; }

        /**
        * Set force in x and y direction.
        */
        void                                        setForce( float x, float y );

        /**
        * Add force in x and y direction.
        */
        void                                        addForce( float x, float y );

        /**
        * Set the boy transformation.
        */
        void                                        setTransformation( const osg::Matrixf& mat );

        /**
        * Stop movement
        */
        void                                        stopMovement();

        /**
        * Get angular force, it's used by EnPlayer to update rotation about up axis
        */
        float                                       getAngularForce() const;

        /**
        * Call this method to force the body to jump
        */
        void                                        jump();

        /**
        * Returns true when the player is in jumping state.
        */
        bool                                        isJumping() const { return _isJumping; }

        /**
        * Indicates whether we are on ground or in air
        */
        bool                                        onGround() const { return !_isAirBorne; }

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
        BasePlayerImplementation*                   getPlayer() { return _p_playerImpl; }

        //! Returns true if we are moving
        bool                                        isMoving() const { return !_isStopped; }
        //-------------------------------------------------------------------------------------------------//

    protected:

        /**
        * Update entity
        * \param deltaTime                          Time passed since last update
        */
        void                                        updateEntity( float deltaTime );

        // handle physics building when notified
        void                                        handleNotification( const EntityNotification& notification );

        // init physics materials
        void                                        initializePhysicsMaterials();

        // find ground under player
        float                                       findGround( NewtonWorld* world, const osg::Vec3f& p0, float maxDist );

        //! The materials must be created only once for all physics entities of same types on every level loading
        static bool                                 _materialsCreated;

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

        BasePlayerImplementation*                   _p_playerImpl;

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

        bool                                        _isJumping;

        enum
        {
            BeginJumping,
            Wait4Landing
        }                                           _jumpState;

        float                                       _jumpForce;

        //! Body matrix
        osg::Matrixf                                _matrix;

};

inline void EnPlayerPhysics::setForce( float x, float y )
{
    _force._v[ 0 ] = x;
    _force._v[ 1 ] = y;
}

inline void EnPlayerPhysics::addForce( float x, float y )
{
    _force._v[ 0 ] += x;
    _force._v[ 1 ] += y;
}

inline void EnPlayerPhysics::stopMovement()
{
    // factor 1.5 is determined by fine-tuning
    osg::Vec3f stopforce( -_force * ( 1.5f / _linearForce ) ); 
    stopforce._v[ 2 ] = 0;
    osg::Vec3f pos( _matrix.getTrans() );
    NewtonAddBodyImpulse( _p_body, &stopforce._v[ 0 ], &pos._v[ 0 ] );
    _force._v[ 0 ] = 0;
    _force._v[ 1 ] = 0;
}

inline float EnPlayerPhysics::getAngularForce() const
{
    return _angularForce;
}

//! Entity type definition used for type registry
class PlayerPhysicsEntityFactory : public BaseEntityFactory
{
    public:
                                                    PlayerPhysicsEntityFactory() :
                                                     BaseEntityFactory( ENTITY_NAME_PLPHYS, BaseEntityFactory::Standalone | BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~PlayerPhysicsEntityFactory() {}

        Macro_CreateEntity( EnPlayerPhysics );
};


} // namespace CTD

#endif // _CTD_PLAYERPHYSICS_H_
