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
 #   author:            ali botorabi (boto)
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_playerphysics.h"
#include "vrc_playersound.h"
#include "vrc_playerimpl.h"

namespace vrc
{

// used to avoid playing too short sound
#define SND_PLAY_RELAX_TIME     0.2f

//! Implement and register the player physics entity factory
YAF3D_IMPL_ENTITYFACTORY( PlayerPhysicsEntityFactory )

bool EnPlayerPhysics::_materialsCreated = false;

//! Data structure used in raycasting
struct PlayerRayCastData
{
                            PlayerRayCastData( NewtonBody* p_me ) :
                             _p_body( p_me ),
                             _parameter( 1.2f )
                            {
                            }

    NewtonBody*             _p_body;

    float                   _parameter;
};

struct  PlayerCollisionStruct
{
    NewtonBody*             _p_body0;

    NewtonBody*             _p_body1;

    osg::Vec3f              _position;

    float                   _contactMaxNormalSpeed;

    float                   _contactMaxTangentSpeed;

    yaf3d::BaseEntity*             _p_otherEntity;

    EnPlayerPhysics*        _p_physics;
};

// player to other materials collision structs
PlayerCollisionStruct player_playerCollStruct;
PlayerCollisionStruct player_levelCollStruct;
PlayerCollisionStruct player_woodCollStruct;
PlayerCollisionStruct player_metalCollStruct;
PlayerCollisionStruct player_grassCollStruct;
PlayerCollisionStruct player_stoneCollStruct;

static PlayerCollisionStruct* s_colStruct;

// begin contact processing
int playerContactBegin( const NewtonMaterial* p_material, const NewtonBody* p_body0, const NewtonBody* p_body1 )
{
    // get the pointer to collision struture
    s_colStruct = static_cast< PlayerCollisionStruct* >( NewtonMaterialGetMaterialPairUserData( p_material ) );
    // save the colliding bodies
    s_colStruct->_p_body0 = const_cast< NewtonBody* >( p_body0 );
    s_colStruct->_p_body1 = const_cast< NewtonBody* >( p_body1 );
    // clear the contact normal speed
    s_colStruct->_contactMaxNormalSpeed  = 0.0f;
    // clear the contact sliding speed
    s_colStruct->_contactMaxTangentSpeed = 0.0f;

    return 1;
}

// physics system callback functions
//---------------------------------------
// player contact process callback function called when the player collides with level
int playerContactProcessLevel( const NewtonMaterial* p_material, const NewtonContact* p_contact )
{
    // determine which body is the player physics's one
    NewtonBody*      p_body     = s_colStruct->_p_body1;
    EnPlayerPhysics* p_phys     = NULL;
    void*            p_userData = NewtonBodyGetUserData( p_body );
    if ( p_userData ) // level geoms have no data associated
    {
        p_phys = static_cast< EnPlayerPhysics* >( p_userData );
    }
    else // this else is only for being on safe side. in normal case all level bodies come in as body0!
    {
        p_phys = static_cast< EnPlayerPhysics* >( NewtonBodyGetUserData( s_colStruct->_p_body0 ) );
       // assert( NULL && "body0 should be static geom!" ); this case comes when we move the character out of world bbox
    }
    s_colStruct->_p_otherEntity = NULL;
    s_colStruct->_p_physics     = p_phys;

    // play appropriate sound only if we are moving
    EnPlayerSound* p_playerSound = p_phys->getPlayer()->getPlayerSound();
    if ( p_playerSound )
    {
        if ( p_phys->getSoundTimer() <= 0.0f )
        {
            // reset sound timer
            p_phys->setSoundTimer( SND_PLAY_RELAX_TIME );

            if ( p_phys->isMoving() )
            {
                unsigned int materialType = static_cast< unsigned int >( NewtonMaterialGetContactFaceAttribute( p_material ) );
                switch ( materialType )
                {
                case yaf3d::Physics::MAT_DEFAULT:
                    p_playerSound->playWalkGround();
                    break;

                case yaf3d::Physics::MAT_WOOD:
                    p_playerSound->playWalkWood();
                    break;

                case yaf3d::Physics::MAT_METALL:
                    p_playerSound->playWalkMetal();
                    break;

                case yaf3d::Physics::MAT_STONE:
                    p_playerSound->playWalkStone();
                    break;

                case yaf3d::Physics::MAT_GRASS:
                    p_playerSound->playWalkGrass();
                    break;

                default:
                    ;
                }
            }
            else
            {
                p_playerSound->stopPlayingAll();
            }
        }
    }

    return p_phys->collideWithLevel( p_material, p_contact );
}

// player contact process callback function called when the player collides with other materials in world than level
int playerContactProcess( const NewtonMaterial* p_material, const NewtonContact* p_contact )
{
    // determine which body is the player physics's one
    NewtonBody*    p_body   = s_colStruct->_p_body0;
    yaf3d::BaseEntity*    p_entity = reinterpret_cast< yaf3d::BaseEntity* >( NewtonBodyGetUserData( p_body ) );
    EnPlayerPhysics* p_phys = dynamic_cast< EnPlayerPhysics* >( p_entity );
    if ( !p_phys )
    {
        s_colStruct->_p_otherEntity = p_entity;
        p_body = s_colStruct->_p_body1;
        p_phys = static_cast< EnPlayerPhysics* >( NewtonBodyGetUserData( p_body ) );
    }
    else
    {
        s_colStruct->_p_otherEntity = reinterpret_cast< yaf3d::BaseEntity* >( NewtonBodyGetUserData( s_colStruct->_p_body1 ) );
    }

    s_colStruct->_p_physics = p_phys;
    return p_phys->collideWithOtherEntities( p_material, p_contact );
}

// this function is called affter all collision contacts are processed
void playerContactEnd( const NewtonMaterial* /*p_material*/ )
{
}

// find ground for character placement
float EnPlayerPhysics::findGround( NewtonWorld* /*p_world*/, const osg::Vec3f& p0, float maxDist )
{
    PlayerRayCastData data( _p_body );

    // shot a vertical ray from a high altitude and collect the intersetion partameter.
    osg::Vec3f p1( p0 );
    p1._v[ 2 ] -= maxDist;

    NewtonWorldRayCast( _p_world, &p0._v[ 0 ], &p1._v[ 0 ], physicsRayCastPlacement, &data, NULL );

    // the intesection is the interpolater value
    return p0._v[ 2 ] - maxDist * data._parameter;
}

// rigid body destructor callback
void EnPlayerPhysics::physicsBodyDestructor( const NewtonBody* /*p_body*/ )
{
}

// transformation callback
void EnPlayerPhysics::physicsSetTransform( const NewtonBody* p_body, const float* p_matrix )
{
    BasePlayerImplementation* p_node = static_cast< EnPlayerPhysics* >( NewtonBodyGetUserData( p_body ) )->_p_playerImpl;
    osg::Matrixf& mat = p_node->getPlayerPhysics()->_matrix;
    mat = osg::Matrixf( p_matrix );
    osg::Quat quat;
    mat.get( quat );

    osg::Vec3f trans = mat.getTrans();
    // set player's position and rotation
    p_node->setPlayerRotation( quat );
    p_node->setPlayerPosition( trans );
}

int EnPlayerPhysics::collideWithLevel( const NewtonMaterial* p_material, const NewtonContact* /*p_contact*/ )
{
    osg::Vec3f point;
    osg::Vec3f normal;
    osg::Vec3f velocity;

    // when we collide with something then set air-borne flag to false
    _isAirBorne = false;

    unsigned int materialType = static_cast< unsigned int >( NewtonMaterialGetContactFaceAttribute( p_material ) );
    if ( materialType == yaf3d::Physics::MAT_WALL )
        _wallCollision = true;

    // avoid sliding down on slopes ( only when player does not wish to move )
    if ( _requestForMovement )
    {
        NewtonMaterialSetContactFrictionState( p_material, 0, 0 );
        NewtonMaterialSetContactFrictionState( p_material, 0, 1 );
    }
    else
    {
        NewtonMaterialSetContactStaticFrictionCoef( p_material, 2.0f, 0 );
        NewtonMaterialSetContactKineticFrictionCoef( p_material, 2.0f, 0 );
        NewtonMaterialSetContactStaticFrictionCoef( p_material, 2.0f, 1 );
        NewtonMaterialSetContactKineticFrictionCoef( p_material, 2.0f, 1 );
    }

    return 1;
}

int EnPlayerPhysics::collideWithOtherEntities( const NewtonMaterial* /*p_material*/, const NewtonContact* /*p_contact*/ )
{
    return 1;
}

void EnPlayerPhysics::physicsApplyForceAndTorque( const NewtonBody* p_body )
{
    float mass;
    float Ixx;
    float Iyy;
    float Izz;

    NewtonBodyGetMassMatrix( p_body, &mass, &Ixx, &Iyy, &Izz );
    EnPlayerPhysics* p_phys = static_cast< EnPlayerPhysics* >( NewtonBodyGetUserData( p_body ) );

    float timestep;
    float timestepInv;
    // get the current timestep
    timestep = p_phys->_deltaTime;
    timestepInv = 1.0f / timestep;

    // calculate force basing on desired velocity
    osg::Vec3f velocity;
    const osg::Vec3f& desiredvelocity = p_phys->getVelocity();
    NewtonBodyGetVelocity( p_phys->_p_body, &velocity._v[ 0 ] );
    osg::Vec3f delta = desiredvelocity - velocity;
    osg::Vec3f force = delta * ( timestepInv * mass );

    osg::Matrixf matrix;
    NewtonBodyGetMatrix( p_phys->_p_body, matrix.ptr() );
    osg::Vec3f pos( matrix.getTrans() );
    // set the stopped flag when no movement
    p_phys->_isStopped = ( p_phys->_lastPosition - pos ).length2() < p_phys->_moveDetectThreshold2;
    p_phys->_lastPosition = pos;

    //const float* matelems = matrix.ptr();
    const float* matelems = matrix.ptr();
    osg::Vec3f front( matelems[ 4 ] , matelems[ 5 ], matelems[ 6 ] );
    // move direction must be normalized
    float steerAngle;
    osg::Vec3f cross( front ^ p_phys->_p_playerImpl->getPlayerMoveDirection() );
    steerAngle = std::min( std::max( cross._v[ 2 ], -1.0f ), 1.0f );
    steerAngle = asinf( steerAngle );
    osg::Vec3f omega;
    NewtonBodyGetOmega( p_phys->_p_body, &omega._v[ 0 ] );
    osg::Vec3f torque( 0.0f, 0.0f, 0.5f * Izz * ( steerAngle * timestepInv - omega._v[ 2 ] ) * timestepInv );
    NewtonBodySetTorque( p_phys->_p_body, &torque._v[ 0 ] );

    // add gravity to force, when on ground then reduce the applied gravity
    force._v[ 2 ] = mass * ( p_phys->_isAirBorne ? p_phys->_airGravity : p_phys->_groundGravity );

    switch ( p_phys->_jumpState )
    {
        case Wait4Jumping:
        {
            if ( p_phys->_isJumping && !p_phys->_wallCollision )
            {
                osg::Vec3f veloc( 0.0f, 0.0f, p_phys->_jumpForce );
                NewtonAddBodyImpulse( p_phys->_p_body, &veloc._v [ 0 ], &pos._v[ 0 ] );
                p_phys->_jumpState = Wait4Landing;
            }
        }
        break;

        case Wait4Landing:
            {
                if ( !p_phys->_isAirBorne )
                {
                    p_phys->_jumpState = Wait4Jumping;
                    p_phys->_isJumping = false;
                }
            }
            break;

        default:
            assert( NULL && "invalid jump state" );
    }

    // set body force
    NewtonBodySetForce( p_body, &force._v[ 0 ] );
}

float EnPlayerPhysics::physicsRayCastPlacement( const NewtonBody* p_body, const float* /*p_normal*/, int /*collisionID*/, void* p_userData, float intersectParam )
{
    float paramPtr = 1.2f;
    PlayerRayCastData& data = *( static_cast< PlayerRayCastData* >( p_userData ) );

    // any body can be a ground
    if ( data._p_body != p_body )
    {
        if ( intersectParam < data._parameter ) {
            data._parameter = intersectParam;
            paramPtr = intersectParam;
        }
    }
    // else continue the search
    return paramPtr;
}
//---------

// implementation of player physics
//---------------------------------
EnPlayerPhysics::EnPlayerPhysics() :
_dimensions( osg::Vec3f( 0.5f, 0.5f, 1.8f ) ),
_mass( 50.0f ),
_speed( 10.0f ),
_angularSpeed( 90.0f ),
_groundGravity( yaf3d::Physics::get()->getWorldGravity() ),
_airGravity( yaf3d::Physics::get()->getWorldGravity() ),
_moveDetectThreshold2( 0.05f ),
_linearDamping( 0.2f ),
_p_playerImpl( NULL ),
_p_world( NULL ),
_p_body( NULL ),
_upVectorJoint( NULL ),
_playerHeight( 1.8f ),
_isStopped( true ),
_isAirBorne( false ),
_wallCollision( false ),
_soundTimer( 0.0f ),
_isJumping( false ),
_jumpState( Wait4Jumping ),
_jumpForce( 5.0f ),
_deltaTime( 0.001f ),
_requestForMovement( false )
{
    // register entity in order to get notifications about physics building
    yaf3d::EntityManager::get()->registerNotification( this, true );

    // add entity attributes
    getAttributeManager().addAttribute( "dimensions"           , _dimensions           );
    getAttributeManager().addAttribute( "speed"                , _speed                );
    getAttributeManager().addAttribute( "angularSpeed"         , _angularSpeed         );
    getAttributeManager().addAttribute( "jumpforce"            , _jumpForce            );
    getAttributeManager().addAttribute( "lineardamping"        , _linearDamping        );
    getAttributeManager().addAttribute( "mass"                 , _mass                 );
    getAttributeManager().addAttribute( "groundGravity"        , _groundGravity        );
    getAttributeManager().addAttribute( "airGravity"           , _airGravity           );
    getAttributeManager().addAttribute( "moveDetectThreshold2" , _moveDetectThreshold2 );
}

EnPlayerPhysics::~EnPlayerPhysics()
{
    if ( _p_body )
        NewtonDestroyBody( yaf3d::Physics::get()->getWorld(), _p_body );
}

void EnPlayerPhysics::handleNotification( const yaf3d::EntityNotification& notification )
{
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_BUILDING_PHYSICSWORLD:

            // create the physics materials only once for every entity type on every level loading
            if ( !_materialsCreated )
            {
                initializePhysicsMaterials();
                _materialsCreated = true;
            }
            break;

        case YAF3D_NOTIFY_DELETING_PHYSICSWORLD:

            _materialsCreated = false;
            break;

        default:
            ;

    }
}

