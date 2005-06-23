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
 ################################################################*/

#include <ctd_main.h>
#include "ctd_playerphysics.h"
#include "ctd_playersound.h"
#include "ctd_playerimpl.h"

using namespace osg;
using namespace std;

namespace CTD
{

//! Implement and register the player physics entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( PlayerPhysicsEntityFactory );

bool EnPlayerPhysics::_materialsCreated = false;

//! Data structure used in raycasting
struct PlayerRayCastData
{
                            PlayerRayCastData( NewtonBody* p_me )
                            {
                                _p_body    = p_me;
                                _parameter = 1.2f;
                            }

    NewtonBody*             _p_body;

    float                   _parameter;
};

struct  PlayerCollisionStruct
{
    NewtonBody*      _p_body0;

    NewtonBody*      _p_body1;
    
    osg::Vec3f       _position;
    
    float            _contactMaxNormalSpeed;
    
    float            _contactMaxTangentSpeed;

    BaseEntity*      _p_otherEntity;

    EnPlayerPhysics*   _p_physics;
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
    // set right parameters for predfined materials
    Physics::levelContactProcess( p_material, p_contact );

    // determine which body is the player physics's one
	NewtonBody*    p_body = s_colStruct->_p_body1;
    EnPlayerPhysics* p_phys = 0;
    void*          p_userData = NewtonBodyGetUserData( p_body );
    if ( p_userData ) // level geoms have no data associated
    {
        p_phys = static_cast< EnPlayerPhysics* >( p_userData );
	} 
    else // this else is only for being on safe side. in normal case all level bodies come in as body0!
    {
        p_phys = static_cast< EnPlayerPhysics* >( NewtonBodyGetUserData( s_colStruct->_p_body0 ) );
       // assert( NULL && "body0 should be static geom!" ); this case comes when i move the character out of world bbox
    }
    s_colStruct->_p_otherEntity = NULL;
    s_colStruct->_p_physics     = p_phys;

    // play appropriate sound only if we are moving
    if ( p_phys->isMoving() && p_phys->getPlayer()->getPlayerSound() )
    {
        unsigned int attribute = ( unsigned int )( NewtonMaterialGetContactFaceAttribute( p_material ) );
        unsigned int materialType = attribute & 0xFF;
        switch ( materialType )
        {
            case Physics::MAT_DEFAULT:
            case Physics::MAT_STONE:
                p_phys->getPlayer()->getPlayerSound()->playWalkGround();
                break;

            case Physics::MAT_WOOD:
                p_phys->getPlayer()->getPlayerSound()->playWalkWood();
                break;

            case Physics::MAT_METALL:
                p_phys->getPlayer()->getPlayerSound()->playWalkMetal();
                break;

            case Physics::MAT_GRASS:
                p_phys->getPlayer()->getPlayerSound()->playWalkGrass();
                break;

            default:
                ;
        }
    }

