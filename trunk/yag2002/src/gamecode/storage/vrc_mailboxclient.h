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
 # class for mailbox functionality used by storage client
 #
 #   date of creation:  07/29/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 ################################################################*/

#ifndef _VRC_MAILBOXCLIENT_H_
#define _VRC_MAILBOXCLIENT_H_

#include <vrc_main.h>
#include "vrc_mailboxnetworking.h"

namespace vrc
{

//! Class pre-declarations
class MailboxNetworking;

namespace gameutils
{
    class VRCStateHandler;
}

//! Default mail box folders
#define VRC_DEFAULT_MAILFOLDER_INBOX      "Inbox"
#define VRC_DEFAULT_MAILFOLDER_SENT       "Sent"


//! Mail header
class MailboxHeader
{
    public:
                                                MailboxHeader() :
                                                 _attributes( 0 ),
                                                 _id( 0 )
                                                {
                                                }

        virtual                                 ~MailboxHeader() {}

        //! Sender
        std::string                             _from;

        //! Comma separated recipients
        std::string                             _to;

        //! Comma separated copy recipients
        std::string                             _cc;

        //! Timestamp of receipt or send
        std::string                             _date;

        //! Mail subject
        std::string                             _subject;

        //! Mail attribute
        enum Attributes
        {
            //! New mail, still not read
            eNotRead        = 0x01,
            //! Normal priority mail
            ePriorityNormal = 0x02,
            //! High priority mail
            ePriorityHigh   = 0x04,
            //! All attributes
            eAll            = 0x07
        };

        //! Mail attributes, see Attributes
        unsigned int                            _attributes;

        //! Unique mail ID
        unsigned int                            _id;
};

//! Class for transfering mail content
class MailboxContent
{
    public:
                                                MailboxContent() :
                                                 _status( 0 )
                                                {
                                                }

        virtual                                 ~MailboxContent() {}

        //! Mail header
        MailboxHeader                           _header;

        //! Mail body
        std::string                             _body;

        //! Used only on request for getting mailbox folders.
        std::vector< std::string >              _folders;

        //! List of headers, used for retrieving only the headers, no bodies.
        std::vector< MailboxHeader >            _headers;

        //! Flags indicating mail content transfer status set by server. The bitfields are combined.
        enum TransferStatus
        {
            eRecvFolders     = 0x0100,
            eRecvHeaders     = 0x0200,
            eRecvMail        = 0x0400,
            eRecvReply       = 0x0800,
            eRecvForward     = 0x1000,
            eSendMail        = 0x2000,

            eOk              = 0x0000,
            eError           = 0x0001
        };

        //! Transfer status, combination of TransferStatus flags
        unsigned int                            _status;
};

//! Callback class for mailbox responses.
class MailboxResponseCallback
{
    public:

                                                MailboxResponseCallback() : _userID( 0 ) {}

        virtual                                 ~MailboxResponseCallback() {}

        //! Get the user ID the mailbox belongs to.
        virtual unsigned int                    mailboxGetUserID() const = 0;

        //! Get the mailbox request response. The response string and status are generated by the mail server.
        virtual void                            mailboxResponse( const MailboxContent& mailcontent, unsigned int status, const std::string& response ) = 0;

    protected:

        //! Unique user ID, owner of the mailbox.
        unsigned int                            _userID;
};


//! Mailbox client side implementation
class MailboxClient : public yaf3d::Singleton< vrc::MailboxClient >, public MailboxNetworking::CallbackMailboxNetwrokingResponse
{
    public:

        /*! NOTE: The methods below need a callback object which is used when the response of a request arrives from the server.
                  On every method call the callback is overridden so take care when using the interface!
        */

        //! Get mail folders.  The result comes via the response callback.
        void                                    getMailFolders( MailboxResponseCallback* p_cbResponse );

        //! Get mail headers with given attribute (see Content::Attributes), they are stored in 'headers' without body.  The result comes via the response callback.
        void                                    getMailHeaders( MailboxResponseCallback* p_cbResponse, unsigned int attribute, const std::string& folder );

        //! Get mail with given ID. The result comes via the response callback.
        void                                    getMail( MailboxResponseCallback* p_cbResponse, unsigned int mailID );

        //! Send mail
        void                                    sendMail( MailboxResponseCallback* p_cbResponse, const MailboxContent& mailcontent );

        //! Delete mail with given ID.
        void                                    deleteMail( MailboxResponseCallback* p_cbResponse, unsigned int mailID );

        //! Move mail with given ID to 'destfolder'. The destination folder must exist.
        void                                    moveMail( MailboxResponseCallback* p_cbResponse, unsigned int mailID, const std::string& destfolder );

        //! Create a new mail folder.
        void                                    createMailFolder( MailboxResponseCallback* p_cbResponse, const std::string& folder );

        //! Delete the given mail folder.
        void                                    deleteMailFolder( MailboxResponseCallback* p_cbResponse, const std::string& folder );

    protected:

        //! Create the mailbox client.
                                                MailboxClient();

        virtual                                 ~MailboxClient();

        //! Initialize the mailbox client.
        void                                    initialize();

        //! Shutdown the mail client
        void                                    shutdown();

        //! Set the networking object used by mailbox networking on creation.
        void                                    setNetworking( MailboxNetworking* p_networking );

        //! Callback: Get the mailbox request response from networking, used on client. The response string and status are generated by the mail server.
        void                                    mailboxNetworkingResponse( const MailboxContent& mailcontent, unsigned int status, const std::string& response );

        //! Networking for mailbox
        MailboxNetworking*                      _p_networking;

        //! Temporary user response callback used on requesting the server for actions.
        MailboxResponseCallback*                _p_cbResponse;

    friend class MailboxNetworking;
    friend class gameutils::VRCStateHandler;
    friend class yaf3d::Singleton< vrc::MailboxClient >;
};

} // namespace vrc

#endif  // _VRC_MAILBOXCLIENT_H_
