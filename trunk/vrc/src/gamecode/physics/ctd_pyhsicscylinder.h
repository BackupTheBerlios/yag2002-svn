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

#ifndef _CTD_PHYSICSCYLINDER_H_
#define _CTD_PHYSICSCYLINDER_H_

#include <ctd_base.h>
#include <ctd_baseentity.h>
#include <ctd_entitymanager.h>

namespace CTD
{

#define ENTITY_NAME_PHYSICSCYLINDER    "PhysicsCylinder"

class En3DSound;

//! PhysicsShpere Entity
class EnPhysicsCylinder : public BaseEntity
{
    public:
                                                    EnPhysicsCylinder();

        virtual                                     ~EnPhysicsCylinder();

        //! Initialize 
        void                                        initialize();

        //! Post-initialize 
        void                                        postInitialize();

        //! This entity needs updating
        void                                        updateEntity( float deltaTime );

        //! Physics system call-back for body destruction
        static void                                 physicsBodyDestructor( const NewtonBody* body );

        //! Physics system call-back for body transformation
        static void                                 physicsSetTransform( const NewtonBody* body, const float* matrix );

        //! Physics system call-back for applying force to body
        static void                                 physicsApplyForceAndTorque( const NewtonBody* body );

        //! Play sound for collision with metal
        void                                        playSoundColMetal();

        //! Play sound for collision with wood
        void                                        playSoundColWood();

        //! Play sound for collision with stone
        void                                        playSoundColStone();

        //! Play sound for collision with grass
        void                                        playSoundColGrass();

    protected:

        // entity attributes
        //--------------------------------------------------------//
        std::string                                 _meshFile;

        float                                       _mass;

        osg::Vec3f                                  _position;

        float                                       _radius;

        float                                       _height;

        //! 4 Sound entities can be used for playing different sounds on collisions
        std::string                                 _soundEntities[ 4 ];
        //--------------------------------------------------------//

    protected:

        // helper method for finding sound entities
        En3DSound*                                  getSoundEntity( const std::string& name );

        //! Sound entities in the same order as in attribute registration
        En3DSound*                                  _pp_sounds[ 4 ];

        NewtonBody*                                 _p_body;

        NewtonWorld*                                _p_world;

        float                                       _pastTime;

    public:

        // contact information
        float                                       _contactMaxNormalSpeed;

        float                                       _contactMaxTangentSpeed;

        //!Note: entity attribute declared as public for faster access in static functions
        // Threshold for normal speed for playing a sound in case of a collision
        float                                       _playThreshold;
};

//! Entity type definition used for type registry
class PhysicsCylinderEntityFactory : public BaseEntityFactory
{
    public:
                                                    PhysicsCylinderEntityFactory() : BaseEntityFactory(ENTITY_NAME_PHYSICSCYLINDER) {}

        virtual                                     ~PhysicsCylinderEntityFactory() {}

        Macro_CreateEntity( EnPhysicsCylinder );
};

}

#endif // _CTD_PHYSICSCYLINDER_H_
