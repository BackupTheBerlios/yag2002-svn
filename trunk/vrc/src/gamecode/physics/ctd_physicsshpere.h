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
 # entity PhysicsSphere
 #
 #   date of creation:  02/25/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_PHYSICSSPHERE_H_
#define _CTD_PHYSICSSPHERE_H_

#include <ctd_base.h>
#include <ctd_baseentity.h>
#include <ctd_entitymanager.h>
#include "ctd_physicsbase.h"

namespace CTD
{

#define ENTITY_NAME_PHYSICSSPHERE    "PhysicsSphere"

//! PhysicsShpere Entity
class EnPhysicsSphere : public BaseEntity, public PhysicsSound
{
    public:
                                                    EnPhysicsSphere();

        virtual                                     ~EnPhysicsSphere();

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

    protected:

        // Entity attributes

        std::string                                 _meshFile;

        float                                       _mass;

        osg::Vec3f                                  _position;

        float                                       _radius;

    protected:

        // Some internal variables

        NewtonBody*                                 _p_body;

        NewtonWorld*                                _p_world;
};

//! Entity type definition used for type registry
class PhysicsSphereEntityFactory : public BaseEntityFactory
{
    public:
                                                    PhysicsSphereEntityFactory() : BaseEntityFactory(ENTITY_NAME_PHYSICSSPHERE) {}

        virtual                                     ~PhysicsSphereEntityFactory() {}

        Macro_CreateEntity( EnPhysicsSphere );
};

}

#endif // _CTD_PHYSICSSPHERE_H_