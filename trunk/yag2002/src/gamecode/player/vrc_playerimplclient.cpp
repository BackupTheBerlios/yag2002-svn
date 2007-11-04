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
 # player implementation for client mode
 #
 #   date of creation:  05/31/2005
 #
 #   author:            ali botorabi (boto)
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_player.h"
#include "chat/vrc_chatmgr.h"
#include "vrc_playersound.h"
#include "vrc_inputhandler.h"
#include "vrc_playerphysics.h"
#include "vrc_playerimplclient.h"
#include "vrc_playernetworking.h"
#include "../visuals/vrc_camera.h"


// some networking related thresholds
#define NW_POS_CORRECTION_THRESHOLD 5.0f
#define NW_JUMP_THRESHOLD           0.3f
#define NW_WALK_THRESHOLD           0.05f
#define NW_ROT_THRESHOLD            0.002f

namespace vrc
{

PlayerImplClient::PlayerImplClient( EnPlayer* player ) :
BasePlayerImplementation( player ),
_isNwInitialized( false ),
_isRemoteClient( false ),
_p_inputHandler( NULL )
{
}

PlayerImplClient::~PlayerImplClient()
{
    if ( !_isRemoteClient )
    {
        // remove local player in player utils
        vrc::gameutils::PlayerUtils::get()->setLocalPlayer( NULL );
    }
    else
    {
        // remove us from remote player list in player utility if we are a remote player
        vrc::gameutils::PlayerUtils::get()->removeRemotePlayer( getPlayerEntity() );
    }

    if ( _p_playerNetworking )
        delete _p_playerNetworking;

    if ( _p_inputHandler )
        _p_inputHandler->destroyHandler();
}

void PlayerImplClient::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle some notifications
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_MENU_ENTER:

            if ( !_isRemoteClient )
            {
                getChatManager()->show( false );
                _p_inputHandler->setMenuEnabled( true );

                // reset player's movement and sound
                _p_playerPhysics->stopMovement();
                _p_playerAnimation->animIdle();
                if ( _p_playerSound )
                    _p_playerSound->stopPlayingAll();

                // very important: diable the camera when we enter menu!
                _p_camera->setEnable( false );

                // players are all rendered in menu, regardless their camera mode
                _p_playerAnimation->enableRendering( true );
                if ( _cameraMode == Ego )
                    addToSceneGraph();
            }
            break;

