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
 # entity spawn point used by player when it enters the world
 #  the player entity can use this entity to get next possible
 #  spawn point considering other players which may occupy some
 #  spawn points ( e.g. because they just respawned too ).
 #
 #   date of creation:  06/21/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_spawnpoint.h"
#include "ctd_player.h"

using namespace std;
using namespace CTD; 

//! Implement and register the spawn point entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( SpawnPointEntityFactory );

std::vector< EnSpawnPoint* > EnSpawnPoint::_allSpawnPoints;

EnSpawnPoint::EnSpawnPoint()
{
    // register entity attributes
    _attributeManager.addAttribute( "position"  , _position  );
    _attributeManager.addAttribute( "rotation"  , _rotation  );

    // store this new spawn point for later lookup
    _allSpawnPoints.push_back( this );

    // register entity in order to get notifications
    EntityManager::get()->registerNotification( this, true );   
}

EnSpawnPoint::~EnSpawnPoint()
{
}

void EnSpawnPoint::handleNotification( const EntityNotification& notification )
{
    // handle notifications
    switch( notification.getId() )
    {
        // clear spawn points list on every level loading
        case CTD_NOTIFY_DELETING_ENTITIES:

            if ( _allSpawnPoints.size() )
                _allSpawnPoints.clear();

            break;

        default:
            ;
    }
}

void EnSpawnPoint::initialize()
{
    _quat = osg::Quat( 
                        osg::DegreesToRadians( _rotation.x() ), osg::Vec3f( 1.0f, 0.0f, 0.0f ),
                        osg::DegreesToRadians( _rotation.y() ), osg::Vec3f( 0.0f, 1.0f, 0.0f ),
                        osg::DegreesToRadians( _rotation.z() ), osg::Vec3f( 0.0f, 0.0f, 1.0f )
                      );
}

bool EnSpawnPoint::getNextSpawnPoint( osg::Vec3f& pos, osg::Quat& rot )
{
    if ( !_allSpawnPoints.size() )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "no SpawnPoint entities found in level, cannot select one!" << endl;
        return false;
    }

    std::vector< EnPlayer* > players;
    std::vector< BaseEntity* > entities;
    EntityManager::get()->getAllEntities( entities );
    std::vector< BaseEntity* >::iterator pp_beg = entities.begin(), pp_end = entities.end();
    for ( ; pp_beg != pp_end; pp_beg++ )
    {
        if ( ( *pp_beg )->getTypeName() == ENTITY_NAME_PLAYER )
            players.push_back( static_cast< EnPlayer* >( *pp_beg ) );
    }
    if ( !players.size() )
        return false;

    // try to find a non-occupied spawn point by choosing a random spawn point and checking it against all player entities
    //  we try that 10 times
    unsigned int  cnt           = 0;
    bool          dobreak       = false;
    EnSpawnPoint* p_spawnentity = NULL;
    do 
    {
        unsigned int randspawn = ( unsigned int )( rand() % _allSpawnPoints.size() );
        EnSpawnPoint* p_spawn  = _allSpawnPoints[ randspawn ];

        // check the randomly choosen spawn point against all players
        std::vector< EnPlayer* >::iterator pp_beg = players.begin(), pp_end = players.end();
        for ( ; pp_beg != pp_end; pp_beg++ )
        {
            if ( ( p_spawn->getSpawnPosition() - ( *pp_beg )->getPosition() ).length2() > ( SPAWN_MIN_FREE_RADIUS * SPAWN_MIN_FREE_RADIUS ) )
                break;
        }
        // if none of players occupy the spawn point then we are done
        if ( pp_beg != pp_end )
        {
            p_spawnentity = p_spawn;
            dobreak = true;
        }

        cnt++;
    } 
    while ( !dobreak && ( cnt < 10 ) );

    // could we find a spawn point?
    if ( !p_spawnentity )
        return false;

    // copy position and rotation
    pos = p_spawnentity->getSpawnPosition();
    rot = p_spawnentity->getSpawnRotation();

    log << Log::LogLevel( Log::L_DEBUG ) << "player takes SpawnPoint '" 
        << p_spawnentity->getInstanceName() 
        << "' at position '" << pos.x() <<  " " <<  pos.y() << " " << pos.z() << endl;

    return true;
}
