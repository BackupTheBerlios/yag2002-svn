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
 # player implementation for standalone mode
 #
 #   date of creation:  05/28/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_playerimplstandalone.h"
#include "vrc_player.h"
#include "chat/vrc_chatmgr.h"
#include "vrc_playerphysics.h"
#include "vrc_playeranim.h"
#include "vrc_playersound.h"
#include "vrc_inputhandler.h"
#include "../visuals/vrc_camera.h"

namespace vrc
{

PlayerImplStandalone::PlayerImplStandalone( EnPlayer* player ) :
BasePlayerImplementation( player ),
_p_inputHandler( NULL )
{
    // add local player in player utils
    vrc::gameutils::PlayerUtils::get()->setLocalPlayer( player );
}

PlayerImplStandalone::~PlayerImplStandalone()
{
    // destroy input handler
    _p_inputHandler->destroyHandler();

    // remove local player in player utils
    vrc::gameutils::PlayerUtils::get()->setLocalPlayer( NULL );
}

void PlayerImplStandalone::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle some notifications
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_MENU_ENTER:

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

            // players are all rendered in menu, regardless their camera mode
            _p_playerAnimation->enableRendering( true );
            addToSceneGraph();

            break;

        case YAF3D_NOTIFY_MENU_LEAVE:
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

            // if we are in ego mode then disable player avatar rendering
            if ( _cameraMode == Ego )
            {
                _p_playerAnimation->enableRendering( false );
                removeFromSceneGraph();
            }
        }
        break;

        case YAF3D_NOTIFY_SHUTDOWN:
        case YAF3D_NOTIFY_UNLOAD_LEVEL:
        {
            // destroy the chat manager
            destroyChatManager();
        }
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
    log_info << "  setup player implementation Standalone ..." << std::endl;

    // attach camera entity
    log_debug << "   - searching for camera entity '" << PLAYER_CAMERA_ENTITIY_NAME << "'..." << std::endl;
    // get camera entity
    _p_camera = dynamic_cast< EnCamera* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_CAMERA, PLAYER_CAMERA_ENTITIY_NAME ) );
    if ( _p_camera )
    {
        log_debug << "   -  camera entity successfully attached" << std::endl;
    }
    else
    {
        log_error << "   could not attach player camera entity" << std::endl;
    }

    // attach physics entity
    log_debug << "   - searching for physics entity '" << _playerAttributes._physicsEntity << "' ..." << std::endl;
    // find and attach physics component
    _p_playerPhysics = dynamic_cast< EnPlayerPhysics* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_PLPHYS, _playerAttributes._physicsEntity ) );
    if ( _p_playerPhysics )
    {
        _p_playerPhysics->setPlayer( this );
        log_debug << "   -  physics entity successfully attached" << std::endl;
    }
    else
    {
        log_error << "   could not attach player physics entity" << std::endl;
    }

    // attach animation entity
    log_debug << "   - searching for animation entity '" << _playerAttributes._animationEntity << "' ..." << std::endl;
    // find and attach animation component
    _p_playerAnimation = dynamic_cast< EnPlayerAnimation* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_PLANIM, _playerAttributes._animationEntity ) );
    if ( _p_playerAnimation )
    {
        _p_playerAnimation->setPlayer( this );
        log_debug << "   -  animation entity successfully attached" << std::endl;

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
        log_error << "   could not attach player animation entity" << std::endl;
    }

    // attach sound entity
    log_debug << "   - searching for sound entity '" << _playerAttributes._soundEntity << "' ..." << std::endl;
    // find and attach sound component, tollerate missing sound for now
    _p_playerSound = dynamic_cast< EnPlayerSound* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_PLSOUND, _playerAttributes._soundEntity ) );
    if ( !_p_playerSound )
        log_error << "   could not find sound entity '" << _playerAttributes._soundEntity << "' of type PlayerSound. player sound deactivated" << std::endl;
    else
    {
        _p_playerSound->setPlayer( this );
        log_debug << "   -  sound entity successfully attached" << std::endl;
    }

    // setup camera mode
    setCameraMode( _cameraMode );

    // create the chat manager
    if ( !createChatManager() )
    {
        log_error << "   -  could not create chat system" << std::endl;
    }

    log_info << "  player implementation successfully initialized" << std::endl;

    // create only the input handler when animation and physics are attached
    if ( _p_playerAnimation && _p_playerPhysics )
    {
        // create a new input handler for this player
        _p_inputHandler = new PlayerIHCharacterCameraCtrl< PlayerImplStandalone >( this, getPlayerEntity() );
        _p_inputHandler->setMenuEnabled( false );

        // get configuration settings
        getConfiguration();
    }

    // set initial camera transformation
    if ( _p_camera )
       _p_camera->setCameraTransformation( getPlayerPosition(), getPlayerRotation() );
}

void PlayerImplStandalone::getConfiguration()
{
    std::string playername;
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_PLAYER_NAME, playername );
    _p_player->setPlayerName( playername );

    // setup key bindings
    std::string keyname;
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_KEY_MOVE_FORWARD, keyname );
    _p_inputHandler->_keyCodeMoveForward = yaf3d::KeyMap::get()->getCode( keyname );

    yaf3d::Configuration::get()->getSettingValue( VRC_GS_KEY_MOVE_BACKWARD, keyname );
    _p_inputHandler->_keyCodeMoveBackward = yaf3d::KeyMap::get()->getCode( keyname );

    yaf3d::Configuration::get()->getSettingValue( VRC_GS_KEY_MOVE_LEFT, keyname );
    _p_inputHandler->_keyCodeMoveLeft = yaf3d::KeyMap::get()->getCode( keyname );

    yaf3d::Configuration::get()->getSettingValue( VRC_GS_KEY_MOVE_RIGHT, keyname );
    _p_inputHandler->_keyCodeMoveRight = yaf3d::KeyMap::get()->getCode( keyname );

    yaf3d::Configuration::get()->getSettingValue( VRC_GS_KEY_JUMP, keyname );
    _p_inputHandler->_keyCodeJump = yaf3d::KeyMap::get()->getCode( keyname );

    yaf3d::Configuration::get()->getSettingValue( VRC_GS_KEY_CAMERAMODE, keyname );
    _p_inputHandler->_keyCodeCameraMode = yaf3d::KeyMap::get()->getCode( keyname );

    yaf3d::Configuration::get()->getSettingValue( VRC_GS_KEY_CHATMODE, keyname );
    _p_inputHandler->_keyCodeChatMode = yaf3d::KeyMap::get()->getCode( keyname );

    yaf3d::Configuration::get()->getSettingValue( VRC_GS_INVERTMOUSE, _p_inputHandler->_invertedMouse );
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_MOUSESENS, _p_inputHandler->_mouseSensitivity );
}

void PlayerImplStandalone::update( float deltaTime )
{
    // first update the physics entity
    getPlayerPhysics()->updateEntity( deltaTime );

    // update player's actual position and rotation once per frame
    getPlayerEntity()->setPosition( _currentPos ); 
    getPlayerEntity()->setRotation( _currentRot ); 

    // adjust the camera to updated position and rotation. the physics updates the translation of player.
    if ( _p_camera )
        _p_camera->setCameraTransformation( getPlayerPosition(), getPlayerRotation() );

    getChatManager()->update( deltaTime );

    // update sound
    getPlayerSound()->updatePosition( _currentPos );
}

} // namespace vrc
