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
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  01/14/2005 boto       creation of Player and its input handler
 #
 #  05/26/2005 boto       shifted the input handler into this file
 #
 ################################################################*/

#ifndef _CTD_INPUTHANDLER_H_
#define _CTD_INPUTHANDLER_H_

#include <ctd_main.h>
#include "ctd_player.h"
#include "ctd_playeranim.h"
#include "ctd_playersound.h"
#include "ctd_playerphysics.h"
#include "ctd_chatgui.h"
#include "../visuals/ctd_camera.h"

namespace CTD
{

// look's pitch limit in ego view mode
#define LIMIT_PITCH_ANGLE               ( PI / 3.0f )
#define LIMIT_PITCH_OFFSET              ( PI / 3.0f )

//! Input handler class for player, it controls player character and camera
template< class PlayerImplT >
class PlayerIHCharacterCameraCtrl : public GenericInputHandler< PlayerImplT >
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

        // used internally
        // ---------------
        PlayerImplT*                        getPlayerImpl() { return _p_userObject; }

        EnPlayer*                           getPlayerEntity() { return _p_playerEntity; }

        EnPlayer*                           _p_playerEntity;

        EnPlayer::PlayerAttributes          _attributeContainer;

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

        bool                                _invertedMouse;
};

#include "ctd_inputhandler.inl"

} // namespace CTD

#endif // _CTD_INPUTHANDLER_H_
