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
#include "ctd_chatgui.h"
#include "ctd_playerphysics.h"
#include "ctd_playeranim.h"
#include "ctd_playersound.h"
#include "ctd_inputhandler.h"
#include "../visuals/ctd_camera.h"

using namespace std;

namespace CTD
{
// entity name of player's camera
#define PLAYER_CAMERA_ENTITIY_NAME      "playercam"

BasePlayerImplStandalone::BasePlayerImplStandalone( EnPlayer* player ) :
BasePlayerImplementation( player ),
_p_chatGui( new PlayerChatGui )
{
    // create a new input handler for this player
    _p_inputHandler = new PlayerIHStandalone( this, player );
}

BasePlayerImplStandalone::~BasePlayerImplStandalone()
{
    // destroy input handler
    _p_inputHandler->destroyHandler();
}

void BasePlayerImplStandalone::handleNotification( EntityNotification& notify )
{
    // handle some notifications
    switch( notify.getId() )
    {
        case CTD_NOTIFY_MENU_ENTER:

            _p_chatGui->show( false );
            _p_inputHandler->setMenuEnabled( true );
            
            // reset player's movement and sound
            _p_playerPhysics->stopMovement();
            _p_playerAnimation->animIdle();
            if ( _p_playerSound )
                _p_playerSound->stopPlayingAll();

            // very important: diable the camera when we enter menu!
            _p_camera->setEnable( false );

            break;

        case CTD_NOTIFY_MENU_LEAVE:
        {
            _p_chatGui->show( true );
            _p_inputHandler->setMenuEnabled( false );

            // refresh our configuration settings
            getConfiguration();
 
            // very important: enable the camera when we leave menu!
            _p_camera->setEnable( true );
        }
        break;

        default:
            ;
    }
}

void BasePlayerImplStandalone::enableControl( bool en )
{
    _enabledControl = en;
    _p_inputHandler->enable( en );
}

void BasePlayerImplStandalone::initialize()
{    
    _p_chatGui->initialize( this, _playerAttributes._chatGuiConfig );
    _p_inputHandler->setMenuEnabled( false );
}

void BasePlayerImplStandalone::postInitialize()
{
    log << Log::LogLevel( Log::L_INFO ) << "  setup player implementation Standalone ..." << endl;

    // attach camera entity
    log << Log::LogLevel( Log::L_DEBUG ) << "   - searching for camera entity '" << PLAYER_CAMERA_ENTITIY_NAME << "'..." << endl;
    // get camera entity
    _p_camera = dynamic_cast< EnCamera* >( EntityManager::get()->findEntity( ENTITY_NAME_CAMERA, PLAYER_CAMERA_ENTITIY_NAME ) );
    assert( _p_camera && "could not find the camera entity!" );
    log << Log::LogLevel( Log::L_DEBUG ) << "   -  camera entity successfully attached" << endl;

    // attach physics entity
    log << Log::LogLevel( Log::L_DEBUG ) << "   - searching for physics entity '" << _playerAttributes._physicsEntity << "' ..." << endl;
    // find and attach physics component
    _p_playerPhysics = dynamic_cast< EnPlayerPhysics* >( EntityManager::get()->findEntity( ENTITY_NAME_PLPHYS, _playerAttributes._physicsEntity ) );
    assert( _p_playerPhysics && "given instance name does not belong to a EnPlayerPhysics entity type!" );
    _p_playerPhysics->setPlayer( this );
    log << Log::LogLevel( Log::L_DEBUG ) << "   -  physics entity successfully attached" << endl;

    // attach animation entity
    log << Log::LogLevel( Log::L_DEBUG ) << "   - searching for animation entity '" << _playerAttributes._animationEntity << "' ..." << endl;
    // find and attach animation component
    _p_playerAnimation = dynamic_cast< EnPlayerAnimation* >( EntityManager::get()->findEntity( ENTITY_NAME_PLANIM, _playerAttributes._animationEntity ) );
    assert( _p_playerAnimation && "given instance name does not belong to a EnPlayerAnimation entity type!" );
    _p_playerAnimation->setPlayer( this );
    if ( _cameraMode == Ego ) // in ego mode we won't render our character
        _p_playerAnimation->enableRendering( false );

    log << Log::LogLevel( Log::L_DEBUG ) << "   -  animation entity successfully attached" << endl;

    // attach sound entity
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

    // get configuration settings
    getConfiguration();

    // setup camera mode
    setCameraMode( _cameraMode );

    log << Log::LogLevel( Log::L_INFO ) << "  player implementation successfully initialized" << endl;
}

void BasePlayerImplStandalone::getConfiguration()
{
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
}

void BasePlayerImplStandalone::update( float deltaTime )
{
    // update player physics
    _p_playerPhysics->update( deltaTime );

    // adjust the camera to updated position and rotation. the physics updates the translation of player.
    _p_camera->setCameraTranslation( getPlayerPosition(), getPlayerRotation() );

    _p_chatGui->update( deltaTime );
}

void BasePlayerImplStandalone::setCameraMode( unsigned int mode )
{
    switch ( mode )
    {
        case Spheric:
        {
            _p_camera->setCameraOffsetPosition( _playerAttributes._camPosOffsetSpheric );
            osg::Quat rot;
            rot.makeRotate( 
                osg::DegreesToRadians( _playerAttributes._camRotOffsetSpheric.x()  ), osg::Vec3f( 0, 1, 0 ), // roll
                osg::DegreesToRadians( _playerAttributes._camRotOffsetSpheric.y()  ), osg::Vec3f( 1, 0, 0 ), // pitch
                osg::DegreesToRadians( _playerAttributes._camRotOffsetSpheric.z()  ), osg::Vec3f( 0, 0, 1 )  // yaw
                );
            _p_camera->setCameraOffsetRotation( rot );
            _p_playerAnimation->enableRendering( true );
        } 
        break;

        case Ego:
        {
            _p_camera->setCameraOffsetPosition( _playerAttributes._camPosOffsetEgo );
            osg::Quat rot;
            rot.makeRotate( 
                osg::DegreesToRadians( _playerAttributes._camRotOffsetEgo.x()  ), osg::Vec3f( 0, 1, 0 ), // roll
                osg::DegreesToRadians( _playerAttributes._camRotOffsetEgo.y()  ), osg::Vec3f( 1, 0, 0 ), // pitch
                osg::DegreesToRadians( _playerAttributes._camRotOffsetEgo.z()  ), osg::Vec3f( 0, 0, 1 )  // yaw
                );
            _p_camera->setCameraOffsetRotation( rot );
            _p_playerAnimation->enableRendering( false );
        }
        break;

        default:
            assert( NULL && "requesting for an invalid camera mode" );

    }
    _cameraMode = ( CameraMode )mode;
}

void BasePlayerImplStandalone::setNextCameraMode()
{
    if ( _cameraMode == Spheric )
        setCameraMode( Ego );
    else
        setCameraMode( Spheric );
}

void BasePlayerImplStandalone::setCameraPitchYaw( float pitch, float yaw )
{
    if ( _cameraMode == Spheric )
    {
        float angleY = yaw * 360.0f;
        float angleX = ( pitch * LIMIT_PITCH_ANGLE ) + LIMIT_PITCH_OFFSET;
        _p_camera->setLocalPitchYaw( angleX, angleY );
    }
    // in ego mode we turn the character instead of yawing the camera!
    else
    {
        float angleX = pitch * LIMIT_PITCH_ANGLE;
        _p_camera->setLocalPitch( angleX );
    }
}

} // namespace CTD
