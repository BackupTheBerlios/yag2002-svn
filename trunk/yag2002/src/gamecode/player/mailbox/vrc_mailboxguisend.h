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
 # player's mail send gui implementation
 #
 #   date of creation:  01/08/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 ****************************************************************/

#ifndef _VRC_MAILBOXGUISEND_H_
#define _VRC_MAILBOXGUISEND_H_

#include <vrc_main.h>
#include <storage/vrc_mailboxclient.h>

namespace vrc
{

class MailboxGui;
class MailboxGuiMain;
class MailboxGuiContacts;

//! Class for player's mail send gui
class MailboxGuiSend : public MailboxResponseCallback
{
    public:

        //! Construct the main gui given the gui file name.
                                                    MailboxGuiSend( MailboxGuiMain* p_mailbox, const std::string& filename );

        virtual                                     ~MailboxGuiSend();

        //! Get the frame object
        CEGUI::FrameWindow*                         getFrame();

        //! Setup the send gui for sending a mail.
        void                                        viewSendMail();

        //! Setup the send gui for a mail reply.
        void                                        replyMail( const MailboxContent& content );

        //! Setup the send gui for a mail forward.
        void                                        forwardMail( const MailboxContent& content );

    protected:

        //! Setup the GUI
        void                                        setupGui( const std::string& filename );

        //! Callback: Get the user ID the mailbox belongs to.
        unsigned int                                mailboxGetUserID() const;

        //! Callback: Get the mailbox request response. The response string and status are generated by the mail server.
        void                                        mailboxResponse( const MailboxContent& mailcontent, unsigned int status, const std::string& response );

        //! Used for reply and forward mails
        void                                        assembleMailbody( const MailboxContent& content, std::string& body, bool reply );

        //! Callback for contacts button
        bool                                        onClickedContacts( const CEGUI::EventArgs& arg );

        //! Callback for cancel button
        bool                                        onClickedCancelSend( const CEGUI::EventArgs& arg );

        //! Callback for send button
        bool                                        onClickedSend( const CEGUI::EventArgs& arg );

        //! Called when contacts gui is closed by pressing Ok or Cancel button
        void                                        onCloseContacts( bool ok );

        //! Main gui object
        MailboxGuiMain*                             _p_mailboxGuiMain;

        //! Contacts gui
        MailboxGuiContacts*                         _p_mailboxGuiContacts;

        //! Frame object
        CEGUI::FrameWindow*                         _p_frame;

        //! Gui elements
        CEGUI::Editbox*                             _p_editboxTo;
        CEGUI::Editbox*                             _p_editboxCC;
        CEGUI::Editbox*                             _p_editboxSubject;
        CEGUI::MultiLineEditbox*                    _p_editboxBody;

    friend class MailboxGuiContacts;
};

} // namespace vrc

#endif // _VRC_MAILBOXGUISEND_H_
