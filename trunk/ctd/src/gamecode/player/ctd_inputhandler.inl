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
 # player's input handler
 #
 #   date of creation:  05/26/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/


template< class PlayerImplT >
PlayerIHCharacterCameraCtrl< PlayerImplT >::PlayerIHCharacterCameraCtrl( PlayerImplT* p_player, EnPlayer* p_playerentity ) : 
GenericInputHandler< PlayerImplT >( p_player ),
_attributeContainer( p_player->getPlayerAttributes() ),
_p_playerEntity( p_playerentity ),
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
_keyCodeChatMode( osgGA::GUIEventAdapter::RIGHT_MOUSE_BUTTON ),
_invertedMouse( false ),
_mouseSensitivity( 1.0f )
{
}

template< class PlayerImplT >
PlayerIHCharacterCameraCtrl< PlayerImplT >::~PlayerIHCharacterCameraCtrl() 
{
}

template< class PlayerImplT >
bool PlayerIHCharacterCameraCtrl< PlayerImplT >::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
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
                enable( s_toggleChatMode );
                s_toggleChatMode = !s_toggleChatMode;
                _chatSwitch = true;

                // let the chat control know that we are in edit mode now
                getPlayerImpl()->_p_chatGui->setEditMode( s_toggleChatMode );

                // show / hide pointer depending on chat mode and camera view mode
                if ( s_toggleChatMode )
                        GuiManager::get()->showMousePointer( true );
                else
                    if ( getPlayerImpl()->_cameraMode == EnPlayer::Ego )
                        GuiManager::get()->showMousePointer( true );
                    else
                        GuiManager::get()->showMousePointer( false );

                // stop player and sound
                getPlayerImpl()->getPlayerAnimation()->animIdle();
                getPlayerImpl()->getPlayerSound()->stopPlayingAll();
                getPlayerImpl()->getPlayerPhysics()->stopMovement();
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
            if ( !_camSwitch ) // spheric mode
            {
                getPlayerImpl()->setNextCameraMode();
                _camSwitch = true;

                // store current pitch / yaw and restore the old values
                _mouseData.pushPitchYaw();
                _mouseData.popPitchYaw();
                // after cam mode switching and restoring the pitch / yaw we have also to update the player pitch / yaw
                updatePlayerPitchYaw( _mouseData._pitch, _mouseData._yaw );
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

        if ( key == _keyCodeJump && !getPlayerImpl()->_p_playerPhysics->isJumping() )
        {
            getPlayerImpl()->_p_playerPhysics->jump();
            getPlayerImpl()->_p_playerAnimation->animIdle(); // stop any movement animation first
            getPlayerImpl()->_p_playerAnimation->animJump();
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

            getPlayerImpl()->_p_playerPhysics->stopMovement();
        }

        if ( ( key == _keyCodeMoveRight ) || ( key == _keyCodeMoveLeft ) )
        {

            if ( key == _keyCodeMoveRight )
                _right = false;

            if ( key == _keyCodeMoveLeft )
                _left = false;

            if ( getPlayerImpl()->_cameraMode == EnPlayer::Ego )
                getPlayerImpl()->_p_playerPhysics->stopMovement();
        }
    }
    //--------

    // execute dispatched commands
    if ( _moveForward )
    {
        getPlayerImpl()->_p_playerPhysics->setForce( getPlayerImpl()->_moveDir._v[ 0 ], getPlayerImpl()->_moveDir._v[ 1 ] );

        if ( !getPlayerImpl()->_p_playerPhysics->isJumping() )
            getPlayerImpl()->_p_playerAnimation->animWalk();
    } 
    
    if ( _moveBackward )
    {
        getPlayerImpl()->_p_playerPhysics->setForce( -getPlayerImpl()->_moveDir._v[ 0 ], -getPlayerImpl()->_moveDir._v[ 1 ] );

        if ( !getPlayerImpl()->_p_playerPhysics->isJumping() )
            getPlayerImpl()->_p_playerAnimation->animWalk();
    }

    if ( _right )
    {
        switch ( getPlayerImpl()->_cameraMode )
        {
            case EnPlayer::Spheric:
            {
                float& rotZ = getPlayerImpl()->_rotZ;
                rotZ += getPlayerImpl()->_p_playerPhysics->getAngularForce();

                getPlayerImpl()->_moveDir._v[ 0 ] = sinf( rotZ );
                getPlayerImpl()->_moveDir._v[ 1 ] = cosf( rotZ );

                getPlayerImpl()->_p_playerAnimation->animTurn();
            }
            break;

            case EnPlayer::Ego:
            {
                osg::Vec3f side;
                side = getPlayerImpl()->_moveDir ^ osg::Vec3f( 0.0f, 0.0f, 1.0f );
                if ( _moveForward || _moveBackward )
                    getPlayerImpl()->_p_playerPhysics->addForce( side._v[ 0 ], side._v[ 1 ] );
                else
                    getPlayerImpl()->_p_playerPhysics->setForce( side._v[ 0 ], side._v[ 1 ] );

                getPlayerImpl()->_p_playerAnimation->animTurn();
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
            case EnPlayer::Spheric:
            {
                float& rotZ = getPlayerImpl()->_rotZ;
                rotZ -= getPlayerImpl()->_p_playerPhysics->getAngularForce();

                getPlayerImpl()->_moveDir._v[ 0 ] = sinf( rotZ );
                getPlayerImpl()->_moveDir._v[ 1 ] = cosf( rotZ );

                getPlayerImpl()->_p_playerAnimation->animTurn();
            }
            break;
            
            case EnPlayer::Ego:
            {
                osg::Vec3f side;
                side = getPlayerImpl()->_moveDir ^ osg::Vec3f( 0.0f, 0.0f, -1.0f );
                if ( _moveForward || _moveBackward )
                    getPlayerImpl()->_p_playerPhysics->addForce( side._v[ 0 ], side._v[ 1 ] );
                else
                    getPlayerImpl()->_p_playerPhysics->setForce( side._v[ 0 ], side._v[ 1 ] );

                getPlayerImpl()->_p_playerAnimation->animTurn();
            }
            break;

            default:
                assert( NULL && "unknown camera state!" );
        }
    }

    // handle stopping movement
    {
        typedef enum { Idle, Stopped } MovementStates;
        static MovementStates s_states = Idle;
        bool movefb = !_moveForward && !_moveBackward;
        bool movelr = !_right && !_left;
        switch ( s_states )
        {
            case Idle:
            {
                if ( getPlayerImpl()->_p_playerPhysics->isJumping() )
                {
                    s_states = Stopped;
                    getPlayerImpl()->_p_playerPhysics->stopMovement();
                } 
                else if ( getPlayerImpl()->_cameraMode == EnPlayer::Spheric )
                {
                    if ( movefb )
                    {
                        s_states = Stopped;
                        getPlayerImpl()->_p_playerPhysics->stopMovement();
                    }
                } 
                else
                {
                    if ( movelr && movefb )
                    {
                        s_states = Stopped;
                        getPlayerImpl()->_p_playerPhysics->stopMovement();
                    }
                }

            }
            break;

            case Stopped:
            {
                if ( !_left && !_right )
                    getPlayerImpl()->_p_playerAnimation->animIdle();

               if ( getPlayerImpl()->getPlayerSound() )
                    getPlayerImpl()->getPlayerSound()->stopPlayingAll();

                if ( ( _moveForward || _moveBackward ) && !getPlayerImpl()->_p_playerPhysics->isJumping() )
                    s_states = Idle;
            }
            break;

            default:
                assert( NULL && "invalid movement state in player's input handler" );

        }
    }

    // get the SDL event in order to extract mouse button and absolute / relative mouse movement coordinates out of it
    const SDL_Event& sdlevent = p_eventAdapter->getSDLEvent();

    // handle mouse wheel changes for camera offsetting in spheric mode
    if ( getPlayerImpl()->_cameraMode == EnPlayer::Spheric )
    {
        if ( sdlevent.button.button == SDL_BUTTON_WHEELUP )
        {
            float& dist = _attributeContainer._camPosOffsetSpheric._v[ 1 ];
            dist = min( dist + SPHERIC_DIST_INCREMENT, -LIMIT_SPHERIC_MIN_DIST );
            getPlayerImpl()->_p_camera->setCameraOffsetPosition( _attributeContainer._camPosOffsetSpheric );
        }
        else if ( sdlevent.button.button == SDL_BUTTON_WHEELDOWN )
        {
            float& dist = _attributeContainer._camPosOffsetSpheric._v[ 1 ];
            dist = max( -LIMIT_SPHERIC_MAX_DIST, dist - SPHERIC_DIST_INCREMENT );
            getPlayerImpl()->_p_camera->setCameraOffsetPosition( _attributeContainer._camPosOffsetSpheric );
        }
    }

    // adjust pitch and yaw depending on camera mode
    if ( eventType == osgGA::GUIEventAdapter::MOVE )
    {
        // skip events which come in when we warp the mouse pointer to middle of app window ( see below )
        if ( (  sdlevent.motion.x == _mouseData._screenMiddleX ) && ( sdlevent.motion.y == _mouseData._screenMiddleY ) )
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
        _mouseData._yaw += xrel / _mouseData._screenSizeX;
        // consider the mouse invert flag
        if ( !_invertedMouse )
            _mouseData._pitch -= yrel / _mouseData._screenSizeY;
        else
            _mouseData._pitch += yrel / _mouseData._screenSizeY;

        // update the player pitch / yaw
        updatePlayerPitchYaw( _mouseData._pitch, _mouseData._yaw );

        // reset mouse position in order to avoid leaving the app window
        Application::get()->getViewer()->requestWarpPointer( _mouseData._screenMiddleX, _mouseData._screenMiddleY );
    }

    return false;
}

template< class PlayerImplT >
void PlayerIHCharacterCameraCtrl< PlayerImplT >::updatePlayerPitchYaw( float& pitch, float& yaw )
{
    // in ego mode the mouse controls the player rotation
    if ( getPlayerImpl()->_cameraMode == EnPlayer::Ego )
    {
        // limit pitch
        if ( pitch > LIMIT_PITCH_ANGLE )
            pitch = LIMIT_PITCH_ANGLE;
        else if ( pitch < -LIMIT_PITCH_ANGLE )
            pitch = -LIMIT_PITCH_ANGLE;

        // calculate yaw and change player direction when in ego mode
        float& rotZ = getPlayerImpl()->_rotZ;
        rotZ = yaw; 
        getPlayerImpl()->_moveDir._v[ 0 ] = sinf( rotZ );
        getPlayerImpl()->_moveDir._v[ 1 ] = cosf( rotZ );
        getPlayerImpl()->_p_playerAnimation->animTurn();

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
