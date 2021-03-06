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

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_mesh.h"

#include <osgShadow/ShadowedScene>
#include <osgShadow/ShadowVolume>
#include <osgShadow/ShadowMap>


//! NOTE GLOD is currently disabled as we have no actual use for it
#define DONT_USE_GLOD

#ifndef DONT_USE_GLOD
 #include "vrc_lod.h"
#endif

namespace vrc
{

//! Implement and register the mesh entity factory
YAF3D_IMPL_ENTITYFACTORY( MeshEntityFactory )

//! Visitor for getting animation path node out of a loaded node
class FindAnimPathVisitor : public osg::NodeVisitor
{
    public:
                                            FindAnimPathVisitor( osg::NodeVisitor::TraversalMode tmode = osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ) :
                                                osg::NodeVisitor( tmode ), _p_animPath( NULL )
                                            {
                                                // we take all nodes
                                                setTraversalMask( 0xffffffff );
                                            }

        virtual                             ~FindAnimPathVisitor() {}

        void                                apply( osg::Group& grp )
                                            {
                                                grp.traverse( *this );
                                            }

        void                                apply( osg::MatrixTransform& mt )
                                            {
                                                if ( !mt.getUpdateCallback() )
                                                    mt.traverse( *this );

                                                osg::AnimationPathCallback* p_ncb = dynamic_cast< osg::AnimationPathCallback* >( mt.getUpdateCallback() );
                                                if ( !p_ncb )
                                                    mt.traverse( *this );

                                                _p_animPath = p_ncb->getAnimationPath();
                                                if ( !_p_animPath )
                                                    mt.traverse( *this );
                                            }

        osg::AnimationPath*                 getAnimPath()
                                            {
                                                return _p_animPath;
                                            }

    protected:

        osg::AnimationPath*                 _p_animPath;
};


EnMesh::EnMesh() :
_scale( osg::Vec3f( 1.0f, 1.0f, 1.0f ) ),
_enable( true ),
_usedInMenu( false ),
_throwShadow( false ),
_receiveShadow( false ),
_useLOD( false ),
_lodErrorThreshold( 0.05f ),
_cgfShadow( false ),
_shadowEnable( false ),
_shadowCullDist( 200.0f ),
_animPosition( true ),
_animRotation( true ),
_animRelative( true ),
_animDelay( 0.0f ),
_animTimeScale( 1.0f ),
_animLoop( true ),
_animTime( 0.0f )
{
    // register entity attributes
    getAttributeManager().addAttribute( "enable"         , _enable         );
    getAttributeManager().addAttribute( "usedInMenu"     , _usedInMenu     );
    getAttributeManager().addAttribute( "meshFile"       , _meshFile       );
    getAttributeManager().addAttribute( "shaderName"     , _shaderName     );
    getAttributeManager().addAttribute( "position"       , _position       );
    getAttributeManager().addAttribute( "rotation"       , _rotation       );
    getAttributeManager().addAttribute( "scale"          , _scale          );
    getAttributeManager().addAttribute( "animFile"       , _animFile       );
    getAttributeManager().addAttribute( "animPostion"    , _animPosition   );
    getAttributeManager().addAttribute( "animRotation"   , _animRotation   );
    getAttributeManager().addAttribute( "animRelative"   , _animRelative   );
    getAttributeManager().addAttribute( "animDelay"      , _animDelay      );
    getAttributeManager().addAttribute( "animTimeScale"  , _animTimeScale  );
    getAttributeManager().addAttribute( "animLoop"       , _animLoop       );
    getAttributeManager().addAttribute( "throwShadow"    , _throwShadow    );
    getAttributeManager().addAttribute( "receiveShadow"  , _receiveShadow  );
    getAttributeManager().addAttribute( "shadowCullDist" , _shadowCullDist );

#ifndef DONT_USE_GLOD
    getAttributeManager().addAttribute( "useLOD"                , _useLOD            );
    getAttributeManager().addAttribute( "lodErrorThreshold"     , _lodErrorThreshold );
#endif
}

EnMesh::~EnMesh()
{
    // remove shadow from shadow manager
    if ( ( _shadowEnable ) && getTransformationNode() )
    {
        yaf3d::ShadowManager::get()->removeShadowNode( getTransformationNode() );
    }
}

void EnMesh::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle notifications, add and remove the transformation node to / from scenegraph on menu entring / leaving
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_MENU_ENTER:
        {
            if ( _enable )
            {
                if ( _usedInMenu )
                {
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
                    addToSceneGraph();
                }
            }
        }
        break;

