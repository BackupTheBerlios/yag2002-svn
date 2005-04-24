/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2002-2005, Ali Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU General Public License 
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public 
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

#include <ctd_base.h>
#include <ctd_application.h>
#include <ctd_levelmanager.h>
#include <ctd_physics.h>
#include <ctd_log.h>
#include <ctd_utils.h>
#include "ctd_player.h"
#include "ctd_playerphysics.h"
#include "ctd_playeranim.h"
#include "ctd_playersound.h"
#include "ctd_chatgui.h"
#include "../visuals/ctd_camera.h"

using namespace osg;
using namespace std;

namespace CTD
{

//! Input handler class for player
class PlayerInputHandler : public GenericInputHandler< EnPlayer >
{
    public:

                                            PlayerInputHandler( EnPlayer*p_player );
                                            
        virtual                             ~PlayerInputHandler();

        /**
        * Handle input events.
        */
        bool                                handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

    protected:

        bool                                _rotateRight;
        bool                                _rotateLeft;
        bool                                _moveForward;
        bool                                _moveBackward;
        bool                                _camSwitch;
};

//! Implement and register the player entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( PlayerEntityFactory );
    
EnPlayer::EnPlayer() :
_p_playerPhysics( NULL ),
_p_playerAnimation( NULL ),
_p_playerSound( NULL ),
_p_chatGui( new PlayerChatGui ),
_playerName( "noname" ),
_moveDir( Vec3f( 0, 1, 0 ) ),
_p_inputHandler( NULL ),
_chatGuiLayoutFile( "gui/chat.xml" ),
_p_camera( NULL ),
_cameraMode( Isometric )
{
    EntityManager::get()->registerUpdate( this );     // register entity in order to get updated per simulation step

    getAttributeManager().addAttribute( "name"                  , _playerName           );
    getAttributeManager().addAttribute( "physicsentity"         , _physicsEntity        );
    getAttributeManager().addAttribute( "animationentity"       , _animationEntity      );
    getAttributeManager().addAttribute( "soundentity"           , _soundEntity          );
    getAttributeManager().addAttribute( "position"              , _position             );
    getAttributeManager().addAttribute( "rotation"              , _rot                  );
    getAttributeManager().addAttribute( "cameraPosOffsetIso"    , _camPosOffsetIso      );
    getAttributeManager().addAttribute( "cameraRotOffsetIso"    , _camRotOffsetIso      );
    getAttributeManager().addAttribute( "cameraPosOffsetEgo"    , _camPosOffsetEgo      );
    getAttributeManager().addAttribute( "cameraRotOffsetEgo"    , _camRotOffsetEgo      );
    getAttributeManager().addAttribute( "chatGuiLayoutFile"     , _chatGuiLayoutFile    );

    // create a new input handler for this player
    _p_inputHandler = new PlayerInputHandler( this );
}

EnPlayer::~EnPlayer()
{
    if ( _p_playerPhysics )
    {
        EntityManager::get()->deleteEntity( _p_playerPhysics );
        _p_playerPhysics->destroy();
    }

    if ( _p_playerAnimation )
    {
        EntityManager::get()->deleteEntity( _p_playerAnimation );
        _p_playerAnimation->destroy();
    }

    if ( _p_playerSound )
    {
        EntityManager::get()->deleteEntity( _p_playerSound );
        _p_playerSound->destroy();
    }

    // destroy input handler
    _p_inputHandler->destroyHandler();
}

void EnPlayer::initialize()
{    
    _p_chatGui->initialize( this, _chatGuiLayoutFile );
}

void EnPlayer::postInitialize()
{
    log << Log::LogLevel( Log::L_INFO ) << "  initializing player instance '" << getInstanceName() << "' ..." << endl;

    log << Log::LogLevel( Log::L_DEBUG ) << "   - searching for camera entity ..." << endl;
    // get camera entity
    _p_camera = dynamic_cast< EnCamera* >( EntityManager::get()->findEntity( ENTITY_NAME_CAMERA ) );
    assert( _p_camera && "could not find the camera entity. you need at lease one!" );

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

    // setup camera mode
    setCameraMode( _cameraMode );

    log << Log::LogLevel( Log::L_INFO ) << "  player instance successfully initialized" << endl;
}

void EnPlayer::updateEntity( float deltaTime )
{
    _p_playerPhysics->update( deltaTime );
    // adjust the camera to updated position and rotation. the physics updates the translation of player.
    // adjust camera to character's head position
    _p_camera->setCameraTranslation( _position, _rotation );
}

void EnPlayer::setCameraMode( unsigned int mode )
{
    switch ( mode )
    {
        case Isometric:
        {
            _p_camera->setCameraOffsetPosition( _camPosOffsetIso );
            osg::Quat rot;
            rot.makeRotate( 
                osg::DegreesToRadians( _camRotOffsetIso.x()  ), osg::Vec3f( 0, 1, 0 ), // roll
                osg::DegreesToRadians( _camRotOffsetIso.y()  ), osg::Vec3f( 1, 0, 0 ), // pitch
                osg::DegreesToRadians( _camRotOffsetIso.z()  ), osg::Vec3f( 0, 0, 1 )  // yaw
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
    if ( _cameraMode == Isometric )
        setCameraMode( Ego );
    else
        setCameraMode( Isometric );
}

void EnPlayer::setCameraPitchYaw( float pitch, float yaw )
{
#define LIMIT_PITCH_ANGLE   45.0f
#define LIMIT_PITCH_OFFSET  -20.0f;

    if ( _cameraMode == Isometric )
    {
        float angleY = yaw * 360.0f;
        float angleX = ( pitch * LIMIT_PITCH_ANGLE ) + LIMIT_PITCH_OFFSET;
        _p_camera->setLocalPitchYaw( angleX, angleY );
    }
    // in ego mode we turn the character instead of yawing the camera!
    else
    {
        float angleY = -yaw * 360.0f;
        float angleX = pitch * LIMIT_PITCH_ANGLE;

        static float lastY = 0;

        _rot += osg::DegreesToRadians( angleY - lastY );
        if ( _rot > PI * 2.0f )
            _rot -= PI * 2.0f;

        lastY = angleY;

        _moveDir._v[ 0 ] = sinf( _rot );
        _moveDir._v[ 1 ] = cosf( _rot );
        _p_playerAnimation->animTurn();

        _p_camera->setLocalPitch( angleX );
    }
}

// input handler implementation
//-----------------------------
PlayerInputHandler::PlayerInputHandler( EnPlayer* p_player ) : 
GenericInputHandler< EnPlayer >( p_player ),
_rotateRight( false ),
_rotateLeft( false ),
_moveForward( false ),
_moveBackward( false ),
_camSwitch( false )
{
}

PlayerInputHandler::~PlayerInputHandler() 
{
}

bool PlayerInputHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
    unsigned int eventType   = ea.getEventType();
    int          key         = ea.getKey();

    // dispatch key activity
    if ( eventType == osgGA::GUIEventAdapter::KEYDOWN )
    {
        // change camera mode
        if ( key == osgGA::GUIEventAdapter::KEY_F1 )
        {
            if ( !_camSwitch )
            {
                _p_userObject->setNextCameraMode();
                _camSwitch = true;
            }
        }

        if ( key == osgGA::GUIEventAdapter::KEY_Up )
            _moveForward = true;

        if ( key == osgGA::GUIEventAdapter::KEY_Down )
            _moveBackward = true;

        if ( key == osgGA::GUIEventAdapter::KEY_Right )
            _rotateRight = true;

        if ( key == osgGA::GUIEventAdapter::KEY_Left )
            _rotateLeft = true;

        if ( key == 'm' )
        {
            _p_userObject->_p_playerPhysics->jump();
            _p_userObject->_p_playerAnimation->animJump();
        }
    }
    else if ( eventType == osgGA::GUIEventAdapter::KEYUP )
    {     
        if ( key == osgGA::GUIEventAdapter::KEY_F1 )
            _camSwitch = false;
            
        if ( ( key == osgGA::GUIEventAdapter::KEY_Up ) || ( key == osgGA::GUIEventAdapter::KEY_Down ) )
        {
            if ( key == osgGA::GUIEventAdapter::KEY_Up )
                _moveForward = false;

            if ( key == osgGA::GUIEventAdapter::KEY_Down )
                _moveBackward = false;

            _p_userObject->_p_playerPhysics->setForce( 0, 0 );
        }

        if ( key == osgGA::GUIEventAdapter::KEY_Right )
            _rotateRight = false;

        if ( key == osgGA::GUIEventAdapter::KEY_Left )
            _rotateLeft = false;
    }

    if ( _rotateRight )
    {
        _p_userObject->_rot += _p_userObject->_p_playerPhysics->getAngularForce();
        if ( _p_userObject->_rot > PI * 2.0f )
            _p_userObject->_rot -= PI * 2.0f;

        _p_userObject->_moveDir._v[ 0 ] = sinf( _p_userObject->_rot );
        _p_userObject->_moveDir._v[ 1 ] = cosf( _p_userObject->_rot );

        _p_userObject->_p_playerAnimation->animTurn();
    }

    if ( _rotateLeft )
    {
        if ( _p_userObject->_rot < 0 )
            _p_userObject->_rot += PI * 2.0f;
        _p_userObject->_rot -= _p_userObject->_p_playerPhysics->getAngularForce();

        _p_userObject->_moveDir._v[ 0 ] = sinf( _p_userObject->_rot );
        _p_userObject->_moveDir._v[ 1 ] = cosf( _p_userObject->_rot );

        _p_userObject->_p_playerAnimation->animTurn();
    }

    if ( _moveForward )
    {
        _p_userObject->_p_playerPhysics->setForce( _p_userObject->_moveDir._v[ 0 ], _p_userObject->_moveDir._v[ 1 ] );
        _p_userObject->_p_playerAnimation->animWalk();
    }

    if ( _moveBackward )
    {
        _p_userObject->_p_playerPhysics->setForce( -_p_userObject->_moveDir._v[ 0 ], -_p_userObject->_moveDir._v[ 1 ] );
        _p_userObject->_p_playerAnimation->animWalk();
    }

    if ( !_moveForward && !_moveBackward )
    {
        if ( !_rotateLeft && !_rotateRight )
            _p_userObject->_p_playerAnimation->animIdle();

        if ( _p_userObject->getPlayerSound() )
            _p_userObject->getPlayerSound()->stopPlayingAll();
    }
    
    // adjust pitch / yaw
    _p_userObject->setCameraPitchYaw( ea.getY(), -ea.getX() );

    return false; // let other handlers get all inputs handled here
}

} // namespace CTD
