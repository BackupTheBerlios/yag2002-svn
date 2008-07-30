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

#include <vrc_main.h>
#include "vrc_mailboxnetworking.h"
#include "vrc_mailboxclient.h"
#include <RNReplicaNet/Inc/DataBlock_Function.h>

using namespace RNReplicaNet;

namespace vrc
{

MailboxNetworking::MailboxNetworking() :
 _p_mailboxResponseCallback( NULL )
{
}

MailboxNetworking::~MailboxNetworking()
{
    // unset the networking object in mailbox client
    MailboxClient::get()->setNetworking( NULL );
}

void MailboxNetworking::PostObjectCreate()
{
    // set the networking object in mailbox client
    MailboxClient::get()->setNetworking( this );
}

void MailboxNetworking::setMailboxResponseCallback( CallbackMailboxResponse* p_cb )
{
    _p_mailboxResponseCallback = p_cb;
}

void MailboxNetworking::getMailHeaders( unsigned int userID, unsigned int attribute )
{
    //! TODO
}

void MailboxNetworking::getMail( unsigned int userID, unsigned int mailID )
{
    //! TODO
}

void MailboxNetworking::sendMail( unsigned int userID, const BaseMailbox::Content& mailcontent )
{
    //! TODO
}

void MailboxNetworking::deleteMail( unsigned int userID, unsigned int mailID )
{
    //! TODO
}

void MailboxNetworking::moveMail( unsigned int userID, unsigned int mailID, const std::string& destfolder )
{
    //! TODO
}

void MailboxNetworking::createMailFolder( unsigned int userID, const std::string& folder )
{
    //! TODO
}

void MailboxNetworking::deleteMailFolder( unsigned int userID, const std::string& folder )
{
    //! TODO
}


void MailboxNetworking::RPC_RequestMailCommand( tMailRequest request )
{
    //! TODO


#if 0
    extern RPC_RequestMailHeaders( unsigned int /*userID*/, unsigned int /*mail_attribute_such_as_high_priority*/ );
    extern RPC_RequestMail( unsigned int /*userID*/, unsigned int /*mailID*/ );
    extern RPC_RequestSendMail( unsigned int /*userID*/, tMailContent );
    extern RPC_RequestDeleteMail( unsigned int /*userID*/, unsigned int /*mailID*/ );
    extern RPC_RequestMoveMail( unsigned int /*userID*/, unsigned int /*mailID*/, char destfolder[ 32 ] /*destination_folder*/ );
    extern RPC_RequestCreateFolder( unsigned int /*userID*/, char folder[ 32 ] /*folder*/ );
    extern RPC_RequestDeleteFolder( unsigned int /*userID*/, char folder[ 32 ] /*folder*/ );
#endif

}

void MailboxNetworking::RPC_RequestSendMail( unsigned int userID, tMailContent content )
{
    //! TODO
}

void MailboxNetworking::RPC_Response( tMailRequest )
{
    //! TODO
}

void MailboxNetworking::RPC_SendMail( tMailContent content )
{
    //! TODO
}

} // namespace vrc
