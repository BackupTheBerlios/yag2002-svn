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
 # player's input handler
 #
 #   date of creation:  05/26/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

//! Camera switch functionality is currently only available in development build
//! NOTE (04/22/2009) uppon many requests, the camera mode was re-enabled, although no cam physics exists!
#ifndef VRC_BUILD_PUBLISH
    #define ENABLE_CAM_SWITCH   1
#else
    #define ENABLE_CAM_SWITCH   1
#endif


template< class PlayerImplT >
PlayerIHCharacterCameraCtrl< PlayerImplT >::PlayerIHCharacterCameraCtrl( PlayerImplT* p_player, EnPlayer* p_playerentity ) :
vrc::gameutils::GenericInputHandler< PlayerImplT >( p_player ),
_p_mouseData( NULL ),
_p_playerEntity( p_playerentity ),
_attributeContainer( p_player->getPlayerAttributes() ),
_enabled( true ),
_menuEnabled( true ),
_right( false ),
_left( false ),
_moveForward( false ),
_moveBackward( false ),
_camSwitch( false ),
_chatSwitch( false ),
_toggleChatMode( false ),
_showFPS( false ),
_keyCodeMoveForward( osgGA::GUIEventAdapter::KEY_Up ),
_keyCodeMoveBackward( osgGA::GUIEventAdapter::KEY_Down ),
_keyCodeMoveLeft( osgGA::GUIEventAdapter::KEY_Left ),
_keyCodeMoveRight( osgGA::GUIEventAdapter::KEY_Right ),
_keyCodeJump( osgGA::GUIEventAdapter::KEY_Space ),
_keyCodeCameraMode( osgGA::GUIEventAdapter::KEY_F1 ),
_keyCodeChatMode( osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON ),
_invertedMouse( false ),
_mouseSensitivity( 1.0f )
{
    // initialize the mouse data considering initial yaw
    _p_mouseData = new MouseData( 0.0f, getPlayerImpl()->_rotZ );
    // initialize the player pitch / yaw
    updatePlayerPitchYaw( _p_mouseData->_pitch, _p_mouseData->_yaw );
}

template< class PlayerImplT >
PlayerIHCharacterCameraCtrl< PlayerImplT >::~PlayerIHCharacterCameraCtrl()
{
    delete _p_mouseData;
}

