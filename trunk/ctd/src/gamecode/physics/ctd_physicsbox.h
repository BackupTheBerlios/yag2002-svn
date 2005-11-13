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

#include <ctd_main.h>
#include "ctd_physicsbase.h"

namespace vrc
{

#define ENTITY_NAME_PHYSICSBOX    "PhysicsBox"

//! PhysicsBox Entity
class EnPhysicsBox : public EnPhysicsBase
{
    public:
                                                    EnPhysicsBox();

        virtual                                     ~EnPhysicsBox();

        //! Initialize 
        void                                        initialize();

        //! Post-initialize 
        void                                        postInitialize();

        //! Physics system call-back for body destruction
        static void                                 physicsBodyDestructor( const NewtonBody* p_body );

        //! Physics system call-back for body transformation
        static void                                 physicsSetTransform( const NewtonBody* p_body, const float* matrix );

        //! Physics system call-back for applying force to body
        static void                                 physicsApplyForceAndTorque( const NewtonBody* p_body );

    protected:

        //! This entity needs updating
        void                                        updateEntity( float deltaTime );

        //! Init physics materials
        void                                        initializePhysicsMaterials();

        std::string                                 _meshFile;

        float                                       _mass;

        osg::Vec3f                                  _position;

        osg::Vec3f                                  _dimensions;

        NewtonBody*                                 _p_body;

        NewtonWorld*                                _p_world;
};

//! Entity type definition used for type registry
class PhysicsBoxEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    PhysicsBoxEntityFactory() : 
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_PHYSICSBOX, yaf3d::BaseEntityFactory::Standalone | yaf3d::BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~PhysicsBoxEntityFactory() {}

        Macro_CreateEntity( EnPhysicsBox );
};

}

#endif // _CTD_PHYSICSBOX_H_
