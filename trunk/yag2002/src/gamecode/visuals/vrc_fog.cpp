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
 # entity for fog setup
 #
 #   date of creation:  04/05/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_fog.h"

namespace vrc
{

//! Implement and register the fog entity factory
YAF3D_IMPL_ENTITYFACTORY( FogEntityFactory )

osg::Fog*    EnFog::_p_fog     = NULL;
unsigned int EnFog::_instCount = 0;

EnFog::EnFog() :
_density( 0.3f ),
_start( 300.0f ),
_end( 1000.0f ),
_fogColor( osg::Vec3f( 0.2f, 0.2f, 0.2f ) ),
_usedInMenu( false ),
_enable( true )
{
    // register entity attributes
    getAttributeManager().addAttribute( "usedInMenu" , _usedInMenu   );
    getAttributeManager().addAttribute( "density"    , _density      );
    getAttributeManager().addAttribute( "start"      , _start        );
    getAttributeManager().addAttribute( "end"        , _end          );
    getAttributeManager().addAttribute( "color"      , _fogColor     );
    getAttributeManager().addAttribute( "enable"     , _enable       );

    ++_instCount;
}

EnFog::~EnFog()
{
    _instCount--;
    if ( _p_fog && _instCount < 1 )
    {
        osg::StateSet* p_stateset = yaf3d::Application::get()->getSceneView()->getGlobalStateSet();
        p_stateset->removeAttribute( _p_fog );
        _p_fog = NULL;
    }
}

void EnFog::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle notifications
    switch( notification.getId() )
    {

        case YAF3D_NOTIFY_MENU_ENTER:

            if ( _usedInMenu && _enable )
                enable();

            break;

        case YAF3D_NOTIFY_MENU_LEAVE:

            if ( !_usedInMenu && _enable )
                enable();

            break;

        // reactivate fog when an attribute changed
        case YAF3D_NOTIFY_ENTITY_ATTRIBUTE_CHANGED:
            
            if ( _enable )
                enable();

            break;

        // we have to trigger the deletion ourselves! ( this entity is peristent if used in menu system )
        case YAF3D_NOTIFY_SHUTDOWN:

            if ( _usedInMenu )
                yaf3d::EntityManager::get()->deleteEntity( this );
            break;

        default:
            ;
    }
}

void EnFog::initialize()
{
    // the fog is global and must be shared between all fog entities
    if ( _p_fog )
    {
        enable();
        
        // register entity in order to get shutdown notification
        yaf3d::EntityManager::get()->registerNotification( this, true );

        return;
    }

    // register entity in order to get shutdown notification
    yaf3d::EntityManager::get()->registerNotification( this, true );   

    osg::StateSet* p_stateset = yaf3d::Application::get()->getSceneView()->getGlobalStateSet();
    _p_fog = new osg::Fog;
    _p_fog->setMode( osg::Fog::LINEAR );
    _p_fog->setDensity( _density );
    _p_fog->setStart( _start );
    _p_fog->setEnd( _end );
    _p_fog->setColor( osg::Vec4f( _fogColor, 1.0f ) );

    p_stateset->setAttributeAndModes( _p_fog, osg::StateAttribute::ON );
    p_stateset->setMode( GL_FOG, osg::StateAttribute::ON );
}

void EnFog::enable()
{
    osg::StateSet* p_stateset = yaf3d::Application::get()->getSceneView()->getGlobalStateSet();
    _p_fog->setDensity( _density );
    _p_fog->setStart( _start );
    _p_fog->setEnd( _end );
    _p_fog->setColor( osg::Vec4f( _fogColor, 1.0f ) );
    p_stateset->setAttributeAndModes( _p_fog, osg::StateAttribute::ON );
    p_stateset->setMode( GL_FOG, osg::StateAttribute::ON );
}

} // namespace vrc
