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
 # some base functionalities are gathered here useful for physics
 #  entities
 #
 #   date of creation:  04/25/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_PHYSICSBASE_H_
#define _CTD_PHYSICSBASE_H_

#include <ctd_main.h>

namespace CTD
{

class En3DSound;

//! Base class for physics entities which need sound
class PhysicsSound
{
    public:
                                                    PhysicsSound() :
                                                     _playThreshold( 10.0f ),
                                                     _pastTime( 0 )
                                                    {}

                                                    ~PhysicsSound() {}

        //! Update sound related stuff
        inline void                                 updateSound( float deltaTime );

        //! Play sound for collision with metal
        virtual void                                playSoundColMetal();

        //! Play sound for collision with wood
        virtual void                                playSoundColWood();

        //! Play sound for collision with stone
        virtual void                                playSoundColStone();

        //! Play sound for collision with grass
        virtual void                                playSoundColGrass();

        //! Threshold for normal speed for playing a sound in case of a collision
        float                                       _playThreshold;

        //! Contact's normal speed
        float                                       _contactMaxNormalSpeed;

        //! Contact tanget speed
        float                                       _contactMaxTangentSpeed;

    protected:

        //! Helper method for finding sound entities
        En3DSound*                                  getSoundEntity( const std::string& name );

        //! 4 Sound entities can be used for playing different sounds on collisions
        std::string                                 _soundEntities[ 4 ];

        //! Sound entities in the same order as in attribute registration
        En3DSound*                                  _pp_sounds[ 4 ];

        //! Past time since last start of playing a sound
        float                                       _pastTime;
};

// Collision struct for internal usage
extern CollisionStruct* s_entityColStruct;

//! Begin contact processing callback
int entityContactBegin( const NewtonMaterial* p_material, const NewtonBody* p_body0, const NewtonBody* p_body1 );

//! Contact process callback function called when the body collides with level or other entities
template< class EntityT >
int entityContactProcessLevel( const NewtonMaterial* p_material, const NewtonContact* p_contact );

//! This function is called affter all collision contacts are proccesed
void entityContactEnd( const NewtonMaterial* p_material );

} // namespace CTD

#include "ctd_physicsbase.inl"

#endif // _CTD_PHYSICSBASE_H_
