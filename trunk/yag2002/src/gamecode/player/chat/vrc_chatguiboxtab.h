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
 #  tabs for chat box gui
 #
 #   date of creation:  01/22/2006
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_CHATGUIBOXTAB_H_
#define _VRC_CHATGUIBOXTAB_H_

#include <vrc_main.h>
#include "vrc_chatmgr.h"
#include "vrc_chatguicondiag.h"

namespace vrc
{

class ChatGuiBox;

//! Class describing a tab pane for one chat channel
class ChannelTabPane : public ChatProtocolCallback
{
    public:

                                                    ChannelTabPane( CEGUI::TabControl* p_tabcontrol, ChatGuiBox* p_guibox, bool isSystemIO = false );

        virtual                                     ~ChannelTabPane();

        //! Return true if this pane is used for system IO
        bool                                        isSystemIO() { return _isSystemIO; }

        //! Set pane title
        void                                        setTitle( const std::string& title );

        //! Add new message into message box, author will be places at begin of message.
        //! It is usually used by protocol handlers or for printing system messages.
        void                                        addMessage( const CEGUI::String& msg, const CEGUI::String& author );

        //! Set the connection configuration
        void                                        setConfiguration( const ChatConnectionConfig& conf );

        //! Return the configuration
        const ChatConnectionConfig&                 getConfiguration();

        //! Set pane selection
        void                                        setSelection();

        //! Get the selection state
        bool                                        isSelected();

        //! Set / Unset focus to edit box
        void                                        setEditBoxFocus( bool en );

        //! Overridden method for receiving the chat traffic.
        void                                        onReceive( const std::string& channel, const std::string& sender, const std::string& msg );

        //! Overridden method for getting nickname changes in channel.
        //! If oldname is empty then newname is the initial one.
        void                                        onNicknameChanged( const std::string& newname, const std::string& oldname );

        //! Overridden method for getting notified when someone joined to channel.
        void                                        onJoinedChannel( const ChatConnectionConfig& cfg );

        //! Overridden method for getting notified when someone left the channel.
        void                                        onLeftChannel( const ChatConnectionConfig& cfg );

        //! Overridden method for getting notified when someone has been kicked from a channel.
        void                                        onKicked( const std::string& channel, const std::string& kicker, const std::string& kicked );

        //! Overridden method for getting notified when member list changes.
        void                                        onReceiveMemberList( const std::string& channel );

    protected:

        //! Callback for sensing cariage return during editing a chat message
        bool                                        onEditboxTextChanged( const CEGUI::EventArgs& arg );

        //! Callback for changing nickname selection in list
        bool                                        onListItemSelChanged( const CEGUI::EventArgs& arg );

        //! Callback for resizing the pane
        bool                                        onSizeChanged( const CEGUI::EventArgs& arg );

        //! Callback for selecting the pane by clicking
        bool                                        onSelected( const CEGUI::EventArgs& arg );

        //! Update the member
        void                                        updateMemberList( std::vector< std::string >& list );

        bool                                        _isSystemIO;

        CEGUI::TabControl*                          _p_tabCtrl;

        ChatGuiBox*                                 _p_guibox;

        CEGUI::Window*                              _p_tabPane;

        std::string                                 _title;

        CEGUI::MultiLineEditbox*                    _p_messagebox;

        CEGUI::Editbox*                             _p_editbox;

        CEGUI::Listbox*                             _p_listbox;

        ChatConnectionConfig                        _configuration;

        std::vector< std::string >                  _nickNames;
};

} // namespace vrc

#endif // _VRC_CHATGUIBOXTAB_H_
