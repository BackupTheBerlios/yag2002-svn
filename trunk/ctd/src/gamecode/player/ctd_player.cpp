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
#include "../visuals/ctd_camera.h"

using namespace osg;
using namespace std;

namespace CTD
{

// entity name of player's camera
#define PLAYER_CAMERA_ENTITIY_NAME      "playercam"

//! Input handler class for player
class PlayerInputHandler : public GenericInputHandler< EnPlayer >
{
    public:

                                            PlayerInputHandler( EnPlayer*p_player );
                                            
        virtual                             ~PlayerInputHandler();

        //! Handle input events.
        bool                                handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

        //! Enable / disable input handling
        void                                enable( bool enabled )
                                            {
                                                _enabled = enabled;
                                                // reset internal flags
                                                if ( !_enabled )
                                                {
                                                    _right          = false;
                                                    _left           = false;
                                                    _moveForward    = false;
                                                    _moveBackward   = false;
                                                    _camSwitch      = false;
                                                    _chatSwitch     = false;
                                                }
                                            }

        void                                setMenuEnabled( bool en )
                                            {
                                                _menuEnabled = en;
                                            }

    protected:

        // some internal variables
        bool                                _enabled;
        bool                                _menuEnabled;
        bool                                _right;
        bool                                _left;
        bool                                _moveForward;
        bool                                _moveBackward;
        bool                                _camSwitch;
        bool                                _chatSwitch;

    public:

        // key binding codes
        unsigned int                        _keyCodeMoveForward;
        unsigned int                        _keyCodeMoveBackward;
        unsigned int                        _keyCodeMoveLeft;
        unsigned int                        _keyCodeMoveRight;
        unsigned int                        _keyCodeJump;
        unsigned int                        _keyCodeCameraMode;
        unsigned int                        _keyCodeChatMode;
};

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
            static bool firstentracne = true;
            if ( firstentracne )
            {
                _p_chatGui->show( true );
                firstentracne = false;
            }

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
#define LIMIT_PITCH_ANGLE   120.0f
#define LIMIT_PITCH_OFFSET  -20.0f;

    if ( _cameraMode == Spheric )
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
_enabled( true ),
_menuEnabled( true ),
_right( false ),
_left( false ),
_moveForward( false ),
_moveBackward( false ),
_camSwitch( false ),
_chatSwitch( false ),
_keyCodeMoveForward( osgGA::GUIEventAdapter::KEY_Up ),
_keyCodeMoveBackward( osgGA::GUIEventAdapter::KEY_Down ),
_keyCodeMoveLeft( osgGA::GUIEventAdapter::KEY_Left ),
_keyCodeMoveRight( osgGA::GUIEventAdapter::KEY_Right ),
_keyCodeJump( osgGA::GUIEventAdapter::KEY_Space ),
_keyCodeCameraMode( osgGA::GUIEventAdapter::KEY_F1 ),
_keyCodeChatMode( osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON )
{
}

PlayerInputHandler::~PlayerInputHandler() 
{
}

bool PlayerInputHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
    // while in menu we skip input processing for player
    if ( _menuEnabled )
        return false;

    unsigned int eventType   = ea.getEventType();

    int          kcode      = ea.getKey();
    unsigned int mouseBtn   = ea.getButton();    
    bool keyDown            = ( eventType == osgGA::GUIEventAdapter::KEYDOWN );
    bool keyUp              = ( eventType == osgGA::GUIEventAdapter::KEYUP   );
    bool mouseButtonPush    = ( eventType == osgGA::GUIEventAdapter::PUSH    );
    bool mouseButtonRelease = ( eventType == osgGA::GUIEventAdapter::RELEASE );
    
    unsigned int key;

    if ( keyDown || keyUp )
        key = kcode;
    else if ( mouseButtonPush || mouseButtonRelease )
        key = mouseBtn;

    // first check the edit / walk toggle command
    //--------
    static bool s_toggleChatMode = false;
    if ( keyDown || mouseButtonPush )
    {
        if ( key == _keyCodeChatMode )
        {
            if ( !_chatSwitch )
            {
                // this method has an side effect on _enable (see below)
                _p_userObject->enableControl( s_toggleChatMode );
                s_toggleChatMode = !s_toggleChatMode;
                _chatSwitch = true;

                // let the chat control know that we are in edit mode now
                _p_userObject->_p_chatGui->setEditMode( s_toggleChatMode );

                // stop player and sound
                _p_userObject->_p_playerAnimation->animIdle();
                _p_userObject->getPlayerSound()->stopPlayingAll();
                _p_userObject->_p_playerPhysics->stopMovement();            
            }
        }
    }
    else if ( keyUp || mouseButtonRelease )
    {
        if ( key == _keyCodeChatMode )
            _chatSwitch = false;
    }
    //--------

    // check for enable flag, e.g. in menu we don't want to control the character
    if ( !_enabled )
        return false;

