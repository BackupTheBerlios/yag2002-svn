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
 # some base functionalities are gathered here useful for physics
 #  entities
 #
 #   date of creation:  04/25/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_physicsbase.h"
#include "../sound/ctd_3dsound.h"

using namespace std;
namespace CTD
{
// Collision struct for internal usage
CollisionStruct* s_entityColStruct = NULL;

void PhysicsSound::playSoundColMetal()
{
    // avoid short intervalls for playing as several collisions can occur in smal amount of time
    if ( _pastTime > 0.5f )
    {
        _pastTime = 0;
        if ( _pp_sounds[ 0 ] )
            _pp_sounds[ 0 ]->startPlaying();
    }
}

void PhysicsSound::playSoundColWood()
{
    if ( _pastTime > 0.5f )
    {
        _pastTime = 0;
        if ( _pp_sounds[ 1 ] )
            _pp_sounds[ 1 ]->startPlaying();
    }
}

void PhysicsSound::playSoundColStone()
{
    if ( _pastTime > 0.5f )
    {
        _pastTime = 0;
        if ( _pp_sounds[ 2 ] )
            _pp_sounds[ 2 ]->startPlaying();
    }
}

void PhysicsSound::playSoundColGrass()
{
    if ( _pastTime > 0.5f )
    {
        _pastTime = 0;
        if ( _pp_sounds[ 3 ] )
            _pp_sounds[ 3 ]->startPlaying();
    }
}

En3DSound* PhysicsSound::getSoundEntity( const string& name )
{
    En3DSound* _p_sndEntity = NULL;
    _p_sndEntity = dynamic_cast< En3DSound* >( EntityManager::get()->findEntity( ENTITY_NAME_3DSOUND, name ) );
    if ( !_p_sndEntity )
    {
        log << Log::LogLevel( Log::L_WARNING ) << "*** entity '" << name << "' is not of type 3DSound or does not exist!" << endl;
    }
    return _p_sndEntity;
}

int entityContactBegin( const NewtonMaterial* p_material, const NewtonBody* p_body0, const NewtonBody* p_body1 )
{
    // get the pointer to collision struture
    s_entityColStruct = static_cast< CollisionStruct* >( NewtonMaterialGetMaterialPairUserData( p_material ) );
    // save the colliding bodies
    s_entityColStruct->_p_body0 = const_cast< NewtonBody* >( p_body0 );
    s_entityColStruct->_p_body1 = const_cast< NewtonBody* >( p_body1 );
    // clear the contact normal speed 
    s_entityColStruct->_contactMaxNormalSpeed  = 0.0f;
    // clear the contact sliding speed 
    s_entityColStruct->_contactMaxTangentSpeed = 0.0f;

    // set also Physics' col struct as we may need some useful Physics' callbacks during contanct processing
    // see implementation of "entityContactProcessLevel"
    Physics::setCollisionStruct( s_entityColStruct );

    return 1;
} 

//! This function is called affter all collision contacts are proccesed
void entityContactEnd( const NewtonMaterial* p_material )
{
}

} // namespace CTD