void EnPlayerPhysics::initializePhysicsMaterials()
{
    _p_world = yaf3d::Physics::get()->getWorld();

    // setup materials
    int playerID   = yaf3d::Physics::get()->createMaterialID( "player" );
    int defaultID  = yaf3d::Physics::get()->getMaterialId( "default" );
    int wallID     = yaf3d::Physics::get()->getMaterialId( "wall"    );
    int levelID    = yaf3d::Physics::get()->getMaterialId( "level"   );
    int woodID     = yaf3d::Physics::get()->getMaterialId( "wood"    );
    int metalID    = yaf3d::Physics::get()->getMaterialId( "metal"   );
    int grassID    = yaf3d::Physics::get()->getMaterialId( "grass"   );
    int stoneID    = yaf3d::Physics::get()->getMaterialId( "stone"   );

    // set non-colliding for player-nocol collisions
    NewtonMaterialSetDefaultCollidable( _p_world, yaf3d::Physics::get()->getMaterialId( "nocol" ), playerID, 0 );

    // set the material properties for player on default
    NewtonMaterialSetDefaultElasticity( _p_world, playerID, defaultID, 0.1f );
    NewtonMaterialSetDefaultSoftness( _p_world, playerID, defaultID, 0.5f );
    NewtonMaterialSetDefaultFriction( _p_world, playerID, defaultID, 0.8f, 0.7f );
    NewtonMaterialSetCollisionCallback( _p_world, playerID, defaultID, &player_levelCollStruct, playerContactBegin, playerContactProcessLevel, playerContactEnd );

    // set the material properties for player on wall
    NewtonMaterialSetDefaultElasticity( _p_world, playerID, wallID, 0.01f );
    NewtonMaterialSetDefaultSoftness( _p_world, playerID, wallID, 0.01f );
    NewtonMaterialSetDefaultFriction( _p_world, playerID, wallID, 0.01f, 0.01f );
    NewtonMaterialSetCollisionCallback( _p_world, playerID, wallID, &player_levelCollStruct, playerContactBegin, playerContactProcessLevel, playerContactEnd );

    // set the material properties for player on level
    NewtonMaterialSetDefaultElasticity( _p_world, playerID, levelID, 0.1f );
    NewtonMaterialSetDefaultSoftness( _p_world, playerID, levelID, 0.8f );
    NewtonMaterialSetDefaultFriction( _p_world, playerID, levelID, 0.8f, 0.7f );
    NewtonMaterialSetCollisionCallback( _p_world, playerID, levelID, &player_levelCollStruct, playerContactBegin, playerContactProcessLevel, playerContactEnd );

    // set the material properties for player on wood
    NewtonMaterialSetDefaultElasticity( _p_world, playerID, woodID, 0.1f );
    NewtonMaterialSetDefaultSoftness( _p_world, playerID, woodID, 0.5f );
    NewtonMaterialSetDefaultFriction( _p_world, playerID, woodID, 0.6f, 0.4f);
    NewtonMaterialSetCollisionCallback( _p_world, playerID, woodID, &player_woodCollStruct, playerContactBegin, playerContactProcess, playerContactEnd );

    // set the material properties for player on metal
    NewtonMaterialSetDefaultElasticity( _p_world, playerID, metalID, 0.1f );
    NewtonMaterialSetDefaultSoftness( _p_world, playerID, metalID, 0.9f );
    NewtonMaterialSetDefaultFriction( _p_world, playerID, metalID, 0.8f, 0.6f );
    NewtonMaterialSetCollisionCallback( _p_world, playerID, metalID, &player_metalCollStruct, playerContactBegin, playerContactProcess, playerContactEnd );

    // set the material properties for player on grass
    NewtonMaterialSetDefaultElasticity( _p_world, playerID, grassID, 0.1f );
    NewtonMaterialSetDefaultSoftness( _p_world, playerID, grassID, 0.3f );
    NewtonMaterialSetDefaultFriction( _p_world, playerID, grassID, 0.8f, 0.7f );
    NewtonMaterialSetCollisionCallback( _p_world, playerID, grassID, &player_grassCollStruct, playerContactBegin, playerContactProcess, playerContactEnd );

    // set the material properties for player on stone
    NewtonMaterialSetDefaultElasticity( _p_world, playerID, stoneID, 0.2f );
    NewtonMaterialSetDefaultSoftness( _p_world, playerID, stoneID, 0.1f );
    NewtonMaterialSetDefaultFriction( _p_world, playerID, stoneID, 0.9f, 0.7f );
    NewtonMaterialSetCollisionCallback( _p_world, playerID, stoneID, &player_stoneCollStruct, playerContactBegin, playerContactProcess, playerContactEnd );
}

