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
 # player's chat gui control
 #
 #   date of creation:  04/19/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_CHATGUI_H_
#define _CTD_CHATGUI_H_

#include <ctd_main.h>

namespace CTD
{

class EnPlayer;

//! Class for player's chat gui control
class PlayerChatGui
{

    public:

                                                    PlayerChatGui();

        virtual                                     ~PlayerChatGui();

        //! Initialize gui layout
        void                                        initialize( EnPlayer* p_player, const std::string& layoutFile );

        //! Update method for chat box animation, etc.
        void                                        update( float deltaTime );

        //! Shows / hides the gui completely
        void                                        show( bool visible );

    protected:

        //! Callback for chat frame window close button
        bool                                        onCloseFrame( const CEGUI::EventArgs& arg );

        //! Callback for sensing cariage return during editing a chat message
        bool                                        onEditboxTextChanged( const CEGUI::EventArgs& arg );

        //! Callback for button 'hide'
        bool                                        onClickedHide( const CEGUI::EventArgs& arg );

        //! Callback for button 'mode'
        bool                                        onClickedMode( const CEGUI::EventArgs& arg );

        //! Add new message into message box, author will be places at begin of message
        void                                        addMessage( const CEGUI::String& msg, const CEGUI::String& author );

        enum {
            Idle,
            BoxFadeIn,
            BoxFadeOut
        }                                           _state;

        EnPlayer*                                   _p_player;

        std::string                                 _playername;

        CEGUI::Window*                              _p_wnd;

        CEGUI::Window*                              _p_frame;

        CEGUI::PushButton*                          _p_btnHide;

        CEGUI::PushButton*                          _p_btnMode;

        CEGUI::Editbox*                             _p_editbox;

        CEGUI::MultiLineEditbox*                    _p_messagebox;

        bool                                        _hidden;

        bool                                        _modeEdit;

        float                                       _fadeTimer;

        osg::Vec2f                                  _boxFrameSize;

        float                                       _frameAlphaValue;
};

} // namespace CTD

#endif // _CTD_CHATGUI_H_
