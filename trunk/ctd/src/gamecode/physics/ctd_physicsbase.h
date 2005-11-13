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

#ifndef _CTD_PHYSICSBASE_H_
#define _CTD_PHYSICSBASE_H_

#include <ctd_main.h>

namespace vrc
{

class En3DSound;

//! Base class for physics entities
class EnPhysicsBase : public yaf3d::BaseEntity
{
    public:
                                                    EnPhysicsBase();

                                                    ~EnPhysicsBase() {}

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

        //! Catch physics notifications
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        //! This method is called on derived classed when a build physics world notification is received
        /**
        * The physics materials must be re-created and initialized during every level loading.
        */
        virtual void                                initializePhysicsMaterials() = 0;

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
extern yaf3d::CollisionStruct* s_entityColStruct;

//! Begin contact processing callback
int entityContactBegin( const NewtonMaterial* p_material, const NewtonBody* p_body0, const NewtonBody* p_body1 );

//! Contact process callback function called when the body collides with level or other entities
template< class EntityT >
int entityContactProcessLevel( const NewtonMaterial* p_material, const NewtonContact* p_contact );

//! This function is called affter all collision contacts are proccesed
void entityContactEnd( const NewtonMaterial* p_material );

} // namespace vrc

#include "ctd_physicsbase.inl"

#endif // _CTD_PHYSICSBASE_H_
