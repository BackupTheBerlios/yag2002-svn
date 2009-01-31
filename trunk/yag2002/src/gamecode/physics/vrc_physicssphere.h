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
 # entity PhysicsSphere
 #
 #   date of creation:  02/25/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _VRC_PHYSICSSPHERE_H_
#define _VRC_PHYSICSSPHERE_H_

#include <vrc_main.h>
#include "vrc_physicsbase.h"

namespace vrc
{

#define ENTITY_NAME_PHYSICSSPHERE    "PhysicsSphere"

//! PhysicsShpere Entity
class EnPhysicsSphere : public EnPhysicsBase
{
    public:
                                                    EnPhysicsSphere();

        virtual                                     ~EnPhysicsSphere();

        //! Initialize 
        void                                        initialize();

        //! Post-initialize 
        void                                        postInitialize();

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

        //! This entity needs updating
        void                                        updateEntity( float deltaTime );

        //! Init physics materials
        void                                        initializePhysicsMaterials();

        // Some internal variables

        NewtonBody*                                 _p_body;

        NewtonWorld*                                _p_world;
};

//! Entity type definition used for type registry
class PhysicsSphereEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    PhysicsSphereEntityFactory() : 
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_PHYSICSSPHERE,
                                                                               yaf3d::BaseEntityFactory::Standalone | yaf3d::BaseEntityFactory::Client,
                                                                               yaf3d::BaseEntityFactory::OnLoadingLevel
                                                                              )
                                                    {}

        virtual                                     ~PhysicsSphereEntityFactory() {}

        Macro_CreateEntity( EnPhysicsSphere );
};

}

#endif // _VRC_PHYSICSSPHERE_H_
