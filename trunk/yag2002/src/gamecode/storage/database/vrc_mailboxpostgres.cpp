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
 # class for mailbox functionality on server
 #
 #   date of creation:  07/29/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_mailboxpostgres.h"


namespace vrc
{

MailboxPostgreSQL::MailboxPostgreSQL( pqxx::connection* p_database ) :
 _p_databaseConnection( p_database )
{
    assert( _p_databaseConnection && "invalid database object!" );
}

MailboxPostgreSQL::~MailboxPostgreSQL()
{
}

bool MailboxPostgreSQL::getMailFolders( unsigned int userID, std::vector< std::string >& folders )
{
//! TODO: remove the test code
folders.push_back( "Inbox" );
folders.push_back( "Sent" );

    return true;
}

bool MailboxPostgreSQL::getMailHeaders(  unsigned int userID, unsigned int attribute, const std::string& folder, std::vector< MailboxHeader >& headers )
{
    //! TODO

//!TODO just for testing, remove this later!
    if ( folder == "Inbox" )
    {
MailboxHeader  header;
header._id = 10;
header._from = "Inbox-user1";
header._date = "2008-08-03";
header._subject = "blaa";
headers.push_back( header );

header._id = 20;
header._from = "Inbox-user2";
header._date = "2008-08-01";
header._subject = "blaa2";
headers.push_back( header );

header._id = 30;
header._from = "Inbox-user2";
header._date = "2008-08-05";
header._subject = "blaa2";
headers.push_back( header );
    }
    else
    {
MailboxHeader  header;
header._id = 10;
header._from = "Sent-user1";
header._date = "2008-08-03";
header._subject = "blaa";
headers.push_back( header );

header._id = 20;
header._from = "Sent-user2";
header._date = "2008-08-01";
header._subject = "blaa2";
headers.push_back( header );

header._id = 30;
header._from = "Sent-user2";
header._date = "2008-08-05";
header._subject = "blaa2";
headers.push_back( header );
    }

    return true;
}

bool MailboxPostgreSQL::getMail(  unsigned int userID, unsigned int mailID, MailboxContent& mailcontent )
{
    //! TODO

log_debug << "MailboxPostgreSQL: deliver mail for " << userID << ", " << mailID << std::endl;
mailcontent._header._id = mailID;
mailcontent._header._from = "thunder";
mailcontent._header._to = "me";
mailcontent._header._cc = "nokky,kami";
mailcontent._header._date = "2008-08-05";
mailcontent._header._subject = "this is a very very very long header, i know";
mailcontent._body = "this is a dummy body ÄÖÜßüäö \nthis is the next line\n third line";

    return true;
}

bool MailboxPostgreSQL::sendMail(  unsigned int userID, const MailboxContent& mailcontent )
{
    //! TODO
    return false;
}

bool MailboxPostgreSQL::deleteMail(  unsigned int userID, unsigned int mailID )
{
    //! TODO
    return false;
}

bool MailboxPostgreSQL::moveMail(  unsigned int userID, unsigned int mailID, const std::string& destfolder )
{
    //! TODO
    return false;
}

bool MailboxPostgreSQL::createMailFolder(  unsigned int userID, const std::string& folder )
{
    //! TODO
    return false;
}

bool MailboxPostgreSQL::deleteMailFolder(  unsigned int userID, const std::string& folder )
{
    //! TODO
    return false;
}

} // namespace vrc
