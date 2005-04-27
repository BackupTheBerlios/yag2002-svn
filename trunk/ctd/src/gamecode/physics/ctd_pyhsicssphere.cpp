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
 # entity PhysicsShpere
 #
 #   date of creation:  02/25/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_base.h>
#include <ctd_physics.h>
#include <ctd_levelmanager.h>
#include "ctd_pyhsicsshpere.h"

using namespace std;
using namespace CTD; 
using namespace osg; 


//! Implement and register the entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( PhysicsShpereEntityFactory );

EnPhysicsShpere::EnPhysicsShpere():
_mass(10.0f),
_radius(2),
_p_body(NULL)
{
    EntityManager::get()->registerUpdate( this );     // register entity in order to get updated per simulation step

    // register entity attributes
    _attributeManager.addAttribute( "meshFile"   , _meshFile   );
    _attributeManager.addAttribute( "position"   , _position   );
    _attributeManager.addAttribute( "mass"       , _mass       );
    _attributeManager.addAttribute( "radius"     , _radius     );
}

EnPhysicsShpere::~EnPhysicsShpere()
{
    // deregister entity, it is not necessary for entities which 'die' at application exit time
    //  as the entity manager clears the entity list on app exit
    EntityManager::get()->registerUpdate( this, false );

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
void EnPhysicsShpere::physicsBodyDestructor( const NewtonBody* body )
{
	EnPhysicsShpere* p_node = ( EnPhysicsShpere* )NewtonBodyGetUserData( body );
    if (p_node)
	    delete( p_node );
}

// transformation callback
void EnPhysicsShpere::physicsSetTransform( const NewtonBody* body, const float* matrix )
{
	EnPhysicsShpere* p_node = static_cast< EnPhysicsShpere* >( NewtonBodyGetUserData( body ) );
    p_node->setPosition( Vec3f( matrix[ 12 ], matrix[ 13 ], matrix[ 14 ] ) );
    Matrixf mat( matrix );
    Quat quat;
    mat.get( quat );
    p_node->setRotation( quat );
}

void EnPhysicsShpere::physicsApplyForceAndTorque( const NewtonBody* body )
{
	float mass;
	float Ixx;
	float Iyy;
	float Izz;

	NewtonBodyGetMassMatrix (body, &mass, &Ixx, &Iyy, &Izz);
    Vec3f force( 0.0f, 0.0f, mass * Physics::get()->getWorldGravity() );
	NewtonBodySetForce( body, &force._v[ 0 ] );
}

void EnPhysicsShpere::initialize()
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
    NewtonCollision *p_collision = NewtonCreateSphere( Physics::get()->getWorld(), _radius, _radius, _radius, NULL ); 
	p_collision = NewtonCreateConvexHullModifier( Physics::get()->getWorld(), p_collision );

    Matrixf mat;
    mat.setTrans( _position ); 

    //create the rigid body
    _p_body = NewtonCreateBody( Physics::get()->getWorld(), p_collision );

    // set metal material
    NewtonBodySetMaterialGroupID( _p_body, Physics::get()->getMaterialId( "grass" ) );

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

    //! TODO remove this later
    Vec3f omega( 0.1f, 0.1f, 0 );
    NewtonBodySetOmega( _p_body, &omega[0] );

}

void EnPhysicsShpere::updateEntity( float deltaTime )
{

}

