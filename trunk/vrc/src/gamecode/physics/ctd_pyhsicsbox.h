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

#ifndef _CTD_PHYSICSBOX_H_
#define _CTD_PHYSICSBOX_H_

#include <ctd_base.h>
#include <ctd_baseentity.h>
#include <ctd_entitymanager.h>

namespace CTD
{

#define ENTITY_NAME_PHYSICSBOX    "PhysicsBox"

//! PhysicsBox Entity
class EnPhysicsBox : public BaseEntity
{
    public:
                                                    EnPhysicsBox();

        virtual                                     ~EnPhysicsBox();

        //! Initialize 
        void                                        initialize();

        //! This entity needs updating
        void                                        updateEntity( float deltaTime );

        //! Physics system call-back for body destruction
        static void                                 physicsBodyDestructor( const NewtonBody* p_body );

        //! Physics system call-back for body transformation
        static void                                 physicsSetTransform( const NewtonBody* p_body, const float* matrix );

        //! Physics system call-back for applying force to body
        static void                                 physicsApplyForceAndTorque( const NewtonBody* p_body );

    protected:

        std::string                                 _meshFile;

        float                                       _mass;

        osg::Vec3f                                  _position;

        osg::Vec3f                                  _dimensions;

        NewtonBody*                                 _p_body;
};

//! Entity type definition used for type registry
class PhysicsBoxEntityFactory : public BaseEntityFactory
{
    public:
                                                    PhysicsBoxEntityFactory() : BaseEntityFactory(ENTITY_NAME_PHYSICSBOX) {}

        virtual                                     ~PhysicsBoxEntityFactory() {}

        Macro_CreateEntity( EnPhysicsBox );
};

}

#endif // _CTD_PHYSICSBOX_H_
