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
#include "ctd_spotlight.h"

using namespace std;

namespace CTD
{
//! Implement and register the spotlight entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( SpotLightEntityFactory );

EnSpotLight::EnSpotLight() :
_maxDistance( 10.0f ),
_direction( osg::Vec3f( 0.0f, 0.0f, -1.0f ) ),
_spotCutOff( 50.0 ),
_spotExponent( 32.0f )
{
    // register entity attributes
    getAttributeManager().addAttribute( "position"             , _position             );
    getAttributeManager().addAttribute( "maxDistance"          , _maxDistance          );
    getAttributeManager().addAttribute( "meshFile"             , _meshFile             );
    getAttributeManager().addAttribute( "ambientColor"         , _ambientColor         );
    getAttributeManager().addAttribute( "diffuseColor"         , _diffuseColor         );
    getAttributeManager().addAttribute( "specularColor"        , _specularColor        );
    getAttributeManager().addAttribute( "constantAttenuation"  , _constAttenuation     );
    getAttributeManager().addAttribute( "linearAttenuation"    , _linearAttenuation    );
    getAttributeManager().addAttribute( "quadraticAttenuation" , _quadraticAttenuation );
    getAttributeManager().addAttribute( "direction"            , _direction            );
    getAttributeManager().addAttribute( "spotCutOff"           , _spotCutOff           );
    getAttributeManager().addAttribute( "spotExponent"         , _spotExponent         );
}

EnSpotLight::~EnSpotLight()
{
}

void EnSpotLight::initialize()
{
    // call the get method of light manager so it can register itself for getting camera callbacks (see constructor)
    LightManager::get();

    // create a new light
    _lightSource = new osg::LightSource;
    osg::Light* p_light = new osg::Light;
    _lightSource->setLight( p_light );
    _lightSource->setLocalStateSetModes( osg::StateAttribute::ON );
    // we do culling ourselves
    _lightSource->setCullingActive( false );

    // set cull callback. it manages the enabling / disabling of lights depending on camera's 
    //  frustum and light radius.
    osg::ref_ptr< LightCallback > cullcallback = new LightCallback( this );
    _lightSource->setCullCallback( cullcallback.get() );

    // setup bounding sphere used for culling
    osg::Vec3f len = _direction;
    len.normalize();
    len *= _maxDistance;
    osg::Vec3f center( _position + len * 0.5f );
    _bSphere.set( center, _maxDistance * 0.5f );
    // the id will be set by light manager
    p_light->setLightNum( _lightId );

    p_light->setPosition( osg::Vec4( 0.0f, 0.0f, 0.0f, 1.0f ) );
    p_light->setDirection( _direction );
    p_light->setSpotCutoff( _spotCutOff );
    p_light->setSpotExponent( _spotExponent );
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

} // namespace CTD