        case YAF3D_NOTIFY_MENU_LEAVE:
        {
            if ( !_isRemoteClient )
            {
                getChatManager()->show( true );

                // update the player name in VRC chat in the case that the player changed the nick name in menu
                std::string playername;
                yaf3d::Configuration::get()->getSettingValue( VRC_GS_PLAYER_NAME, playername );
                if ( getPlayerName() != playername )
                    getChatManager()->setVRCNickName( playername );

                _p_inputHandler->setMenuEnabled( false );

                // refresh our configuration settings
                getConfiguration();

                // very important: enable the camera when we leave menu!
                _p_camera->setEnable( true );

                // if we are in ego mode then disable player avatar rendering
                if ( _cameraMode == Ego )
                {
                    _p_playerAnimation->enableRendering( false );
                    removeFromSceneGraph();
                }
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

void PlayerImplClient::onServerDisconnect( int /*sessionID*/ )
{
    yaf3d::MessageBoxDialog* p_msg = new yaf3d::MessageBoxDialog( "Networking Problem", "Disconnected from server.\nLeave the level and try to re-connect to server.\n", yaf3d::MessageBoxDialog::OK, true );
    p_msg->show();
}

void PlayerImplClient::initialize()
{
    // setup position, rotation and move direction
    _currentPos = getPlayerEntity()->getPosition();
    _currentRot = getPlayerEntity()->getRotation();
    _moveDir    = _currentRot * _moveDir;

    // get configuration settings for player control keys
    getConfiguration();

    // if the player networking component is created already then this implementation is created
    //  for a remote client on local machine, otherwise it's a local client.
    if ( !getPlayerNetworking() )
    {
        // for local client we create the networking component
        _p_playerNetworking = new PlayerNetworking( this );
        _p_playerNetworking->Publish();
        _isRemoteClient = false;

        // register for getting network session notifications
        yaf3d::NetworkDevice::get()->registerSessionNotify( this );
    }
    else
    {
        _isRemoteClient = true;
    }

    // actions to be taken for local client
    if ( !_isRemoteClient )
    {
        // get player's remote client config file so its ghosts load the right config while they get setup on remote machines
        std::string playerconfig;
        gameutils::PlayerUtils::get()->getPlayerConfig( yaf3d::GameState::get()->getMode(), true, playerconfig );
        // init player's networking
        getPlayerNetworking()->initialize( _currentPos, getPlayerEntity()->getPlayerName(), playerconfig );
    }
}

void PlayerImplClient::postInitialize()
{
    log_info << "  setup player implementation Client ..." << std::endl;

    // local client specific setup
    if ( !_isRemoteClient )
    {
        // set us as local player entity in player utility; other entities may need us
        vrc::gameutils::PlayerUtils::get()->setLocalPlayer( getPlayerEntity() );

        // attach camera entity
        {
            log_debug << "   - searching for camera entity '" << PLAYER_CAMERA_ENTITIY_NAME << "'..." << std::endl;
            // get camera entity
            _p_camera = dynamic_cast< EnCamera* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_CAMERA, PLAYER_CAMERA_ENTITIY_NAME ) );
            assert( _p_camera && "could not find the camera entity!" );
            log_debug << "   -  camera entity successfully attached" << std::endl;
        }
        // attach physics entity
        {
            log_debug << "   - searching for physics entity '" << _playerAttributes._physicsEntity << "' ..." << std::endl;
            // find and attach physics component
            _p_playerPhysics = dynamic_cast< EnPlayerPhysics* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_PLPHYS, _playerAttributes._physicsEntity ) );
            assert( _p_playerPhysics && "given instance name does not belong to a EnPlayerPhysics entity type, or entity is missing!" );
            _p_playerPhysics->setPlayer( this );
            log_debug << "   -  physics entity successfully attached" << std::endl;
        }
        // attach sound entity
        {
            log_debug << "   - searching for sound entity '" << _playerAttributes._soundEntity << "' ..." << std::endl;
            // find and attach sound component, tollerate missing sound for now
            _p_playerSound = dynamic_cast< EnPlayerSound* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_PLSOUND, _playerAttributes._soundEntity ) );
            if ( !_p_playerSound )
                log_error << "  *** could not find sound entity '" << _playerAttributes._soundEntity << "' of type PlayerSound. player sound deactivated" << std::endl;
            else
            {
                _p_playerSound->setPlayer( this );
                log_debug << "   -  sound entity successfully attached" << std::endl;
            }
        }
        // attach animation entity
        {
            log_debug << "   - searching for animation entity '" << _playerAttributes._animationEntity << "' ..." << std::endl;
            // find and attach animation component
            _p_playerAnimation = dynamic_cast< EnPlayerAnimation* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_PLANIM, _playerAttributes._animationEntity ) );
            assert( _p_playerAnimation && "given instance name does not belong to a EnPlayerAnimation entity type, or entity is missing!" );
            _p_playerAnimation->setPlayer( this );
        }
        log_debug << "   -  animation entity successfully attached" << std::endl;

        if ( _cameraMode == Ego ) // in ego mode we won't render our character
        {
            _p_playerAnimation->enableRendering( false );
        }
        else // if in spheric mode disable the mouse pointer
        {
            gameutils::GuiUtils::get()->showMousePointer( false );
        }

        // create the chat manager
        if ( !getChatManager() )
        {
            log_error << "   -  could not create chat system" << std::endl;
        }

        // create a new input handler for this player
        _p_inputHandler = new PlayerIHCharacterCameraCtrl< PlayerImplClient >( this, _p_player );
        _p_inputHandler->setMenuEnabled( false );

        // setup camera mode
        setCameraMode( _cameraMode );
    }
    else // setup remote client ( note, the remote instance names have a postfix )
    {
        // set us as remote player entity in player utility
        vrc::gameutils::PlayerUtils::get()->addRemotePlayer( getPlayerEntity() );

        // attach physics entity
        {
            log_debug << "   - searching for physics entity '" << _playerAttributes._physicsEntity + _loadingPostFix << "' ..." << std::endl;
            // find and attach physics component
            _p_playerPhysics = dynamic_cast< EnPlayerPhysics* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_PLPHYS, _playerAttributes._physicsEntity + _loadingPostFix ) );
            assert( _p_playerPhysics && "given instance name does not belong to a EnPlayerPhysics entity type, or entity is missing!" );
            _p_playerPhysics->setPlayer( this );
            log_debug << "   -  physics entity successfully attached" << std::endl;
        }
        // attach sound entity
        {
            log_debug << "   - searching for sound entity '" << _playerAttributes._soundEntity + _loadingPostFix << "' ..." << std::endl;
            // find and attach sound component, tollerate missing sound for now
            _p_playerSound = dynamic_cast< EnPlayerSound* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_PLSOUND, _playerAttributes._soundEntity + _loadingPostFix ) );
            if ( !_p_playerSound )
                log_error << "  *** could not find sound entity '" << _playerAttributes._soundEntity + _loadingPostFix << "' of type PlayerSound. player sound deactivated" << std::endl;
            else
            {
                _p_playerSound->setPlayer( this );
                log_debug << "   -  sound entity successfully attached" << std::endl;
            }
        }

        // attach animation entity
        {
            log_debug << "   - searching for animation entity '" << _playerAttributes._animationEntity + _loadingPostFix << "' ..." << std::endl;
            // find and attach animation component
            _p_playerAnimation = dynamic_cast< EnPlayerAnimation* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_PLANIM, _playerAttributes._animationEntity + _loadingPostFix ) );
            assert( _p_playerAnimation && "given instance name does not belong to a EnPlayerAnimation entity type, or entity is missing!" );
            _p_playerAnimation->setPlayer( this );
            // enable rendering for remote clients
            _p_playerAnimation->enableRendering( true );
            addToSceneGraph();
            _p_playerAnimation->setAnimation( EnPlayerAnimation::eIdle );

            log_debug << "   -  animation entity successfully attached" << std::endl;
        }
    }

    log_info << "  player implementation successfully initialized" << std::endl;
}

