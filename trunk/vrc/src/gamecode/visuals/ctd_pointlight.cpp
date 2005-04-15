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
// maximal lights enabled during one frame
#define MAX_GL_LIGHTS   8
//! Class for enabling up to 8 gl lights per frame considering light source culling
class LightManager : public Singleton< LightManager >, public Producer::Camera::Callback
{
    public:

       void                             operator()( const Producer::Camera & ) 
                                        {
                                            // enable lights
                                            flush();
                                        }

        void                            addLight( EnPointLight* p_light )
                                        {
                                            _renderLights.push_back( p_light );
                                            // assign an available light id to light object
                                            p_light->_lightSource->getLight()->setLightNum( _currId );
                                            if ( _currId < MAX_GL_LIGHTS ) 
                                                _currId++;
                                        }

    protected:

                                        LightManager() : _currId( 0 )
                                        {
                                            // ouch, registing a callback for camera in constructor of a singleton! but here it is no problem as
                                            //  the singleton is created in EnPointLight::initialize()
                                            osgProducer::Viewer* p_viewer = Application::get()->getViewer();
                                            Producer::Camera* p_cam = p_viewer->getCamera( 0 );
                                            p_cam->addPreDrawCallback( this ); 
                                        }
   
        virtual                         ~LightManager(){}

        //! Enable those lights which are not culled during this frame
        void                            flush()
                                        {
                                            osg::StateSet*  p_stateset = Application::get()->getViewer()->getGlobalStateSet();
                                            // first turn off all lights
                                            unsigned int numlights = min( ( unsigned int)_renderLights.size(), MAX_GL_LIGHTS - 1 );
                                            for ( unsigned int cnt = 0; cnt < numlights; cnt++ )
                                                p_stateset->setMode( GL_LIGHT0 + cnt, osg::StateAttribute::OFF );
                                            
                                            // now turn on lights which are not culled
                                            std::vector< EnPointLight* >::iterator p_beg = _renderLights.begin();
                                            for ( unsigned int cnt = 0; cnt < numlights; p_beg++, cnt++ )
                                            {
                                                if ( cnt < MAX_GL_LIGHTS )
                                                {
                                                    ( *p_beg )->_lightId = cnt;
                                                    p_stateset->setMode( GL_LIGHT0 + cnt, osg::StateAttribute::ON );
                                                }
                                            }

                                            // check if we can see more than maximal allowed count of lights
                                            if ( _renderLights.size() > MAX_GL_LIGHTS )
                                                log << Log::LogLevel( Log::L_WARNING ) << " *** LightManager: more than " << MAX_GL_LIGHTS <<
                                                " lights can be seen by camera! ( total count: " << _renderLights.size() << " )" << endl;

                                            // clear the list for next frame
                                            _renderLights.clear();
                                            _currId = 0;
                                        }

        //! List of lights which are not culled during one rendering frame
        std::vector< EnPointLight* >    _renderLights;

        //! Is used for holding an available light id for next light object
        GLint                           _currId;

    friend class Singleton< LightManager >;
};
CTD_SINGLETON_IMPL( LightManager );

//-----------------
//! Implement and register the pointlight entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( PointLightEntityFactory );

EnPointLight::EnPointLight() :
_ambientColor( osg::Vec3f( 0.2f, 0.2f, 0.2f ) ),
_diffuseColor( osg::Vec3f( 0.7f, 0.7f, 0.7f ) ),
_specularColor( osg::Vec3f( 0.9f, 0.9f, 0.9f ) ),
_constAttenuation( 0.01f ),
_linearAttenuation( 0.001f ),
_quadraticAttenuation( 0.0001f ),
_lightRadius( 100.0f ),
_lightId( 0 )
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
}

void EnPointLight::initialize()
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
    osg::ref_ptr< EnPointLight::LightCallback > cullcallback = new LightCallback( this );
    _lightSource->setCullCallback( cullcallback.get() );

    // setup bounding sphere used for culling
    _bSphere.set( _position, _lightRadius );
    // the id will be set by light manager
    p_light->setLightNum( _lightId );

    p_light->setPosition( osg::Vec4( 0, 0, 0, 1.0f ) );
    p_light->setAmbient( osg::Vec4( _ambientColor, 1.0f ) );
    p_light->setDiffuse( osg::Vec4( _diffuseColor, 1.0f ) );
    p_light->setSpecular( osg::Vec4( _specularColor, 1.0f ) );
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

// do light culling and add the light to light manager list if it is not culled
void EnPointLight::LightCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
{
    // add the light into light manager if it is not culled
    osgUtil::CullVisitor* p_cv = static_cast< osgUtil::CullVisitor* >( nv );
    if ( !p_cv->isCulled( _lightEntity->_bSphere ) )
        LightManager::get()->addLight( _lightEntity );    
}

}