        //! Note: by default the level manager re-adds persistent entity transformation nodes to its entity group while unloading a level.
        //        thus we have to remove shadow nodes from that entity group on unloading a level; addToSceneGraph() does this job.
        case YAF3D_NOTIFY_ENTITY_TRANSNODE_CHANGED:
        {
            if ( _usedInMenu )
            {
                // this method really removes the node from all its parents
                yaf3d::EntityManager::get()->removeFromScene( this );
                // now add it to scene graph again
                addToSceneGraph();
            }
        }
        break;

        case YAF3D_NOTIFY_ENTITY_ATTRIBUTE_CHANGED:
        {
            // re-setup mesh
            if ( _mesh.valid() )
                removeFromTransformationNode( _mesh.get() );

            _mesh = setupMesh();

            if ( _mesh.valid() && _enable )
                addToTransformationNode( _mesh.get() );

            // if the attribute changes then we enable rendering regardless of current state

            // this method really removes the node from all its parents
            yaf3d::EntityManager::get()->removeFromScene( this );
            // now add it to scene graph again
            addToSceneGraph();
        }
        break;

        case YAF3D_NOTIFY_UNLOAD_LEVEL:

            if ( !_usedInMenu )
                removeFromSceneGraph();

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

    getTransformationNode()->setName( getInstanceName() );

    // is dynamic shadows enabled in the configuration?
    yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_SHADOW_ENABLE, _cgfShadow );

    // is dynamic shadow for this mesh desired?
    _shadowEnable = ( _throwShadow || _receiveShadow ) && _cgfShadow;

    if ( _mesh.valid() )
        addToTransformationNode( _mesh.get() );

    // the node is added and removed by notification callback!
    yaf3d::EntityManager::get()->removeFromScene( this );

    // register entity in order to get menu notifications
    yaf3d::EntityManager::get()->registerNotification( this, true );
}

void EnMesh::updateEntity( float deltaTime )
{
    // animate position and rotation if an animation is defined
    if ( !_animPath.get() )
        return;

    //! TODO: get the animation length and handle Loop option
    _animTime += deltaTime;
    if ( _animTime > _animDelay )
    {
        osg::AnimationPath::ControlPoint cp;
        _animPath->getInterpolatedControlPoint( _animTime * _animTimeScale, cp );
        if ( _animRelative )
        {
            if ( _animPosition )
                getTransformationNode()->setPosition( cp.getPosition() + _orgPostition );
            if ( _animRotation )
                getTransformationNode()->setAttitude( cp.getRotation() * _orgRotation );
        }
        else
        {
            if ( _animPosition )
                getTransformationNode()->setPosition( cp.getPosition() );
            if ( _animRotation )
                getTransformationNode()->setAttitude( cp.getRotation() );
        }
    }
}

void EnMesh::removeFromSceneGraph()
{
    if ( !_mesh.valid() )
        return;

    if ( _shadowEnable )
        yaf3d::ShadowManager::get()->removeShadowNode( getTransformationNode() );

    yaf3d::EntityManager::get()->removeFromScene( this );
}

