/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2004-2006, Ali Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU General Public License 
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public 
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

#include <ctd_base.h>
#include <ctd_application.h>
#include <ctd_levelmanager.h>
#include <ctd_log.h>
#include "ctd_pointlight.h"

using namespace std;

namespace CTD
{

//! Implement and register the pointlight entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( PointLightEntityFactory );

GLint EnPointLight::_totalLights = 0;

EnPointLight::EnPointLight() :
_ambientColor( osg::Vec3f( 0.2f, 0.2f, 0.2f ) ),
_diffuseColor( osg::Vec3f( 0.7f, 0.7f, 0.7f ) ),
_specularColor( osg::Vec3f( 0.9f, 0.9f, 0.9f ) ),
_constAttenuation( 0.01f ),
_linearAttenuation( 0.001f ),
_quadraticAttenuation( 0.0001f ),
_lightRadius( 100.0f )
{
    // register entity attributes
    _attributeManager.addAttribute( "position"             , _position             );
    _attributeManager.addAttribute( "radius"               , _lightRadius          );
    _attributeManager.addAttribute( "meshFile"             , _meshFile             );
    _attributeManager.addAttribute( "ambientColor"         , _ambientColor         );
    _attributeManager.addAttribute( "diffuseColor"         , _diffuseColor         );
    _attributeManager.addAttribute( "specularColor"        , _specularColor        );
    _attributeManager.addAttribute( "constantAttenuation"  , _constAttenuation     );
    _attributeManager.addAttribute( "linearAttenuation"    , _linearAttenuation    );
    _attributeManager.addAttribute( "quadraticAttenuation" , _quadraticAttenuation );
}

EnPointLight::~EnPointLight()
{
    _totalLights--;
    assert( _totalLights >= 0 );
}

void EnPointLight::initialize()
{
    // create a new light
    _lightSource = new osg::LightSource;
    osg::Light* p_light = new osg::Light;
    _lightSource->setLight( p_light );
    _lightSource->setLocalStateSetModes( osg::StateAttribute::ON );
    // we do culling ourselves
    _lightSource->setCullingActive( false );

    // set cull callback. it manages the enabling / disabling of lights depending on camera's 
    //  frustum and light radius.
    osg::ref_ptr< EnPointLight::LightCallback > cullcallback = new LightCallback( this );
    _lightSource->setCullCallback( cullcallback.get() );

    // setup bounding sphere used for culling
    _bSphere.set( _position, _lightRadius );

    p_light->setPosition( osg::Vec4( 0, 0, 0, 1.0f ) ); //! TODO check what the position of light actually do! it has to do with bsphere calculation too.
    p_light->setAmbient( osg::Vec4( _ambientColor, 1.0f ) );
    p_light->setDiffuse( osg::Vec4( _diffuseColor, 1.0f ) );
    p_light->setSpecular( osg::Vec4( _specularColor, 1.0f ) );

    _lightId = _totalLights;
    p_light->setLightNum( _totalLights++ );

    p_light->setConstantAttenuation( _constAttenuation );
    p_light->setLinearAttenuation( _linearAttenuation );
    p_light->setQuadraticAttenuation( _quadraticAttenuation );

    // add light to entity's transform node
    addToTransformableNode( _lightSource.get() );
       
    // set mesh if one defined
    if ( _meshFile.length() )
    {
        osg::ref_ptr< osg::Node > mesh = LevelManager::get()->loadMesh( _meshFile );
        if ( !mesh.valid() ) 
            log << Log::LogLevel( Log::L_WARNING ) << " cannot find mesh file" << _meshFile << endl;
        else
            addToTransformableNode( mesh.get() );
    }
    
    // set entity position which is also the light source position
    setPosition( _position );
}

void EnPointLight::LightCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
{
    osgUtil::CullVisitor* p_cv = static_cast< osgUtil::CullVisitor* >( nv );
    osg::StateSet*  p_stateset = Application::get()->getViewer()->getGlobalStateSet();

    // check whether the light source is culled, then enable / disable light
    if ( p_cv->isCulled( _lightEntity->_bSphere ) )
        p_stateset->setMode( GL_LIGHT0 + _lightEntity->_lightId, osg::StateAttribute::OFF );
    else
        p_stateset->setMode( GL_LIGHT0 + _lightEntity->_lightId, osg::StateAttribute::ON );
}

}
