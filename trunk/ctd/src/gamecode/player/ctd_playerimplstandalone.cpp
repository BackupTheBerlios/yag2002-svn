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
 # player implementation for standalone mode
 #
 #   date of creation:  05/28/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_playerimplstandalone.h"
#include "ctd_player.h"
#include "chat/ctd_chatmgr.h"
#include "ctd_playerphysics.h"
#include "ctd_playeranim.h"
#include "ctd_playersound.h"
#include "ctd_inputhandler.h"
#include "../ctd_gameutils.h"
#include "../visuals/ctd_camera.h"

using namespace std;

namespace CTD
{

PlayerImplStandalone::PlayerImplStandalone( EnPlayer* player ) :
BasePlayerImplementation( player ),
_p_inputHandler( NULL )
{
}

PlayerImplStandalone::~PlayerImplStandalone()
{
    // destroy input handler
    _p_inputHandler->destroyHandler();
    // destroy the chat manager
    destroyChatManager();
}

void PlayerImplStandalone::handleNotification( const EntityNotification& notification )
{
    // handle some notifications
    switch( notification.getId() )
    {
        case CTD_NOTIFY_MENU_ENTER:

            getChatManager()->show( false );
            if ( _p_inputHandler )
                _p_inputHandler->setMenuEnabled( true );
            
            // reset player's movement and sound
            if ( _p_playerPhysics )
                _p_playerPhysics->stopMovement();
            if ( _p_playerAnimation )
                _p_playerAnimation->animIdle();
            if ( _p_playerSound )
                _p_playerSound->stopPlayingAll();

            // very important: diable the camera when we enter menu!
            if ( _p_camera )
                _p_camera->setEnable( false );

            break;

        case CTD_NOTIFY_MENU_LEAVE:
        {
            getChatManager()->show( true );

            if ( _p_inputHandler )
            {
                _p_inputHandler->setMenuEnabled( false );
                // refresh our configuration settings
                getConfiguration();
            }
 
            // very important: enable the camera when we leave menu!
            if ( _p_camera )
                _p_camera->setEnable( true );
        }
        break;

        case CTD_NOTIFY_UNLOAD_LEVEL:
        break;

        default:
            ;
    }
}

void PlayerImplStandalone::initialize()
{    
    // setup position, rotation and move direction
    _currentPos = getPlayerEntity()->getPosition();
    _currentRot = getPlayerEntity()->getRotation();
    _moveDir    = _currentRot * osg::Vec3f( 0, 1, 0 );
    osg::Quat::value_type rot;
    osg::Vec3f            vec;
    getPlayerEntity()->getRotation().getRotate( rot, vec );
    _rotZ = rot;
}

void PlayerImplStandalone::postInitialize()
{
    log << Log::LogLevel( Log::L_INFO ) << "  setup player implementation Standalone ..." << endl;

    // attach camera entity
    log << Log::LogLevel( Log::L_DEBUG ) << "   - searching for camera entity '" << PLAYER_CAMERA_ENTITIY_NAME << "'..." << endl;
    // get camera entity
    _p_camera = dynamic_cast< EnCamera* >( EntityManager::get()->findEntity( ENTITY_NAME_CAMERA, PLAYER_CAMERA_ENTITIY_NAME ) );
    if ( _p_camera )
    {
        log << Log::LogLevel( Log::L_DEBUG ) << "   -  camera entity successfully attached" << endl;
    }
    else
    {
        log << Log::LogLevel( Log::L_ERROR ) << "   could not attach player camera entity" << endl;
    }

    // attach physics entity
    log << Log::LogLevel( Log::L_DEBUG ) << "   - searching for physics entity '" << _playerAttributes._physicsEntity << "' ..." << endl;
    // find and attach physics component
    _p_playerPhysics = dynamic_cast< EnPlayerPhysics* >( EntityManager::get()->findEntity( ENTITY_NAME_PLPHYS, _playerAttributes._physicsEntity ) );
    if ( _p_playerPhysics )
    {
        _p_playerPhysics->setPlayer( this );
        log << Log::LogLevel( Log::L_DEBUG ) << "   -  physics entity successfully attached" << endl;
    }
    else
    {
        log << Log::LogLevel( Log::L_ERROR ) << "   could not attach player physics entity" << endl;
    }

    // attach animation entity
    log << Log::LogLevel( Log::L_DEBUG ) << "   - searching for animation entity '" << _playerAttributes._animationEntity << "' ..." << endl;
    // find and attach animation component
    _p_playerAnimation = dynamic_cast< EnPlayerAnimation* >( EntityManager::get()->findEntity( ENTITY_NAME_PLANIM, _playerAttributes._animationEntity ) );
    if ( _p_playerAnimation )
    {
        _p_playerAnimation->setPlayer( this );
        log << Log::LogLevel( Log::L_DEBUG ) << "   -  animation entity successfully attached" << endl;

        if ( _cameraMode == Ego ) // in ego mode we won't render our character
        {
            _p_playerAnimation->enableRendering( false );
        }
        else // if in spheric mode disable the mouse pointer
        {
            gameutils::GuiUtils::get()->showMousePointer( false );
        }
    }
    else
    {
        log << Log::LogLevel( Log::L_ERROR ) << "   could not attach player animation entity" << endl;
    }

    // attach sound entity
    log << Log::LogLevel( Log::L_DEBUG ) << "   - searching for sound entity '" << _playerAttributes._soundEntity << "' ..." << endl;
    // find and attach sound component, tollerate missing sound for now
    _p_playerSound = dynamic_cast< EnPlayerSound* >( EntityManager::get()->findEntity( ENTITY_NAME_PLSOUND, _playerAttributes._soundEntity ) );
    if ( !_p_playerSound )
        log << Log::LogLevel( Log::L_ERROR ) << "   could not find sound entity '" << _playerAttributes._soundEntity << "' of type PlayerSound. player sound deactivated" << endl;
    else
    {
        _p_playerSound->setPlayer( this );
        log << Log::LogLevel( Log::L_DEBUG ) << "   -  sound entity successfully attached" << endl;
    }

    // setup camera mode
    setCameraMode( _cameraMode );

    // create the chat manager
    if ( !createChatManager() )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "   -  could not create chat system" << endl;
    }

