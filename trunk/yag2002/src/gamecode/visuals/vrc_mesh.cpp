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

#include <vrc_main.h>
#include "vrc_mesh.h"

namespace vrc
{

//! Implement and register the mesh entity factory
YAF3D_IMPL_ENTITYFACTORY( MeshEntityFactory );

EnMesh::EnMesh() :
_enable( true ),
_usedInMenu( false )
{
    // register entity attributes
    getAttributeManager().addAttribute( "usedInMenu"    , _usedInMenu );
    getAttributeManager().addAttribute( "enable"        , _enable     );
    getAttributeManager().addAttribute( "meshFile"      , _meshFile   );
    getAttributeManager().addAttribute( "position"      , _position   );
    getAttributeManager().addAttribute( "rotation"      , _rotation   );
}

EnMesh::~EnMesh()
{
}

void EnMesh::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle notifications, add and remove the mesh to / from scenegraph on menu entring / leaving
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_MENU_ENTER:
        {
            if ( _enable )
            {
                if ( _usedInMenu )
                    addToTransformationNode( _mesh.get() );
                else
                    removeFromTransformationNode( _mesh.get() );
            }
        }
        break;

        case YAF3D_NOTIFY_MENU_LEAVE:
        {
            if ( _enable )
            {
                if ( _usedInMenu )
                    removeFromTransformationNode( _mesh.get() );
                else
                    addToTransformationNode( _mesh.get() );
            }
        }
        break;

        case YAF3D_NOTIFY_ENTITY_ATTRIBUTE_CHANGED:
        {         
            removeFromTransformationNode( _mesh.get() );
            // re-setup mesh
            _mesh = setupMesh();
            if ( _mesh.valid() && _enable )
                addToTransformationNode( _mesh.get() );
        }
        break;

        // if used in menu then this entity is persisten, so we have to trigger its deletion on shutdown
        case YAF3D_NOTIFY_SHUTDOWN:
        {
            if ( _usedInMenu )
                yaf3d::EntityManager::get()->deleteEntity( this );
        }
        break;

        default:
            ;
    }
}

void EnMesh::initialize()
{
    _mesh = setupMesh();

    if ( _usedInMenu && _mesh.valid() )
        // we may use this entity also in menu loader, so add it to scenegraph in this case during initialization
        addToTransformationNode( _mesh.get() );

    // register entity in order to get menu notifications
    yaf3d::EntityManager::get()->registerNotification( this, true );    
}

osg::ref_ptr< osg::Node > EnMesh::setupMesh()
{
    osg::Node* p_node = yaf3d::LevelManager::get()->loadMesh( _meshFile );
    if ( !p_node )
    {
        log_error << "*** could not load mesh file: " << _meshFile << ", in '" << getInstanceName() << "'" << std::endl;
        return NULL;
    }

    setPosition( _position );
    osg::Quat   rot( 
                     osg::DegreesToRadians( _rotation.x() ), osg::Vec3f( 1.0f, 0.0f, 0.0f ),
                     osg::DegreesToRadians( _rotation.y() ), osg::Vec3f( 0.0f, 1.0f, 0.0f ),
                     osg::DegreesToRadians( _rotation.z() ), osg::Vec3f( 0.0f, 0.0f, 1.0f )
                    );
    setRotation( rot );

    return p_node;
}

void EnMesh::enable( bool en )
{
    if ( _enable == en )
        return;

    if ( _mesh.valid() )
    {
        if ( en )
        {
            addToTransformationNode( _mesh.get() );
        }
        else
        {
            removeFromTransformationNode( _mesh.get() );
        }

        _enable = en;
    }
    else
    {
        _enable = false;
    }
}

} // namespace vrc
