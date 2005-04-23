/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2004-2006, Ali Botorabi
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
 # entity PhysicsCylinder
 #
 #   date of creation:  02/25/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_base.h>
#include <ctd_physics.h>
#include <ctd_log.h>
#include <ctd_levelmanager.h>
#include "ctd_pyhsicscylinder.h"
#include "../sound/ctd_3dsound.h"

using namespace std;
using namespace CTD; 
using namespace osg; 


//! Implement and register the entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( PhysicsCylinderEntityFactory );


struct CylinderCollisionStruct
{
    NewtonBody*      _p_body0;

    NewtonBody*      _p_body1;
        
    float            _contactMaxNormalSpeed;
    
    float            _contactMaxTangentSpeed;
};

// Cylinder to other materials collision struct
CylinderCollisionStruct cylinderCollStruct;
static CylinderCollisionStruct* s_colStruct = &cylinderCollStruct;

// begin contact processing
int cylinderContactBegin( const NewtonMaterial* p_material, const NewtonBody* p_body0, const NewtonBody* p_body1 )
{
    // get the pointer to collision struture
    s_colStruct = static_cast< CylinderCollisionStruct* >( NewtonMaterialGetMaterialPairUserData( p_material ) );
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

// contact process callback function called when the cylinder collides with level or player
int cylinderContactProcessLevel( const NewtonMaterial* p_material, const NewtonContact* p_contact )
{
    // set right parameters for predfined materials
    Physics::levelContactProcess( p_material, p_contact );

    // determine which body is the cylinder entity
	NewtonBody*        p_body = s_colStruct->_p_body1;
    EnPhysicsCylinder* p_cyl  = NULL;
    void*              p_userData = NewtonBodyGetUserData( p_body );
    if ( p_userData ) // check for entity collision
    {
        BaseEntity* p_entity = reinterpret_cast< BaseEntity* >( p_userData );
        p_cyl  = dynamic_cast< EnPhysicsCylinder* >( p_entity );
        if ( !p_cyl )
        {
            p_cyl = static_cast< EnPhysicsCylinder* >( NewtonBodyGetUserData( s_colStruct->_p_body0 ) );
        }

	} 
    else // level collision
    {
        p_cyl = static_cast< EnPhysicsCylinder* >( NewtonBodyGetUserData( s_colStruct->_p_body0 ) );
    }

    // let the phyiscs core calculate the tangent and normal speeds
    Physics::genericContactProcess( p_material, p_contact );
    CollisionStruct* p_colStruct = Physics::getCollisionStruct();
    p_cyl->_contactMaxNormalSpeed  = p_colStruct->_contactMaxNormalSpeed;
    p_cyl->_contactMaxTangentSpeed = p_colStruct->_contactMaxTangentSpeed;

    // play appropriate sound only if the cylinder bumps to something
    if ( p_colStruct->_contactMaxNormalSpeed > p_cyl->_playThreshold )
    {
        unsigned int attribute = ( unsigned int )( NewtonMaterialGetContactFaceAttribute( p_material ) );
        unsigned int materialType = attribute & 0xFF;
        switch ( materialType )
        {
            case 0: // only entities have face attribute 0
                // for a sofisticated design here a sound callback function of the colising entity could be called.
                //  but for now we play the default sound.
                p_cyl->playSoundColMetal();
                break;

            case Physics::MAT_DEFAULT:
            case Physics::MAT_METALL:
                p_cyl->playSoundColMetal();
                break;

            case Physics::MAT_WOOD:
                p_cyl->playSoundColWood();
                break;

            case Physics::MAT_STONE:
                p_cyl->playSoundColStone();
                break;

            case Physics::MAT_GRASS:
                p_cyl->playSoundColGrass();
                break;

            default:
                p_cyl->playSoundColMetal();
        }
    }

    return 1;
}

// this function is called affter all collision contacts are proccesed
void cylinderContactEnd( const NewtonMaterial* p_material )
{
}

EnPhysicsCylinder::EnPhysicsCylinder():
_mass(10.0f),
_radius(2.0f),
_height(4.0f),
_playThreshold( 10.0f ),
_pastTime( 0 ),
_p_body(NULL),
_p_world( Physics::get()->getWorld() )
{
    EntityManager::get()->registerUpdate( this );     // register entity in order to get updated per simulation step

    // register entity attributes
    _attributeManager.addAttribute( "meshFile"      , _meshFile      );
    _attributeManager.addAttribute( "position"      , _position      );
    _attributeManager.addAttribute( "mass"          , _mass          );
    _attributeManager.addAttribute( "radius"        , _radius        );
    _attributeManager.addAttribute( "height"        , _height        );
    _attributeManager.addAttribute( "playThreshold" , _playThreshold );
    // 3d sound entities
    _attributeManager.addAttribute( "enSndWood"  , _soundEntities[ 0 ]  );
    _attributeManager.addAttribute( "enSndMetal" , _soundEntities[ 1 ]  );
    _attributeManager.addAttribute( "enSndStone" , _soundEntities[ 2 ]  );
    _attributeManager.addAttribute( "enSndGrass" , _soundEntities[ 3 ]  );

    // the materials _must_ be created only once!
    static cylinderMatrialCreated = false;
    if ( !cylinderMatrialCreated )
    {
        cylinderMatrialCreated = true;

        // create a collision callbacks for player object and other materials
        //  it is important that own materials are created in constructor!
        //-------------------------------------------------------------------
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
        NewtonMaterialSetCollisionCallback( _p_world, cylinderID, cylinderID, s_colStruct, cylinderContactBegin, cylinderContactProcessLevel, cylinderContactEnd ); 

        // set the material properties for cylinder on default
        NewtonMaterialSetDefaultElasticity( _p_world, cylinderID, defaultID, 0.3f );
        NewtonMaterialSetDefaultSoftness( _p_world, cylinderID, defaultID, 0.8f );
        NewtonMaterialSetDefaultFriction( _p_world, cylinderID, defaultID, 0.8f, 0.7f );
        NewtonMaterialSetCollisionCallback( _p_world, cylinderID, defaultID, s_colStruct, cylinderContactBegin, cylinderContactProcessLevel, cylinderContactEnd ); 

        // set the material properties for cylinder on level
        NewtonMaterialSetDefaultElasticity( _p_world, cylinderID, levelID, 0.3f );
        NewtonMaterialSetDefaultSoftness( _p_world, cylinderID, levelID, 0.8f );
        NewtonMaterialSetDefaultFriction( _p_world, cylinderID, levelID, 0.8f, 0.7f );
        NewtonMaterialSetCollisionCallback( _p_world, cylinderID, levelID, s_colStruct, cylinderContactBegin, cylinderContactProcessLevel, cylinderContactEnd ); 

        // set the material properties for cylinder on wood
        NewtonMaterialSetDefaultElasticity( _p_world, cylinderID, woodID, 0.5f );
        NewtonMaterialSetDefaultSoftness( _p_world, cylinderID, woodID, 0.5f );
        NewtonMaterialSetDefaultFriction( _p_world, cylinderID, woodID, 0.6f, 0.4f);
        NewtonMaterialSetCollisionCallback( _p_world, cylinderID, woodID, s_colStruct, cylinderContactBegin, cylinderContactProcessLevel, cylinderContactEnd ); 

        // set the material properties for cylinder on metal
        NewtonMaterialSetDefaultElasticity( _p_world, cylinderID, metalID, 0.7f );
        NewtonMaterialSetDefaultSoftness( _p_world, cylinderID, metalID, 0.9f );
        NewtonMaterialSetDefaultFriction( _p_world, cylinderID, metalID, 0.8f, 0.6f );
        NewtonMaterialSetCollisionCallback( _p_world, cylinderID, metalID, s_colStruct, cylinderContactBegin, cylinderContactProcessLevel, cylinderContactEnd ); 

        // set the material properties for cylinder on grass
        NewtonMaterialSetDefaultElasticity( _p_world, cylinderID, grassID, 0.2f );
        NewtonMaterialSetDefaultSoftness( _p_world, cylinderID, grassID, 0.3f );
        NewtonMaterialSetDefaultFriction( _p_world, cylinderID, grassID, 0.8f, 0.7f );
        NewtonMaterialSetCollisionCallback( _p_world, cylinderID, grassID, s_colStruct, cylinderContactBegin, cylinderContactProcessLevel, cylinderContactEnd ); 

        // set the material properties for cylinder on stone
        NewtonMaterialSetDefaultElasticity( _p_world, cylinderID, stoneID, 0.45f );
        NewtonMaterialSetDefaultSoftness( _p_world, cylinderID, stoneID, 0.9f );
        NewtonMaterialSetDefaultFriction( _p_world, cylinderID, stoneID, 0.9f, 0.7f );
        NewtonMaterialSetCollisionCallback( _p_world, cylinderID, stoneID, s_colStruct, cylinderContactBegin, cylinderContactProcessLevel, cylinderContactEnd ); 
    }
}

EnPhysicsCylinder::~EnPhysicsCylinder()
{
    // deregister entity, it is not necessary for entities which 'die' at application exit time or which are removed by the 
    // the mean of entity manager's deleteEntity method, as the entity manager clears the entity list on app exit
    EntityManager::get()->registerUpdate( this, false );

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
        log << Log::LogLevel( Log::L_ERROR ) << "*** error loading mesh file" << endl;
        activate( false );
        return;
    }
    // now we add the new mesh into our transformable scene group
    addToTransformationNode( p_mesh );
    setPosition( _position );

    // create the collision
    NewtonCollision *p_collision = NewtonCreateCylinder( Physics::get()->getWorld(), _radius, _height, NULL ); 
	p_collision = NewtonCreateConvexHullModifier( Physics::get()->getWorld(), p_collision );

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
    //! TODO: add sound resources
    static int autoindex = 0;
    char buf[ 8 ];

    // find all sound entities and clone them
    _pp_sounds[ 0 ] = NULL;
    if ( _soundEntities[ 0 ].length() )
    {
        _pp_sounds[ 0 ] = getSoundEntity( _soundEntities[ 0 ] );
        _pp_sounds[ 0 ] = static_cast< En3DSound* >( _pp_sounds[ 0 ]->cloneAndInitialize( getInstanceName() + itoa( autoindex++, buf, 10 ) ) );
    }
    else
    {
        log << Log::LogLevel( Log::L_WARNING ) << "* wood sound is not defined. it will be disabled " << endl;
    }

    _pp_sounds[ 1 ] = NULL;
    if ( _soundEntities[ 1 ].length() )
    {
        _pp_sounds[ 1 ] = getSoundEntity( _soundEntities[ 1 ] );
        _pp_sounds[ 1 ] = static_cast< En3DSound* >( _pp_sounds[ 1 ]->cloneAndInitialize( getInstanceName() + itoa( autoindex++, buf, 10 ) ) );
    }
    else
    {
        log << Log::LogLevel( Log::L_WARNING ) << "* metal sound is not defined. it will be disabled " << endl;
    }

    _pp_sounds[ 2 ] = NULL;
    if ( _soundEntities[ 2 ].length() )
    {
        _pp_sounds[ 2 ] = getSoundEntity( _soundEntities[ 2 ] );
        _pp_sounds[ 2 ] = static_cast< En3DSound* >( _pp_sounds[ 2 ]->cloneAndInitialize( getInstanceName() + itoa( autoindex++, buf, 10 ) ) );
    }
    else
    {
        log << Log::LogLevel( Log::L_WARNING ) << "* stone sound is not defined. it will be disabled " << endl;
    }

    _pp_sounds[ 3 ] = NULL;
    if ( _soundEntities[ 3 ].length() )
    {
        _pp_sounds[ 3 ] = getSoundEntity( _soundEntities[ 3 ] );
        _pp_sounds[ 3 ] = static_cast< En3DSound* >( _pp_sounds[ 3 ]->cloneAndInitialize( getInstanceName() + itoa( autoindex++, buf, 10 ) ) );
    }
    else
    {
        log << Log::LogLevel( Log::L_WARNING ) << "* grass sound is not defined. it will be disabled " << endl;
    }

}

