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

#include <vrc_main.h>
#include "vrc_physicssphere.h"

namespace vrc
{

// Internal used collision struct
static yaf3d::CollisionStruct s_sphereCollStruct;

//! Implement and register the entity factory
YAF3D_IMPL_ENTITYFACTORY( PhysicsSphereEntityFactory );

EnPhysicsSphere::EnPhysicsSphere():
_mass( 1.0f ),
_radius( 2 ),
_p_body( NULL ),
_p_world( NULL )
{
    // register entity attributes
    getAttributeManager().addAttribute( "meshFile"      , _meshFile             );
    getAttributeManager().addAttribute( "position"      , _position             );
    getAttributeManager().addAttribute( "mass"          , _mass                 );
    getAttributeManager().addAttribute( "radius"        , _radius               );
    
    // 3d sound entities, following order is important!
    getAttributeManager().addAttribute( "enSndWood"     , _soundEntities[ 0 ]   );
    getAttributeManager().addAttribute( "enSndMetal"    , _soundEntities[ 1 ]   );
    getAttributeManager().addAttribute( "enSndStone"    , _soundEntities[ 2 ]   );
    getAttributeManager().addAttribute( "enSndGrass"    , _soundEntities[ 3 ]   );
    
    getAttributeManager().addAttribute( "playThreshold" , _playThreshold        );
}

EnPhysicsSphere::~EnPhysicsSphere()
{
    // remove physics body
    if ( _p_body )
    {
        NewtonBodySetUserData( _p_body, NULL );
        NewtonDestroyBody( yaf3d::Physics::get()->getWorld(), _p_body );
    }
}

void EnPhysicsSphere::initializePhysicsMaterials()
{
    _p_world = yaf3d::Physics::get()->getWorld();

    // create and setup collision matrials
    int sphereID   = yaf3d::Physics::get()->createMaterialID( "sphere" );
    int defaultID  = yaf3d::Physics::get()->getMaterialId( "default" );
    int levelID    = yaf3d::Physics::get()->getMaterialId( "level" );
    int woodID     = yaf3d::Physics::get()->getMaterialId( "wood" );
    int metalID    = yaf3d::Physics::get()->getMaterialId( "metal" );
    int grassID    = yaf3d::Physics::get()->getMaterialId( "grass" );
    int stoneID    = yaf3d::Physics::get()->getMaterialId( "stone" );

    // set non-colliding for cylinder-nocol collisions
    NewtonMaterialSetDefaultCollidable( _p_world, yaf3d::Physics::get()->getMaterialId( "nocol" ), sphereID, 0 );

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
    p_node->setPosition( osg::Vec3f( matrix[ 12 ], matrix[ 13 ], matrix[ 14 ] ) );
    osg::Matrixf mat( matrix );
    osg::Quat quat;
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
    osg::Vec3f force( 0.0f, 0.0f, mass * yaf3d::Physics::get()->getWorldGravity() );
	NewtonBodySetForce( body, &force._v[ 0 ] );
}

void EnPhysicsSphere::initialize()
{
    osg::Node* p_mesh = yaf3d::LevelManager::get()->loadMesh( _meshFile );
    if ( !p_mesh ) 
    {
        log_error << "*** error loading mesh file" << std::endl;
        return;
    }
    // now we add the new mesh into our transformable scene group
    addToTransformationNode( p_mesh );
    setPosition( _position );

    // create the collision 
    NewtonCollision* p_col = NewtonCreateSphere( yaf3d::Physics::get()->getWorld(), _radius, _radius, _radius, NULL ); 
	NewtonCollision* p_collision = NewtonCreateConvexHullModifier( yaf3d::Physics::get()->getWorld(), p_col );
    NewtonReleaseCollision( yaf3d::Physics::get()->getWorld(), p_col );

    osg::Matrixf mat;
    mat.setTrans( _position ); 

    //create the rigid body
    _p_body = NewtonCreateBody( yaf3d::Physics::get()->getWorld(), p_collision );

    // set metal material
    NewtonBodySetMaterialGroupID( _p_body, yaf3d::Physics::get()->getMaterialId( "sphere" ) );
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

    NewtonReleaseCollision( yaf3d::Physics::get()->getWorld(), p_collision );
}

void EnPhysicsSphere::postInitialize()
{
    // find all sound entities and clone them
    _pp_sounds[ 0 ] = NULL;
    if ( _soundEntities[ 0 ].length() )
        _pp_sounds[ 0 ] = getSoundEntity( _soundEntities[ 0 ] );
    else
        log_warning << "* wood sound is not defined. it will be disabled " << std::endl;

    _pp_sounds[ 1 ] = NULL;
    if ( _soundEntities[ 1 ].length() )
        _pp_sounds[ 1 ] = getSoundEntity( _soundEntities[ 1 ] );
    else
        log_warning << "* metal sound is not defined. it will be disabled " << std::endl;

    _pp_sounds[ 2 ] = NULL;
    if ( _soundEntities[ 2 ].length() )
        _pp_sounds[ 2 ] = getSoundEntity( _soundEntities[ 2 ] );
    else
        log_warning << "* stone sound is not defined. it will be disabled " << std::endl;

    _pp_sounds[ 3 ] = NULL;
    if ( _soundEntities[ 3 ].length() )
        _pp_sounds[ 3 ] = getSoundEntity( _soundEntities[ 3 ] );
    else
        log_warning << "* grass sound is not defined. it will be disabled " << std::endl;

    // register entity in order to get updated per simulation step
    yaf3d::EntityManager::get()->registerUpdate( this ); 
}

void EnPhysicsSphere::updateEntity( float deltaTime )
{
    updateSound( deltaTime );
}

} // namespace vrc

