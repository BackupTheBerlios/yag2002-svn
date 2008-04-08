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
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  01/14/2005 boto       creation of Player and its input handler
 #
 #  05/26/2005 boto       shifted the input handler into this file
 #
 ################################################################*/

#ifndef _VRC_INPUTHANDLER_H_
#define _VRC_INPUTHANDLER_H_

#include <vrc_main.h>
#include "vrc_gameutils.h"
#include "vrc_player.h"
#include "vrc_playerimpl.h"
#include "vrc_playeranim.h"
#include "vrc_playersound.h"
#include "vrc_playerphysics.h"
#include "../visuals/vrc_camera.h"
#include "../tools/vrc_fpsdisplay.h"

namespace vrc
{

// Pitch limit
#define LIMIT_PITCH_ANGLE               ( osg::PI / 3.0f )
// Mouse movement limit ( in pixels )
#define LIMIT_MMOVE_DELTA               100.0f
// Spheric mode related parameters ( mouse wheel functions )
#define LIMIT_SPHERIC_MAX_DIST          100.0f
#define LIMIT_SPHERIC_MIN_DIST          2.0f
#define SPHERIC_DIST_INCREMENT          0.5f

//! Input handler class for player, it controls player character and camera
template< class PlayerImplT >
class PlayerIHCharacterCameraCtrl : public vrc::gameutils::GenericInputHandler< PlayerImplT >
{
    public:

                                            PlayerIHCharacterCameraCtrl( PlayerImplT* p_playerimpl, EnPlayer* p_playerentity );
                                            
        virtual                             ~PlayerIHCharacterCameraCtrl();

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

        //! Internal class for handling with mouse position data
        class MouseData
        {
            public:
                                                MouseData( float pitch = 0.0f, float yaw = 0.0f )
                                                {
                                                    reset( pitch, yaw );
                                                }

                virtual                         ~MouseData() {}

                //! Reset the mouse data state
                void                            reset( float pitch = 0.0f, float yaw = 0.0f )
                                                {
                                                    // get the current screen size
                                                    unsigned int width, height;
                                                    yaf3d::Application::get()->getScreenSize( width, height );
                                                    _screenSizeX = float( width );
                                                    _screenSizeY = float( height );

                                                    // calculate the middle of app window
                                                    _screenMiddleX = static_cast< Uint16 >( _screenSizeX * 0.5f );
                                                    _screenMiddleY = static_cast< Uint16 >( _screenSizeY * 0.5f );
                                                    // init pitch and yaw
                                                    _yaw         = yaw;
                                                    _pitch       = pitch;
                                                    _pyQueue.push( std::make_pair( _pitch, _yaw ) );
                                                }

                //! Stores the current pitch / yaw values, used for camera mode switching
                void                            pushbackPitchYaw()
                                                {
                                                    _pyQueue.push( std::make_pair( _pitch, _yaw ) );
                                                }

                //! Restores pitch / yaw values, used for camera mode switching
                void                            popfrontPitchYaw()
                                                {
                                                    _pitch = _pyQueue.front().first;
                                                    _yaw   = _pyQueue.front().second;
                                                    _pyQueue.pop();
                                                }

                //! Accumulated mouse coords ( pitch / yaw )
                float                           _yaw;
                float                           _pitch;
                //! Queue for storing / restoring < pitch, yaw > values on switching
                std::queue< std::pair< float, float > > _pyQueue;

                float                           _screenSizeX;
                float                           _screenSizeY;

                Uint16                          _screenMiddleX;
                Uint16                          _screenMiddleY;
        };

        // used internally
        // ---------------
        PlayerImplT*                        getPlayerImpl() { return vrc::gameutils::GenericInputHandler< PlayerImplT >::_p_userObject; }

        EnPlayer*                           getPlayerEntity() { return _p_playerEntity; }

        void                                updatePlayerPitchYaw( float& pitch, float& yaw );

        MouseData*                          _p_mouseData;

        EnPlayer*                           _p_playerEntity;

        EnPlayer::PlayerAttributes&         _attributeContainer;

        // some internal variables
        bool                                _enabled;
        bool                                _menuEnabled;
        bool                                _right;
        bool                                _left;
        bool                                _moveForward;
        bool                                _moveBackward;
        bool                                _camSwitch;
        bool                                _chatSwitch;
        bool                                _toggleChatMode;
        bool                                _showFPS;

    public:

        // key binding codes
        unsigned int                        _keyCodeMoveForward;
        unsigned int                        _keyCodeMoveBackward;
        unsigned int                        _keyCodeMoveLeft;
        unsigned int                        _keyCodeMoveRight;
        unsigned int                        _keyCodeJump;
        unsigned int                        _keyCodeCameraMode;
        unsigned int                        _keyCodeChatMode;

        bool                                _invertedMouse;
        float                               _mouseSensitivity;
};

#include "vrc_inputhandler.inl"

} // namespace vrc

#endif // _VRC_INPUTHANDLER_H_
