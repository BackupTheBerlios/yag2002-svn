/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2006, A. Botorabi
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
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  03/01/2005 boto       creation of PlayerPhysics
 #
 ################################################################*/

#ifndef _VRC_PLAYERPHYSICS_H_
#define _VRC_PLAYERPHYSICS_H_

#include <vrc_main.h>

namespace vrc
{

#define ENTITY_NAME_PLPHYS  "PlayerPhysics"

class BasePlayerImplementation;

//! Player physics entity
class EnPlayerPhysics : public yaf3d::BaseEntity
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
        * Update entity, note: player entity has to update the physics!
        */
        void                                        updateEntity( float deltaTime );

        /**
        * This entity needs no transformation node.
        */
        const bool                                  isTransformable() const { return false; }

        /**
        * Initialize the physics translation and rotation. Call this before the first update.
        */
        void                                        initializePhysics( const osg::Vec3f& pos, const osg::Quat& rot );

        /**
        * Set moving direction
        */
        inline void                                 setDirection( float x, float y );

        /**
        * Add moving direction
        */
        inline void                                 addDirection( float x, float y );

        /**
        * Get moving direction ( it is the same as normalize( getVelocity() ) )
        */
        inline const osg::Vec3f&                    getDirection() const;

        /**
        * Get velocity ( dir * speed )
        */
        inline const osg::Vec3f&                    getVelocity() const;

        /**
        * Get movement speed
        */
        inline float                                getSpeed() const;

        /**
        * Get angular speed
        */
        inline float                                getAngularSpeed() const;

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
        bool                                        onGround() const { return !_isOnAir; }

        //! Internal callbacks and methods for static functions
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

        //! Returns the sound timer used for playing various sounds
        float                                       getSoundTimer() const { return _soundTimer; }

        //! Set sound timer
        void                                        setSoundTimer( float value ) { _soundTimer = value; }

        //-------------------------------------------------------------------------------------------------//

    protected:        

        // handle physics building when notified
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

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

        //! Player's mass
        float                                       _mass;

        //! Velocity
        osg::Vec3f                                  _velocity;

        //! Max speed for movement
        float                                       _speed;

        //! Max speed for rotation
        float                                       _angularSpeed;

        //! Player's gravity when walking on ground, default is the Physics system gravity
        float                                       _groundGravity;

        //! Player's gravity when in air, default is the Physics system gravity
        float                                       _airGravity;

        //! Square of delta-movement for recognizing the palyer is moving.
        //! The player is always moving a little also when standing on ground ( in particular on uneven terrains )
        float                                       _moveDetectThreshold2;

        //! Linear damping
        float                                       _linearDamping;

        //! Jump velocity
        float                                       _jumpVelocity;

        //! Period of time applying jump force when jumping, this and '_jumpVelocity' determine the maximal jumping height
        float                                       _jumpPeriod;

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

        //! Internal flag for OnAir detection
        bool                                        _isAirBorne;

        //! Timer for OnAir detection
        float                                        _onAirTimer;

        //! Indicates whether we are on ground or in air
        bool                                        _isOnAir;

        //! Indicates a collision with special material "wall"
        bool                                        _wallCollision;

        //! Sound timer
        float                                       _soundTimer;

        //! Shows if the player is in jumping state
        bool                                        _isJumping;

        //! Used for calculating jump force
        float                                       _jumpTimer;

        //! Internally calculated jump force
        float                                       _jumpForce;

        enum
        {
            Wait4Jumping,
            Wait4Landing
        }                                           _jumpState;

        //! Body matrix
        osg::Matrixf                                _matrix;

        //! Stored last position for detecting player sliding
        osg::Vec3f                                  _lastPosition;

        //! Internal used flag indicating that the player requests for movement ( by setting velocity etc. )
        bool                                        _requestForMovement;
};

inline void EnPlayerPhysics::setDirection( float x, float y )
{
    _velocity._v[ 0 ] = x * _speed;
    _velocity._v[ 1 ] = y * _speed;
    _requestForMovement = true;
}

inline void EnPlayerPhysics::addDirection( float x, float y )
{
    _velocity._v[ 0 ] += x * _speed;
    _velocity._v[ 1 ] += y * _speed;
    _requestForMovement = true;
}

inline const osg::Vec3f& EnPlayerPhysics::getDirection() const
{
    static osg::Vec3f dir;
    dir = _velocity;
    dir.normalize();
    return dir;
}

inline float EnPlayerPhysics::getSpeed() const
{
    return _speed;
}

inline float EnPlayerPhysics::getAngularSpeed() const
{
    return _angularSpeed;
}

inline const osg::Vec3f& EnPlayerPhysics::getVelocity() const
{
    return _velocity;
}

inline void EnPlayerPhysics::stopMovement()
{
    _velocity._v[ 0 ] = 0;
    _velocity._v[ 1 ] = 0;
    _requestForMovement = false;
}

//! Entity type definition used for type registry
class PlayerPhysicsEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    PlayerPhysicsEntityFactory() :
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_PLPHYS, yaf3d::BaseEntityFactory::Standalone | yaf3d::BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~PlayerPhysicsEntityFactory() {}

        Macro_CreateEntity( EnPlayerPhysics );
};


} // namespace vrc

#endif // _VRC_PLAYERPHYSICS_H_
