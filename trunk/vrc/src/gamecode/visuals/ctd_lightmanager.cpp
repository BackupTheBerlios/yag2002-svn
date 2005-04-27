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

void LightManager::addLight( BaseLight* p_light )
{
    _lights.push_back( p_light );
    // assign an available light id to light object
    p_light->_lightSource->getLight()->setLightNum( _currId );
    if ( _currId < CTD_MAX_GL_LIGHTS ) 
        _currId++;
}


LightManager::LightManager() : _currId( 0 )
{
    // ouch, registing a callback for camera in constructor of a singleton! but here it is no problem as
    //  the singleton is created in first light's initialize() method
    osgProducer::Viewer* p_viewer = Application::get()->getViewer();
    Producer::Camera* p_cam = p_viewer->getCamera( 0 );
    p_cam->addPreDrawCallback( this ); 
}
   
void LightManager::flush()
{
    osg::StateSet*  p_stateset = Application::get()->getViewer()->getGlobalStateSet();
    // first turn off all lights
    unsigned int numlights = min( ( unsigned int)_lights.size(), CTD_MAX_GL_LIGHTS - 1 );
    for ( unsigned int cnt = 0; cnt < numlights; cnt++ )
        p_stateset->setMode( GL_LIGHT0 + cnt, osg::StateAttribute::OFF );

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
    osgUtil::CullVisitor* p_cv = static_cast< osgUtil::CullVisitor* >( nv );
    if ( !p_cv->isCulled( _lightEntity->_bSphere ) )
        LightManager::get()->addLight( _lightEntity );    
}

} // namespace CTD
