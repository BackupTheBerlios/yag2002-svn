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

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "physicsstaticgeom.h"


//! Implement and register the mesh entity factory
YAF3D_IMPL_ENTITYFACTORY( PhysicsStaticGeomEntityFactory )

class GeodeVisitor : public osg::NodeVisitor
{
    public:
                                        GeodeVisitor( unsigned int geodemask ) :
                                         _nodeMask( geodemask )
                                        {
                                        }

                                        ~GeodeVisitor() {}

        void                            apply( osg::Group& group )
                                        {
                                            group.traverse( *this );
                                        }

        void                            apply( osg::Geode& node )
                                        {
                                            node.setNodeMask( _nodeMask );
                                        }

    protected:

        //! Node make
        unsigned int                    _nodeMask;
};


EnPhysicsStaticGeom::EnPhysicsStaticGeom() :
 _scale( osg::Vec3f( 1.0f, 1.0f, 1.0f ) ),
 _enable( true ),
 _materialID( 1 )
{
    // register entity attributes
    getAttributeManager().addAttribute( "enable"         , _enable     );
    getAttributeManager().addAttribute( "meshFile"       , _meshFile   );
    getAttributeManager().addAttribute( "position"       , _position   );
    getAttributeManager().addAttribute( "rotation"       , _rotation   );
    getAttributeManager().addAttribute( "scale"          , _scale      );
    getAttributeManager().addAttribute( "material ID"    , _materialID );
}

EnPhysicsStaticGeom::~EnPhysicsStaticGeom()
{
}

void EnPhysicsStaticGeom::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle notifications, most of the notifications are no-cares for this entity
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_MENU_ENTER:
        {
        }
        break;

        case YAF3D_NOTIFY_MENU_LEAVE:
        {
        }
        break;

        case YAF3D_NOTIFY_ENTITY_ATTRIBUTE_CHANGED:
        {
            yaf3d::EntityManager::get()->removeFromScene( this );
            if ( _enable )
                yaf3d::EntityManager::get()->addToScene( this );

            // re-setup the mesh if the mesh file name changed
            if ( _meshFile != _lastMeshFile )
            {
                getTransformationNode()->removeChildren( 0, getTransformationNode()->getNumChildren() );
                _mesh = setupMesh();
                if ( _mesh.valid() )
                    getTransformationNode()->addChild( _mesh );
            }

            _lastMeshFile = _meshFile;

            // set position, rotation and scale
            setTransformation();

            // set node mask
            GeodeVisitor gvis( _materialID );
            gvis.apply( *getTransformationNode() );
        }
        break;

        case YAF3D_NOTIFY_UNLOAD_LEVEL:
        {
        }
        break;

        case YAF3D_NOTIFY_SHUTDOWN:
        {
        }
        break;

        default:
            ;
    }
}

void EnPhysicsStaticGeom::initialize()
{
    getTransformationNode()->setName( getInstanceName() );

    _mesh = setupMesh();
    if ( _mesh.valid() )
        getTransformationNode()->addChild( _mesh );

    GeodeVisitor gvis( _materialID );
    gvis.apply( *getTransformationNode() );

    yaf3d::EntityManager::get()->registerNotification( this, true );

    enable( _enable );
}

osg::Node* EnPhysicsStaticGeom::setupMesh()
{
    osg::Node* p_node;

    p_node = yaf3d::LevelManager::get()->loadMesh( _meshFile, true );

    if ( !p_node )
    {
        log_error << "EnPhysicsStaticGeom: could not load mesh file: " << _meshFile << ", in '" << getInstanceName() << "'" << std::endl;
        return NULL;
    }

    p_node->setNodeMask( static_cast< osg::Node::NodeMask >( _materialID ) );

    _lastMeshFile = _meshFile;

    // set position, rotation and scale
    setTransformation();

    return p_node;
}

void EnPhysicsStaticGeom::setTransformation()
{
    setPosition( _position );
    osg::Quat   rot(
                     osg::DegreesToRadians( _rotation.x() ), osg::Vec3f( 1.0f, 0.0f, 0.0f ),
                     osg::DegreesToRadians( _rotation.y() ), osg::Vec3f( 0.0f, 1.0f, 0.0f ),
                     osg::DegreesToRadians( _rotation.z() ), osg::Vec3f( 0.0f, 0.0f, 1.0f )
                    );
    setRotation( rot );
    setScale( _scale );
}

void EnPhysicsStaticGeom::enable( bool en )
{
    if ( _enable == en )
        return;

    yaf3d::EntityManager::get()->removeFromScene( this );

    if ( _mesh.valid() && en )
    {
        yaf3d::EntityManager::get()->addToScene( this );
        _enable = en;
    }
    else
    {
        _enable = false;
    }
}

bool EnPhysicsStaticGeom::isEnabled() const
{
    return _enable;
}
