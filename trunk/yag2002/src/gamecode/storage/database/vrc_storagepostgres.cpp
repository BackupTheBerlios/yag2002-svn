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
 # implementation of PostgreSQL storage
 #
 #   date of creation:  10/18/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_storagepostgres.h"
#include "../vrc_userinventory.h"
#include <pqxx/transaction.hxx>


//! Database table names
#define TBL_NAME_USERACCOUNTS       "user_account"
#define TBL_NAME_USERINVENTORY      "inventory"

//! Fileds of user account table
#define F_USERACC_UID               "id_account"
#define F_USERACC_NAME              "name"
#define F_USERACC_LOGIN             "login_name"
#define F_USERACC_EMAIL             "email"
#define F_USERACC_LASTLOGIN         "last_login"
#define F_USERACC_ONLINETIME        "online_time"
#define F_USERACC_PRIV              "priviledges"
#define F_USERACC_STATUS            "status"
#define F_USERACC_REGDATE           "registration_date"
#define F_USERACC_USERDESC          "user_description"

#define F_USERDATA_DATA_ID          "id_user_data"
#define F_USERDATA_INV_ID           "fk_id_user_inventory"
#define F_USERDATA_MAILBOX_ID       "fk_id_user_mailbox"
#define F_USERDATA_SKILLS_ID        "fk_id_user_skills"
#define F_USERDATA_CONTACTS         "contacts"

#define F_INV_ID                    "id_inventory"
#define F_INV_USER_DATA_ID          "fk_id_user_data"
#define F_INV_DATA                  "inv_data"

//! Storage functions
#define FCN_INIT_DB                 "init_db"
#define FCN_USER_LOGIN              "user_login"
#define FCN_USER_LOGOUT             "user_logout"
#define FCN_USER_GET_ACC            "user_getaccountdata"
#define FCN_USER_UPDATE_ACC         "user_updateaccountdata"
#define FCN_USER_PUBLIC_ACC_INFO    "user_getpublicinfo"
#define FCN_USER_REGISTER           "user_register"
#define FCN_USER_DATA               "user_getdata"
#define FCN_USER_INV                "user_getinventory"

