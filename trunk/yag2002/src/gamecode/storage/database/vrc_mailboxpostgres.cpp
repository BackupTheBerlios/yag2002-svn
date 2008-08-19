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
#define FCN_MAIL_GETHEADER      "mail_getheader"
#define FCN_MAIL_GETBODY        "mail_getbody"
#define FCN_MAIL_SEND           "mail_send"

//! Mail table fields
#define F_MAIL_FROM             "sender"
#define F_MAIL_TO               "recipient"
#define F_MAIL_CC               "cc"
#define F_MAIL_ATTR             "attributes"
#define F_MAIL_DATE             "creation_date"
#define F_MAIL_SUBJECT          "subject"
#define F_MAIL_FOLDER           "folder"
#define F_MAIL_BODY_ID          "fk_id_mailbody"

//! Mail body table fields
#define F_MAILBODY_BODY         "body"


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

std::string MailboxPostgreSQL::cleanString( const std::string& str )
{
    std::string::size_type len = str.length();
    std::string            cleanstr;
    // replace special characters in string
    for ( std::string::size_type cnt = 0; cnt < len; cnt++ )
    {
        if ( str[ cnt ] == '\'' )
            cleanstr += "''";
        else
            cleanstr += str[ cnt ];
    }

    return cleanstr;
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

    try
    {
        pqxx::result res;
        pqxx::work        transaction( *_p_databaseConnection, "mail_getheaders" );
        std::string       query;
        std::stringstream userid;
        userid << userID;
        std::stringstream attr;
        attr << attributes;

        // call the function
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
            // get the fields
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
            if ( elements.size() < 7 )
            {
                log_error << "MailboxPostgreSQL: invalid mail header data detected: " << retvalue << std::endl;
                continue;
            }

            std::stringstream attr, id;
            id << elements[ 0 ];
            id >> header._id;
            attr << elements[ 1 ];
            attr >> header._attributes;
            header._from = elements[ 2 ];
            header._to   = elements[ 3 ];
            header._cc   = elements[ 4 ];
            header._date = elements[ 5 ];

            // note: the subject text can contain semicolons!
            for ( unsigned int h = 6; h < elements.size(); h++ )
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
    try
    {
        pqxx::result resheader, resbody;
        pqxx::work        transaction( *_p_databaseConnection, "mail_get" );
        std::string       query;
        std::stringstream userid, mailid;

        userid << userID;
        mailid << mailID;

        // get mail header
        query = std::string( "SELECT * FROM " FCN_MAIL_GETHEADER "(" + userid.str() + "," + mailid.str() + ");" );

        resheader = transaction.exec( query );

        // can find the header?
        if ( !resheader.size() )
        {
            log_error << "MailboxPostgreSQL: problem getting mail header: " << userID << ", mail ID " << mailID << std::endl;
            return false;
        }

        resheader[ 0 ][ F_MAIL_FROM ].to ( mailcontent._header._from );
        resheader[ 0 ][ F_MAIL_TO ].to ( mailcontent._header._to );
        resheader[ 0 ][ F_MAIL_CC ].to ( mailcontent._header._cc );
        resheader[ 0 ][ F_MAIL_ATTR ].to ( mailcontent._header._attributes );
        resheader[ 0 ][ F_MAIL_DATE ].to ( mailcontent._header._date );
        resheader[ 0 ][ F_MAIL_SUBJECT ].to ( mailcontent._header._subject );
        resheader[ 0 ][ F_MAIL_SUBJECT ].to ( mailcontent._header._subject );

        std::string mailbodyid;
        resheader[ 0 ][ F_MAIL_BODY_ID ].to ( mailbodyid );

        // get mail body
        query = std::string( "SELECT * FROM " FCN_MAIL_GETBODY "(" + userid.str() + "," + mailbodyid + ");" );

        resbody = transaction.exec( query );

        // can find the header?
        if ( !resbody.size() )
        {
            log_error << "MailboxPostgreSQL: problem getting mail body: " << userID << ", mailbody ID " << mailbodyid << std::endl;
            return false;
        }

        resbody[ 0 ][ F_MAILBODY_BODY ].to ( mailcontent._body );

        // commit the transaction
        transaction.commit();
    }
    catch( const std::exception& e )
    {
        log_error << "MailboxPostgreSQL: problem on getting mail: " << userID << ", reason: " << e.what()  << std::endl;
        return false;
    }

    return true;
}

bool MailboxPostgreSQL::sendMail(  unsigned int userID, const MailboxContent& mailcontent )
{
    try
    {
        pqxx::result res;
        pqxx::work        transaction( *_p_databaseConnection, "mail_send" );
        std::string       query;
        std::stringstream userid;
        userid << userID;
        std::stringstream attr;
        attr << mailcontent._header._attributes;

        // strip out white spaces from names
        std::string to, cc;
        for ( std::size_t cnt = 0; cnt < mailcontent._header._to.size(); cnt++ )
        {
            if ( mailcontent._header._to[ cnt ] != ' ' )
                to += mailcontent._header._to[ cnt ];
        }
        for ( std::size_t cnt = 0; cnt < mailcontent._header._cc.size(); cnt++ )
        {
            if ( mailcontent._header._cc[ cnt ] != ' ' )
                cc += mailcontent._header._cc[ cnt ];
        }

        // make sure that the names are not repeated in the cc and to fields
        std::map< std::string, std::string > tomap;
        std::map< std::string, std::string > ccmap;
        std::vector< std::string > names;
        yaf3d::explode( to, ",", &names );

        for ( std::size_t cnt = 0; cnt < names.size(); cnt++ )
        {
            tomap[ names[ cnt ] ] = names[ cnt ];
        }

        names.clear();
        yaf3d::explode( cc, ",", &names );
        for ( std::size_t cnt = 0; cnt < names.size(); cnt++ )
        {
            if ( tomap.find( names[ cnt ] ) == tomap.end() )
                ccmap[ names[ cnt ] ] = names[ cnt ];
        }

        // re-assemble the cc and to fields via comma separation
        to.clear();
        cc.clear();
        std::map< std::string, std::string >::iterator p_to = tomap.begin(), p_toend = tomap.end();
        std::map< std::string, std::string >::iterator p_cc = ccmap.begin(), p_ccend = ccmap.end();;
        for ( std::size_t cnt = 0; p_to != p_toend; ++p_to, cnt++ )
        {
            to += "'" + p_to->first + "'";
            if ( cnt != tomap.size() - 1 )
                to += ",";
        }
        // consider also empty cc
        if ( p_cc == p_ccend )
        {
            cc = "''";
        }
        else
        {
            for ( std::size_t cnt = 0; p_cc != p_ccend; ++p_cc, cnt++ )
            {
                cc += "'" + p_cc->first + "'";
                if ( cnt != ccmap.size() - 1 )
                    cc += ",";
            }
        }

        // call the function for user login
        query = std::string( "SELECT " FCN_MAIL_SEND "(" + userid.str() + "," + attr.str() + ",ARRAY[" + to + "], ARRAY[" + cc + "], '" + cleanString( mailcontent._header._subject ) + "', '" +  cleanString( mailcontent._body ) + "');" );

        res = transaction.exec( query );

        // empty folder?
        if ( !res.size() )
        {
            log_error << "MailboxPostgreSQL: problem on sending mail " << userID << std::endl;
            return false;
        }

        int retvalue;
        res[ 0 ][ FCN_MAIL_SEND ].to( retvalue );
        if ( retvalue < 0 )
        {
            log_error << "MailboxPostgreSQL: problem on sending mail " << userID << ", error code: " << retvalue  << std::endl;
            return false;
        }

        // commit the transaction
        transaction.commit();
    }
    catch( const std::exception& e )
    {
        log_error << "MailboxPostgreSQL: problem on sending mail " << userID << ", reason: " << e.what()  << std::endl;
        return false;
    }

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
