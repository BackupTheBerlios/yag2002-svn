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
 # player
 #
 # this class implements the player
 #
 #
 #   date of creation:  01/14/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_player.h"
#include "ctd_playerphysics.h"
#include "ctd_playeranim.h"
#include "ctd_playersound.h"
#include "ctd_chatgui.h"
#include "ctd_inputhandler.h"
#include "../visuals/ctd_camera.h"

using namespace osg;
using namespace std;

namespace CTD
{

// entity name of player's camera
#define PLAYER_CAMERA_ENTITIY_NAME      "playercam"

//! Implement and register the player entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( PlayerEntityFactory );
    
EnPlayer::EnPlayer() :
_p_playerPhysics( NULL ),
_p_playerAnimation( NULL ),
_p_playerSound( NULL ),
_p_chatGui( new PlayerChatGui ),
_playerName( "noname" ),
_rot( 0 ),
_moveDir( Vec3f( 0, 1, 0 ) ),
_p_inputHandler( NULL ),
_chatGuiConfig( "gui/chat.xml" ),
_p_camera( NULL ),
_cameraMode( Spheric ),
_enabledControl( true )
{
    EntityManager::get()->registerUpdate( this, true );         // register entity in order to get updated per simulation step
    EntityManager::get()->registerNotification( this, true );   // register entity in order to get notifications (e.g. from menu entity)

    getAttributeManager().addAttribute( "name"                      , _playerName           );
    getAttributeManager().addAttribute( "physicsentity"             , _physicsEntity        );
    getAttributeManager().addAttribute( "animationentity"           , _animationEntity      );
    getAttributeManager().addAttribute( "soundentity"               , _soundEntity          );
    getAttributeManager().addAttribute( "position"                  , _position             );
    getAttributeManager().addAttribute( "rotation"                  , _rot                  );
    getAttributeManager().addAttribute( "cameraPosOffsetSpheric"    , _camPosOffsetSpheric  );
    getAttributeManager().addAttribute( "cameraRotOffsetSpheric"    , _camRotOffsetSpheric  );
    getAttributeManager().addAttribute( "cameraPosOffsetEgo"        , _camPosOffsetEgo      );
    getAttributeManager().addAttribute( "cameraRotOffsetEgo"        , _camRotOffsetEgo      );
    getAttributeManager().addAttribute( "chatGuiConfig"             , _chatGuiConfig        );

    // create a new input handler for this player
    _p_inputHandler = new PlayerInputHandler( this );
}

EnPlayer::~EnPlayer()
{
    if ( _p_playerPhysics )
    {
        EntityManager::get()->deleteEntity( _p_playerPhysics );
    }

    if ( _p_playerAnimation )
    {
        EntityManager::get()->deleteEntity( _p_playerAnimation );
    }

    if ( _p_playerSound )
    {
        EntityManager::get()->deleteEntity( _p_playerSound );
    }

    // destroy input handler
    _p_inputHandler->destroyHandler();
}

void EnPlayer::handleNotification( EntityNotification& notify )
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

void EnPlayer::enableControl( bool en )
{
    _enabledControl = en;
    _p_inputHandler->enable( en );
}

void EnPlayer::initialize()
{    
    _p_chatGui->initialize( this, _chatGuiConfig );
    _p_inputHandler->setMenuEnabled( false );
}

void EnPlayer::postInitialize()
{
    log << Log::LogLevel( Log::L_INFO ) << "  initializing player instance '" << getInstanceName() << "' ..." << endl;

    log << Log::LogLevel( Log::L_DEBUG ) << "   - searching for camera entity ..." << endl;
    // get camera entity
    _p_camera = dynamic_cast< EnCamera* >( EntityManager::get()->findEntity( ENTITY_NAME_CAMERA, PLAYER_CAMERA_ENTITIY_NAME ) );
    assert( _p_camera && "could not find the camera entity!" );

    log << Log::LogLevel( Log::L_DEBUG ) << "   - searching for physics entity '" << _physicsEntity << "' ..." << endl;
    // find and attach physics component
    _p_playerPhysics = dynamic_cast< EnPlayerPhysics* >( EntityManager::get()->findEntity( ENTITY_NAME_PLPHYS, _physicsEntity ) );
    assert( _p_playerPhysics && "given instance name does not belong to a EnPlayerPhysics entity type!" );
    _p_playerPhysics->setPlayer( this );
    log << Log::LogLevel( Log::L_DEBUG ) << "   -  physics entity successfully attached" << endl;

    log << Log::LogLevel( Log::L_DEBUG ) << "   - searching for animation entity '" << _animationEntity << "' ..." << endl;
    // find and attach animation component
    _p_playerAnimation = dynamic_cast< EnPlayerAnimation* >( EntityManager::get()->findEntity( ENTITY_NAME_PLANIM, _animationEntity ) );
    assert( _p_playerAnimation && "given instance name does not belong to a EnPlayerAnimation entity type!" );
    _p_playerAnimation->setPlayer( this );
    if ( _cameraMode == Ego ) // in ego mode we won't render our character
        _p_playerAnimation->enableRendering( false );

    log << Log::LogLevel( Log::L_DEBUG ) << "   -  animation entity successfully attached" << endl;

    log << Log::LogLevel( Log::L_DEBUG ) << "   - searching for sound entity '" << _soundEntity << "' ..." << endl;
    // find and attach sound component, tollerate missing sound for now
    _p_playerSound = dynamic_cast< EnPlayerSound* >( EntityManager::get()->findEntity( ENTITY_NAME_PLSOUND, _soundEntity ) );
    if ( !_p_playerSound )
        log << Log::LogLevel( Log::L_ERROR ) << "  *** could not find sound entity '" << _soundEntity << "' of type PlayerSound. player sound deactivated" << endl;
    else
    {
        _p_playerSound->setPlayer( this );
        log << Log::LogLevel( Log::L_DEBUG ) << "   -  sound entity successfully attached" << endl;
    }

    setPosition( _position );

    // get configuration settings
    getConfiguration();

    // setup camera mode
    setCameraMode( _cameraMode );

    log << Log::LogLevel( Log::L_INFO ) << "  player instance successfully initialized" << endl;
}

void EnPlayer::getConfiguration()
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

void EnPlayer::updateEntity( float deltaTime )
{
    // update player physics
    _p_playerPhysics->update( deltaTime );
    // adjust the camera to updated position and rotation. the physics updates the translation of player.
    // adjust camera to character's head position
    _p_camera->setCameraTranslation( _position, _rotation );

    _p_chatGui->update( deltaTime );
}

void EnPlayer::setCameraMode( unsigned int mode )
{
    switch ( mode )
    {
        case Spheric:
        {
            _p_camera->setCameraOffsetPosition( _camPosOffsetSpheric );
            osg::Quat rot;
            rot.makeRotate( 
                osg::DegreesToRadians( _camRotOffsetSpheric.x()  ), osg::Vec3f( 0, 1, 0 ), // roll
                osg::DegreesToRadians( _camRotOffsetSpheric.y()  ), osg::Vec3f( 1, 0, 0 ), // pitch
                osg::DegreesToRadians( _camRotOffsetSpheric.z()  ), osg::Vec3f( 0, 0, 1 )  // yaw
                );
            _p_camera->setCameraOffsetRotation( rot );
            _p_playerAnimation->enableRendering( true );
        } 
        break;

        case Ego:
        {
            _p_camera->setCameraOffsetPosition( _camPosOffsetEgo );
            osg::Quat rot;
            rot.makeRotate( 
                osg::DegreesToRadians( _camRotOffsetEgo.x()  ), osg::Vec3f( 0, 1, 0 ), // roll
                osg::DegreesToRadians( _camRotOffsetEgo.y()  ), osg::Vec3f( 1, 0, 0 ), // pitch
                osg::DegreesToRadians( _camRotOffsetEgo.z()  ), osg::Vec3f( 0, 0, 1 )  // yaw
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

void EnPlayer::setNextCameraMode()
{
    if ( _cameraMode == Spheric )
        setCameraMode( Ego );
    else
        setCameraMode( Spheric );
}

void EnPlayer::setCameraPitchYaw( float pitch, float yaw )
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
