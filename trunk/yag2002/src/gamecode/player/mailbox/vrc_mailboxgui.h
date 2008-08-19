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
 # player's mailbox gui implementation
 #
 #   date of creation:  31/07/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 ****************************************************************/

#ifndef _VRC_MAILBOXGUI_H_
#define _VRC_MAILBOXGUI_H_

#include <vrc_main.h>
#include <storage/vrc_mailboxclient.h>

namespace vrc
{

class MailboxGuiMain;

//! Class for player's mailbox gui
class MailboxGui
{
    public:

                                                    MailboxGui();

        virtual                                     ~MailboxGui();

    protected:

        //! Setup the GUI
        void                                        setupGui();

        //! Called when main mailbox gui requests for closing.
        void                                        mainOnClose();

        //! Callback for on-open event of mail box button
        bool                                        onClickedOpenMailbox( const CEGUI::EventArgs& arg );

        //! Callback for on-hover event of mail box button
        bool                                        onHoverOpenMailbox( const CEGUI::EventArgs& arg );

        //! Open mailbox GUI button
        CEGUI::PushButton*                          _p_btnOpenMailbox;

        //! Main mailbox gui
        MailboxGuiMain*                             _p_mailboxMain;

    friend class MailboxGuiMain;
};

} // namespace vrc

#endif // _VRC_MAILBOXGUI_H_
