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
 # networking for mail transfer
 #
 # this class implements the networking functionality for the mailbox
 #
 #
 #   date of creation:  29/07/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#ifndef _VRC_MAILBOXNETWORKING_H_
#define _VRC_MAILBOXNETWORKING_H_

#include <vrc_main.h>
#include "networkingRoles/_RO_MailboxNetworking.h"
#include <RNReplicaNet/Inc/DataBlock_Function.h>

namespace vrc
{

class MailboxContent;
class BaseMailboxStorage;


//! Networking class for delivering mails.
class MailboxNetworking : _RO_DO_PUBLIC_RO( MailboxNetworking )
{
    public:

        //! Callback class for mailbox responses used on client.
        class CallbackMailboxNetwrokingResponse
        {
            public:

                                                    CallbackMailboxNetwrokingResponse() {}

                virtual                             ~CallbackMailboxNetwrokingResponse() {}

                //! Get the mailbox request response. The response string and status are generated by the mail server.
                virtual void                        mailboxNetworkingResponse( const MailboxContent& mailcontent, unsigned int status, const std::string& response ) = 0;
        };

    public:

        explicit                                   MailboxNetworking( BaseMailboxStorage* p_storage = NULL );

        virtual                                    ~MailboxNetworking();

        //! Object has been created.
        void                                        PostObjectCreate();

        //! Set the callback for delivering the mailbox request results.
        void                                        setMailboxResponseCallback( CallbackMailboxNetwrokingResponse* p_cb );

        //! The response of the functions below is delivered by the callback above, when arrived from server.
        //-------------------------------------------------------------------------------------------------//

        //! Get all mailbox folders.
        void                                        getMailFolders();

        //! Get mail headers with given attribute (see Content::Attributes).
        void                                        getMailHeaders( unsigned int attribute, const std::string& folder );

        //! Get mail with given ID.
        void                                        getMail( unsigned int mailID );

        //! Send mail
        void                                        sendMail( const MailboxContent& mailcontent );

        //! Delete mail with given ID.
        void                                        deleteMail( unsigned int mailID );

        //! Move mail with given ID to 'destfolder'. The destination folder must exist.
        void                                        moveMail( unsigned int mailID, const std::string& destfolder );

        //! Create a new mail folder.
        void                                        createMailFolder( const std::string& folder );

        //! Delete the given mail folder.
        void                                        deleteMailFolder( const std::string& folder );

    protected:

        // Internal RN Overrides, do not use these methods!
        //-----------------------------------------------//

        //! Called by client for requesting a mail command such as send, get, move, delete, get mail headers, etc.
        void                                        RPC_RequestMailCommand( tMailData data );

        //! Called by server for responding to a request.
        void                                        RPC_Response( tMailData data );

        //! Mailbox commands used for transfer
        enum MailboxCmds
        {
            eMailboxCmdGetFolders   = 0x0001,
            eMailboxCmdGetHeaders   = 0x0002,
            eMailboxCmdGetMail      = 0x0004,
            eMailboxCmdMoveMail     = 0x0008,
            eMailboxCmdDeleMail     = 0x0010,
            eMailboxCmdCreateFolder = 0x0020,
            eMailboxCmdDeleFolder   = 0x0040,
            eMailboxCmdSendMail     = 0x0080
        };

        // !Used for serializing mail data into data buffer. 'p_buffer' contains the packed data and the method returns the length of data.
        unsigned int                                packData( const std::vector< std::string >& elements, const std::string& separator, char* p_buffer );

        // !Used for deserializing mail data into data buffer. After the call 'elements' contains the uppacked data. It returns the length of 'elements'.
        unsigned int                                unpackData( std::vector< std::string >& elements, const std::string& separator, char* p_buffer, unsigned int buffersize );

        //! Mailbox results callback
        CallbackMailboxNetwrokingResponse*          _p_mailboxResponseCallback;

        //! Mailbox storage used on server, only on server!
        BaseMailboxStorage*                         _p_mailboxStorage;

    friend class _MAKE_RO( MailboxNetworking );
};

} // namespace vrc

#endif //_VRC_MAILBOXNETWORKING_H_
