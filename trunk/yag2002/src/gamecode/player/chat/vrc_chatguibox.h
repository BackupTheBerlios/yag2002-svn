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
 # player's chat box gui
 #
 #   date of creation:  04/19/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _VRC_CHATGUIBOX_H_
#define _VRC_CHATGUIBOX_H_

#include <vrc_main.h>
#include "vrc_chatmgr.h"
#include "vrc_chatguicondiag.h"

namespace vrc
{

//! Sound related defs
#define SND_CLOSE_CHANNEL           "sound/chatgui/closechannel.wav"
#define SND_CONNECT                 "sound/chatgui/connect.wav"
#define SND_TYPING                  "sound/chatgui/typing.wav"
#define SND_VOL_CLOSE_CHANNEL       0.2f
#define SND_VOL_CONNECT             0.2f
#define SND_VOL_TYPING              0.5f
#define SND_NAME_CLOSE_CHANNEL      "cgui_clc"
#define SND_NAME_CONNECT            "cgui_con"
#define SND_NAME_TYPING             "cgui_typ"

//! Tab pane related defs
#define GUI_PANE_SPACING            5.0f
#define GUI_PANE_MSG_OFFSET_RIGHT   150.0f
#define GUI_PANE_MSG_OFFSET_BUTTOM  45.0f


class ChannelTabPane;

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

        //! Open the chat box if it is not opened already.
        void                                        openBox();

        //! Set / Unset focus to edit box
        void                                        setEditBoxFocus( bool en );

        //! Setup a new chat input / output gui for a channel
        //! Pass true for systemIO if the pane is used for server connection info, not for actual channels
        void                                        setupChatIO( const ChatConnectionConfig& config, bool systemIO = false );

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

        //! Create tabpane for a new chat channel, pass true for isSystemIO if the pane is used only for server connection info
        ChannelTabPane*                             getOrCreateChannelPane( const ChatConnectionConfig& cfg, bool isSystemIO );

        //! Remove the given tab pane from tab control
        void                                        removeTabPane( ChannelTabPane* p_pane );

        //! Destroy a tab pane
        void                                        destroyChannelPane( const ChatConnectionConfig& cfg );

        //! Returns the associated tabpane given a channel configuration
        ChannelTabPane*                             getTabPane( const ChatConnectionConfig& cfg );

        //! Get the short message box gui
        CEGUI::StaticText*                          getShortMsgBox();

        //! Typedef for a tab pane queue
        typedef std::queue< ChannelTabPane* >       TabQueue;

        //! A queue for removing tab panes on next update
        TabQueue                                    _queueRemoveTabPane;

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
            Connecting
        }                                           _connectionState;

        ChatManager*                                _p_chatMgr;

        std::string                                 _nickname;

        //! Gui stuff
        CEGUI::Window*                              _p_frame;

        CEGUI::PushButton*                          _p_btnCloseChannel;

        CEGUI::PushButton*                          _p_btnConnectIRC;

        CEGUI::PushButton*                          _p_btnOpen;

        CEGUI::PushButton*                          _p_btnMsgArrived;

        CEGUI::TabControl*                          _p_tabCtrl;

        CEGUI::StaticText*                          _p_shortMsgBox;

        float                                       _shortMsgBoxAcceccCounter;

        //! Type for listing chat channels and their associated tabpanes ( gui )
        typedef std::vector< std::pair< ChatConnectionConfig, ChannelTabPane* > >  TabPanePairList;

        TabPanePairList                             _tabpanes;

        //! Edit box activation stuff
        float                                       _editBoxActivationCounter;

        ChannelTabPane*                             _editBoxActivationTab;

        //! Fading in / out stuff
        float                                       _fadeTimer;

        float                                       _frameAlphaValue;

        ConnectionDialog< ChatGuiBox >*             _p_connectionDialog;

    friend class ConnectionDialog< ChatGuiBox >;
    friend class ChannelTabPane;
};

} // namespace vrc

#endif // _VRC_CHATGUIBOX_H_
