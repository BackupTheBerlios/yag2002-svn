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

#include <vrc_main.h>
#include "vrc_spawnpoint.h"
#include "vrc_player.h"

namespace vrc
{

//! Implement and register the spawn point entity factory
YAF3D_IMPL_ENTITYFACTORY( SpawnPointEntityFactory );

std::vector< EnSpawnPoint* > EnSpawnPoint::_allSpawnPoints;

EnSpawnPoint::EnSpawnPoint()
{
    // register entity attributes
    getAttributeManager().addAttribute( "position"  , _position  );
    getAttributeManager().addAttribute( "rotation"  , _rotation  );

    // store this new spawn point for later lookup
    _allSpawnPoints.push_back( this );
}

EnSpawnPoint::~EnSpawnPoint()
{
}

void EnSpawnPoint::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle notifications
    switch( notification.getId() )
    {
        // clear spawn points list on every level loading
        case YAF3D_NOTIFY_DELETING_ENTITIES:

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

    // register entity in order to get notifications
    yaf3d::EntityManager::get()->registerNotification( this, true );   
}

bool EnSpawnPoint::getNextSpawnPoint( osg::Vec3f& pos, osg::Quat& rot )
{
    if ( !_allSpawnPoints.size() )
    {
        log_error << "no SpawnPoint entities found in level, cannot select one!" << std::endl;
        return false;
    }

    std::vector< EnPlayer* > players;
    std::vector< yaf3d::BaseEntity* > entities;

    // get all existing player entities
    {
        yaf3d::EntityManager::get()->getAllEntities( entities );
        std::vector< yaf3d::BaseEntity* >::iterator p_beg = entities.begin(), p_end = entities.end();
        for ( ; p_beg != p_end; ++p_beg )
        {
            if ( ( *p_beg )->getTypeName() == ENTITY_NAME_PLAYER )
                players.push_back( static_cast< EnPlayer* >( *p_beg ) );
        }
        if ( !players.size() )
            return false;
    }
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
        std::vector< EnPlayer* >::iterator p_beg = players.begin(), p_end = players.end();
        for ( ; p_beg != p_end; ++p_beg )
        {
            if ( ( p_spawn->getSpawnPosition() - ( *p_beg )->getPosition() ).length2() > ( SPAWN_MIN_FREE_RADIUS * SPAWN_MIN_FREE_RADIUS ) )
                break;
        }
        // if none of players occupy the spawn point then we are done
        if ( p_beg != p_end )
        {
            p_spawnentity = p_spawn;
            dobreak = true;
        }

        ++cnt;
    } 
    while ( !dobreak && ( cnt < 10 ) );

    // could we find a spawn point?
    if ( !p_spawnentity )
        return false;

    // copy position and rotation
    pos = p_spawnentity->getSpawnPosition();
    rot = p_spawnentity->getSpawnRotation();

    log_debug << "player takes SpawnPoint '" 
        << p_spawnentity->getInstanceName() 
        << "' at position '" << pos.x() <<  " " <<  pos.y() << " " << pos.z() << std::endl;

    return true;
}

} // namespace vrc