    log << Log::LogLevel( Log::L_INFO ) << "  player implementation successfully initialized" << endl;

    // create only the input handler when animation and physics are attached
    if ( _p_playerAnimation && _p_playerPhysics )
    {
        // create a new input handler for this player
        _p_inputHandler = new PlayerIHCharacterCameraCtrl< PlayerImplStandalone >( this, getPlayerEntity() );
        _p_inputHandler->setMenuEnabled( false );

        // get configuration settings
        getConfiguration();
    }
}

void PlayerImplStandalone::getConfiguration()
{
    std::string playername;
    Configuration::get()->getSettingValue( CTD_GS_PLAYER_NAME, playername );
    _p_player->setPlayerName( playername );

    // setup key bindings
    std::string keyname;
    Configuration::get()->getSettingValue( CTD_GS_KEY_MOVE_FORWARD, keyname );
    _p_inputHandler->_keyCodeMoveForward = KeyMap::get()->getCode( keyname );

    Configuration::get()->getSettingValue( CTD_GS_KEY_MOVE_BACKWARD, keyname );
    _p_inputHandler->_keyCodeMoveBackward = KeyMap::get()->getCode( keyname );

    Configuration::get()->getSettingValue( CTD_GS_KEY_MOVE_LEFT, keyname );
    _p_inputHandler->_keyCodeMoveLeft = KeyMap::get()->getCode( keyname );

    Configuration::get()->getSettingValue( CTD_GS_KEY_MOVE_RIGHT, keyname );
    _p_inputHandler->_keyCodeMoveRight = KeyMap::get()->getCode( keyname );

    Configuration::get()->getSettingValue( CTD_GS_KEY_JUMP, keyname );
    _p_inputHandler->_keyCodeJump = KeyMap::get()->getCode( keyname );

    Configuration::get()->getSettingValue( CTD_GS_KEY_CAMERAMODE, keyname );
    _p_inputHandler->_keyCodeCameraMode = KeyMap::get()->getCode( keyname );

    Configuration::get()->getSettingValue( CTD_GS_KEY_CHATMODE, keyname );
    _p_inputHandler->_keyCodeChatMode = KeyMap::get()->getCode( keyname );

    Configuration::get()->getSettingValue( CTD_GS_INVERTMOUSE, _p_inputHandler->_invertedMouse );
    Configuration::get()->getSettingValue( CTD_GS_MOUSESENS, _p_inputHandler->_mouseSensitivity );
}

void PlayerImplStandalone::update( float deltaTime )
{
    // update player's actual position and rotation once per frame
    getPlayerEntity()->setPosition( _currentPos ); 
    getPlayerEntity()->setRotation( _currentRot ); 

    // adjust the camera to updated position and rotation. the physics updates the translation of player.
    if ( _p_camera )
        _p_camera->setCameraTranslation( getPlayerPosition(), getPlayerRotation() );

    getChatManager()->update( deltaTime );
}

} // namespace CTD
