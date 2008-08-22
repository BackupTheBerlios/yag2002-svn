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
 # base class for mailbox storage
 #
 #   date of creation:  08/05/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 ################################################################*/

#ifndef _VRC_BASEMAILBOXSTORAGE_H_
#define _VRC_BASEMAILBOXSTORAGE_H_

#include <vrc_main.h>
#include "../vrc_mailboxclient.h"
#include <pqxx/connection.hxx>

namespace vrc
{

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
            eCreateFolder    = 0x0200,
            eDeleteFolder    = 0x0400,
            eRecvHeaders     = 0x0800,
            eRecvMail        = 0x1000,
            eDeleteMail      = 0x2000,
            eMoveMail        = 0x4000,
            eSendMail        = 0x8000,

            eOk              = 0x0001,
            eError           = 0x0002
        };

        //! Transfer status, combination of TransferStatus flags
        unsigned int                            _status;
};


//! Base class for database access for the mailbox functionality
class BaseMailboxStorage
{
    public:

                                                BaseMailboxStorage();

        virtual                                 ~BaseMailboxStorage();

        //! Get mail folders.
        virtual bool                            getMailFolders( unsigned int userID, std::vector< std::string >& folders ) = 0;

        //! Get mail headers with given attribute (see Content::Attributes), they are stored in 'headers' without body.
        virtual bool                            getMailHeaders( unsigned int userID, unsigned int attributes, const std::string& folder, std::vector< MailboxHeader >& headers ) = 0;

        //! Get mail with given ID.
        virtual bool                            getMail(  unsigned int userID, unsigned int mailID, MailboxContent& mailcontent ) = 0;

        //! Send mail
        virtual bool                            sendMail(  unsigned int userID, const MailboxContent& mailcontent ) = 0;

        //! Delete mail with given ID.
        virtual bool                            deleteMail(  unsigned int userID, unsigned int mailID ) = 0;

        //! Move mail with given ID to 'destfolder'. The destination folder must exist.
        virtual bool                            moveMail(  unsigned int userID, unsigned int mailID, const std::string& destfolder ) = 0;

        //! Create a new mail folder.
        virtual bool                            createMailFolder( unsigned int userID, const std::string& folder ) = 0;

        //! Delete the given mail folder.
        virtual bool                            deleteMailFolder( unsigned int userID, const std::string& folder ) = 0;
};

} // namespace vrc

#endif  // _VRC_BASEMAILBOXSTORAGE_H_
