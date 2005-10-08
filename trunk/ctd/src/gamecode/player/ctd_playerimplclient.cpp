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
 # player implementation for client mode
 #
 #   date of creation:  05/31/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include <ctd_gameutils.h>
#include "ctd_player.h"
#include "ctd_chatgui.h"
#include "ctd_playersound.h"
#include "ctd_inputhandler.h"
#include "ctd_playerphysics.h"
#include "ctd_playerimplclient.h"
#include "ctd_playernetworking.h"
#include "../visuals/ctd_camera.h"

using namespace std;

namespace CTD
{

PlayerImplClient::PlayerImplClient( EnPlayer* player ) :
BasePlayerImplementation( player ),
_p_inputHandler( NULL )
{
}

PlayerImplClient::~PlayerImplClient()
{
    if ( _p_playerNetworking )
        delete _p_playerNetworking;
}

void PlayerImplClient::handleNotification( const EntityNotification& notification )
{
    // handle some notifications
    switch( notification.getId() )
    {
        case CTD_NOTIFY_MENU_ENTER:

            if ( !getPlayerNetworking()->isRemoteClient() )
            {
                _p_chatGui->show( false );
                _p_inputHandler->setMenuEnabled( true );

                // reset player's movement and sound
                _p_playerPhysics->stopMovement();
                _p_playerAnimation->animIdle();
                if ( _p_playerSound )
                    _p_playerSound->stopPlayingAll();

                // very important: diable the camera when we enter menu!
                _p_camera->setEnable( false );
            }
            break;

        case CTD_NOTIFY_MENU_LEAVE:
        {
            if ( !getPlayerNetworking()->isRemoteClient() )
            {
                _p_chatGui->show( true );
                _p_inputHandler->setMenuEnabled( false );

                // refresh our configuration settings
                getConfiguration();

                // very important: enable the camera when we leave menu!
                _p_camera->setEnable( true );
            }
        }
        break;

        case CTD_NOTIFY_UNLOAD_LEVEL:
        {
            // deleting the gui object must happen at last as goodby messages must be sent out
            PlayerChatGui* p_chatGui = PlayerChatGui::get();
            if ( p_chatGui )
                delete p_chatGui;
        }
        break;

        default:
            ;
    }
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
    }
    // actions to be taken for local client
    if ( !getPlayerNetworking()->isRemoteClient() )
    {
        // get player's remote client config file so its ghosts load the right config while they get setup on remote machines
        std::string playerconfig;
        gameutils::getPlayerConfig( CTD::GameState::get()->getMode(), true, playerconfig );
        // init player's networking 
        getPlayerNetworking()->initialize( _currentPos, getPlayerEntity()->getPlayerName(), playerconfig );

        // create chat gui
        _p_chatGui = new PlayerChatGui;
        _p_chatGui->initialize( this, _playerAttributes._chatGuiConfig );
    }
}

