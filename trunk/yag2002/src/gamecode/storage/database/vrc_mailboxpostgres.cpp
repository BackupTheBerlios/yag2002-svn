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
#include <pqxx/transaction.hxx>

//! Mailbox database function names
#define FCN_MAIL_GETFOLDERS     "mail_getfolders"
#define FCN_MAIL_GETHEADERS     "mail_getheaders"


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
    pqxx::result res;
    try
    {
        pqxx::work        transaction( *_p_databaseConnection, "mail_getfolders" );
        std::string       query;
        std::stringstream userid;
        userid << userID;

        // call the function for user login
        query = std::string( "SELECT " FCN_MAIL_GETFOLDERS "(" + userid.str() + ");" );

        res = transaction.exec( query );

        if ( res.size() < 1 )
        {
            log_error << "MailboxPostgreSQL: internal error when getting mail folders for user: " << userID << std::endl;
            return false;
        }

        // get the return value of the login function
        std::string retvalue;
        res[ 0 ][ FCN_MAIL_GETFOLDERS ].to( retvalue );
        if ( !retvalue.length() )
        {
            log_warning << "MailboxPostgreSQL: user has no mailbox folders" << std::endl;
            return false;
        }

        folders.clear();
        yaf3d::explode( retvalue, ";", &folders );

        // commit the transaction
        transaction.commit();
    }
    catch( const std::exception& e )
    {
        log_error << "MailboxPostgreSQL: problem on getting user's mailbox " << userID << ", reason: " << e.what()  << std::endl;
        return false;
    }

    return true;
}

bool MailboxPostgreSQL::getMailHeaders(  unsigned int userID, unsigned int attributes, const std::string& folder, std::vector< MailboxHeader >& headers )
{
    if ( !folder.length() )
    {
        log_warning << "MailboxPostgreSQL: invalid folder for get mail headers, user: " << userID << std::endl;
        return false;
    }

    pqxx::result res;
    try
    {
        pqxx::work        transaction( *_p_databaseConnection, "mail_getheaders" );
        std::string       query;
        std::stringstream userid;
        userid << userID;
        std::stringstream attr;
        attr << attributes;

        // call the function for user login
        query = std::string( "SELECT * FROM " FCN_MAIL_GETHEADERS "(" + userid.str() + "," + attr.str() + ",'" + folder + "');" );

        res = transaction.exec( query );

        // empty folder?
        if ( !res.size() )
        {
            return true;
        }
        // setup the header list
        for ( unsigned int cnt = 0; cnt < res.size(); cnt++ )
        {
            // get the return value of the login function
            std::string retvalue;
            res[ cnt ][ 0].to( retvalue );
            if ( !retvalue.length() )
            {
                log_warning << "MailboxPostgreSQL: invalid mail header detected" << std::endl;
                continue;
            }

            MailboxHeader header;
            std::vector< std::string > elements;
            yaf3d::explode( retvalue, ";", &elements );
            if ( elements.size() < 5 )
            {
                log_error << "MailboxPostgreSQL: invalid mail header data detected: " << retvalue << std::endl;
                continue;
            }
            header._from = elements[ 0 ];
            header._to   = elements[ 1 ];
            header._cc   = elements[ 2 ];
            header._date = elements[ 3 ];
            // note: the subject text can contain semicolons!
            for ( unsigned int h = 4; h < elements.size(); h++ )
                header._subject += elements[ h ];

            headers.push_back( header );
        }

        // commit the transaction
        transaction.commit();
    }
    catch( const std::exception& e )
    {
        log_error << "MailboxPostgreSQL: problem on getting user's mail headers " << userID << ", reason: " << e.what()  << std::endl;
        return false;
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
log_debug << "MailboxPostgreSQL: send mail for user " << userID << std::endl;
log_debug << " TO:"  << mailcontent._header._to << std::endl;
log_debug << " CC:"  << mailcontent._header._cc << std::endl;
log_debug << " Subject:"  << mailcontent._header._subject << std::endl;
log_debug << " Attr:"  << mailcontent._header._attributes << std::endl;
log_debug << " Body:"  << mailcontent._body << std::endl;

    return true;
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
