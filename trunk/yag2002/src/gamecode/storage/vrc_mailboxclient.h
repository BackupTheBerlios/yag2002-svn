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

#ifndef _VRC_MAILBOXCLIENTP_H_
#define _VRC_MAILBOXCLIENTP_H_

#include <vrc_main.h>
#include "database/vrc_basemailbox.h"

namespace vrc
{

//! Class pre-declarations
class MailboxNetworking;

namespace gameutils
{
    class VRCStateHandler;
}


class MailboxClient : public yaf3d::Singleton< vrc::MailboxClient >, BaseMailbox
{
    public:

        //! Get mail headers with given attribute (see Content::Attributes), they are stored in 'headers' without body.
        virtual bool                            getMailHeaders( unsigned int userID, unsigned int attribute, const std::string& folder, std::vector< Content >& headers );

        //! Get mail with given ID.
        virtual bool                            getMail( unsigned int userID, unsigned int mailID, Content& mailcontent );

        //! Send mail
        virtual bool                            sendMail( unsigned int userID, const Content& mailcontent );

        //! Delete mail with given ID.
        virtual bool                            deleteMail( unsigned int userID, unsigned int mailID );

        //! Move mail with given ID to 'destfolder'. The destination folder must exist.
        virtual bool                            moveMail( unsigned int userID, unsigned int mailID, const std::string& destfolder );

        //! Create a new mail folder.
        virtual bool                            createMailFolder( unsigned int userID, const std::string& folder );

        //! Delete the given mail folder.
        virtual bool                            deleteMailFolder( unsigned int userID, const std::string& folder );

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

        //! Networking for mailbox
        MailboxNetworking*                      _p_networking;

    friend class MailboxNetworking;
    friend class gameutils::VRCStateHandler;
    friend class yaf3d::Singleton< vrc::MailboxClient >;
};

} // namespace vrc

#endif  // _VRC_MAILBOXCLIENT_H_
