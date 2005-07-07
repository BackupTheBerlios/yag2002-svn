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

#include <ctd_main.h>
#include "ctd_pointlight.h"
#include "ctd_lightmanager.h"

using namespace std;

namespace CTD
{
//-----------------
//! Implement and register the pointlight entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( PointLightEntityFactory );

EnPointLight::EnPointLight() :
_lightRadius( 100.0f )
{
    // register entity attributes
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

void EnPointLight::initialize()
{
    // light manager's initialize method is resistent against multiple calls ( we can have several light entities )
    LightManager::get()->initialize();

    // create a new light
    _lightSource = new osg::LightSource;
    osg::Light* p_light = new osg::Light;
    _lightSource->setLight( p_light );
    _lightSource->setLocalStateSetModes( osg::StateAttribute::ON );
    _lightSource->setReferenceFrame( osg::LightSource::RELATIVE_RF );
    // we do culling ourselves
    _lightSource->setCullingActive( false );

    // set cull callback. it manages the enabling / disabling of lights depending on camera's 
    //  frustum and light radius.
    osg::ref_ptr< LightCallback > cullcallback = new LightCallback( this );
    _lightSource->setCullCallback( cullcallback.get() );

    // setup bounding sphere used for culling
    _bSphere.set( osg::Vec3f( 0, 0, 0 ), _lightRadius );
    // the actual id will be set by light manager
    p_light->setLightNum( _lightId );

    p_light->setPosition( osg::Vec4( 0, 0, 0, 1.0f ) );
    p_light->setAmbient( osg::Vec4( _ambientColor, 1.0f ) );
    p_light->setDiffuse( osg::Vec4( _diffuseColor, 1.0f ) );
    p_light->setSpecular( osg::Vec4( _specularColor, 1.0f ) );
    p_light->setConstantAttenuation( _constAttenuation );
    p_light->setLinearAttenuation( _linearAttenuation );
    p_light->setQuadraticAttenuation( _quadraticAttenuation );

    // add light to entity's transform node
    addToTransformationNode( _lightSource.get() );
    // set mesh if one defined
    if ( _meshFile.length() )
    {
        osg::ref_ptr< osg::Node > mesh = LevelManager::get()->loadMesh( _meshFile );
        if ( !mesh.valid() ) 
            log << Log::LogLevel( Log::L_WARNING ) << " cannot find mesh file" << _meshFile << endl;
        else
            addToTransformationNode( mesh.get() );
    }
    
    // set entity position which is also the light source position
    setPosition( _position );
}

}
