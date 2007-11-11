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
 # some base functionalities are gathered here useful for physics
 #  entities
 #
 #   date of creation:  04/25/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_physicsbase.h"
#include "../sound/vrc_3dsound.h"

namespace vrc
{

// Collision struct for internal usage
yaf3d::CollisionStruct* s_entityColStruct = NULL;

std::map< std::string, bool > s_entityMaterialInits;

EnPhysicsBase::EnPhysicsBase() :
_playThreshold( 10.0f ),
_pastTime( 0 )
{
    // register entity in order to get notifications about building physics
    yaf3d::EntityManager::get()->registerNotification( this, true );
}

// catch the physics notifications
void EnPhysicsBase::handleNotification( const yaf3d::EntityNotification& notification )
{
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_BUILDING_PHYSICSWORLD:

            // setup materials only once per entity type
            if ( !s_entityMaterialInits[ getTypeName() ] )
            {
                s_entityMaterialInits[ getTypeName() ] = true;
                initializePhysicsMaterials();
            }
            break;

        case YAF3D_NOTIFY_DELETING_PHYSICSWORLD:

            // clear map
            s_entityMaterialInits.clear();
            break;

        default:
            ;

    }
}

void EnPhysicsBase::playSoundColMetal()
{
    // avoid short intervalls for playing as several collisions can occur in smal amount of time
    if ( _pastTime > 0.5f )
    {
        _pastTime = 0;
        if ( _pp_sounds[ 0 ] )
            _pp_sounds[ 0 ]->startPlaying();
    }
}

void EnPhysicsBase::playSoundColWood()
{
    if ( _pastTime > 0.5f )
    {
        _pastTime = 0;
        if ( _pp_sounds[ 1 ] )
            _pp_sounds[ 1 ]->startPlaying();
    }
}

void EnPhysicsBase::playSoundColStone()
{
    if ( _pastTime > 0.5f )
    {
        _pastTime = 0;
        if ( _pp_sounds[ 2 ] )
            _pp_sounds[ 2 ]->startPlaying();
    }
}

void EnPhysicsBase::playSoundColGrass()
{
    if ( _pastTime > 0.5f )
    {
        _pastTime = 0;
        if ( _pp_sounds[ 3 ] )
            _pp_sounds[ 3 ]->startPlaying();
    }
}

En3DSound* EnPhysicsBase::getSoundEntity( const std::string& name )
{
    En3DSound* _p_sndEntity = NULL;
    _p_sndEntity = dynamic_cast< En3DSound* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_3DSOUND, name ) );
    if ( !_p_sndEntity )
    {
        log_warning << "*** entity '" << name << "' is not of type 3DSound or does not exist!" << std::endl;
    }
    return _p_sndEntity;
}

int entityContactBegin( const NewtonMaterial* p_material, const NewtonBody* p_body0, const NewtonBody* p_body1 )
{
    // get the pointer to collision struture
    s_entityColStruct = static_cast< yaf3d::CollisionStruct* >( NewtonMaterialGetMaterialPairUserData( p_material ) );
    // save the colliding bodies
    s_entityColStruct->_p_body0 = const_cast< NewtonBody* >( p_body0 );
    s_entityColStruct->_p_body1 = const_cast< NewtonBody* >( p_body1 );
    // clear the contact normal speed
    s_entityColStruct->_contactMaxNormalSpeed  = 0.0f;
    // clear the contact sliding speed
    s_entityColStruct->_contactMaxTangentSpeed = 0.0f;

    // set also Physics' col struct as we may need some useful Physics' callbacks during contanct processing
    // see implementation of "entityContactProcessLevel"
    yaf3d::Physics::setCollisionStruct( s_entityColStruct );

    return 1;
}

//! This function is called affter all collision contacts are proccesed
void entityContactEnd( const NewtonMaterial* /*p_material*/ )
{
}

} // namespace vrc
