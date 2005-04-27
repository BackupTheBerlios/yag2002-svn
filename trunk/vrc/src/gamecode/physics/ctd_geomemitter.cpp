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
 # entity GeoEmitter, it emitts box, sphere, cube, etc, into world
 #  it is just a kind of physics demonstrator
 #
 #   date of creation:  02/25/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_geomemitter.h"

using namespace std;
using namespace CTD; 
using namespace osg; 


//! Implement and register the entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( GeomEmitteEntityFactory );

EnGeomEmitter::EnGeomEmitter():
_time( 0 ),
_geomCount( 0 )
{
    EntityManager::get()->registerUpdate( this );     // register entity in order to get updated per simulation step

    // register entity attributes
    _attributeManager.addAttribute( "geomTypes"  , _geomTypes  );
    _attributeManager.addAttribute( "position"   , _position   );
}

EnGeomEmitter::~EnGeomEmitter()
{
    // deregister entity, it is not necessary for entities which 'die' at application exit time
    //  as the entity manager clears the entity list on app exit
    EntityManager::get()->registerUpdate( this, false );
}

void EnGeomEmitter::postInitialize()
{
    setPosition( _position );
    vector< string > tokens;
    BaseEntity*      p_entity = NULL;
    explode( _geomTypes, " ", &tokens );
    for ( size_t cnt = 0; cnt < tokens.size(); cnt++ )
    {
        p_entity = EntityManager::get()->findInstance( tokens[ cnt ] );
        if ( !p_entity )
        {
            cout << "***EnGeomEmitter: entity type not found : '" << tokens[ cnt ] << "'" << endl;
            continue;
        }

        _geomStock.push_back( p_entity );
    }
}

void EnGeomEmitter::updateEntity( float deltaTime )
{
    _time += deltaTime;

    // create new geom every 2 seconds
    if ( _time > 2.5f )
    {
        _time = 0;

        // take a random geom type
        int ran = rand() % _geomStock.size();
        BaseEntity* p_entity = _geomStock[ ran ];

        // clone the found geom
        char buf[ 4 ];
        p_entity = p_entity->clone( p_entity->getInstanceName() + string( itoa( _geomCount, buf, 10 ) ), ( Group* )Application::get()->getSceneRootNode() );

        // set a random position before entity gets initialized
        Vec3f randPos( ( float )( ( rand() % 100 ) - 100 ), ( float )( ( rand() % 80 ) - 80 ), ( float )( rand() % 30 ) );
        p_entity->getAttributeManager().setAttributeValue( "position", _position + randPos );

        // setup entities
        p_entity->initialize();
        p_entity->postInitialize();

        float live = ( float )( 15 + rand() % 20 );
        _geoms.push_back( make_pair( p_entity, live ) );
    }

    // update geoms
    vector< std::pair< BaseEntity*, float > >::iterator geom = _geoms.begin(), geomEnd = _geoms.end();
    for ( ; geom != geomEnd; geom++ )
    {
        if ( ( geom->second -= deltaTime ) < 0 )
        {
            EntityManager::get()->deleteEntity( geom->first );
            _geoms.erase( geom );
            // we delete objects in a smooth way
            break;
        }
    }
}
