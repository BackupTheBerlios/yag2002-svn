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

#include <ctd_main.h>
#include "ctd_inputhandler.h"
#include "ctd_player.h"
#include "ctd_playeranim.h"
#include "ctd_playersound.h"
#include "ctd_playerphysics.h"
#include "ctd_chatgui.h"
#include "../visuals/ctd_camera.h"

using namespace std;

namespace CTD
{

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

    unsigned int eventType  = ea.getEventType();
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

        if ( key == _keyCodeJump && !_p_userObject->_p_playerPhysics->isJumping() )
        {
            _p_userObject->_p_playerPhysics->jump();
            _p_userObject->_p_playerAnimation->animIdle(); // stop any movement animation first
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
    if ( _moveForward )
    {
        _p_userObject->_p_playerPhysics->setForce( _p_userObject->_moveDir._v[ 0 ], _p_userObject->_moveDir._v[ 1 ] );

        if ( !_p_userObject->_p_playerPhysics->isJumping() )
            _p_userObject->_p_playerAnimation->animWalk();
    } 
    
    if ( _moveBackward )
    {
        _p_userObject->_p_playerPhysics->setForce( -_p_userObject->_moveDir._v[ 0 ], -_p_userObject->_moveDir._v[ 1 ] );

        if ( !_p_userObject->_p_playerPhysics->isJumping() )
            _p_userObject->_p_playerAnimation->animWalk();
    }

    if ( _right )
    {
        switch ( _p_userObject->_cameraMode )
        {
            case EnPlayer::Spheric:
            {
                _p_userObject->_rot += _p_userObject->_p_playerPhysics->getAngularForce();
                if ( _p_userObject->_rot > osg::PI * 2.0f )
                    _p_userObject->_rot -= osg::PI * 2.0f;

                _p_userObject->_moveDir._v[ 0 ] = sinf( _p_userObject->_rot );
                _p_userObject->_moveDir._v[ 1 ] = cosf( _p_userObject->_rot );

                _p_userObject->_p_playerAnimation->animTurn();
            }
            break;

            case EnPlayer::Ego:
            {
                osg::Vec3f side;
                side = _p_userObject->_moveDir ^ osg::Vec3f( 0, 0, 1.0f );
                if ( _moveForward || _moveBackward )
                    _p_userObject->_p_playerPhysics->addForce( side._v[ 0 ], side._v[ 1 ] );
                else
                    _p_userObject->_p_playerPhysics->setForce( side._v[ 0 ], side._v[ 1 ] );

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
                _p_userObject->_rot -= _p_userObject->_p_playerPhysics->getAngularForce();
                if ( _p_userObject->_rot < 0 )
                    _p_userObject->_rot += osg::PI * 2.0f;

                _p_userObject->_moveDir._v[ 0 ] = sinf( _p_userObject->_rot );
                _p_userObject->_moveDir._v[ 1 ] = cosf( _p_userObject->_rot );

                _p_userObject->_p_playerAnimation->animTurn();
            }
            break;
            
            case EnPlayer::Ego:
            {
                osg::Vec3f side;
                side = _p_userObject->_moveDir ^ osg::Vec3f( 0, 0, -1.0f );
                if ( _moveForward || _moveBackward )
                    _p_userObject->_p_playerPhysics->addForce( side._v[ 0 ], side._v[ 1 ] );
                else
                    _p_userObject->_p_playerPhysics->setForce( side._v[ 0 ], side._v[ 1 ] );

                _p_userObject->_p_playerAnimation->animTurn();
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
                if ( _p_userObject->_cameraMode == EnPlayer::Spheric )
                {
                    if ( movefb )
                    {
                        s_states = Stopped;
                        _p_userObject->_p_playerPhysics->stopMovement();
                    }
                } 
                else
                {
                    if ( movelr & movefb )
                    {
                        s_states = Stopped;
                        _p_userObject->_p_playerPhysics->stopMovement();
                    }
                }

            }
            break;

            case Stopped:
            {
                if ( !_left && !_right )
                    _p_userObject->_p_playerAnimation->animIdle();

                //_p_userObject->_p_playerPhysics->stopMovement();

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

    // handle mouse pointer movement
    //! TODO: avoid mouse pointer leaving the window area
    if ( eventType == osgGA::GUIEventAdapter::MOVE )
    {
        float mcoordX = ea.getXnormalized();
        float mcoordY = ea.getYnormalized();

        // in ego mode the mouse controls the player rotation
        if ( _p_userObject->_cameraMode == EnPlayer::Ego )
        {
            float& rot = _p_userObject->_rot;

            static float lastX = 0;
            rot = mcoordX * osg::PI * 2.0f; 
            lastX = mcoordX;

            if ( rot > osg::PI * 2.0f )
                rot -= osg::PI * 2.0f;
            else if ( rot < 0 )
                rot += osg::PI * 2.0f;

            _p_userObject->_moveDir._v[ 0 ] = sinf( rot );
            _p_userObject->_moveDir._v[ 1 ] = cosf( rot );
            _p_userObject->_p_playerAnimation->animTurn();

            // adjust pitch / yaw depending on mouse movement
            _p_userObject->setCameraPitchYaw( mcoordY, 0 );
        }
        else
        {
            // adjust pitch / yaw depending on mouse movement
            _p_userObject->setCameraPitchYaw( mcoordY, -mcoordX );
        }
    }

    return false; // let other handlers get all inputs handled here
}

} // namespace CTD
