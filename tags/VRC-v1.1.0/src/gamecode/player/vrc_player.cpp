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

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_player.h"
#include "vrc_playerimplstandalone.h"
#include "vrc_playerimplserver.h"
#include "vrc_playerimplclient.h"
#include "vrc_spawnpoint.h"

namespace vrc
{

//! Implement and register the player entity factory
YAF3D_IMPL_ENTITYFACTORY( PlayerEntityFactory );
    
EnPlayer::EnPlayer() :
_gameMode( yaf3d::GameState::get()->getMode() ),
_p_playerImpl( NULL ),
_deltaTime( 0.03f )
{
    log_debug << "creating player entity"  << getInstanceName() << ", time: " << yaf3d::getTimeStamp() << std::endl;

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
    log_debug << "destroying player entity '"  << getInstanceName() << "', time: " << yaf3d::getTimeStamp() << std::endl;

    // send out notification to registered entities
    std::vector< yaf3d::BaseEntity* >::iterator p_beg = _deletionNotifications.begin(), p_end = _deletionNotifications.end();
    yaf3d::EntityNotification ennotify( YAF3D_NOTIFY_PLAYER_DESTRUCTION );
    for ( ; p_beg != p_end; ++p_beg )
        yaf3d::EntityManager::get()->sendNotification( ennotify, *p_beg );
    
    if ( _p_playerImpl )
        delete _p_playerImpl;
}

void EnPlayer::handleNotification( const yaf3d::EntityNotification& notification )
{
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_SHUTDOWN:
        {
            // send out deletion notification to registered entities
            std::vector< yaf3d::BaseEntity* >::iterator p_beg = _deletionNotifications.begin(), p_end = _deletionNotifications.end();
            yaf3d::EntityNotification ennotify( YAF3D_NOTIFY_PLAYER_DESTRUCTION );
            for ( ; p_beg != p_end; ++p_beg )
                yaf3d::EntityManager::get()->sendNotification( ennotify, *p_beg );

            _deletionNotifications.clear();
        }
        break;

        default:
            ;
    }

    if ( _p_playerImpl )
        _p_playerImpl->handleNotification( notification );
}

void EnPlayer::registerNotifyDeletion( yaf3d::BaseEntity* p_entity )
{
    // check if the entity is already registered
    std::vector< yaf3d::BaseEntity* >::iterator p_beg = _deletionNotifications.begin(), p_end = _deletionNotifications.end();
    for ( ; p_beg != p_end; ++p_beg )
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
        case yaf3d::GameState::Standalone:
        {
            spawn();
            _p_playerImpl = new PlayerImplStandalone( this );
            _p_playerImpl->initialize();
        }
        break;

        case yaf3d::GameState::Client:
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

        case yaf3d::GameState::Server:
            // player entity is created in networking component for server
            spawn();
            break;

        default:
            assert( NULL && "unsupported game mode" );
    }

    yaf3d::EntityManager::get()->registerNotification( this, true );   // register entity in order to get notifications (e.g. from menu entity)
}

void EnPlayer::postInitialize()
{
    if ( !_p_playerImpl )
    {
        log_error << "player implementation does not exist! are you loading the player entity in right game mode?" << std::endl;
        return;
    }

    _p_playerImpl->postInitialize();
    // register entity in order to get updated per simulation step.
    yaf3d::EntityManager::get()->registerUpdate( this, true );
}

void EnPlayer::spawn()
{
    osg::Quat  rotation;
    osg::Vec3f position;

    // check if the level has spawn points
    EnSpawnPoint* p_spwanEntity = static_cast< EnSpawnPoint* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_SPAWNPOINT ) );
    if ( p_spwanEntity )
    {
        if ( !EnSpawnPoint::getNextSpawnPoint( position, rotation ) )
        {
            log_error << "EnPlayer: all spawn points are occupied, taking default position and rotation!" << std::endl;
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

const std::string EnPlayer::getPlayerName() const
{
    if ( !_p_playerImpl )
        return std::string( "NOT-SET" );

    return _p_playerImpl->getPlayerName();
}

void EnPlayer::setPlayerName( const std::string& name )
{
    if( !_p_playerImpl )
        return;

    _p_playerImpl->setPlayerName( name );
}

void EnPlayer::updateEntity( float deltaTime )
{
    _deltaTime = deltaTime;
    _p_playerImpl->update( deltaTime );
}

} // namespace vrc
