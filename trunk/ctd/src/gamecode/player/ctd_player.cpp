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
 # player entiy
 #
 # the actual behaviour is defined by one of its implementations 
 #  for Server, Client, or Standalone.
 #
 #   date of creation:  01/14/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_player.h"
#include "ctd_playerimplstandalone.h"
#include "ctd_playerimplserver.h"
#include "ctd_playerimplclient.h"
#include "ctd_spawnpoint.h"

using namespace osg;
using namespace std;

namespace CTD
{

//! Implement and register the player entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( PlayerEntityFactory );
    
EnPlayer::EnPlayer() :
_gameMode( GameState::get()->getMode() ),
_p_playerImpl( NULL )
{
    CTD::log << CTD::Log::LogLevel( CTD::Log::L_DEBUG ) << "creating player entity"  << getInstanceName() << ", time: " << CTD::getTimeStamp() << endl;

    EntityManager::get()->registerNotification( this, true );   // register entity in order to get notifications (e.g. from menu entity)

    // assign some defaults
    _attributeContainer._chatGuiConfig = "gui/chat.xml";
    _attributeContainer._rot = 0;

    getAttributeManager().addAttribute( "physicsentity"             , _attributeContainer._physicsEntity        );
    getAttributeManager().addAttribute( "animationentity"           , _attributeContainer._animationEntity      );
    getAttributeManager().addAttribute( "soundentity"               , _attributeContainer._soundEntity          );
    getAttributeManager().addAttribute( "position"                  , _attributeContainer._pos                  );
    getAttributeManager().addAttribute( "rotation"                  , _attributeContainer._rot                  );
    getAttributeManager().addAttribute( "cameraPosOffsetSpheric"    , _attributeContainer._camPosOffsetSpheric  );
    getAttributeManager().addAttribute( "cameraRotOffsetSpheric"    , _attributeContainer._camRotOffsetSpheric  );
    getAttributeManager().addAttribute( "cameraPosOffsetEgo"        , _attributeContainer._camPosOffsetEgo      );
    getAttributeManager().addAttribute( "cameraRotOffsetEgo"        , _attributeContainer._camRotOffsetEgo      );
    getAttributeManager().addAttribute( "chatGuiConfig"             , _attributeContainer._chatGuiConfig        );
}

EnPlayer::~EnPlayer()
{
    CTD::log << CTD::Log::LogLevel( CTD::Log::L_DEBUG ) << "destroying player entity '"  << getInstanceName() << "', time: " << CTD::getTimeStamp() << endl;

    // send out notification to registered entities
    std::vector< BaseEntity* >::iterator p_beg = _deletionNotifications.begin(), p_end = _deletionNotifications.end();
    EntityNotification ennotify( CTD_NOTIFY_PLAYER_DESTRUCTION );
    for ( ; p_beg != p_end; p_beg++ )
        EntityManager::get()->sendNotification( ennotify, *p_beg );
    
    if ( _p_playerImpl )
        delete _p_playerImpl;
}

void EnPlayer::handleNotification( const EntityNotification& notification )
{
    switch( notification.getId() )
    {
        case CTD_NOTIFY_SHUTDOWN:
        {
            // send out deletion notification to registered entities
            std::vector< BaseEntity* >::iterator p_beg = _deletionNotifications.begin(), p_end = _deletionNotifications.end();
            EntityNotification ennotify( CTD_NOTIFY_PLAYER_DESTRUCTION );
            for ( ; p_beg != p_end; p_beg++ )
                EntityManager::get()->sendNotification( ennotify, *p_beg );

            _deletionNotifications.clear();
        }
        break;

        default:
            ;
    }

    if ( _p_playerImpl )
        _p_playerImpl->handleNotification( notification );
}

void EnPlayer::registerNotifyDeletion( BaseEntity* p_entity )
{
    // check if the entity is already registered
    std::vector< BaseEntity* >::iterator p_beg = _deletionNotifications.begin(), p_end = _deletionNotifications.end();
    for ( ; p_beg != p_end; p_beg++ )
        if ( *p_beg == p_entity )
            break;

    // if entity is already registered then ignore the request
    if ( _deletionNotifications.size() && ( p_beg != p_end ) )
        return;

    _deletionNotifications.push_back( p_entity );
}

void EnPlayer::initialize()
{
    // build and init the player implementation
    switch ( _gameMode )
    {
        case GameState::Standalone:
        {
            spawn();
            _p_playerImpl = new PlayerImplStandalone( this );
            _p_playerImpl->initialize();
        }
        break;

        case GameState::Client:
        {
            // the client can be local or remote. if it is remote then the player entity has been created
            //  via player networking component; for remote clients _p_playerImpl ist aready created when we are a this point
            if ( !_p_playerImpl )
            {
                _p_playerImpl = new PlayerImplClient( this );
                _p_playerImpl->initialize();
            }
        }
        break;

        case GameState::Server:
            spawn();
            _p_playerImpl = new PlayerImplServer( this );
            _p_playerImpl->initialize();
            break;

        default:
            assert( NULL && "unsupported game mode" );
    }
}

void EnPlayer::postInitialize()
{
    _p_playerImpl->postInitialize();
    // register entity in order to get updated per simulation step.
    EntityManager::get()->registerUpdate( this, true );
}

void EnPlayer::spawn()
{
    osg::Quat  rotation;
    osg::Vec3f position;

    // check if the level has spawn points
    EnSpawnPoint* p_spwanEntity = static_cast< EnSpawnPoint* >( EntityManager::get()->findEntity( ENTITY_NAME_SPAWNPOINT ) );
    if ( p_spwanEntity )
    {
        if ( !EnSpawnPoint::getNextSpawnPoint( position, rotation ) )
        {
            log << Log::LogLevel( Log::L_ERROR ) << "EnPlayer: all spawn points are occupied, taking default position and rotation!" << endl;
        }
    }
    else
    {
        position = _attributeContainer._pos;
        rotation = osg::Quat( osg::DegreesToRadians( _attributeContainer._rot ), osg::Vec3f( 0.0f, 0.0f, 1.0f ) );
    }
    // set initial rotation and position
    setPosition( position );
    setRotation( rotation );
}

void EnPlayer::updateEntity( float deltaTime )
{
    _p_playerImpl->update( deltaTime );
}

} // namespace CTD
