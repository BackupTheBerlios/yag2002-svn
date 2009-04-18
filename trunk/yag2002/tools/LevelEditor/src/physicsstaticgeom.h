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
 # entity for creating physics static geometry, used only during 
 #  level creation.
 #
 #   date of creation:  16/04/2009
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#ifndef _PHYSICSSTATICGEOM_H_
#define _PHYSICSSTATICGEOM_H_

#include <vrc_main.h>


#define ENTITY_NAME_PHYSSTATGEOM    "PhysicsStaticGeom"

//! Mesh entity
class EnPhysicsStaticGeom :  public yaf3d::BaseEntity
{
    public:
                                                    EnPhysicsStaticGeom();

        virtual                                     ~EnPhysicsStaticGeom();

        //! Initializing function, this is called after all engine modules are initialized and a map is loaded.
        void                                        initialize();

        //! Enable / disable entity
        void                                        enable( bool en );

        //! Is entity enabled?
        bool                                        isEnabled() const;

        //! Set physics material ID.
        void                                        setMaterialID( int id );

        //! Get physics material ID.
        int                                         getMaterialID() const;

    protected:

        //! Handle entity notifications.
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        //! Setup the mesh
        osg::Node*                                  setupMesh();

        //! Set node transformation.
        void                                        setTransformation();

        //! Entity attributes

        std::string                                 _meshFile;

        osg::Vec3f                                  _position;

        osg::Vec3f                                  _rotation;

        osg::Vec3f                                  _scale;

        bool                                        _enable;

        int                                         _materialID;

        //! Internals

        osg::ref_ptr< osg::Node >                   _mesh;

        std::string                                 _lastMeshFile;
};

//! Entity type definition used for type registry
class PhysicsStaticGeomEntityFactory : public yaf3d::BaseEntityFactory
{
    public:

                                                    PhysicsStaticGeomEntityFactory() :
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_PHYSSTATGEOM, 
                                                         yaf3d::BaseEntityFactory::Standalone | yaf3d::BaseEntityFactory::Client | yaf3d::BaseEntityFactory::Server )
                                                    {}

        virtual                                     ~PhysicsStaticGeomEntityFactory() {}

        Macro_CreateEntity( EnPhysicsStaticGeom );
};

#endif // _PHYSICSSTATICGEOM_H_
