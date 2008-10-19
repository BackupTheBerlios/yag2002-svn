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
 # player's contact gui implementation
 #
 #   date of creation:  18/10/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 ****************************************************************/

#ifndef _VRC_MAILBOXGUICONTACTS_H_
#define _VRC_MAILBOXGUICONTACTS_H_

#include <vrc_main.h>
#include <storage/vrc_storageclient.h>
#include <storage/vrc_mailboxclient.h>

namespace vrc
{

class MailboxGuiSend;

//! Class for player's contacts gui
class MailboxGuiContacts : public StorageClient::AccountInfoResult
{
    public:

        //! Construct the main gui given the gui file name.
                                                    MailboxGuiContacts( MailboxGuiSend* p_mailboxsend, const std::string& filename );

        virtual                                     ~MailboxGuiContacts();

        //! Get the frame object
        CEGUI::FrameWindow*                         getFrame();

        //! View contacts.
        void                                        viewContacts();

        //! Set TO and CC fields for short view mode.
        void                                        setRecipients( const std::string& to, const std::string& cc );

        //! Get TO and CC fields.
        void                                        getRecipients( std::string& to, std::string& cc );

    protected:

        //! Setup the GUI
        void                                        setupGui( const std::string& filename );

        //! Callback for storage client requests
        void                                        contactsResult( bool success, const std::vector< std::string >& contacts );

        //! Callback for storage client requests
        void                                        accountInfoResult( tAccountInfoData& info );

        //! Callback for details button
        bool                                        onClickedDetails( const CEGUI::EventArgs& arg );

        //! Callback for add TO contact button
        bool                                        onClickedAddTO( const CEGUI::EventArgs& arg );

        //! Callback for add CC contact button
        bool                                        onClickedAddCC( const CEGUI::EventArgs& arg );

        //! Callback for ok button
        bool                                        onClickedOk( const CEGUI::EventArgs& arg );

        //! Callback for cancel button
        bool                                        onClickedCancel( const CEGUI::EventArgs& arg );

        //! Dialog callback for changed contact selection
        bool                                        onListContactSelChanged( const CEGUI::EventArgs& arg );

        //! Send mail gui object
        MailboxGuiSend*                             _p_mailboxGuiSend;

        //! Frame object
        CEGUI::FrameWindow*                         _p_frame;

        //! Show contact details
        bool                                        _showDetails;

        //! Gui elements
        CEGUI::Listbox*                             _p_listContacts;
        CEGUI::PushButton*                          _p_btnDetails;
        CEGUI::PushButton*                          _p_btnAddTO;
        CEGUI::PushButton*                          _p_btnAddCC;
        CEGUI::Editbox*                             _p_editboxTo;
        CEGUI::Editbox*                             _p_editboxCC;

        //! Helper class for holding details gui stuff together
        class ContactDetails
        {
            public:
                                                     ContactDetails( CEGUI::StaticText* p_wnd ) :
                                                     _p_wnd( p_wnd ),
                                                     _p_editMemberSince( NULL ),
                                                     _p_editStatus( NULL ),
                                                     _p_cbOnline( NULL ),
                                                     _p_editABout( NULL )
                                                     {
                                                     }

                void                                hide()
                                                    {
                                                        if ( _p_wnd )
                                                            _p_wnd->hide();
                                                    }

                void                                show()
                                                    {
                                                        if ( _p_wnd )
                                                            _p_wnd->show();
                                                    }

                CEGUI::StaticText*                   _p_wnd;
                CEGUI::Editbox*                      _p_editMemberSince;
                CEGUI::Editbox*                      _p_editStatus;
                CEGUI::Checkbox*                     _p_cbOnline;
                CEGUI::MultiLineEditbox*             _p_editABout;

        };

        ContactDetails*                             _p_detailsGui;

};

} // namespace vrc

#endif // _VRC_MAILBOXGUICONTACTS_H_