void PlayerImplClient::postInitialize()
{
    log << Log::LogLevel( Log::L_INFO ) << "  setup player implementation Client ..." << endl;

    // local client specific setup
    if ( !getPlayerNetworking()->isRemoteClient() )
    {
        // attach camera entity
        {
            log << Log::LogLevel( Log::L_DEBUG ) << "   - searching for camera entity '" << PLAYER_CAMERA_ENTITIY_NAME << "'..." << endl;
            // get camera entity
            _p_camera = dynamic_cast< EnCamera* >( EntityManager::get()->findEntity( ENTITY_NAME_CAMERA, PLAYER_CAMERA_ENTITIY_NAME ) );
            assert( _p_camera && "could not find the camera entity!" );
            log << Log::LogLevel( Log::L_DEBUG ) << "   -  camera entity successfully attached" << endl;
        }
        // attach physics entity
        {
            log << Log::LogLevel( Log::L_DEBUG ) << "   - searching for physics entity '" << _playerAttributes._physicsEntity << "' ..." << endl;
            // find and attach physics component
            _p_playerPhysics = dynamic_cast< EnPlayerPhysics* >( EntityManager::get()->findEntity( ENTITY_NAME_PLPHYS, _playerAttributes._physicsEntity ) );
            assert( _p_playerPhysics && "given instance name does not belong to a EnPlayerPhysics entity type, or entity is missing!" );
            _p_playerPhysics->setPlayer( this );
            log << Log::LogLevel( Log::L_DEBUG ) << "   -  physics entity successfully attached" << endl;
        }
        // attach sound entity
        {
            log << Log::LogLevel( Log::L_DEBUG ) << "   - searching for sound entity '" << _playerAttributes._soundEntity << "' ..." << endl;
            // find and attach sound component, tollerate missing sound for now
            _p_playerSound = dynamic_cast< EnPlayerSound* >( EntityManager::get()->findEntity( ENTITY_NAME_PLSOUND, _playerAttributes._soundEntity ) );
            if ( !_p_playerSound )
                log << Log::LogLevel( Log::L_ERROR ) << "  *** could not find sound entity '" << _playerAttributes._soundEntity << "' of type PlayerSound. player sound deactivated" << endl;
            else
            {
                _p_playerSound->setPlayer( this );
                log << Log::LogLevel( Log::L_DEBUG ) << "   -  sound entity successfully attached" << endl;
            }
        }
        // attach animation entity
        {
            log << Log::LogLevel( Log::L_DEBUG ) << "   - searching for animation entity '" << _playerAttributes._animationEntity << "' ..." << endl;
            // find and attach animation component
            _p_playerAnimation = dynamic_cast< EnPlayerAnimation* >( EntityManager::get()->findEntity( ENTITY_NAME_PLANIM, _playerAttributes._animationEntity ) );
            assert( _p_playerAnimation && "given instance name does not belong to a EnPlayerAnimation entity type, or entity is missing!" );
            _p_playerAnimation->setPlayer( this );
        }
        log << Log::LogLevel( Log::L_DEBUG ) << "   -  animation entity successfully attached" << endl;

        if ( _cameraMode == Ego ) // in ego mode we won't render our character
        {
            _p_playerAnimation->enableRendering( false );
        }
        else // if in spheric mode disable the mouse pointer
        {
            GuiManager::get()->showMousePointer( false );
        }

        // create a new input handler for this player
        _p_inputHandler = new PlayerIHCharacterCameraCtrl< PlayerImplClient >( this, _p_player );
        _p_inputHandler->setMenuEnabled( false );

        // setup camera mode
        setCameraMode( _cameraMode );
    }
    else // setup remote client ( note, the remote instance names have a postfix )
    {
        // attach physics entity
        {
            log << Log::LogLevel( Log::L_DEBUG ) << "   - searching for physics entity '" << _playerAttributes._physicsEntity + _loadingPostFix << "' ..." << endl;
            // find and attach physics component
            _p_playerPhysics = dynamic_cast< EnPlayerPhysics* >( EntityManager::get()->findEntity( ENTITY_NAME_PLPHYS, _playerAttributes._physicsEntity + _loadingPostFix ) );
            assert( _p_playerPhysics && "given instance name does not belong to a EnPlayerPhysics entity type, or entity is missing!" );
            _p_playerPhysics->setPlayer( this );
            log << Log::LogLevel( Log::L_DEBUG ) << "   -  physics entity successfully attached" << endl;
        }
        // attach sound entity
        {
            log << Log::LogLevel( Log::L_DEBUG ) << "   - searching for sound entity '" << _playerAttributes._soundEntity + _loadingPostFix << "' ..." << endl;
            // find and attach sound component, tollerate missing sound for now
            _p_playerSound = dynamic_cast< EnPlayerSound* >( EntityManager::get()->findEntity( ENTITY_NAME_PLSOUND, _playerAttributes._soundEntity + _loadingPostFix ) );
            if ( !_p_playerSound )
                log << Log::LogLevel( Log::L_ERROR ) << "  *** could not find sound entity '" << _playerAttributes._soundEntity + _loadingPostFix << "' of type PlayerSound. player sound deactivated" << endl;
            else
            {
                _p_playerSound->setPlayer( this );
                log << Log::LogLevel( Log::L_DEBUG ) << "   -  sound entity successfully attached" << endl;
            }
        }

        // attach animation entity
        {
            log << Log::LogLevel( Log::L_DEBUG ) << "   - searching for animation entity '" << _playerAttributes._animationEntity + _loadingPostFix << "' ..." << endl;
            // find and attach animation component
            _p_playerAnimation = dynamic_cast< EnPlayerAnimation* >( EntityManager::get()->findEntity( ENTITY_NAME_PLANIM, _playerAttributes._animationEntity + _loadingPostFix ) );
            assert( _p_playerAnimation && "given instance name does not belong to a EnPlayerAnimation entity type, or entity is missing!" );
            _p_playerAnimation->setPlayer( this );
            // enable rendering for remote clients
            _p_playerAnimation->enableRendering( true );
            _p_playerAnimation->setAnimation( EnPlayerAnimation::eIdle );

            log << Log::LogLevel( Log::L_DEBUG ) << "   -  animation entity successfully attached" << endl;
        }
    }

    log << Log::LogLevel( Log::L_INFO ) << "  player implementation successfully initialized" << endl;
}

