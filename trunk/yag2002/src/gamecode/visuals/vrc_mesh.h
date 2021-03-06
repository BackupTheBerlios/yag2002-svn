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
 # mesh entity
 #
 #   date of creation:  04/05/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#ifndef _VRC_MESH_H_
#define _VRC_MESH_H_

#include <vrc_main.h>

namespace vrc
{

#define ENTITY_NAME_MESH    "Mesh"

//! Mesh entity
class EnMesh :  public yaf3d::BaseEntity
{
    public:
                                                    EnMesh();

        virtual                                     ~EnMesh();

        //! Initializing function, this is called after all engine modules are initialized and a map is loaded.
        void                                        initialize();

        //! If this entity is used in menu system then we want it to be persistent
        bool                                        isPersistent() const { return _usedInMenu; }

        //! Enable / disable mesh rendering
        void                                        enable( bool en );

    protected:

        //! Update entity
        void                                        updateEntity( float deltaTime );

        //! Handle system notifications
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        //! Setup a node using LOD
        osg::Node*                                  setupLODObject( osg::Node* p_node );

        //! Setup the mesh
        osg::Node*                                  setupMesh();

        //! Add transformation node to scenegraph
        void                                        addToSceneGraph();

        //! Remove transformation node from scenegraph
        void                                        removeFromSceneGraph();

        //! Entity attributes

        std::string                                 _meshFile;

        std::string                                 _shaderName;

        osg::Vec3f                                  _position;

        osg::Vec3f                                  _rotation;

        osg::Vec3f                                  _scale;

        bool                                        _enable;

        bool                                        _usedInMenu;

        bool                                        _throwShadow;

        bool                                        _receiveShadow;

        bool                                        _useLOD;

        float                                       _lodErrorThreshold;

        //! Internal attributes

        bool                                        _cgfShadow;

        bool                                        _shadowEnable;

        float                                       _shadowCullDist;

        std::string                                 _animFile;

        bool                                        _animPosition;

        bool                                        _animRotation;

        bool                                        _animRelative;

        float                                       _animDelay;

        float                                       _animTimeScale;

        bool                                        _animLoop;

        float                                       _animTime;

        osg::Vec3f                                  _orgPostition;

        osg::Quat                                   _orgRotation;

        osg::ref_ptr< osg::AnimationPath >          _animPath;

        osg::ref_ptr< osg::Node >                   _mesh;
};

//! Entity type definition used for type registry
class MeshEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    MeshEntityFactory() :
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_MESH, yaf3d::BaseEntityFactory::Standalone | yaf3d::BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~MeshEntityFactory() {}

        Macro_CreateEntity( EnMesh );
};

} // namespace vrc

#endif // _VRC_MESH_H_
