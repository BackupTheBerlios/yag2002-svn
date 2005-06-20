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
 # entity for fog setup
 #
 #   date of creation:  04/05/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_fog.h"

namespace CTD
{

//! Implement and register the fog entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( FogEntityFactory );

osg::Fog*    EnFog::_p_fog     = NULL;
unsigned int EnFog::_instCount = 0;

EnFog::EnFog() :
_density( 0.3f ),
_start( 300.0f ),
_end( 1000.0f ),
_fogColor( osg::Vec3f( 0.2f, 0.2f, 0.2f ) ),
_isPersistent( false )
{
    // register entity attributes
    _attributeManager.addAttribute( "persistence", _isPersistent    );
    _attributeManager.addAttribute( "density"    , _density         );
    _attributeManager.addAttribute( "start"      , _start           );
    _attributeManager.addAttribute( "end"        , _end             );
    _attributeManager.addAttribute( "color"      , _fogColor        );

    _instCount++;

    // register entity in order to get shutdown notification
    EntityManager::get()->registerNotification( this, true );   
}

EnFog::~EnFog()
{
    if ( _p_fog && _instCount < 1 )
    {
        osgProducer::Viewer* p_viewer = Application::get()->getViewer();
        osg::StateSet* p_stateset = p_viewer->getGlobalStateSet();
        p_stateset->removeAttribute( _p_fog );
        _p_fog = NULL;
    }

    _instCount--;
}

void EnFog::handleNotification( const EntityNotification& notify )
{
    // handle notifications
    switch( notify.getId() )
    {
        // we have to trigger the deletion ourselves! ( this entity can be peristent )
        case CTD_NOTIFY_SHUTDOWN:

            if ( _isPersistent )
                EntityManager::get()->deleteEntity( this );
            break;

        default:
            ;
    }
}

void EnFog::initialize()
{
    // the fog is global and must be shared between all fog entities
    if ( _p_fog )
        return;

    osgProducer::Viewer* p_viewer = Application::get()->getViewer();
    osg::StateSet* p_stateset = p_viewer->getGlobalStateSet();
    _p_fog = new osg::Fog;
    _p_fog->setMode( osg::Fog::LINEAR );
    _p_fog->setDensity( _density );
    _p_fog->setStart( _start );
    _p_fog->setEnd( _end );
    _p_fog->setColor( osg::Vec4f( _fogColor, 1.0f ) );

    p_stateset->setAttributeAndModes( _p_fog, osg::StateAttribute::ON );
    p_stateset->setMode( GL_FOG, osg::StateAttribute::ON );
}

void EnFog::enable( bool en )
{
    if ( en )
    {
        osgProducer::Viewer* p_viewer = Application::get()->getViewer();
        osg::StateSet* p_stateset = p_viewer->getGlobalStateSet();
        _p_fog->setDensity( _density );
        _p_fog->setStart( _start );
        _p_fog->setEnd( _end );
        _p_fog->setColor( osg::Vec4f( _fogColor, 1.0f ) );
        p_stateset->setAttributeAndModes( _p_fog, osg::StateAttribute::ON );
        p_stateset->setMode( GL_FOG, osg::StateAttribute::ON );
    }
    else
    {
        osgProducer::Viewer* p_viewer = Application::get()->getViewer();
        osg::StateSet* p_stateset = p_viewer->getGlobalStateSet();
        p_stateset->setAttributeAndModes( _p_fog, osg::StateAttribute::OFF );
        p_stateset->setMode( GL_FOG, osg::StateAttribute::OFF );
    }
}

} // namespace CTD
