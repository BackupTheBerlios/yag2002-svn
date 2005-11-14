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
 # mesh entity
 #
 #   date of creation:  04/05/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_MESH_H_
#define _CTD_MESH_H_

#include <ctd_main.h>

namespace CTD
{

#define ENTITY_NAME_MESH    "Mesh"

//! Mesh entity
class EnMesh :  public BaseEntity
{
    public:
                                                    EnMesh();

        virtual                                     ~EnMesh();

        //! Initializing function, this is called after all engine modules are initialized and a map is loaded.
        void                                        initialize();

        //! Enable / disable mesh rendering
        void                                        enable( bool en );

    protected:

        std::string                                 _meshFile;

        osg::Vec3f                                  _position;

        osg::Vec3f                                  _rotation;

    protected:

        osg::ref_ptr< osg::Node >                   _mesh;

        bool                                        _enable;
};

//! Entity type definition used for type registry
class MeshEntityFactory : public BaseEntityFactory
{
    public:
                                                    MeshEntityFactory() : 
                                                     BaseEntityFactory( ENTITY_NAME_MESH, BaseEntityFactory::Standalone | BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~MeshEntityFactory() {}

        Macro_CreateEntity( EnMesh );
};

} // namespace CTD

#endif // _CTD_MESH_H_