En3DSound* EnPhysicsCylinder::getSoundEntity( const string& name )
{
    En3DSound* _p_sndEntity = NULL;
    if ( _soundEntities[ 0 ].length() )
    {
        _p_sndEntity = dynamic_cast< En3DSound* >( EntityManager::get()->findEntity( ENTITY_NAME_3DSOUND, name ) );
        if ( !_p_sndEntity )
        {
            log << Log::LogLevel( Log::L_WARNING ) << "*** entity '" << name << "' is not of type 3DSound!" << endl;
        }
    }
    else
    {
        log << Log::LogLevel( Log::L_WARNING ) << "* sound '" << name << "' is not defined. it will be disabled " << endl;
    }
    return _p_sndEntity;
}

void EnPhysicsCylinder::updateEntity( float deltaTime )
{
    _pastTime += deltaTime;
}

//! TODO: sound playing methods
// evaluate: play sound depending on normal and tangent speeds!?

void EnPhysicsCylinder::playSoundColMetal()
{
    // avoid short intervalls for playing as several collisions can some in smal amount of time
    if ( _pastTime > 0.5f )
    {
        _pastTime = 0;
        if ( _pp_sounds[ 0 ] )
            _pp_sounds[ 0 ]->startPlaying();
    }
}

void EnPhysicsCylinder::playSoundColWood()
{
    if ( _pastTime > 0.5f )
    {
        _pastTime = 0;
        if ( _pp_sounds[ 1 ] )
            _pp_sounds[ 1 ]->startPlaying();
    }
}

void EnPhysicsCylinder::playSoundColStone()
{
    if ( _pastTime > 0.5f )
    {
        _pastTime = 0;
        if ( _pp_sounds[ 2 ] )
            _pp_sounds[ 2 ]->startPlaying();
    }
}

void EnPhysicsCylinder::playSoundColGrass()
{
    if ( _pastTime > 0.5f )
    {
        _pastTime = 0;
        if ( _pp_sounds[ 3 ] )
            _pp_sounds[ 3 ]->startPlaying();
    }
}