void PlayerImplClient::getConfiguration()
{
    std::string playername;
    Configuration::get()->getSettingValue( CTD_GS_PLAYER_NAME, playername );
    _p_player->setPlayerName( playername );

    // setup key bindings if the handler is already created (e.g. remote clients have no handler)
    if ( _p_inputHandler )
    {
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
}

void PlayerImplClient::update( float deltaTime )
{
    if ( !getPlayerNetworking()->isRemoteClient() )
    {
        // update player's actual position and rotation once per frame
        getPlayerEntity()->setPosition( _currentPos );
        getPlayerEntity()->setRotation( _currentRot );

        getPlayerNetworking()->updatePosition( _currentPos._v[ 0 ], _currentPos._v[ 1 ], _currentPos._v[ 2 ] );
        getPlayerNetworking()->updateRotation( _rotZ + osg::PI );
        getPlayerNetworking()->updateAnimationFlags( getPlayerAnimation()->getAnimationFlags() );

        // adjust the camera to updated position and rotation. the physics updates the translation of player.
        _p_camera->setCameraTranslation( getPlayerPosition(), getPlayerRotation() );
        // update chat gui
        _p_chatGui->update( deltaTime );
    }
    else
    {
        // update remote client's rotation and position
        osg::Vec3f lastpos = _currentPos;
        osg::Vec3f clientpos;

        float      lastrot = _rotZ;
        getPlayerNetworking()->getPosition( clientpos._v[ 0 ], clientpos._v[ 1 ], clientpos._v[ 2 ] );
        getPlayerNetworking()->getRotation( _rotZ );
        _currentRot.makeRotate( -_rotZ + osg::PI, osg::Vec3f( 0.0f, 0.0f, 1.0f ) );
        getPlayerEntity()->setRotation( _currentRot );
        getPlayerEntity()->setPosition( _currentPos );

        // calculate the current velocity
        osg::Vec3f vel( clientpos - lastpos );
        // do we need a hard position correction?
        if ( vel.length2() > 4.0f )
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
            // limit velocity
            if ( vel.length2() > 1.0f )
                vel.normalize();

            getPlayerPhysics()->setDirection( vel.x(), vel.y() );
        }

        // set animation depending on position and rotation changes
        if ( ( clientpos.z() - lastpos.z() ) > 0.1f )
        {
            getPlayerAnimation()->setAnimation( EnPlayerAnimation::eIdle );
            getPlayerAnimation()->setAnimation( EnPlayerAnimation::eJump );
        }
        else if ( vel.length2() > 0.001f )
        {
            getPlayerAnimation()->setAnimation( EnPlayerAnimation::eWalk );
        } else
        {
            getPlayerAnimation()->setAnimation( EnPlayerAnimation::eIdle );
        }
        
        if ( fabs( lastrot - _rotZ ) > 0.01f )
            getPlayerAnimation()->setAnimation( EnPlayerAnimation::eTurn );
    }
}

} // namespace CTD