    return p_phys->collideWithLevel( p_material, p_contact );
}

// player contact process callback function called when the player collides with other materials in world than level
int playerContactProcess( const NewtonMaterial* p_material, const NewtonContact* p_contact )
{
    // determine which body is the player physics's one
	NewtonBody*    p_body   = s_colStruct->_p_body0;
    BaseEntity*    p_entity = reinterpret_cast< BaseEntity* >( NewtonBodyGetUserData( p_body ) );
    EnPlayerPhysics* p_phys   = dynamic_cast< EnPlayerPhysics* >( p_entity );
    if ( !p_phys )
    {
        s_colStruct->_p_otherEntity = p_entity;
        p_body = s_colStruct->_p_body1;
        p_phys = static_cast< EnPlayerPhysics* >( NewtonBodyGetUserData( p_body ) );
    }
    else
    {
        s_colStruct->_p_otherEntity = reinterpret_cast< BaseEntity* >( NewtonBodyGetUserData( s_colStruct->_p_body1 ) );
    }

    s_colStruct->_p_physics = p_phys;
    return p_phys->collideWithOtherEntities( p_material, p_contact );
}

// this function is called affter all collision contacts are proccesed
void playerContactEnd( const NewtonMaterial* p_material )
{
}

// find floor for character placement
float EnPlayerPhysics::findFloor( NewtonWorld* world, const Vec3f& p0, float maxDist )
{
    PlayerRayCastData data( _p_body );
    
    // shot a vertical ray from a high altitude and collect the intersetion partameter.
    Vec3f p1( p0 ); 
    p1._v[ 2 ] -= maxDist;

    NewtonWorldRayCast( _p_world, &p0._v[ 0 ], &p1._v[ 0 ], physicsRayCastPlacement, &data );

    // the intesection is the interpolater value
    return p0._v[ 2 ] - maxDist * data._parameter;
}

// rigid body destructor callback
void EnPlayerPhysics::physicsBodyDestructor( const NewtonBody* p_body )
{
}

// transformation callback
void EnPlayerPhysics::physicsSetTransform( const NewtonBody* p_body, const float* p_matrix )
{
    BasePlayerImplementation* p_node = static_cast< EnPlayerPhysics* >( NewtonBodyGetUserData( p_body ) )->_p_playerImpl;
    Matrixf& mat = p_node->getPlayerPhysics()->_matrix;
    mat = Matrixf( p_matrix );    
    Quat quat;
    mat.get( quat );

    osg::Vec3f trans = mat.getTrans();
    // set player's position and rotation
    p_node->setPlayerRotation( quat );
    p_node->setPlayerPosition( trans );
}

int EnPlayerPhysics::collideWithLevel( const NewtonMaterial* p_material, const NewtonContact* p_contact )
{
	Vec3f point;
	Vec3f normal;
	Vec3f velocity;

	_isAirBorne = false;

	// Get the collision and normal
    NewtonMaterialGetContactPositionAndNormal( p_material, &point._v[ 0 ], &normal._v[ 0 ] );

    Vec3f pos = _p_playerImpl->getPlayerPosition();
	// consider the player height
    if ( ( point._v[ 2 ] - ( pos._v[ 2 ] - _playerHeight * 0.5f ) ) < _stepHeight ) 
    {
        Vec3f localpoint = pos - point;
        Matrixf::transform3x3( _matrix, localpoint );
		// save the elevation of the highest step to take
		if ( localpoint._v[ 2 ] > _climbContact._v[ 2 ] ) 
			if ( fabsf( normal._v[ 2 ] ) < 0.8f )
				_climbContact = localpoint;
	}

    return 1;
}

int EnPlayerPhysics::collideWithOtherEntities( const NewtonMaterial* p_material, const NewtonContact* p_contact )
{ 
    //! TODO
    return 1;
}

void EnPlayerPhysics::physicsApplyForceAndTorque( const NewtonBody* p_body )
{
    float mass;
    float Ixx;
    float Iyy;
    float Izz;
	float floor;
	float accelZ;
	float deltaHeight;
	float steerAngle;
	float timestep;
	float timestepInv;
	Vec3f omega;
	Vec3f alpha;
	Vec3f heading;
	Vec3f velocity;
    Vec3f pos;
	Matrixf matrix;

    NewtonBodyGetMassMatrix( p_body, &mass, &Ixx, &Iyy, &Izz );
    EnPlayerPhysics* p_phys = static_cast< EnPlayerPhysics* >( NewtonBodyGetUserData( p_body ) );
    Vec3f& force  = p_phys->_force;
    force._v[ 0 ] *= p_phys->_linearForce;
    force._v[ 1 ] *= p_phys->_linearForce;

    // get the current world timestep
	timestep = NewtonGetTimeStep( p_phys->_p_world );
	timestepInv = 1.0f / timestep;

	NewtonBodyGetVelocity( p_phys->_p_body, &velocity._v[ 0 ] );
    NewtonBodyGetMatrix( p_phys->_p_body, matrix.ptr() );
    pos = matrix.getTrans();

	// calculate the torque vector
    const float* matelems = matrix.ptr();
    Vec3f front( matelems[ 4 ] , matelems[ 5 ], matelems[ 6 ] );
    // _moveDir must be normalized
    Vec3f cross( front ^ p_phys->_p_playerImpl->getPlayerMoveDirection() );
    steerAngle = min( max( cross._v[ 2 ], -1.0f ), 1.0f );
	steerAngle = asinf( steerAngle );
	NewtonBodyGetOmega( p_phys->_p_body, &omega._v[ 0 ] );
	Vec3f torque( 0.0f, 0.0f, 0.5f * Izz * ( steerAngle * timestepInv - omega._v[ 2 ] ) * timestepInv );
    NewtonBodySetTorque( p_phys->_p_body, &torque._v[ 0 ] );
	
    // climb steps by adding a force
    float climbforce = 0;
    if ( p_phys->_climbContact._v[ 2 ] > 0 )
    {
        climbforce = p_phys->_climbForce;
    }
    // reset climb contact
    p_phys->_climbContact.set( 0.0f, 0.0f, 0.0f );

    force._v[ 2 ] = mass * ( p_phys->_gravity + climbforce ); // add gravity and climbing force

    // snap to ground
    if ( p_phys->_isAirBorne && !p_phys->_isJumping ) 
    {
        floor = p_phys->findFloor( p_phys->_p_world, pos, p_phys->_playerHeight + p_phys->_stepHeight );
		deltaHeight = ( pos._v[ 2 ] - 0.5f * p_phys->_playerHeight ) - floor;
		if ( ( deltaHeight < p_phys->_stepHeight ) && ( deltaHeight > 0.01f ) ) 
        {
			// snap to floor ony if the floor is lower than the character feet	
			accelZ = -( deltaHeight * timestepInv + velocity._v[ 2 ] ) * timestepInv;
			force._v[ 2 ] += mass * accelZ;
		}
	}
    else if ( p_phys->_isJumping )
    {
        switch ( p_phys->_jumpState )
        {
            case BeginJumping:
            {
                Vec3f veloc( 0.0f, 0.0f, p_phys->_jumpForce );
                NewtonAddBodyImpulse( p_phys->_p_body, &veloc._v [ 0 ], &pos._v[ 0 ] );
                p_phys->_jumpState = Wait4Landing;
            }
            break;

            case Wait4Landing:
            {
                if ( !p_phys->_isAirBorne )
                {
                    p_phys->_jumpState = BeginJumping;
                    p_phys->_isJumping = false;
                }
            }
            break;

            default:
                assert( NULL && "invalid jump state" );
        }
    }
    p_phys->_jumpTimer = p_phys->_jumpTimer ? p_phys->_jumpTimer - 1 : 0;

    //!FIXME: rotate the force direction to align with the camera, currently it does not work
    //heading = matrix * p_phys->getPlayer()->getPlayerMoveDirection();
    //heading *= ( 1.0f / sqrtf( ( heading * heading ) + 1.0e-6f ) );
    //force += ( heading * ( mass /** 30.0f*/ - 2.0f * ( velocity * heading ) ) ); 

    NewtonBodySetForce( p_body, &force._v[ 0 ] );

    p_phys->_isStopped = ( ( force._v[ 0 ] == 0 ) && ( force._v[ 1 ] == 0 ) );	
}

float EnPlayerPhysics::physicsRayCastPlacement( const NewtonBody* p_body, const float* p_normal, int collisionID, void* p_userData, float intersectParam )
{
    float paramPtr = 1.2f;
    PlayerRayCastData& data = *( static_cast< PlayerRayCastData* >( p_userData ) );

    // any body can be a floor
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
_p_playerImpl( NULL ),
_p_world( Physics::get()->getWorld() ),
_p_body( NULL ),
_isAirBorne( false ),
_playerHeight( 1.8f ),
_jumpTimer( 0 ),
_isJumping( false ),
_jumpState( BeginJumping ),
_jumpForce( 5.0f ),
_climbForce( 30.0f ),
_dimensions( Vec3f( 0.5f, 0.5f, 1.8f ) ),
_stepHeight( 0.5f ),
_linearForce( 0.1f ),
_angularForce( 0.05f ),
_linearDamping( 0.2f ),
_gravity( Physics::get()->getWorldGravity() )
{ 
    EntityManager::get()->registerNotification( this, true );   // register entity in order to get notifications about physics building

    // add entity attributes
    getAttributeManager().addAttribute( "dimensions"    , _dimensions    );
    getAttributeManager().addAttribute( "stepheight"    , _stepHeight    );
    getAttributeManager().addAttribute( "jumpforce"     , _jumpForce     );
    getAttributeManager().addAttribute( "linearforce"   , _linearForce   );
    getAttributeManager().addAttribute( "lineardamping" , _linearDamping );
    getAttributeManager().addAttribute( "angularforce"  , _angularForce  );
    getAttributeManager().addAttribute( "mass"          , _mass          );
    getAttributeManager().addAttribute( "gravity"       , _gravity       );
}

EnPlayerPhysics::~EnPlayerPhysics()
{
    if ( _p_body )
        NewtonDestroyBody( Physics::get()->getWorld(), _p_body );
}

void EnPlayerPhysics::handleNotification( const EntityNotification& notification )
{
    switch( notification.getId() )
    {
        case CTD_NOTIFY_BUILDING_PHYSICSWORLD:

            // create the physics materials only once for every entity type on every level loading
            if ( !_materialsCreated )
            {
                initializePhysicsMaterials();
                _materialsCreated = true;
            }
            break;

        case CTD_NOTIFY_DELETING_PHYSICSWORLD:

            _materialsCreated = false;
            break;

        default:
            ;

    }
}

void EnPlayerPhysics::initializePhysicsMaterials()
{
    // setup materials
    int playerID   = Physics::get()->createMaterialID( "player" );
    int defaultID  = Physics::get()->getMaterialId( "default" );
    int levelID    = Physics::get()->getMaterialId( "level" );
    int woodID     = Physics::get()->getMaterialId( "wood" );
    int metalID    = Physics::get()->getMaterialId( "metal" );
    int grassID    = Physics::get()->getMaterialId( "grass" );
    int stoneID    = Physics::get()->getMaterialId( "stone" );

    // set non-colliding for player-nocol collisions
    NewtonMaterialSetDefaultCollidable( _p_world, Physics::get()->getMaterialId( "nocol" ), playerID, 0 );

    // set the material properties for player on default
    NewtonMaterialSetDefaultElasticity( _p_world, playerID, defaultID, 0.3f );
    NewtonMaterialSetDefaultSoftness( _p_world, playerID, defaultID, 0.8f );
    NewtonMaterialSetDefaultFriction( _p_world, playerID, defaultID, 0.8f, 0.7f );
    NewtonMaterialSetCollisionCallback( _p_world, playerID, defaultID, &player_levelCollStruct, playerContactBegin, playerContactProcessLevel, playerContactEnd ); 

    // set the material properties for player on level
    NewtonMaterialSetDefaultElasticity( _p_world, playerID, levelID, 0.3f );
    NewtonMaterialSetDefaultSoftness( _p_world, playerID, levelID, 0.8f );
    NewtonMaterialSetDefaultFriction( _p_world, playerID, levelID, 0.8f, 0.7f );
    NewtonMaterialSetCollisionCallback( _p_world, playerID, levelID, &player_levelCollStruct, playerContactBegin, playerContactProcessLevel, playerContactEnd ); 

    // set the material properties for player on wood
    NewtonMaterialSetDefaultElasticity( _p_world, playerID, woodID, 0.5f );
    NewtonMaterialSetDefaultSoftness( _p_world, playerID, woodID, 0.5f );
    NewtonMaterialSetDefaultFriction( _p_world, playerID, woodID, 0.6f, 0.4f);
    NewtonMaterialSetCollisionCallback( _p_world, playerID, woodID, &player_woodCollStruct, playerContactBegin, playerContactProcess, playerContactEnd ); 

    // set the material properties for player on metal
    NewtonMaterialSetDefaultElasticity( _p_world, playerID, metalID, 0.7f );
    NewtonMaterialSetDefaultSoftness( _p_world, playerID, metalID, 0.9f );
    NewtonMaterialSetDefaultFriction( _p_world, playerID, metalID, 0.8f, 0.6f );
    NewtonMaterialSetCollisionCallback( _p_world, playerID, metalID, &player_metalCollStruct, playerContactBegin, playerContactProcess, playerContactEnd ); 

    // set the material properties for player on grass
    NewtonMaterialSetDefaultElasticity( _p_world, playerID, grassID, 0.2f );
    NewtonMaterialSetDefaultSoftness( _p_world, playerID, grassID, 0.3f );
    NewtonMaterialSetDefaultFriction( _p_world, playerID, grassID, 0.8f, 0.7f );
    NewtonMaterialSetCollisionCallback( _p_world, playerID, grassID, &player_grassCollStruct, playerContactBegin, playerContactProcess, playerContactEnd ); 

    // set the material properties for player on stone
    NewtonMaterialSetDefaultElasticity( _p_world, playerID, stoneID, 0.45f );
    NewtonMaterialSetDefaultSoftness( _p_world, playerID, stoneID, 0.9f );
    NewtonMaterialSetDefaultFriction( _p_world, playerID, stoneID, 0.9f, 0.7f );
    NewtonMaterialSetCollisionCallback( _p_world, playerID, stoneID, &player_stoneCollStruct, playerContactBegin, playerContactProcess, playerContactEnd ); 
}

void EnPlayerPhysics::setPlayer( BasePlayerImplementation* p_player )
{
    _p_playerImpl = p_player;
}

void EnPlayerPhysics::initialize()
{
    // set the step and player height
    _playerHeight = _dimensions._v[ 2 ] * 2.0f;

    // create the collision 
    NewtonCollision* p_col = NewtonCreateSphere( _p_world, _dimensions._v[ 0 ], _dimensions._v[ 1 ], _dimensions._v[ 2 ], NULL );
    //NewtonCollision* p_col = NewtonCreateBox( _p_world, _dimensions._v[ 0 ], _dimensions._v[ 1 ], _dimensions._v[ 2 ], NULL );
    NewtonCollision* p_collision = NewtonCreateConvexHullModifier( _p_world, p_col );
    NewtonReleaseCollision( Physics::get()->getWorld(), p_col );

    //create the rigid body
    _p_body = NewtonCreateBody( _p_world, p_collision );

    // release the collision object, we don't need it anymore
    NewtonReleaseCollision( _p_world, p_collision );

    // set wood material
    NewtonBodySetMaterialGroupID( _p_body, Physics::get()->getMaterialId( "player" ) );

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
    Vec3f& dim = _dimensions;
    float Ixx = 0.4f * _mass * dim.x() * dim.x();
    float Iyy = 0.4f * _mass * dim.y() * dim.y();
    float Izz = 0.4f * _mass * dim.z() * dim.z();
    NewtonBodySetMassMatrix( _p_body, _mass, Ixx, Iyy, Izz );

    // create an up vector joint, this way we constraint the body to keep up
    Vec3f upDirection (0.0f, 0.0f, 1.0f);
    _upVectorJoint = NewtonConstraintCreateUpVector( _p_world, &upDirection._v[ 0 ], _p_body ); 
}

void EnPlayerPhysics::postInitialize()
{
    // check if the player has already set its association
    assert( _p_playerImpl && "player implementation has to set its association in initialize phase!" );

    // give this object the same name as player's
    setInstanceName( _p_playerImpl->getPlayerEntity()->getInstanceName() );

    Matrixf mat;
    mat.setTrans( _p_playerImpl->getPlayerPosition() ); 

    // find floor under the initial position and adapt body matrix
    float z = findFloor( _p_world, _p_playerImpl->getPlayerPosition(), 1000.0f );
    mat.ptr()[ 14 ] = z + _dimensions._v[ 2 ] + 0.2f; // add an offset of player height plus 0.2 meters
    // set the matrix for both the rigid body and the entity
    NewtonBodySetMatrix ( _p_body, mat.ptr() );
    physicsSetTransform ( _p_body, mat.ptr() );

    // register entity for getting updated
    EntityManager::get()->registerUpdate( this, true );
}

void EnPlayerPhysics::updateEntity( float deltaTime )
{
    _isAirBorne = true;
}

void EnPlayerPhysics::jump()
{
    if ( !_isAirBorne )
        _isJumping = true;
}

} // namespace CTD
