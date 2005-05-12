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
 # entity PhysicsShpere
 #
 #   date of creation:  02/25/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_physicsshpere.h"

using namespace std;
using namespace osg; 

namespace CTD
{

// Internal used collision struct
static CollisionStruct s_sphereCollStruct;

//! Implement and register the entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( PhysicsSphereEntityFactory );

EnPhysicsSphere::EnPhysicsSphere():
_mass( 1.0f ),
_radius( 2 ),
_p_body( NULL ),
_p_world( Physics::get()->getWorld() )
{
    // register entity in order to get updated per simulation step
    EntityManager::get()->registerUpdate( this ); 

    // register entity attributes
    _attributeManager.addAttribute( "meshFile"      , _meshFile             );
    _attributeManager.addAttribute( "position"      , _position             );
    _attributeManager.addAttribute( "mass"          , _mass                 );
    _attributeManager.addAttribute( "radius"        , _radius               );
    
    // 3d sound entities, following order is important!
    _attributeManager.addAttribute( "enSndWood"     , _soundEntities[ 0 ]   );
    _attributeManager.addAttribute( "enSndMetal"    , _soundEntities[ 1 ]   );
    _attributeManager.addAttribute( "enSndStone"    , _soundEntities[ 2 ]   );
    _attributeManager.addAttribute( "enSndGrass"    , _soundEntities[ 3 ]   );
    
    _attributeManager.addAttribute( "playThreshold" , _playThreshold        );
}

void EnPhysicsSphere::initializePhysicsMaterials()
{
    // create and setup collision matrials
    unsigned int sphereID   = Physics::get()->createMaterialID( "sphere" );
    unsigned int defaultID  = Physics::get()->getMaterialId( "default" );
    unsigned int levelID    = Physics::get()->getMaterialId( "level" );
    unsigned int woodID     = Physics::get()->getMaterialId( "wood" );
    unsigned int metalID    = Physics::get()->getMaterialId( "metal" );
    unsigned int grassID    = Physics::get()->getMaterialId( "grass" );
    unsigned int stoneID    = Physics::get()->getMaterialId( "stone" );

    // set non-colliding for cylinder-nocol collisions
    NewtonMaterialSetDefaultCollidable( _p_world, Physics::get()->getMaterialId( "nocol" ), sphereID, 0 );

    // set the material properties for cylinder on cylinder
    NewtonMaterialSetDefaultElasticity( _p_world, sphereID, sphereID, 0.3f );
    NewtonMaterialSetDefaultSoftness( _p_world, sphereID, sphereID, 0.3f );
    NewtonMaterialSetDefaultFriction( _p_world, sphereID, sphereID, 0.6f, 0.5f );
    NewtonMaterialSetCollisionCallback( _p_world, sphereID, sphereID, &s_sphereCollStruct, entityContactBegin, entityContactProcessLevel< EnPhysicsSphere >, entityContactEnd ); 

    // set the material properties for cylinder on default
    NewtonMaterialSetDefaultElasticity( _p_world, sphereID, defaultID, 0.3f );
    NewtonMaterialSetDefaultSoftness( _p_world, sphereID, defaultID, 0.8f );
    NewtonMaterialSetDefaultFriction( _p_world, sphereID, defaultID, 0.8f, 0.7f );
    NewtonMaterialSetCollisionCallback( _p_world, sphereID, defaultID, &s_sphereCollStruct, entityContactBegin, entityContactProcessLevel< EnPhysicsSphere >, entityContactEnd ); 

    // set the material properties for cylinder on level
    NewtonMaterialSetDefaultElasticity( _p_world, sphereID, levelID, 0.3f );
    NewtonMaterialSetDefaultSoftness( _p_world, sphereID, levelID, 0.8f );
    NewtonMaterialSetDefaultFriction( _p_world, sphereID, levelID, 0.8f, 0.7f );
    NewtonMaterialSetCollisionCallback( _p_world, sphereID, levelID, &s_sphereCollStruct, entityContactBegin, entityContactProcessLevel< EnPhysicsSphere >, entityContactEnd ); 

    // set the material properties for cylinder on wood
    NewtonMaterialSetDefaultElasticity( _p_world, sphereID, woodID, 0.5f );
    NewtonMaterialSetDefaultSoftness( _p_world, sphereID, woodID, 0.5f );
    NewtonMaterialSetDefaultFriction( _p_world, sphereID, woodID, 0.6f, 0.4f);
    NewtonMaterialSetCollisionCallback( _p_world, sphereID, woodID, &s_sphereCollStruct, entityContactBegin, entityContactProcessLevel< EnPhysicsSphere >, entityContactEnd ); 

    // set the material properties for cylinder on metal
    NewtonMaterialSetDefaultElasticity( _p_world, sphereID, metalID, 0.7f );
    NewtonMaterialSetDefaultSoftness( _p_world, sphereID, metalID, 0.9f );
    NewtonMaterialSetDefaultFriction( _p_world, sphereID, metalID, 0.8f, 0.6f );
    NewtonMaterialSetCollisionCallback( _p_world, sphereID, metalID, &s_sphereCollStruct, entityContactBegin, entityContactProcessLevel< EnPhysicsSphere >, entityContactEnd ); 

    // set the material properties for cylinder on grass
    NewtonMaterialSetDefaultElasticity( _p_world, sphereID, grassID, 0.2f );
    NewtonMaterialSetDefaultSoftness( _p_world, sphereID, grassID, 0.3f );
    NewtonMaterialSetDefaultFriction( _p_world, sphereID, grassID, 0.8f, 0.7f );
    NewtonMaterialSetCollisionCallback( _p_world, sphereID, grassID, &s_sphereCollStruct, entityContactBegin, entityContactProcessLevel< EnPhysicsSphere >, entityContactEnd ); 

    // set the material properties for cylinder on stone
    NewtonMaterialSetDefaultElasticity( _p_world, sphereID, stoneID, 0.45f );
    NewtonMaterialSetDefaultSoftness( _p_world, sphereID, stoneID, 0.9f );
    NewtonMaterialSetDefaultFriction( _p_world, sphereID, stoneID, 0.9f, 0.7f );
    NewtonMaterialSetCollisionCallback( _p_world, sphereID, stoneID, &s_sphereCollStruct, entityContactBegin, entityContactProcessLevel< EnPhysicsSphere >, entityContactEnd ); 
}

EnPhysicsSphere::~EnPhysicsSphere()
{
    // remove physics body
    if ( _p_body )
    {
        NewtonBodySetUserData( _p_body, NULL );
        NewtonDestroyBody( Physics::get()->getWorld(), _p_body );
    }
}

// some physics system callback functions
//---------------------------------------
// rigid body destructor callback
void EnPhysicsSphere::physicsBodyDestructor( const NewtonBody* body )
{
	EnPhysicsSphere* p_node = ( EnPhysicsSphere* )NewtonBodyGetUserData( body );
    if (p_node)
	    delete( p_node );
}

// transformation callback
void EnPhysicsSphere::physicsSetTransform( const NewtonBody* body, const float* matrix )
{
	EnPhysicsSphere* p_node = static_cast< EnPhysicsSphere* >( NewtonBodyGetUserData( body ) );
    p_node->setPosition( Vec3f( matrix[ 12 ], matrix[ 13 ], matrix[ 14 ] ) );
    Matrixf mat( matrix );
    Quat quat;
    mat.get( quat );
    p_node->setRotation( quat );
}

void EnPhysicsSphere::physicsApplyForceAndTorque( const NewtonBody* body )
{
	float mass;
	float Ixx;
	float Iyy;
	float Izz;

	NewtonBodyGetMassMatrix (body, &mass, &Ixx, &Iyy, &Izz);
    Vec3f force( 0.0f, 0.0f, mass * Physics::get()->getWorldGravity() );
	NewtonBodySetForce( body, &force._v[ 0 ] );
}

void EnPhysicsSphere::initialize()
{
    osg::Node* p_mesh = LevelManager::get()->loadMesh( _meshFile );
    if ( !p_mesh ) 
    {
        cout << "*** error loading mesh file" << endl;
        return;
    }
    // now we add the new mesh into our transformable scene group
    addToTransformationNode( p_mesh );
    setPosition( _position );

    // create the collision 
    NewtonCollision* p_col = NewtonCreateSphere( Physics::get()->getWorld(), _radius, _radius, _radius, NULL ); 
	NewtonCollision* p_collision = NewtonCreateConvexHullModifier( Physics::get()->getWorld(), p_col );
    NewtonReleaseCollision( Physics::get()->getWorld(), p_col );

    Matrixf mat;
    mat.setTrans( _position ); 

    //create the rigid body
    _p_body = NewtonCreateBody( Physics::get()->getWorld(), p_collision );

    // set metal material
    NewtonBodySetMaterialGroupID( _p_body, Physics::get()->getMaterialId( "sphere" ) );
    NewtonBodySetUserData( _p_body, this );

    // set callbacks
    NewtonBodySetDestructorCallback (_p_body, physicsBodyDestructor );
    NewtonBodySetTransformCallback( _p_body, physicsSetTransform );
    NewtonBodySetForceAndTorqueCallback( _p_body, physicsApplyForceAndTorque );

    // set the mass matrix
    NewtonBodySetMassMatrix( _p_body, _mass, 1.0f, 1.0f, 1.0f );

    // set the matrix for both the rigid body and the entity
    NewtonBodySetMatrix ( _p_body, mat.ptr() );
    physicsSetTransform ( _p_body, mat.ptr() );

    NewtonReleaseCollision( Physics::get()->getWorld(), p_collision );
}

void EnPhysicsSphere::postInitialize()
{
    // find all sound entities and clone them
    _pp_sounds[ 0 ] = NULL;
    if ( _soundEntities[ 0 ].length() )
        _pp_sounds[ 0 ] = getSoundEntity( _soundEntities[ 0 ] );
    else
        log << Log::LogLevel( Log::L_WARNING ) << "* wood sound is not defined. it will be disabled " << endl;

    _pp_sounds[ 1 ] = NULL;
    if ( _soundEntities[ 1 ].length() )
        _pp_sounds[ 1 ] = getSoundEntity( _soundEntities[ 1 ] );
    else
        log << Log::LogLevel( Log::L_WARNING ) << "* metal sound is not defined. it will be disabled " << endl;

    _pp_sounds[ 2 ] = NULL;
    if ( _soundEntities[ 2 ].length() )
        _pp_sounds[ 2 ] = getSoundEntity( _soundEntities[ 2 ] );
    else
        log << Log::LogLevel( Log::L_WARNING ) << "* stone sound is not defined. it will be disabled " << endl;

    _pp_sounds[ 3 ] = NULL;
    if ( _soundEntities[ 3 ].length() )
        _pp_sounds[ 3 ] = getSoundEntity( _soundEntities[ 3 ] );
    else
        log << Log::LogLevel( Log::L_WARNING ) << "* grass sound is not defined. it will be disabled " << endl;
}

void EnPhysicsSphere::updateEntity( float deltaTime )
{
    updateSound( deltaTime );
}

} // namespace CTD