template< class PlayerImplT >
bool PlayerIHCharacterCameraCtrl< PlayerImplT >::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*aa*/ )
{
    unsigned int controlmodes = gameutils::PlayerUtils::get()->getPlayerControlModes();

    // while in menu we skip input processing for player
    if ( _menuEnabled )
        return false;

    const osgSDL::SDLEventAdapter* p_eventAdapter = dynamic_cast< const osgSDL::SDLEventAdapter* >( &ea );
    assert( p_eventAdapter && "invalid event adapter received" );

    unsigned int eventType  = p_eventAdapter->getEventType();
    int          kcode      = p_eventAdapter->getSDLKey();
    unsigned int mouseBtn   = p_eventAdapter->getButton();
    bool keyDown            = ( eventType == osgGA::GUIEventAdapter::KEYDOWN );
    bool keyUp              = ( eventType == osgGA::GUIEventAdapter::KEYUP   );
    bool mouseButtonPush    = ( eventType == osgGA::GUIEventAdapter::PUSH    );
    bool mouseButtonRelease = ( eventType == osgGA::GUIEventAdapter::RELEASE );

    float deltaTime         = getPlayerEntity()->getDeltaTime();
    unsigned int key        = 0;
    unsigned int orgkey     = 0;

    if ( keyDown || keyUp )
        key = kcode;
    else if ( mouseButtonPush || mouseButtonRelease )
        key = mouseBtn;

    // unfiltered key code
    orgkey = key;

    // check the control mode and filter the key
    if ( controlmodes & ( gameutils::PlayerUtils::eMailBoxOpen | gameutils::PlayerUtils::eInventoryOpen ) )
    {
        if ( 
            ( key == _keyCodeChatMode     ) ||
            ( key == _keyCodeMoveForward  ) ||
            ( key == _keyCodeMoveBackward ) ||
            ( key == _keyCodeMoveRight    ) ||
            ( key == _keyCodeMoveLeft     ) ||
            ( key == _keyCodeJump         ) ||
            ( key == SDLK_RETURN          ) ||
            ( key == SDLK_KP_ENTER        )
            )
            key = 0;
    }

    // activate the chat box on pressing ENTER key
    if ( keyDown && ( ( key == SDLK_RETURN ) || ( key == SDLK_KP_ENTER ) ) )
    {
        getPlayerImpl()->getChatManager()->activateBox( true, true );
        enable( false );
        _toggleChatMode = true;
    }

    // show up the fps display
    if ( keyDown && ( orgkey == SDLK_F9 ) )
    {
        // first check if the fps entity already exists
        EnFPSDisplay* p_fps = static_cast< EnFPSDisplay* >( yaf3d::EntityManager::get()->findEntity( ENTITY_NAME_FPSDISPLAY ) );
        if ( p_fps )
        {
            _showFPS = !_showFPS;
            p_fps->enable( _showFPS );
        }
        else
        {
            _showFPS = true;
            p_fps = static_cast< EnFPSDisplay* >( yaf3d::EntityManager::get()->createEntity( ENTITY_NAME_FPSDISPLAY, "_fps_" ) );
            p_fps->initialize();
            p_fps->postInitialize();
            p_fps->enable( _showFPS );
        }
    }

    // check the edit / walk toggle command
    //--------
    if ( keyDown || mouseButtonPush )
    {
        if ( key == _keyCodeChatMode )
        {
            if ( !_chatSwitch )
            {
                if ( !_toggleChatMode )
                {
                    _chatSwitch = true;

                    // check if we are allowed to change to chat mode
                    unsigned int newmode = gameutils::PlayerUtils::get()->setPlayerControlMode( gameutils::PlayerUtils::eEditting );
                    if ( newmode & gameutils::PlayerUtils::eEditting )
                    {
                        // let the chat control know that we are in edit mode now
                        getPlayerImpl()->getChatManager()->activateBox( !_toggleChatMode );
                    }
                }
                else
                {
                    // reset editting mode
                    gameutils::PlayerUtils::get()->resetPlayerControlMode( gameutils::PlayerUtils::eEditting );
                    getPlayerImpl()->getChatManager()->activateBox( !_toggleChatMode );

                    // ensures continue the current movement when toggling from walk to edit mode
                    //  so allowing chatting during moving ( it was requested by the community ), but toggling back stops movement
                    // stop player and sound
                    getPlayerImpl()->getPlayerAnimation()->animIdle();
                    getPlayerImpl()->getPlayerPhysics()->stopMovement();
                }

                // this method has an side effect on _enable (see below)
                enable( _toggleChatMode );

                _toggleChatMode = !_toggleChatMode;
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
#if ENABLE_CAM_SWITCH
        // change camera mode
        if ( key == _keyCodeCameraMode )
        {
            if ( !_camSwitch ) // spheric mode
            {
                getPlayerImpl()->setNextCameraMode();
                _camSwitch = true;

                // store current pitch / yaw and restore the old values
                _p_mouseData->pushbackPitchYaw();
                _p_mouseData->popfrontPitchYaw();

                // synchronize the mouse data yaw on switching to ego mode, as in spheric mode
                //  the player rotation can also be changed
                if ( getPlayerImpl()->_cameraMode == vrc::BasePlayerImplementation::Ego )
                    _p_mouseData->_yaw = getPlayerImpl()->_rotZ;

                // after cam mode switching and restoring the pitch / yaw we have also to update the player pitch / yaw
                updatePlayerPitchYaw( _p_mouseData->_pitch, _p_mouseData->_yaw );
            }
        }
#endif

        if ( key == _keyCodeMoveForward )
            _moveForward = true;

        if ( key == _keyCodeMoveBackward )
            _moveBackward = true;

        if ( key == _keyCodeMoveRight )
            _right = true;

        if ( key == _keyCodeMoveLeft )
            _left = true;

        if ( ( key == _keyCodeJump ) && ( !getPlayerImpl()->getPlayerPhysics()->isJumping() ) )
        {
            getPlayerImpl()->getPlayerPhysics()->jump();
            getPlayerImpl()->getPlayerAnimation()->animIdle();
            getPlayerImpl()->getPlayerAnimation()->animJump();
        }
    }
    else if ( keyUp || mouseButtonRelease )
    {
#if ENABLE_CAM_SWITCH
        if ( key == _keyCodeCameraMode )
            _camSwitch = false;
#endif
        if ( ( key == _keyCodeMoveForward ) || ( key == _keyCodeMoveBackward ) )
        {
            if ( key == _keyCodeMoveForward )
                _moveForward = false;

            if ( key == _keyCodeMoveBackward )
                _moveBackward = false;

            getPlayerImpl()->getPlayerPhysics()->stopMovement();
        }

        if ( ( key == _keyCodeMoveRight ) || ( key == _keyCodeMoveLeft ) )
        {

            if ( key == _keyCodeMoveRight )
                _right = false;

            if ( key == _keyCodeMoveLeft )
                _left = false;

            getPlayerImpl()->getPlayerPhysics()->stopMovement();
        }
    }
    //--------

    // execute dispatched commands

    if ( _moveForward )
    {
        getPlayerImpl()->getPlayerPhysics()->setDirection( getPlayerImpl()->_moveDir._v[ 0 ], getPlayerImpl()->_moveDir._v[ 1 ] );
        if ( getPlayerImpl()->getPlayerPhysics()->onGround() )
            getPlayerImpl()->getPlayerAnimation()->animWalk();
        else
            getPlayerImpl()->getPlayerAnimation()->animIdle();
    }
    else if ( _moveBackward )
    {
        getPlayerImpl()->getPlayerPhysics()->setDirection( -getPlayerImpl()->_moveDir._v[ 0 ], -getPlayerImpl()->_moveDir._v[ 1 ] );
        if ( getPlayerImpl()->getPlayerPhysics()->onGround() )
            getPlayerImpl()->getPlayerAnimation()->animWalk();
        else
            getPlayerImpl()->getPlayerAnimation()->animIdle();
    }

    if ( _right )
    {
        switch ( getPlayerImpl()->_cameraMode )
        {
            case BasePlayerImplementation::Spheric:
            {
                float& rotZ = getPlayerImpl()->_rotZ;
                rotZ += getPlayerImpl()->getPlayerPhysics()->getAngularSpeed() * deltaTime;
                getPlayerImpl()->_moveDir._v[ 0 ] = sinf( rotZ );
                getPlayerImpl()->_moveDir._v[ 1 ] = cosf( rotZ );

                if ( !( _moveForward || _moveBackward ) )
                    getPlayerImpl()->getPlayerAnimation()->animTurn();
            }
            break;

            case BasePlayerImplementation::Ego:
            {
                osg::Vec3f side;
                side = getPlayerImpl()->_moveDir ^ osg::Vec3f( 0.0f, 0.0f, 1.0f );
                if ( _moveForward || _moveBackward )
                    getPlayerImpl()->getPlayerPhysics()->addDirection( side._v[ 0 ], side._v[ 1 ] );
                else
                    getPlayerImpl()->getPlayerPhysics()->setDirection( side._v[ 0 ], side._v[ 1 ] );

                if ( !( _moveForward || _moveBackward ) )
                    getPlayerImpl()->getPlayerAnimation()->animTurn();
            }
            break;

            default:
                assert( NULL && "unknown camera state!" );
        }
    }

    if ( _left )
    {
        switch ( getPlayerImpl()->_cameraMode )
        {
            case BasePlayerImplementation::Spheric:
            {
                float& rotZ = getPlayerImpl()->_rotZ;
                rotZ -= getPlayerImpl()->getPlayerPhysics()->getAngularSpeed() * deltaTime;
                getPlayerImpl()->_moveDir._v[ 0 ] = sinf( rotZ );
                getPlayerImpl()->_moveDir._v[ 1 ] = cosf( rotZ );

                if ( !( _moveForward || _moveBackward ) )
                    getPlayerImpl()->getPlayerAnimation()->animTurn();
            }
            break;

            case BasePlayerImplementation::Ego:
            {
                osg::Vec3f side;
                side = getPlayerImpl()->_moveDir ^ osg::Vec3f( 0.0f, 0.0f, -1.0f );
                if ( _moveForward || _moveBackward )
                    getPlayerImpl()->getPlayerPhysics()->addDirection( side._v[ 0 ], side._v[ 1 ] );
                else
                    getPlayerImpl()->getPlayerPhysics()->setDirection( side._v[ 0 ], side._v[ 1 ] );

                if ( !( _moveForward || _moveBackward ) )
                    getPlayerImpl()->getPlayerAnimation()->animTurn();
            }
            break;

            default:
                assert( NULL && "unknown camera state!" );
        }
    }

    // handle stopping animation
    {
        if ( !getPlayerImpl()->getPlayerPhysics()->isMoving() && !_right && !_left )
            getPlayerImpl()->getPlayerAnimation()->animIdle();
    }

    // check if looking is allowed
    if ( !( controlmodes & ( gameutils::PlayerUtils::ePropertyBoxOpen | gameutils::PlayerUtils::eEditting ) ) )
    {
        // get the SDL event in order to extract mouse button and absolute / relative mouse movement coordinates out of it
        const SDL_Event& sdlevent = p_eventAdapter->getSDLEvent();

        // handle mouse wheel changes for camera offsetting in spheric mode
        if ( getPlayerImpl()->_cameraMode == BasePlayerImplementation::Spheric )
        {
            if ( sdlevent.button.type == SDL_MOUSEBUTTONDOWN )
            {
                if ( sdlevent.button.button == SDL_BUTTON_WHEELUP )
                {
                    float& dist = _attributeContainer._camPosOffsetSpheric._v[ 1 ];
                    dist = std::min( dist + SPHERIC_DIST_INCREMENT, -LIMIT_SPHERIC_MIN_DIST );
                    getPlayerImpl()->_p_camera->setCameraOffsetPosition( _attributeContainer._camPosOffsetSpheric );
                }
                else if ( sdlevent.button.button == SDL_BUTTON_WHEELDOWN )
                {
                    float& dist = _attributeContainer._camPosOffsetSpheric._v[ 1 ];
                    dist = std::max( -LIMIT_SPHERIC_MAX_DIST, dist - SPHERIC_DIST_INCREMENT );
                    getPlayerImpl()->_p_camera->setCameraOffsetPosition( _attributeContainer._camPosOffsetSpheric );
                }
            }
        }

        // adjust pitch and yaw depending on camera mode
        if ( eventType == osgGA::GUIEventAdapter::MOVE )
        {
            // skip events which come in when we warp the mouse pointer to middle of app window ( see below )
            if ( (  sdlevent.motion.x == _p_mouseData->_screenMiddleX ) && ( sdlevent.motion.y == _p_mouseData->_screenMiddleY ) )
                return false;

            // limit the movement gradient in x and y direction and multiply with mouse sensitivity factor
            float xrel = float( sdlevent.motion.xrel );
            if ( xrel > LIMIT_MMOVE_DELTA )
                xrel = LIMIT_MMOVE_DELTA;
            else if ( xrel < -LIMIT_MMOVE_DELTA )
                xrel = -LIMIT_MMOVE_DELTA;
            xrel *= _mouseSensitivity;

            float yrel = float( sdlevent.motion.yrel );
            if ( yrel > LIMIT_MMOVE_DELTA )
                yrel = LIMIT_MMOVE_DELTA;
            else if ( yrel < -LIMIT_MMOVE_DELTA )
                yrel = -LIMIT_MMOVE_DELTA;
            yrel *= _mouseSensitivity;

            // update accumulated mouse coords ( pitch / yaw )
            _p_mouseData->_yaw += xrel / _p_mouseData->_screenSizeX;
            // consider the mouse invert flag
            if ( !_invertedMouse )
                _p_mouseData->_pitch -= yrel / _p_mouseData->_screenSizeY;
            else
                _p_mouseData->_pitch += yrel / _p_mouseData->_screenSizeY;

            // update the player pitch / yaw
            updatePlayerPitchYaw( _p_mouseData->_pitch, _p_mouseData->_yaw );

            // calculate yaw and change player direction when in ego mode
            if ( getPlayerImpl()->_cameraMode == vrc::BasePlayerImplementation::Ego )
            {
                float& rotZ = getPlayerImpl()->_rotZ;
                rotZ = _p_mouseData->_yaw;
                getPlayerImpl()->_moveDir._v[ 0 ] = sinf( rotZ );
                getPlayerImpl()->_moveDir._v[ 1 ] = cosf( rotZ );
            }

            // reset mouse position in order to avoid leaving the app window
            yaf3d::Application::get()->getViewer()->requestWarpPointer( _p_mouseData->_screenMiddleX, _p_mouseData->_screenMiddleY );
        }
    }

    return false;
}

template< class PlayerImplT >
void PlayerIHCharacterCameraCtrl< PlayerImplT >::updatePlayerPitchYaw( float& pitch, float& yaw )
{
    // in ego mode the mouse controls the player rotation
    if ( getPlayerImpl()->_cameraMode == vrc::BasePlayerImplementation::Ego )
    {
        // limit pitch
        if ( pitch > LIMIT_PITCH_ANGLE )
            pitch = LIMIT_PITCH_ANGLE;
        else if ( pitch < -LIMIT_PITCH_ANGLE )
            pitch = -LIMIT_PITCH_ANGLE;

        // set pitch
        getPlayerImpl()->setCameraPitchYaw( pitch, 0 );
    }
    else
    {
        // limit pitch
        if ( pitch > 0 )
            pitch = 0;
        else if ( pitch < -LIMIT_PITCH_ANGLE )
            pitch = -LIMIT_PITCH_ANGLE;

        // set pitch / yaw
        getPlayerImpl()->setCameraPitchYaw( pitch, -yaw );
    }
}