    // dispatch player control commands
    //--------
    if ( keyDown || mouseButtonPush )
    {
        // change camera mode
        if ( key == _keyCodeCameraMode )
        {
            if ( !_camSwitch )
            {
                _p_userObject->setNextCameraMode();
                _camSwitch = true;
            }
        }

        if ( key == _keyCodeMoveForward )
            _moveForward = true;

        if ( key == _keyCodeMoveBackward )
            _moveBackward = true;

        if ( key == _keyCodeMoveRight )
            _right = true;

        if ( key == _keyCodeMoveLeft )
            _left = true;

        if ( key == _keyCodeJump )
        {
            _p_userObject->_p_playerPhysics->jump();
            _p_userObject->_p_playerAnimation->animJump();
        }
    } 
    else if ( keyUp || mouseButtonRelease )
    {
        if ( key == _keyCodeCameraMode )
            _camSwitch = false;

        if ( ( key == _keyCodeMoveForward ) || ( key == _keyCodeMoveBackward ) )
        {
            if ( key == _keyCodeMoveForward )
                _moveForward = false;

            if ( key == _keyCodeMoveBackward )
                _moveBackward = false;

            _p_userObject->_p_playerPhysics->stopMovement();
        }

        if ( ( key == _keyCodeMoveRight ) || ( key == _keyCodeMoveLeft ) )
        {

            if ( key == _keyCodeMoveRight )
                _right = false;

            if ( key == _keyCodeMoveLeft )
                _left = false;

            if ( _p_userObject->_cameraMode == EnPlayer::Ego )
                _p_userObject->_p_playerPhysics->stopMovement();
        }
    }
    //--------

    // execute dispatched commands
    if ( _right )
    {
        switch ( _p_userObject->_cameraMode )
        {
            case EnPlayer::Spheric:
            {
                _p_userObject->_rot -= _p_userObject->_p_playerPhysics->getAngularForce();
                if ( _p_userObject->_rot > PI * 2.0f )
                    _p_userObject->_rot += PI * 2.0f;

                _p_userObject->_moveDir._v[ 0 ] = sinf( _p_userObject->_rot );
                _p_userObject->_moveDir._v[ 1 ] = cosf( _p_userObject->_rot );

                _p_userObject->_p_playerAnimation->animTurn();
            }
            break;

            case EnPlayer::Ego:
            {
                osg::Vec3f side;
                side = _p_userObject->_moveDir ^ osg::Vec3f( 0, 0, -1 );
                _p_userObject->_moveDir._v[ 0 ] = side.x();
                _p_userObject->_moveDir._v[ 1 ] = side.y();                
                _p_userObject->_p_playerPhysics->setForce( _p_userObject->_moveDir._v[ 0 ], _p_userObject->_moveDir._v[ 1 ] );
                _p_userObject->_p_playerAnimation->animTurn();
            }
            break;

            default:
                assert( NULL && "unknown camera state!" );
        }
    }

    if ( _left )
    {
        switch ( _p_userObject->_cameraMode )
        {
            case EnPlayer::Spheric:
            {
                if ( _p_userObject->_rot < 0 )
                    _p_userObject->_rot -= PI * 2.0f;
                _p_userObject->_rot += _p_userObject->_p_playerPhysics->getAngularForce();

                _p_userObject->_moveDir._v[ 0 ] = sinf( _p_userObject->_rot );
                _p_userObject->_moveDir._v[ 1 ] = cosf( _p_userObject->_rot );

                _p_userObject->_p_playerAnimation->animTurn();
            }
            break;
            
            case EnPlayer::Ego:
            {
                osg::Vec3f side;
                side = _p_userObject->_moveDir ^ osg::Vec3f( 0, 0, 1 );
                _p_userObject->_moveDir._v[ 0 ] = side.x();
                _p_userObject->_moveDir._v[ 1 ] = side.y();                
                _p_userObject->_p_playerPhysics->setForce( _p_userObject->_moveDir._v[ 0 ], _p_userObject->_moveDir._v[ 1 ] );
                _p_userObject->_p_playerAnimation->animTurn();
            }
            break;

            default:
                assert( NULL && "unknown camera state!" );
        }
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

    // handle stopping movement
    {
        typedef enum { Idle, Stopped } MovementStates;
        static MovementStates s_states = Idle;
        switch ( s_states )
        {
            case Idle:
            {
                if ( !_moveForward && !_moveBackward )
                    s_states = Stopped;

            }
            break;

            case Stopped:
            {
                if ( !_left && !_right )
                    _p_userObject->_p_playerAnimation->animIdle();

                _p_userObject->_p_playerPhysics->stopMovement();

               if ( _p_userObject->getPlayerSound() )
                    _p_userObject->getPlayerSound()->stopPlayingAll();

                if ( _moveForward || _moveBackward )
                    s_states = Idle;
            }
            break;

            default:
                assert( NULL && "invalid movement state in player's input handler" );

        }
    }

    // handle mouse wheel changes for camera offsetting in spheric mode
    if ( _p_userObject->_cameraMode == EnPlayer::Spheric )
    {
        if ( eventType == osgGA::GUIEventAdapter::SCROLLUP )
        {
            _p_userObject->_camPosOffsetSpheric._v[ 1 ] += 0.5f;
            _p_userObject->_p_camera->setCameraOffsetPosition( _p_userObject->_camPosOffsetSpheric );
        }
        else if ( eventType == osgGA::GUIEventAdapter::SCROLLDOWN )
        {
            float& dist = _p_userObject->_camPosOffsetSpheric._v[ 1 ];
            dist = std::min( 0.0f, dist - 0.5f );
            _p_userObject->_p_camera->setCameraOffsetPosition( _p_userObject->_camPosOffsetSpheric );
        }
    }

    // adjust pitch / yaw depending on mouse movement
    _p_userObject->setCameraPitchYaw( ea.getY(), -ea.getX() );

    return false; // let other handlers get all inputs handled here
}

} // namespace CTD
