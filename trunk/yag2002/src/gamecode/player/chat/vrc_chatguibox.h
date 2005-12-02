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
 # player's chat box gui
 #
 #   date of creation:  04/19/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_CHATGUIBOX_H_
#define _VRC_CHATGUIBOX_H_

#include <vrc_main.h>
#include "vrc_chatmgr.h"
#include "vrc_chatguicondiag.h"

namespace vrc
{

//! Class for chat gui control
class ChatGuiBox
{

    public:

                                                    ChatGuiBox();

        virtual                                     ~ChatGuiBox();

        //! Initialize gui
        void                                        initialize( ChatManager* p_chatMgr );

        //! Update method for chat box animation, etc.
        void                                        update( float deltaTime );

        //! Shows / hides the gui completely
        void                                        show( bool visible );

        //! Set / Unset focus to edit box
        void                                        setEditBoxFocus( bool en );

        //! Setup a new chat input / output gui for a channel
        void                                        setupChatIO( const ChatConnectionConfig& config );

        //! Print a text into message box of given channel, this can be used for system messages
        //! Use wildcard "*" for channel in order to address all channels.
        void                                        outputText( const std::string& channel, const std::string& msg );

    protected:

        //! Callback for chat frame window close button
        bool                                        onCloseFrame( const CEGUI::EventArgs& arg );

        //! Callback for button 'open'
        bool                                        onClickedOpen( const CEGUI::EventArgs& arg );

        //! Callback for mouse over button 'open'
        bool                                        onHoverOpen( const CEGUI::EventArgs& arg );

        //! Fade in / fade out the chat box
        void                                        fadeChatbox( bool fadeout );

        //! Shows if the chat box is hidden ( faded out )
        bool                                        isHidden() const { return ( _boxState == BoxHidden ) || ( _boxState == BoxFadeOut ); }

        //! Show / hide the "message arrived" icon
        void                                        showMsgArrived( bool show );

        //! Callback for button "IRC connect"
        bool                                        onClickedConnectIRC( const CEGUI::EventArgs& arg );

        //! Class describing a tab pane for one chat channel
        class ChannelTabPane : public ChatProtocolCallback
        {
            public:

                                                            ChannelTabPane( CEGUI::TabControl* p_tabcontrol, ChatGuiBox* p_guibox );

                virtual                                     ~ChannelTabPane();

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

                //! Overriden method for receiving the chat traffic.
                void                                        onReceive( const std::string& channel, const std::string& sender, const std::string& msg );

                //! Overriden method for getting nickname changes in channel.
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

        //! Create tabpane for a new chat channel
        ChannelTabPane*                             getOrCreateChannelPane( const ChatConnectionConfig& cfg );

        //! Remove the given tab pane from tab control
        void                                        removeTabPane( ChatGuiBox::ChannelTabPane* p_pane )
                                                    {   
                                                        _queueRemoveTabPane.push( p_pane );
                                                    }

        //! Destroy a tab pane
        void                                        destroyChannelPane( const ChatConnectionConfig& cfg );

        //! Returns the associated tabpane given a channel configuration
        ChannelTabPane*                             getTabPane( const ChatConnectionConfig& cfg );

        //! A queue for removing tab panes on next update
        std::queue< ChatGuiBox::ChannelTabPane*, std::deque< ChatGuiBox::ChannelTabPane* > >   _queueRemoveTabPane;

        //! Callback for resizing the chat box frame
        bool                                        onSizeChanged( const CEGUI::EventArgs& arg );

        //! Callback for changing channel tab
        bool                                        onChannelTabChanged( const CEGUI::EventArgs& arg );

        //! Callback for clicking channel close button
        bool                                        onClickedCloseChannelPane( const CEGUI::EventArgs& arg );

        //! Callback for connection dialog's 'connect' button
        void                                        onConnectionDialogClickedConnect( const ChatConnectionConfig& conf );

        //! Callback for connection dialog's 'cancel' button
        void                                        onConnectionDialogClickedCancel();

        //! Box frame states
        enum {
            BoxVisible,
            BoxHidden,
            BoxFadeIn,
            BoxFadeOut
        }                                           _boxState;

        //! Connection states
        enum
        {
            ConnectionIdle,
            Connecting,
        }                                           _connectionState;

        ChatManager*                                _p_chatMgr;

        std::string                                 _nickname;

        //! Gui stuff
        CEGUI::Window*                              _p_frame;

        CEGUI::PushButton*                          _p_btnCloseChannel;

        CEGUI::PushButton*                          _p_btnConnectIRC;

        CEGUI::PushButton*                          _p_btnOpen;

        CEGUI::StaticImage*                         _p_btnMsgArrived;

        CEGUI::TabControl*                          _p_tabCtrl;

        //! Type for listing chat channels and their associated tabpanes ( gui )
        typedef std::vector< std::pair< ChatConnectionConfig, ChannelTabPane* > >  TabPanePairList;

        TabPanePairList                             _tabpanes;

        //! Edit box activation stuff
        float                                       _editBoxActivationCounter;

        ChannelTabPane*                             _editBoxActivationTab;

        //! Fading in / out stuff
        float                                       _fadeTimer;

        osg::Vec2f                                  _boxFrameSize;

        float                                       _frameAlphaValue;

        ConnectionDialog< ChatGuiBox >*             _p_connectionDialog;

    friend class ConnectionDialog< ChatGuiBox >;
};

} // namespace vrc
#endif // _VRC_CHATGUI_H_