void EnMesh::addToSceneGraph()
{
    if ( !_mesh.valid() )
        return;

    // is a shader name given?
    osg::Group* p_shadernode = NULL;
    if ( _shaderName.length() )
    {
        // try to get the shader node and append our node to it
        p_shadernode = yaf3d::ShaderContainer::get()->getShaderNode( _shaderName ).get();
        if ( !p_shadernode )
        {
            log_error << "EnMesh: invalid shader name: " << _shaderName << " in mesh instance " << getInstanceName() << std::endl;
        }
    }

    // enable shadow only if it is enabled in configuration
    if ( _shadowEnable )
    {
        // set the shadow mode
        unsigned int shadowmode = 0;
        if ( _throwShadow )
            shadowmode |= yaf3d::ShadowManager::eThrowShadow;
        if ( _receiveShadow )
            shadowmode |= yaf3d::ShadowManager::eReceiveShadow;

        // first remove it for the case that this method is called on modification of entity attributes
        yaf3d::ShadowManager::get()->removeShadowNode( p_shadernode ? p_shadernode : getTransformationNode() );
        yaf3d::ShadowManager::get()->addShadowNode( p_shadernode ? p_shadernode : getTransformationNode(), shadowmode, _shadowCullDist );

        // the mesh needs at least one subgraph for getting rendered; the shadow throwing subgraph does not render the mesh (but only its shadow)
        // so we put meshes which should throw shadow and not receive shadow to the default scene node.
        if ( !_receiveShadow && !p_shadernode )
            yaf3d::EntityManager::get()->addToScene( this );
    }
    else if ( !p_shadernode ) // add to scene only if the mesh has no shader
    {
        yaf3d::EntityManager::get()->addToScene( this );
    }

    // if the node has a shader append it also to the shader node
    if ( p_shadernode )
    {
        // if not already added then add it now ( this method is called on every modification of entity attributes! )
        if ( !p_shadernode->containsNode( getTransformationNode() ) )
            p_shadernode->addChild( getTransformationNode() );
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
        log_error << "EnMesh: could not load mesh file: " << _meshFile << ", in '" << getInstanceName() << "'" << std::endl;
        return NULL;
    }

    setPosition( _position );
    osg::Quat   rot(
                     osg::DegreesToRadians( _rotation.x() ), osg::Vec3f( 1.0f, 0.0f, 0.0f ),
                     osg::DegreesToRadians( _rotation.y() ), osg::Vec3f( 0.0f, 1.0f, 0.0f ),
                     osg::DegreesToRadians( _rotation.z() ), osg::Vec3f( 0.0f, 0.0f, 1.0f )
                    );
    setRotation( rot );
    setScale( _scale );

    // store the original position and rotation for animation in relative mode
    _orgPostition = _position;
    _orgRotation  = rot;

    // invalidate a previously loaded animation
    if ( _animPath.valid() )
        _animPath = NULL;

    // is an animation file defined?
    if ( _animFile.length() )
    {
        osg::Node* p_animnode = yaf3d::LevelManager::get()->loadMesh( _animFile, true );
        if ( !p_animnode )
        {
            log_warning << "EnMesh: invalid animation mesh file (only osg and ive file formats are accepted)" << _animFile << std::endl;

            // remove update registration, it is needed only for animation
            if ( yaf3d::EntityManager::get()->isRegisteredUpdate( this ) )
                yaf3d::EntityManager::get()->registerUpdate( this, false );
        }
        else
        {
            osg::AnimationPath* p_path   = NULL;
            osg::Group*         p_topgrp = dynamic_cast< osg::Group* >( p_animnode );

            // find the animation callback in loaded mesh
            FindAnimPathVisitor v;
            v.apply( *p_topgrp );
            p_path = v.getAnimPath();
            // any animation path found?
            if ( p_path )
            {
                _animPath = p_path;
                // if a valid animation is defined then we need the update function
                yaf3d::EntityManager::get()->registerUpdate( this, true );

                // reset animation time
                _animTime = 0.0f;
            }
        }
    }

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
            addToSceneGraph();
        }
        else
        {
            removeFromSceneGraph();
        }

        _enable = en;
    }
    else
    {
        _enable = false;
    }
}

} // namespace vrc
