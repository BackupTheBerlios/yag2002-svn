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
 # entity PhysicsBox
 #
 #   date of creation:  02/24/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_physicsbox.h"

using namespace std;
using namespace osg; 

namespace CTD
{

// Internally used collision struct
static CollisionStruct s_boxCollStruct;

//! Implement and register the entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( PhysicsBoxEntityFactory );


EnPhysicsBox::EnPhysicsBox():
_mass( 1.0f ),
_dimensions( Vec3f( 1.0f, 1.0f, 1.0f ) ),
_p_body( NULL ),
_p_world( Physics::get()->getWorld() )
{
    EntityManager::get()->registerUpdate( this );     // register entity in order to get updated per simulation step

    // register entity attributes
    // note: this must be done in constructor!
    _attributeManager.addAttribute( "meshFile"      , _meshFile             );
    _attributeManager.addAttribute( "position"      , _position             );
    _attributeManager.addAttribute( "mass"          , _mass                 );
    _attributeManager.addAttribute( "dimensions"    , _dimensions           );

    // 3d sound entities, following order is important!
    _attributeManager.addAttribute( "enSndWood"     , _soundEntities[ 0 ]   );
    _attributeManager.addAttribute( "enSndMetal"    , _soundEntities[ 1 ]   );
    _attributeManager.addAttribute( "enSndStone"    , _soundEntities[ 2 ]   );
    _attributeManager.addAttribute( "enSndGrass"    , _soundEntities[ 3 ]   );

    _attributeManager.addAttribute( "playThreshold" , _playThreshold        );

    // the materials _must_ be created only once!
    static boxMatrialCreated = false;
    if ( !boxMatrialCreated )
    {
        boxMatrialCreated = true;

        // create a collision callbacks for player object and other materials
        //  it is important that own materials are created in constructor!
        //-------------------------------------------------------------------
        unsigned int boxID      = Physics::get()->createMaterialID( "box" );
        unsigned int defaultID  = Physics::get()->getMaterialId( "default" );
        unsigned int levelID    = Physics::get()->getMaterialId( "level" );
        unsigned int woodID     = Physics::get()->getMaterialId( "wood" );
        unsigned int metalID    = Physics::get()->getMaterialId( "metal" );
        unsigned int grassID    = Physics::get()->getMaterialId( "grass" );
        unsigned int stoneID    = Physics::get()->getMaterialId( "stone" );

        // set non-colliding for cylinder-nocol collisions
        NewtonMaterialSetDefaultCollidable( _p_world, Physics::get()->getMaterialId( "nocol" ), boxID, 0 );

        // set the material properties for cylinder on cylinder
        NewtonMaterialSetDefaultElasticity( _p_world, boxID, boxID, 0.3f );
        NewtonMaterialSetDefaultSoftness( _p_world, boxID, boxID, 0.3f );
        NewtonMaterialSetDefaultFriction( _p_world, boxID, boxID, 0.6f, 0.5f );
        NewtonMaterialSetCollisionCallback( _p_world, boxID, boxID, &s_boxCollStruct, entityContactBegin, entityContactProcessLevel< EnPhysicsBox >, entityContactEnd ); 

        // set the material properties for cylinder on default
        NewtonMaterialSetDefaultElasticity( _p_world, boxID, defaultID, 0.3f );
        NewtonMaterialSetDefaultSoftness( _p_world, boxID, defaultID, 0.8f );
        NewtonMaterialSetDefaultFriction( _p_world, boxID, defaultID, 0.8f, 0.7f );
        NewtonMaterialSetCollisionCallback( _p_world, boxID, defaultID, &s_boxCollStruct, entityContactBegin, entityContactProcessLevel< EnPhysicsBox >, entityContactEnd ); 

        // set the material properties for cylinder on level
        NewtonMaterialSetDefaultElasticity( _p_world, boxID, levelID, 0.3f );
        NewtonMaterialSetDefaultSoftness( _p_world, boxID, levelID, 0.8f );
        NewtonMaterialSetDefaultFriction( _p_world, boxID, levelID, 0.8f, 0.7f );
        NewtonMaterialSetCollisionCallback( _p_world, boxID, levelID, &s_boxCollStruct, entityContactBegin, entityContactProcessLevel< EnPhysicsBox >, entityContactEnd ); 

        // set the material properties for cylinder on wood
        NewtonMaterialSetDefaultElasticity( _p_world, boxID, woodID, 0.5f );
        NewtonMaterialSetDefaultSoftness( _p_world, boxID, woodID, 0.5f );
        NewtonMaterialSetDefaultFriction( _p_world, boxID, woodID, 0.6f, 0.4f);
        NewtonMaterialSetCollisionCallback( _p_world, boxID, woodID, &s_boxCollStruct, entityContactBegin, entityContactProcessLevel< EnPhysicsBox >, entityContactEnd ); 

        // set the material properties for cylinder on metal
        NewtonMaterialSetDefaultElasticity( _p_world, boxID, metalID, 0.7f );
        NewtonMaterialSetDefaultSoftness( _p_world, boxID, metalID, 0.9f );
        NewtonMaterialSetDefaultFriction( _p_world, boxID, metalID, 0.8f, 0.6f );
        NewtonMaterialSetCollisionCallback( _p_world, boxID, metalID, &s_boxCollStruct, entityContactBegin, entityContactProcessLevel< EnPhysicsBox >, entityContactEnd ); 

        // set the material properties for cylinder on grass
        NewtonMaterialSetDefaultElasticity( _p_world, boxID, grassID, 0.2f );
        NewtonMaterialSetDefaultSoftness( _p_world, boxID, grassID, 0.3f );
        NewtonMaterialSetDefaultFriction( _p_world, boxID, grassID, 0.8f, 0.7f );
        NewtonMaterialSetCollisionCallback( _p_world, boxID, grassID, &s_boxCollStruct, entityContactBegin, entityContactProcessLevel< EnPhysicsBox >, entityContactEnd ); 

        // set the material properties for cylinder on stone
        NewtonMaterialSetDefaultElasticity( _p_world, boxID, stoneID, 0.45f );
        NewtonMaterialSetDefaultSoftness( _p_world, boxID, stoneID, 0.9f );
        NewtonMaterialSetDefaultFriction( _p_world, boxID, stoneID, 0.9f, 0.7f );
        NewtonMaterialSetCollisionCallback( _p_world, boxID, stoneID, &s_boxCollStruct, entityContactBegin, entityContactProcessLevel< EnPhysicsBox >, entityContactEnd ); 
    }
}

EnPhysicsBox::~EnPhysicsBox()
{
    // remove physics p_body
    if ( _p_body )
    {
        NewtonBodySetUserData( _p_body, NULL );
        NewtonDestroyBody( Physics::get()->getWorld(), _p_body );
    }     
}

// some physics system callback functions
//---------------------------------------
// rigid p_body destructor callback
void EnPhysicsBox::physicsBodyDestructor( const NewtonBody* p_body )
{
    EnPhysicsBox* p_node = ( EnPhysicsBox* )NewtonBodyGetUserData( p_body );
    if (p_node)
        delete( p_node );
}

// transformation callback
void EnPhysicsBox::physicsSetTransform( const NewtonBody* p_body, const float* matrix )
{
    EnPhysicsBox* p_node = static_cast< EnPhysicsBox* >( NewtonBodyGetUserData( p_body ) );
    p_node->setPosition( Vec3f( matrix[ 12 ], matrix[ 13 ], matrix[ 14 ] ) );
    Matrixf mat( matrix );
    Quat quat;
    mat.get( quat );
    p_node->setRotation( quat );
}

void EnPhysicsBox::physicsApplyForceAndTorque( const NewtonBody* p_body )
{
    float mass;
    float Ixx;
    float Iyy;
    float Izz;

    NewtonBodyGetMassMatrix( p_body, &mass, &Ixx, &Iyy, &Izz );
    Vec3f force( 0.0f, 0.0f, mass * Physics::get()->getWorldGravity() );
    NewtonBodySetForce( p_body, &force._v[ 0 ] );
}

void EnPhysicsBox::initialize()
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
    NewtonCollision *p_collision = NewtonCreateBox( Physics::get()->getWorld(), _dimensions._v[ 0 ], _dimensions._v[ 1 ], _dimensions._v[ 2 ], NULL); 
    p_collision = NewtonCreateConvexHullModifier( Physics::get()->getWorld(), p_collision );

    Matrixf mat;
    mat.setTrans( _position ); 

    //create the rigid p_body
    _p_body = NewtonCreateBody( Physics::get()->getWorld(), p_collision );

    // set material, see material definition above
    NewtonBodySetMaterialGroupID( _p_body, Physics::get()->getMaterialId( "box" ) );
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

void EnPhysicsBox::postInitialize()
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

void EnPhysicsBox::updateEntity( float deltaTime )
{
    updateSound( deltaTime );
}

} // namespace CTD