void PlayerImplClient::getConfiguration()
{
    std::string playername;
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_PLAYER_NAME, playername );
    _p_player->setPlayerName( playername );

    // setup key bindings if the handler is already created (e.g. remote clients have no handler)
    if ( _p_inputHandler )
    {
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
}

void PlayerImplClient::setNetworkInitialized( bool init )
{
    _isNwInitialized = init;
    getPlayerPhysics()->initializePhysics( _currentPos, _currentRot );
    // update player's actual position and rotation once per frame
    getPlayerEntity()->setPosition( _currentPos );
    getPlayerEntity()->setRotation( _currentRot );
}

void PlayerImplClient::update( float deltaTime )
{
    // if networking not initialized then do not update!
    if ( !_isNwInitialized )
        return;

    if ( !_isRemoteClient )
    {
        // update player's actual position and rotation once per frame
        getPlayerEntity()->setPosition( _currentPos );
        getPlayerEntity()->setRotation( _currentRot );

        getPlayerNetworking()->updatePosition( _currentPos._v[ 0 ], _currentPos._v[ 1 ], _currentPos._v[ 2 ] );
        getPlayerNetworking()->updateRotation( _rotZ + osg::PI );
        getPlayerNetworking()->updateAnimationFlags( getPlayerAnimation()->getAnimationFlags() );

        // adjust the camera to updated position and rotation. the physics updates the translation of player.
        _p_camera->setCameraTransformation( getPlayerPosition(), getPlayerRotation() );
        // update chat gui
        getChatManager()->update( deltaTime );
    }
    else
    {
        // update remote client's rotation and position
        osg::Vec3f lastpos = _currentPos;
        float      lastrot = _rotZ;
        osg::Vec3f clientpos;

        getPlayerNetworking()->getPosition( clientpos._v[ 0 ], clientpos._v[ 1 ], clientpos._v[ 2 ] );
        getPlayerNetworking()->getRotation( _rotZ );
        _currentRot.makeRotate( -_rotZ + osg::PI, osg::Vec3f( 0.0f, 0.0f, 1.0f ) );
        getPlayerEntity()->setRotation( _currentRot );
        getPlayerEntity()->setPosition( _currentPos );

        // calculate the current velocity
        osg::Vec3f vel( clientpos - lastpos );

        // do we need a hard position correction?
        if ( vel.length2() > NW_POS_CORRECTION_THRESHOLD )
        {
            osg::Matrix mat;
            mat.makeRotate( _currentRot );
            mat.setTrans( clientpos );
            getPlayerPhysics()->setTransformation( mat );
            getPlayerPhysics()->setDirection( 0.0f, 0.0f );
        }
        else
        {
            vel._v[ 2 ] = 0.0f;
            getPlayerPhysics()->setDirection( vel.x(), vel.y() );
        }

        // set animation depending on position and rotation changes
        if ( ( ( clientpos.z() - lastpos.z() ) > NW_JUMP_THRESHOLD ) && !getPlayerPhysics()->isJumping() )
        {
            getPlayerPhysics()->jump();
            getPlayerAnimation()->setAnimation( EnPlayerAnimation::eIdle );
            getPlayerAnimation()->setAnimation( EnPlayerAnimation::eJump );
        }
        else if ( vel.length2() > NW_WALK_THRESHOLD )
        {
            getPlayerAnimation()->setAnimation( EnPlayerAnimation::eWalk );
        }
        else
        {
            getPlayerAnimation()->setAnimation( EnPlayerAnimation::eIdle );
        }

        if ( fabs( lastrot - _rotZ ) > NW_ROT_THRESHOLD )
            getPlayerAnimation()->setAnimation( EnPlayerAnimation::eTurn );

    }

    // now update the physics entity
    getPlayerPhysics()->updateEntity( deltaTime );

    // update sound
    getPlayerSound()->updatePosition( _currentPos );
}

} // namespace vrc