void EnPlayerPhysics::setPlayer( BasePlayerImplementation* p_player )
{
    _p_playerImpl = p_player;
}

void EnPlayerPhysics::initialize()
{
    // note: remote clients don't get the physics build callback!
    _p_world = yaf3d::Physics::get()->getWorld();

    // set the step and player height
    _playerHeight = _dimensions._v[ 2 ];

    // convert angular speed from degrees to radiants
    _angularSpeed = osg::DegreesToRadians( _angularSpeed );

    // create the collision
    NewtonCollision* p_col = NewtonCreateSphere( _p_world, _dimensions._v[ 0 ] * 0.5f, _dimensions._v[ 1 ] * 0.5f, _dimensions._v[ 2 ] * 0.5f, NULL );
    NewtonCollision* p_collision = NewtonCreateConvexHullModifier( _p_world, p_col );
    NewtonReleaseCollision( yaf3d::Physics::get()->getWorld(), p_col );

    //create the rigid body
    _p_body = NewtonCreateBody( _p_world, p_collision );

    // release the collision object, we don't need it anymore
    NewtonReleaseCollision( _p_world, p_collision );

    // set player material
    NewtonBodySetMaterialGroupID( _p_body, yaf3d::Physics::get()->getMaterialId( "player" ) );

    NewtonBodySetUserData( _p_body, this );

    NewtonBodySetLinearDamping( _p_body, _linearDamping );

    float damp[3];
    damp[0] = 0.0f;
    damp[1] = 0.0f;
    damp[2] = 0.0f;
    NewtonBodySetAngularDamping( _p_body, damp );

    // disable auto freeze management for the player
    NewtonBodySetAutoFreeze( _p_body, 0 );
    // keep the player always active
    NewtonWorldUnfreezeBody( _p_world, _p_body );

    // set callbacks
    NewtonBodySetDestructorCallback( _p_body, physicsBodyDestructor );
    NewtonBodySetTransformCallback( _p_body, physicsSetTransform );
    NewtonBodySetForceAndTorqueCallback( _p_body, physicsApplyForceAndTorque );

    // set the mass matrix
    osg::Vec3f& dim = _dimensions;
    float Ixx = 0.4f * _mass * dim.x() * dim.x();
    float Iyy = 0.4f * _mass * dim.y() * dim.y();
    float Izz = 0.4f * _mass * dim.z() * dim.z();
    NewtonBodySetMassMatrix( _p_body, _mass, Ixx, Iyy, Izz );

    // create an up vector joint, this way we constraint the body to keep up
    osg::Vec3f upDirection (0.0f, 0.0f, 1.0f);
    _upVectorJoint = NewtonConstraintCreateUpVector( _p_world, &upDirection._v[ 0 ], _p_body );
}

