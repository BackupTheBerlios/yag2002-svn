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
_enable( true )
{
    // register entity attributes
    getAttributeManager().addAttribute( "meshFile"    , _meshFile    );
    getAttributeManager().addAttribute( "position"    , _position    );
    getAttributeManager().addAttribute( "rotation"    , _rotation    );
    getAttributeManager().addAttribute( "enable"      , _enable      );
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
            removeFromTransformationNode( _mesh.get() );
        }
        break;

        case YAF3D_NOTIFY_MENU_LEAVE:
        {
            if ( _enable )
                addToTransformationNode( _mesh.get() );
        }
        break;


        default:
            ;
    }
}

void EnMesh::initialize()
{
    osg::Node* p_node = yaf3d::LevelManager::get()->loadMesh( _meshFile );
    if ( !p_node )
    {
        yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "*** could not load mesh file: " << _meshFile << ", in '" << getInstanceName() << "'" << std::endl;
        return;
    }

    _mesh = p_node;
    setPosition( _position );
    osg::Quat   rot( 
                     osg::DegreesToRadians( _rotation.x() ), osg::Vec3f( 1.0f, 0.0f, 0.0f ),
                     osg::DegreesToRadians( _rotation.y() ), osg::Vec3f( 0.0f, 1.0f, 0.0f ),
                     osg::DegreesToRadians( _rotation.z() ), osg::Vec3f( 0.0f, 0.0f, 1.0f )
                    );
    setRotation( rot );

    // register entity in order to get menu notifications
    yaf3d::EntityManager::get()->registerNotification( this, true );
}

void EnMesh::enable( bool en )
{
    if ( _enable == en )
        return;

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

} // namespace vrc