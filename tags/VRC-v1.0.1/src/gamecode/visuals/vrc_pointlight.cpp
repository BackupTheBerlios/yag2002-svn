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
 # entity pointlight
 #
 #   date of creation:  04/14/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_pointlight.h"
#include "vrc_lightmanager.h"

namespace vrc
{
//-----------------
//! Implement and register the pointlight entity factory
YAF3D_IMPL_ENTITYFACTORY( PointLightEntityFactory );

EnPointLight::EnPointLight() :
_lightRadius( 100.0f ),
_usedInMenu( false ),
_enable( true )
{
    // register entity attributes
    getAttributeManager().addAttribute( "usedInMenu"           , _usedInMenu           );
    getAttributeManager().addAttribute( "enable"               , _enable               );
    getAttributeManager().addAttribute( "position"             , _position             );
    getAttributeManager().addAttribute( "radius"               , _lightRadius          );
    getAttributeManager().addAttribute( "meshFile"             , _meshFile             );
    getAttributeManager().addAttribute( "ambientColor"         , _ambientColor         );
    getAttributeManager().addAttribute( "diffuseColor"         , _diffuseColor         );
    getAttributeManager().addAttribute( "specularColor"        , _specularColor        );
    getAttributeManager().addAttribute( "constantAttenuation"  , _constAttenuation     );
    getAttributeManager().addAttribute( "linearAttenuation"    , _linearAttenuation    );
    getAttributeManager().addAttribute( "quadraticAttenuation" , _quadraticAttenuation );
}

EnPointLight::~EnPointLight()
{
}

void EnPointLight::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle notifications
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_MENU_ENTER:
        {
            if ( _enable )
            {
                if ( _usedInMenu )
                    addToTransformationNode( _lightSource.get() );
                else
                    removeFromTransformationNode( _lightSource.get() );
            }
        }
        break;

        case YAF3D_NOTIFY_MENU_LEAVE:
        {
            if ( _enable )
            {
                if ( _usedInMenu )
                    removeFromTransformationNode( _lightSource.get() );
                else
                    addToTransformationNode( _lightSource.get() );
            }
        }
        break;

        // update the light settings when attributes are changed (e.g. by a level editor)
        case YAF3D_NOTIFY_ENTITY_ATTRIBUTE_CHANGED:
        {
            removeFromTransformationNode( _lightSource.get() );
            if ( _enable )
            {
                _lightSource->setLight( setupLight().get() );
                addToTransformationNode( _lightSource.get() );
            }
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

void EnPointLight::initialize()
{
    // light manager's initialize method is resistent against multiple calls ( we can have several light entities )
    LightManager::get()->initialize();

    // create a new light
    _lightSource = new osg::LightSource;
    _lightSource->setLight( setupLight().get() );
    _lightSource->setLocalStateSetModes( osg::StateAttribute::ON );
    _lightSource->setReferenceFrame( osg::LightSource::RELATIVE_RF );
    // we do culling ourselves
    _lightSource->setCullingActive( false );

    // set cull callback. it manages the enabling / disabling of lights depending on camera's 
    //  frustum and light radius.
    osg::ref_ptr< LightCallback > cullcallback = new LightCallback( this );
    _lightSource->setCullCallback( cullcallback.get() );

    // now add light to entity's transform node if this entity is used in menu system, otherwise on "leve menu" event
    //  the light source is added -- see notification handler
    if ( _usedInMenu )
        addToTransformationNode( _lightSource.get() );

    // set mesh if one defined
    if ( _meshFile.length() )
    {
        osg::ref_ptr< osg::Node > mesh = yaf3d::LevelManager::get()->loadMesh( _meshFile );
        if ( !mesh.valid() ) 
            log_warning << " cannot find mesh file" << _meshFile << std::endl;
        else
            addToTransformationNode( mesh.get() );
    }
    
    // register entity in order to get menu notifications
    yaf3d::EntityManager::get()->registerNotification( this, true );    
}

osg::ref_ptr< osg::Light > EnPointLight::setupLight()
{
    osg::ref_ptr< osg::Light > light = new osg::Light;

    // setup bounding sphere used for culling
    _bSphere.set( osg::Vec3f( 0, 0, 0 ), _lightRadius );
    // the actual id will be set by light manager
    light->setLightNum( _lightId );

    light->setPosition( osg::Vec4( 0, 0, 0, 1.0f ) );
    light->setAmbient( osg::Vec4( _ambientColor, 1.0f ) );
    light->setDiffuse( osg::Vec4( _diffuseColor, 1.0f ) );
    light->setSpecular( osg::Vec4( _specularColor, 1.0f ) );
    light->setConstantAttenuation( _constAttenuation );
    light->setLinearAttenuation( _linearAttenuation );
    light->setQuadraticAttenuation( _quadraticAttenuation );

    // set entity position which is also the light source position
    setPosition( _position );

    return light;
}

} // namespace vrc
