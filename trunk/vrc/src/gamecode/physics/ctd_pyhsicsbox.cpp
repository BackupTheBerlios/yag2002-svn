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
 # entity PhysicsBox
 #
 #   date of creation:  02/24/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_base.h>
#include <ctd_physics.h>
#include <ctd_levelmanager.h>
#include "ctd_pyhsicsbox.h"

using namespace std;
using namespace CTD; 
using namespace osg; 


//! Implement and register the entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( PhysicsBoxEntityFactory );


EnPhysicsBox::EnPhysicsBox():
_mass( 10.0f ),
_dimensions( Vec3f( 1.0f, 1.0f, 1.0f ) ),
_p_body( NULL )
{
    EntityManager::get()->registerUpdate( this );     // register entity in order to get updated per simulation step

    // register entity attributes
    // note: this must be done in constructor!
    _attributeManager.addAttribute( "meshFile"   , _meshFile   );
    _attributeManager.addAttribute( "position"   , _position   );
    _attributeManager.addAttribute( "mass"       , _mass       );
    _attributeManager.addAttribute( "dimensions" , _dimensions );
}

EnPhysicsBox::~EnPhysicsBox()
{
    // deregister entity, it is not necessary for entities which 'die' at application exit time
    //  as the entity manager clears the entity list on app exit
    EntityManager::get()->registerUpdate( this, false );

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
    addTransformableNode( p_mesh );
    setPosition( _position );

    // create the collision 
    NewtonCollision *p_collision = NewtonCreateBox( Physics::get()->getWorld(), _dimensions._v[ 0 ], _dimensions._v[ 1 ], _dimensions._v[ 2 ], NULL); 
    p_collision = NewtonCreateConvexHullModifier( Physics::get()->getWorld(), p_collision );

    Matrixf mat;
    mat.setTrans( _position ); 

    //create the rigid p_body
    _p_body = NewtonCreateBody( Physics::get()->getWorld(), p_collision );

    // set wood material
    NewtonBodySetMaterialGroupID( _p_body, Physics::get()->getMaterialId( "wood" ) );
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

void EnPhysicsBox::updateEntity( float deltaTime )
{
}
