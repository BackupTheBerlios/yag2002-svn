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
 # entity GeoEmitter, it emitts box, sphere, cube, etc, into world
 #  it is just a kind of physics demonstrator
 #
 #   date of creation:  02/25/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_geomemitter.h"

namespace vrc
{
	
//! Implement and register the entity factory
YAF3D_IMPL_ENTITYFACTORY( GeomEmitteEntityFactory );

EnGeomEmitter::EnGeomEmitter():
_dimensions( osg::Vec3f( 5.0f, 5.0f, 5.0f ) ),
_period( 10.0f ),
_life( 15.0f ),
_time( 0 ),
_geomCount( 0 )
{
    // register entity attributes
    getAttributeManager().addAttribute( "geomTypes"  , _geomTypes  );
    getAttributeManager().addAttribute( "position"   , _position   );
    getAttributeManager().addAttribute( "dimensions" , _dimensions );
    getAttributeManager().addAttribute( "periode"    , _period     );
    getAttributeManager().addAttribute( "life"       , _life       );
}

EnGeomEmitter::~EnGeomEmitter()
{
}

void EnGeomEmitter::postInitialize()
{
    setPosition( _position );
    std::vector< std::string > tokens;
    yaf3d::BaseEntity*      p_entity = NULL;
    yaf3d::explode( _geomTypes, " ", &tokens );
    for ( size_t cnt = 0; cnt < tokens.size(); ++cnt )
    {
        p_entity = yaf3d::EntityManager::get()->findInstance( tokens[ cnt ] );
        if ( !p_entity )
        {
            log_error << "***EnGeomEmitter: entity type not found : '" << tokens[ cnt ] << "'" << std::endl;
            continue;
        }

        _geomStock.push_back( p_entity );
    }

    yaf3d::EntityManager::get()->registerUpdate( this );     // register entity in order to get updated per simulation step
}

void EnGeomEmitter::updateEntity( float deltaTime )
{
    _time += deltaTime;

    // create new geom every 2 seconds
    if ( _time > _period )
    {
        _time = 0;

        // take a random geom type
        int ran = rand() % _geomStock.size();
        yaf3d::BaseEntity* p_entity = _geomStock[ ran ];

        // clone the found geom
        std::stringstream geomcnt;
        geomcnt << _geomCount;
        p_entity = p_entity->clone( p_entity->getInstanceName() + geomcnt.str(), yaf3d::Application::get()->getSceneRootNode() );

        // set a random position before entity gets initialized
        osg::Vec3f randPos( ( float )( ( rand() % ( int ) _dimensions.x() ) - _dimensions.x() ), ( float )( ( rand() % ( int )_dimensions.y() ) - _dimensions.y() ), ( float )( ( rand() % ( int )_dimensions.z() ) - _dimensions.z() ) );
        p_entity->getAttributeManager().setAttributeValue( "position", _position + randPos );

        // setup entities
        p_entity->initialize();
        p_entity->postInitialize();

        float live = ( ( _life * 0.5f ) + ( float ) ( rand() % ( int )_life ) );
        _geoms.push_back( std::make_pair( p_entity, live ) );
    }

    // update geoms
    std::vector< std::pair< yaf3d::BaseEntity*, float > >::iterator geom = _geoms.begin(), geomEnd = _geoms.end();
    for ( ; geom != geomEnd; ++geom )
    {
        if ( ( geom->second -= deltaTime ) < 0 )
        {
            yaf3d::EntityManager::get()->deleteEntity( geom->first );
            _geoms.erase( geom );
            // we delete objects in a smooth way
            break;
        }
    }
}

} // namespace vrc