void EnPlayerPhysics::postInitialize()
{
    // check if the player has already set its association
    assert( _p_playerImpl && "player implementation has to set its association in initialize phase!" );

    osg::Matrixf mat;
    mat *= mat.rotate( _p_playerImpl->getPlayerRotation() );
    mat.setTrans( _p_playerImpl->getPlayerPosition() );

    // find ground under the initial position and adapt body matrix
    float z = findGround( _p_world, _p_playerImpl->getPlayerPosition(), 1000.0f );
    mat.ptr()[ 14 ] = z + _dimensions._v[ 2 ] + 0.2f; // add an offset of player height plus 0.2 meters
    // set the matrix for both the rigid body and the entity
    NewtonBodySetMatrix ( _p_body, mat.ptr() );
    physicsSetTransform ( _p_body, mat.ptr() );
}

void EnPlayerPhysics::updateEntity( float deltaTime )
{
    _isAirBorne     = true;
    _wallCollision  = false;

    _deltaTime      = deltaTime;

    if ( _soundTimer > 0.0f )
        _soundTimer -= deltaTime;
}

void EnPlayerPhysics::jump()
{
    if ( !_isAirBorne )
        _isJumping = true;
}

void EnPlayerPhysics::setTransformation( const osg::Matrixf& mat )
{
    NewtonBodySetMatrix ( _p_body, mat.ptr() );
    physicsSetTransform ( _p_body, mat.ptr() );
}

} // namespace vrc
