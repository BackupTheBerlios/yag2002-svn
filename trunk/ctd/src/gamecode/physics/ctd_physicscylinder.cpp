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
 # entity PhysicsCylinder
 #
 #   date of creation:  02/25/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_physicscylinder.h"
#include "../sound/ctd_3dsound.h"

using namespace std;
using namespace osg; 

namespace CTD
{

// Internally used collision struct
static CollisionStruct s_cylinderCollStruct;

//! Implement and register the entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( PhysicsCylinderEntityFactory );

EnPhysicsCylinder::EnPhysicsCylinder():
_mass( 1.0f ),
_radius( 0.5f ),
_height( 1.0f ),
_p_body( NULL ),
_p_world( Physics::get()->getWorld() )
{
    // register entity attributes
    _attributeManager.addAttribute( "meshFile"      , _meshFile             );
    _attributeManager.addAttribute( "position"      , _position             );
    _attributeManager.addAttribute( "mass"          , _mass                 );
    _attributeManager.addAttribute( "radius"        , _radius               );
    _attributeManager.addAttribute( "height"        , _height               );

    // 3d sound entities, following order is important!
    _attributeManager.addAttribute( "enSndWood"     , _soundEntities[ 0 ]   );
    _attributeManager.addAttribute( "enSndMetal"    , _soundEntities[ 1 ]   );
    _attributeManager.addAttribute( "enSndStone"    , _soundEntities[ 2 ]   );
    _attributeManager.addAttribute( "enSndGrass"    , _soundEntities[ 3 ]   );

    _attributeManager.addAttribute( "playThreshold" , _playThreshold        );
}

EnPhysicsCylinder::~EnPhysicsCylinder()
{
    // remove physics body
    if ( _p_body )
    {
        NewtonBodySetUserData( _p_body, NULL );
        NewtonDestroyBody( Physics::get()->getWorld(), _p_body );
    }

    // remove all sound entities
    for ( unsigned int cnt = 0; cnt < 4; cnt++ )
        EntityManager::get()->deleteEntity( _pp_sounds[ cnt ] );
}

void EnPhysicsCylinder::initializePhysicsMaterials()
{
    // create and setup collision matrials
    unsigned int cylinderID = Physics::get()->createMaterialID( "cylinder" );
    unsigned int defaultID  = Physics::get()->getMaterialId( "default" );
    unsigned int levelID    = Physics::get()->getMaterialId( "level" );
    unsigned int woodID     = Physics::get()->getMaterialId( "wood" );
    unsigned int metalID    = Physics::get()->getMaterialId( "metal" );
    unsigned int grassID    = Physics::get()->getMaterialId( "grass" );
    unsigned int stoneID    = Physics::get()->getMaterialId( "stone" );

    // set non-colliding for cylinder-nocol collisions
    NewtonMaterialSetDefaultCollidable( _p_world, Physics::get()->getMaterialId( "nocol" ), cylinderID, 0 );

    // set the material properties for cylinder on cylinder
    NewtonMaterialSetDefaultElasticity( _p_world, cylinderID, cylinderID, 0.3f );
    NewtonMaterialSetDefaultSoftness( _p_world, cylinderID, cylinderID, 0.3f );
    NewtonMaterialSetDefaultFriction( _p_world, cylinderID, cylinderID, 0.6f, 0.5f );
    NewtonMaterialSetCollisionCallback( _p_world, cylinderID, cylinderID, &s_cylinderCollStruct, entityContactBegin, entityContactProcessLevel< EnPhysicsCylinder >, entityContactEnd ); 

    // set the material properties for cylinder on default
    NewtonMaterialSetDefaultElasticity( _p_world, cylinderID, defaultID, 0.3f );
    NewtonMaterialSetDefaultSoftness( _p_world, cylinderID, defaultID, 0.8f );
    NewtonMaterialSetDefaultFriction( _p_world, cylinderID, defaultID, 0.8f, 0.7f );
    NewtonMaterialSetCollisionCallback( _p_world, cylinderID, defaultID, &s_cylinderCollStruct, entityContactBegin, entityContactProcessLevel< EnPhysicsCylinder >, entityContactEnd ); 

    // set the material properties for cylinder on level
    NewtonMaterialSetDefaultElasticity( _p_world, cylinderID, levelID, 0.3f );
    NewtonMaterialSetDefaultSoftness( _p_world, cylinderID, levelID, 0.8f );
    NewtonMaterialSetDefaultFriction( _p_world, cylinderID, levelID, 0.8f, 0.7f );
    NewtonMaterialSetCollisionCallback( _p_world, cylinderID, levelID, &s_cylinderCollStruct, entityContactBegin, entityContactProcessLevel< EnPhysicsCylinder >, entityContactEnd ); 

    // set the material properties for cylinder on wood
    NewtonMaterialSetDefaultElasticity( _p_world, cylinderID, woodID, 0.5f );
    NewtonMaterialSetDefaultSoftness( _p_world, cylinderID, woodID, 0.5f );
    NewtonMaterialSetDefaultFriction( _p_world, cylinderID, woodID, 0.6f, 0.4f);
    NewtonMaterialSetCollisionCallback( _p_world, cylinderID, woodID, &s_cylinderCollStruct, entityContactBegin, entityContactProcessLevel< EnPhysicsCylinder >, entityContactEnd ); 

    // set the material properties for cylinder on metal
    NewtonMaterialSetDefaultElasticity( _p_world, cylinderID, metalID, 0.7f );
    NewtonMaterialSetDefaultSoftness( _p_world, cylinderID, metalID, 0.9f );
    NewtonMaterialSetDefaultFriction( _p_world, cylinderID, metalID, 0.8f, 0.6f );
    NewtonMaterialSetCollisionCallback( _p_world, cylinderID, metalID, &s_cylinderCollStruct, entityContactBegin, entityContactProcessLevel< EnPhysicsCylinder >, entityContactEnd ); 

    // set the material properties for cylinder on grass
    NewtonMaterialSetDefaultElasticity( _p_world, cylinderID, grassID, 0.2f );
    NewtonMaterialSetDefaultSoftness( _p_world, cylinderID, grassID, 0.3f );
    NewtonMaterialSetDefaultFriction( _p_world, cylinderID, grassID, 0.8f, 0.7f );
    NewtonMaterialSetCollisionCallback( _p_world, cylinderID, grassID, &s_cylinderCollStruct, entityContactBegin, entityContactProcessLevel< EnPhysicsCylinder >, entityContactEnd ); 

    // set the material properties for cylinder on stone
    NewtonMaterialSetDefaultElasticity( _p_world, cylinderID, stoneID, 0.45f );
    NewtonMaterialSetDefaultSoftness( _p_world, cylinderID, stoneID, 0.9f );
    NewtonMaterialSetDefaultFriction( _p_world, cylinderID, stoneID, 0.9f, 0.7f );
    NewtonMaterialSetCollisionCallback( _p_world, cylinderID, stoneID, &s_cylinderCollStruct, entityContactBegin, entityContactProcessLevel< EnPhysicsCylinder >, entityContactEnd ); 
}

// some physics system callback functions
//---------------------------------------
// rigid body destructor callback
void EnPhysicsCylinder::physicsBodyDestructor( const NewtonBody* body )
{
	EnPhysicsCylinder* p_node = ( EnPhysicsCylinder* )NewtonBodyGetUserData( body );
    if ( p_node )
        EntityManager::get()->deleteEntity( p_node );
}

// transformation callback
void EnPhysicsCylinder::physicsSetTransform( const NewtonBody* body, const float* matrix )
{
	EnPhysicsCylinder* p_node = static_cast< EnPhysicsCylinder* >( NewtonBodyGetUserData( body ) );
    p_node->setPosition( Vec3f( matrix[ 12 ], matrix[ 13 ], matrix[ 14 ] ) );
    Matrixf mat( matrix );
    Quat quat;
    mat.get( quat );
    p_node->setRotation( quat );
}

void EnPhysicsCylinder::physicsApplyForceAndTorque( const NewtonBody* body )
{
	float mass;
	float Ixx;
	float Iyy;
	float Izz;

	NewtonBodyGetMassMatrix( body, &mass, &Ixx, &Iyy, &Izz );
    Vec3f force( 0, 0, mass * Physics::get()->getWorldGravity() );
	NewtonBodySetForce( body, &force._v[ 0 ] );
}

void EnPhysicsCylinder::initialize()
{
    osg::Node* p_mesh = LevelManager::get()->loadMesh( _meshFile );
    if ( !p_mesh ) 
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** error loading mesh file '" << _meshFile << "'" << endl;
        EntityManager::get()->registerUpdate( this, false );   // deregister entity
        return;
    }

    // now we add the new mesh into our transformable scene group
    addToTransformationNode( p_mesh );
    setPosition( _position );

    // create the collision
    NewtonCollision* p_col = NewtonCreateCylinder( Physics::get()->getWorld(), _radius, _height, NULL ); 
	NewtonCollision* p_collision = NewtonCreateConvexHullModifier( Physics::get()->getWorld(), p_col );
    NewtonReleaseCollision( Physics::get()->getWorld(), p_col );

    Matrixf mat;
    mat.setTrans( _position ); 

    //create the rigid body
    _p_body = NewtonCreateBody( Physics::get()->getWorld(), p_collision );
    
    // set material, see material definition above
    NewtonBodySetMaterialGroupID( _p_body, Physics::get()->getMaterialId( "cylinder" ) );

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

void EnPhysicsCylinder::postInitialize()
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

    // register entity in order to get updated per simulation step
    EntityManager::get()->registerUpdate( this ); 
}

void EnPhysicsCylinder::updateEntity( float deltaTime )
{
    updateSound( deltaTime );
}

} // namespace CTD
