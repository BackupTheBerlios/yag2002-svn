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
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include <vrc_shadowmanager.h>
#include "vrc_mesh.h"

//GLOD is currently disabled as we have no actual use for it
#define DONT_USE_GLOD

#ifndef DONT_USE_GLOD
 #include "vrc_lod.h"
#endif

namespace vrc
{

//! Implement and register the mesh entity factory
YAF3D_IMPL_ENTITYFACTORY( MeshEntityFactory )

EnMesh::EnMesh() :
_enable( true ),
_usedInMenu( false ),
_shadowEnable( false ),
_useLOD( false ),
_lodErrorThreshold( 0.05f )
{
    // register entity attributes
    getAttributeManager().addAttribute( "usedInMenu"            , _usedInMenu        );
    getAttributeManager().addAttribute( "enable"                , _enable            );
    getAttributeManager().addAttribute( "meshFile"              , _meshFile          );
    getAttributeManager().addAttribute( "position"              , _position          );
    getAttributeManager().addAttribute( "rotation"              , _rotation          );
    getAttributeManager().addAttribute( "shadowEnable"          , _shadowEnable      );
    //getAttributeManager().addAttribute( "useLOD"                , _useLOD            );
    //getAttributeManager().addAttribute( "lodErrorThreshold"     , _lodErrorThreshold );
}

EnMesh::~EnMesh()
{
    // remove shadow from shadow manager
    if ( ( _shadowEnable ) && getTransformationNode() )
    {
        vrc::ShadowManager::get()->removeShadowNode( getTransformationNode() );
    }
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
                {
                    // re-setup shadow
                    addToSceneGraph();
                }
                else
                {
                    removeFromSceneGraph();
                }
            }
        }
        break;

        case YAF3D_NOTIFY_MENU_LEAVE:
        {
            if ( _enable )
            {
                if ( _usedInMenu )
                {
                    removeFromSceneGraph();
                }
                else
                {
                    // re-setup shadow
                    addToSceneGraph();
                }
            }
        }
        break;

        case YAF3D_NOTIFY_ENTITY_ATTRIBUTE_CHANGED:
        {
            // re-setup mesh
            removeFromSceneGraph();
            _mesh = setupMesh();
            if ( _mesh.valid() && _enable )
                addToSceneGraph();
        }
        break;

        // if used in menu then this entity is persisten, so we have to trigger its deletion on shutdown
        case YAF3D_NOTIFY_SHUTDOWN:
        {
            removeFromTransformationNode( _mesh.get() );

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

    if ( _mesh.valid() )
        addToTransformationNode( _mesh.get() );

    if ( _usedInMenu )
    {
        // we may use this entity also in menu loader, so add it to scenegraph in this case during initialization
        addToSceneGraph();        
    }

    // register entity in order to get menu notifications
    yaf3d::EntityManager::get()->registerNotification( this, true );
}

void EnMesh::removeFromSceneGraph()
{
    if ( !_mesh.valid() )
        return;

    // remove the transformation node from its parents
    unsigned int parents = getTransformationNode()->getNumParents();
    for ( unsigned int cnt = 0; cnt < parents; ++cnt )        
        getTransformationNode()->getParent( 0 )->removeChild( getTransformationNode() );
}

void EnMesh::addToSceneGraph()
{
    if ( !_mesh.valid() )
        return;

    // get the shadow flag in configuration
    bool shadow;
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_SHADOW_ENABLE, shadow );

    // first remove the transformation node from scenegraph
    removeFromSceneGraph();

    // enable shadow only if it is enabled
    if ( shadow && _shadowEnable )
    {
        vrc::ShadowManager::get()->addShadowNode( getTransformationNode() );
    }
    else
    {
        // if no shadowing is desired then add our transformation node to entity group
        yaf3d::EntityManager::get()->addToScene( this );
    }
}

osg::Node* EnMesh::setupMesh()
{
    osg::Node* p_node;

    // don't cache the mesh if lod is used
    if ( _useLOD )
        p_node = yaf3d::LevelManager::get()->loadMesh( _meshFile, false );
    else
        p_node = yaf3d::LevelManager::get()->loadMesh( _meshFile, true );

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

#ifndef DONT_USE_GLOD
    if ( _useLOD )
        p_node = setupLODObject( p_node );
#endif

    return p_node;
}

osg::Node* EnMesh::setupLODObject( osg::Node* p_node )
{
    osg::Node* p_lodnode = p_node;

#ifndef DONT_USE_GLOD

    // build the GLOD groups and objects
    osg::ref_ptr< LODSettings > p_lodsettings = new LODSettings( LODSettings::LOD_DISCRETE );
    p_lodsettings->setupScreenSpaceThreshold( _lodErrorThreshold );

    LODVisitor lodvis;
    lodvis.setLODSettings( p_lodsettings.get() );
    p_lodnode->accept( lodvis );

#endif

    return p_lodnode;
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
