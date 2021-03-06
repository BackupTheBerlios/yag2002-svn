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
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_player.h"
#include "vrc_playerimplstandalone.h"
#include "vrc_playerimplserver.h"
#include "vrc_playerimplclient.h"
#include "vrc_spawnpoint.h"
#include "properties/vrc_propgui.h"
#include "../storage/vrc_storageclient.h"
#include "../storage/vrc_userinventory.h"

namespace vrc
{

//! Implement and register the player entity factory
YAF3D_IMPL_ENTITYFACTORY( PlayerEntityFactory )

EnPlayer::EnPlayer() :
_gameMode( yaf3d::GameState::get()->getMode() ),
_p_playerImpl( NULL ),
_p_propertyGui( NULL ),
_p_userInventory( NULL ),
_networkID( -1 ),
_voiceChatEnabled( false ),
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
    
    if ( _p_playerImpl )
        delete _p_playerImpl;

    if ( _p_propertyGui )
        delete _p_propertyGui;
}

void EnPlayer::handleNotification( const yaf3d::EntityNotification& notification )
{
    if ( _p_playerImpl )
        _p_playerImpl->handleNotification( notification );

    // handle the menu entering/leaving for property GUI
    switch ( notification.getId() )
    {
        case YAF3D_NOTIFY_MENU_ENTER:
        {
            if ( _p_propertyGui )
                _p_propertyGui->enable( false );
        }
        break;

        case YAF3D_NOTIFY_MENU_LEAVE:
        {
            if ( _p_propertyGui )
                _p_propertyGui->enable( true );
        }
        break;

        default:
            ;
    }
}

void EnPlayer::initialize()
{
    // set initial position and rotation, this will be changed later by server when in client mode
    setPosition( _attributeContainer._pos );
    setRotation( osg::Quat( osg::DegreesToRadians( _attributeContainer._rot ), osg::Vec3f( 0.0f, 0.0f, 1.0f ) ) );

    // build and init the player implementation
    switch ( _gameMode )
    {
        case yaf3d::GameState::Standalone:
        {
            spawn();
            _p_playerImpl = new PlayerImplStandalone( this );
            _p_playerImpl->initialize();

            // create the player inventory for standalone mode
            _p_userInventory = new UserInventory( 0 );
            // create the property gui
            _p_propertyGui = new PropertyGui( _p_userInventory );
            // set the inventory in player utils
            gameutils::PlayerUtils::get()->setPlayerInventory( _p_userInventory );
        }
        break;

        case yaf3d::GameState::Client:
        {
            // the client can be local or remote. if it is remote then the player entity has been created
            //  via player networking component; for remote clients _p_playerImpl ist aready created when we are at this point
            if ( !_p_playerImpl )
            {
                _p_playerImpl = new PlayerImplClient( this );
                _p_playerImpl->initialize();

                // create the player inventory for clients without authentification (guests)
                _p_userInventory = gameutils::PlayerUtils::get()->getPlayerInventory();
                if ( !_p_userInventory )
                    _p_userInventory = new UserInventory( 0 );

                // set the inventory in player utils
                gameutils::PlayerUtils::get()->setPlayerInventory( _p_userInventory );

                // create the property gui with given user inventory
                _p_propertyGui = new PropertyGui( _p_userInventory );
            }
        }
        break;

        case yaf3d::GameState::Server:
        {
            // player entity is created in networking component for server
            spawn();
        }
        break;

        default:
            assert( NULL && "unsupported game mode" );
    }

    // set initial position and rotation in player implementation
    if ( _p_playerImpl )
    {
        _p_playerImpl->setPlayerPosition( getPosition() );
        _p_playerImpl->setPlayerRotation( getRotation() );
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

void EnPlayer::enableVoiceChat( bool en )
{
    if ( _p_playerImpl )
        _p_playerImpl->enableVoiceChat( en );
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
