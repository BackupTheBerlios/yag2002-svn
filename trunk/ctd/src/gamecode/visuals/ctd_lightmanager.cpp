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
 # classes needed for various light types
 #
 #   date of creation:  04/16/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_lightmanager.h"

using namespace std;

namespace CTD
{

CTD_SINGLETON_IMPL( LightManager );

LightManager::LightManager() : 
_currId( 0 ),
_initialized( false )
{
}

void LightManager::initialize()
{

    // register a pre-draw callback where the light management happens
    if ( !_initialized ) // we register the callback only once
    {
        _initialized = true;
        Application::get()->getViewer()->addPreDrawCallback( this );

        // enable the lighting for scene
        Application::get()->getSceneView()->getGlobalStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::ON );
    }
}

void LightManager::addLight( BaseLight* p_light )
{
    _lights.push_back( p_light );
    // assign an available light id to light object
    p_light->_lightSource->getLight()->setLightNum( _currId );
    if ( _currId < CTD_MAX_GL_LIGHTS - 1 ) 
        _currId++;
}

void LightManager::flush()
{
    unsigned int numlights = min( ( int )_lights.size(), CTD_MAX_GL_LIGHTS - 1 );
    
    osg::StateSet*  p_stateset = Application::get()->getSceneView()->getGlobalStateSet();
    // first turn off all lights
    for ( unsigned int l = 0; l < CTD_MAX_GL_LIGHTS - 1; l++ )
        p_stateset->setMode( GL_LIGHT0 + l, osg::StateAttribute::OFF );

    // now turn on lights which are not culled
    std::vector< BaseLight* >::iterator p_beg = _lights.begin();
    for ( unsigned int cnt = 0; cnt < numlights; p_beg++, cnt++ )
    {
        if ( cnt < CTD_MAX_GL_LIGHTS )
        {
            ( *p_beg )->_lightId = cnt;
            p_stateset->setMode( GL_LIGHT0 + cnt, osg::StateAttribute::ON );
        }
    }

    // check if we can see more than maximal allowed count of lights
    if ( _lights.size() > CTD_MAX_GL_LIGHTS )
        log << Log::LogLevel( Log::L_WARNING ) << " *** LightManager: more than " << CTD_MAX_GL_LIGHTS <<
        " lights can be seen by camera! ( total count: " << _lights.size() << " )" << endl;

    // clear the list for next frame
    _lights.clear();
    _currId = 0;
}


// do light culling and add the light to light manager list if it is not culled
void LightCallback::operator()( osg::Node* node, osg::NodeVisitor* nv )
{
    // add the light into light manager if it is not culled
    osgUtil::CullVisitor* p_cv = dynamic_cast< osgUtil::CullVisitor* >( nv );
    if ( !p_cv )
        return;

    if ( !p_cv->isCulled( _lightEntity->getBoundingSphere() ) )
        LightManager::get()->addLight( _lightEntity );    
}

} // namespace CTD