namespace vrc
{

StoragePostgreSQL::StoragePostgreSQL() :
 _p_databaseConnection( NULL )
{
}

StoragePostgreSQL::~StoragePostgreSQL()
{
    if ( _p_databaseConnection )
       release();
}

bool StoragePostgreSQL::initialize( const ConnectionData& connData )
{
    if ( !_p_databaseConnection )
    {
        try
        {
            std::stringstream conargs;
            conargs << "dbname=" << connData._dbname;
            conargs << " user=" << connData._user;
            conargs << " password=" << connData._passwd;

            if ( connData._port )
                conargs << " port=" << connData._port;

            if ( connData._server.length() )
                conargs << " host=" << connData._server;

            _p_databaseConnection = new pqxx::connection( conargs.str() );

            // remove sensitive data from memory -- is this method really secure enough on stringstream!?
            conargs.str( "" );
        }
        catch ( const std::exception& e )
        {
            delete _p_databaseConnection;
            _p_databaseConnection = NULL;
            log_error << "PostgreSQL: " << e.what() << std::endl;
            return false;
        }
        catch ( ... )
        {
            delete _p_databaseConnection;
            _p_databaseConnection = NULL;
            log_error << "PostgreSQL: unhandled exception" << std::endl;
            return false;
        }
    }
    else
    {
        log_error << "cannot initialize StoragePostgreSQL: connection to database already exists" << std::endl;
        return false;
    }

    // call the database function for initialization
    try
    {
        pqxx::work transaction( *_p_databaseConnection, "init_db" );
        std::string query( "SELECT " FCN_INIT_DB "();" );
        pqxx::result res = transaction.exec( query );

        // commit the transaction
        transaction.commit();
    }
    catch( const std::exception& e )
    {
        log_error << "PostgreSQL: problem on initializing the database, reason: " << e.what()  << std::endl;
        return false;
    }

    return true;
}

void StoragePostgreSQL::release()
{
    if ( !_p_databaseConnection )
        return;

    delete _p_databaseConnection;
    _p_databaseConnection = NULL;
}

std::string StoragePostgreSQL::cleanString( const std::string& str )
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

bool StoragePostgreSQL::loginUser( const std::string login, const std::string passwd, UserAccount& acc )
{
    pqxx::result res;
    try
    {
        std::string enc_passwd = pqxx::encrypt_password( "_", passwd );
        enc_passwd = enc_passwd.substr( 3 );
        pqxx::work transaction( *_p_databaseConnection, "login" );
        std::string query;

        // call the function for user login
        query = std::string( "SELECT " FCN_USER_LOGIN "( '" + cleanString( login ) + "', '" + enc_passwd + "' );" );

        res = transaction.exec( query );

        if ( res.size() < 1 )
        {
            log_error << "PostgreSQL: internal error when loggin in user: " << login << std::endl;
            return false;
        }

        // get the return value of the login function
        int retvalue;
        res[ 0 ][ FCN_USER_LOGIN ].to( retvalue );
        switch ( retvalue )
        {
            case -1:
            {
                log_info << "PostgreSQL: non-existing user '" << login << "' tried to login" << std::endl;
                return false;
            }
            break;

            case -2:
            {
                log_info << "PostgreSQL: user '" << login << "' tried to login with wrong password" << std::endl;
                return false;
            }
            break;

            case -3:
            {
                log_info << "PostgreSQL: banned user '" << login << "' tried to login" << std::endl;
                return false;
            }
            break;

            case -4:
            {
                log_info << "PostgreSQL: player is already logged in: " << login << std::endl;
                return false;
            }
            break;

            default:
                ;
        }

        // retrieve the account data
        query = std::string( "SELECT * FROM " FCN_USER_GET_ACC "( '" + cleanString( login ) + "' );" );
        res = transaction.exec( query );

        if ( res.size() < 1 )
        {
            log_error << "PostgreSQL: internal error when getting account data for user: " << login << std::endl;
            return false;
        }

        res[ 0 ][ F_USERACC_UID ].to ( acc._userID );
        res[ 0 ][ F_USERACC_NAME ].to ( acc._userName );
        res[ 0 ][ F_USERACC_EMAIL ].to ( acc._email );
        res[ 0 ][ F_USERACC_PRIV ].to ( acc._priviledges );
        res[ 0 ][ F_USERACC_STATUS ].to ( acc._status );
        res[ 0 ][ F_USERACC_LASTLOGIN ].to ( acc._lastLogin );
        res[ 0 ][ F_USERACC_ONLINETIME ].to ( acc._onlineTime );
        res[ 0 ][ F_USERACC_REGDATE ].to ( acc._registrationDate );
        res[ 0 ][ F_USERACC_USERDESC ].to ( acc._userDescription );

        acc._nickName =  login;

        // commit the transaction
        transaction.commit();
    }
    catch( const std::exception& e )
    {
        log_error << "PostgreSQL: problem on logging in user " << login << ", reason: " << e.what()  << std::endl;
        return false;
    }

    return true;
}

bool StoragePostgreSQL::logoutUser( const std::string login )
{
    try
    {
        pqxx::work transaction( *_p_databaseConnection, "logout" );
        std::string query;

        // logout the user
        query = std::string( "SELECT " FCN_USER_LOGOUT "( '" + cleanString( login ) + "' );" );
        pqxx::result res = transaction.exec( query );

        if ( res.size() < 1 )
        {
            log_error << "PostgreSQL: 1) internal error on loggin out user " << login << std::endl;
            return false;
        }

        int retvalue;
        res[ 0 ][ FCN_USER_LOGOUT ].to( retvalue );
        if ( retvalue < 0 )
        {
            log_error << "PostgreSQL: 2) internal error on logging out user " << login << " : " << retvalue << std::endl;
        }

        // commit the update
        transaction.commit();
    }
    catch( const std::exception& e )
    {
        log_error << "PostgreSQL: problem on logging out user " << login << ", reason: " << e.what()  << std::endl;
        return false;
    }

    return true;
}

bool StoragePostgreSQL::registerUser( const std::string& name, const std::string& login, const std::string& passwd, const std::string& email )
{
    try
    {
        std::string enc_passwd = pqxx::encrypt_password( "_", passwd );
        enc_passwd = enc_passwd.substr( 3 );
        pqxx::work transaction( *_p_databaseConnection, "register" );
        std::string query;

        // register a new user
        query = std::string( "SELECT " FCN_USER_REGISTER "( '" + cleanString( login ) + "', '" + enc_passwd + "', '" + cleanString( name ) + "', '" + cleanString( email ) + "' );" );
        pqxx::result res = transaction.exec( query );

        if ( res.size() < 1 )
        {
            log_error << "PostgreSQL: internal error, could register user " << login << std::endl;
            return false;
        }

        int retvalue;
        res[ 0 ][ FCN_USER_REGISTER ].to( retvalue );
        if ( retvalue < 0 )
        {
            log_info << "PostgreSQL: registration of user " << login << " failed, login name already exist" << std::endl;
            return false;
        }

        // commit the update
        transaction.commit();
    }
    catch( const std::exception& e )
    {
        log_error << "PostgreSQL: problem on registration of user " << login << ", reason: " << e.what()  << std::endl;
        return false;
    }

    return true;
}

bool StoragePostgreSQL::updateUserAccount( const UserAccount& acc )
{
    try
    {
        pqxx::work transaction( *_p_databaseConnection, "updateaccount" );
        std::string query;

        // update the user account
        std::stringstream userid;
        userid << acc._userID;
        query = std::string( "SELECT " FCN_USER_UPDATE_ACC "( '" + userid.str() + "', '" + cleanString( acc._userDescription ) + "' );" );
        pqxx::result res = transaction.exec( query );

        if ( res.size() < 1 )
        {
            log_error << "PostgreSQL: 1) internal error on updating user account, user ID " << acc._userID << std::endl;
            return false;
        }

        int retvalue;
        res[ 0 ][ FCN_USER_UPDATE_ACC ].to( retvalue );
        if ( retvalue < 0 )
        {
            log_warning << "PostgreSQL: 2) cannot update user account, user ID " << acc._userID << " : " << retvalue << std::endl;
        }

        // commit the update
        transaction.commit();
    }
    catch( const std::exception& e )
    {
        log_error << "PostgreSQL: problem on updating account, user ID " << acc._userID << ", reason: " << e.what()  << std::endl;
        return false;
    }

    return true;
}

bool StoragePostgreSQL::getPublicUserAccountInfo( UserAccount& acc )
{
    try
    {
        pqxx::work transaction( *_p_databaseConnection, "publicinfo" );
        std::string query;

        // find the user with given name
        query = std::string( "SELECT * FROM " FCN_USER_PUBLIC_ACC_INFO "( '" + cleanString( acc.getNickname() ) + "' );" );
        pqxx::result res = transaction.exec( query );

        if ( res.size() < 1 )
        {
            log_warning << "PostgreSQL: trying to get public info of a non-existing user " << acc.getNickname() << std::endl;
            return false;
        }

        res[ 0 ][ F_USERACC_LOGIN ].to ( acc._nickName );
        res[ 0 ][ F_USERACC_STATUS ].to ( acc._status );
        res[ 0 ][ F_USERACC_PRIV ].to ( acc._priviledges );
        res[ 0 ][ F_USERACC_ONLINETIME ].to ( acc._onlineTime );
        res[ 0 ][ F_USERACC_REGDATE ].to ( acc._registrationDate );
        res[ 0 ][ F_USERACC_USERDESC ].to ( acc._userDescription );

        // commit the update
        transaction.commit();
    }
    catch( const std::exception& e )
    {
        log_error << "PostgreSQL: problem on getting public user info " << acc.getNickname() << ", reason: " << e.what()  << std::endl;
        return false;
    }

    return true;
}

bool StoragePostgreSQL::getUserData( unsigned int userID, UserData& data )
{
    pqxx::result res;
    try
    {
        pqxx::work transaction( *_p_databaseConnection, "userdata" );
        std::string query;
        std::stringstream uid;
        uid << userID;

        // call the function for user data
        query = std::string( "SELECT * FROM " FCN_USER_DATA "( " + uid.str() + " );" );

        res = transaction.exec( query );

        if ( res.size() < 1 )
        {
            log_error << "PostgreSQL: internal error when getting user data: " << userID << std::endl;
            return false;
        }

        // check if we got a valid user data row
        if ( res[ 0 ][ F_USERDATA_DATA_ID ].is_null() )
        {
            log_info << "PostgreSQL: could not get user data: '" << userID << std::endl;
            return false;
        }
        else
        {
            res[ 0 ][ F_USERDATA_DATA_ID ].to( data._dataID );
            res[ 0 ][ F_USERDATA_INV_ID ].to( data._inventoryID );
            res[ 0 ][ F_USERDATA_MAILBOX_ID ].to( data._mailboxID );
            res[ 0 ][ F_USERDATA_SKILLS_ID ].to( data._skillsID );
            res[ 0 ][ F_USERDATA_CONTACTS ].to( data._contacts );
        }

        // commit the transaction
        transaction.commit();
    }
    catch( const std::exception& e )
    {
        log_info << "PostgreSQL: problem on getting user data: " << userID << ", reason: " << e.what()  << std::endl;
        return false;
    }

    return true;
}

bool StoragePostgreSQL::getUserInventory( unsigned int userID, UserInventory* p_inv )
{
    pqxx::result res;
    try
    {
        pqxx::work transaction( *_p_databaseConnection, "inventory" );
        std::string query;
        std::stringstream userid;
        userid << userID;

        // call the function for user inventory
        query = std::string( "SELECT * FROM " FCN_USER_INV "( " + userid.str() + " );" );

        res = transaction.exec( query );

        if ( res[ 0 ][ F_INV_ID ].is_null() )
        {
            log_warning << "PostgreSQL: trying to get a non-existing user inventory: " << userID << std::endl;
            return false;
        }

        // import the inventory items
        std::string  invdata;
        res[ 0 ][ F_INV_DATA ].to( invdata );
        p_inv->importItems( invdata );

        // commit the transaction
        transaction.commit();
    }
    catch( const std::exception& e )
    {
        log_info << "PostgreSQL: problem on getting user inventory: " << userID << ", reason: " << e.what()  << std::endl;
        return false;
    }

    return true;
}

} // namespace vrc
