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

#include <ctd_base.h>
#include <ctd_baseentity.h>
#include <ctd_entitymanager.h>
#include <ctd_guimanager.h>

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

    protected:

        //! Callback for sensing cariage return during editing a chat message
        bool                                        onEditboxTextChanged( const CEGUI::EventArgs& arg );

        //! Add new message into message box, author will be places at begin of message
        void                                        addMessage( const CEGUI::String& msg, const CEGUI::String& author );

        EnPlayer*                                   _p_player;

        std::string                                 _playername;

        CEGUI::Window*                              _p_wnd;

        CEGUI::MultiLineEditbox*                    _p_editbox;

        CEGUI::MultiLineEditbox*                    _p_messagebox;
};

} // namespace CTD

#endif // _CTD_CHATGUI_H_
