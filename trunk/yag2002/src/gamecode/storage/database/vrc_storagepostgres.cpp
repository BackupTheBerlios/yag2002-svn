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
#define TBL_NAME_USERACCOUNTS   "user_accounts"
#define TBL_NAME_USERINVENTORY  "inventory"

//! Fileds of user account table
#define F_USERACC_UID           "account_id"
#define F_USERACC_NAME          "name"
#define F_USERACC_LOGIN         "login_name"
#define F_USERACC_EMAIL         "email"
#define F_USERACC_LASTLOGIN     "last_login"
#define F_USERACC_ONLINETIME    "online_time"
#define F_USERACC_PRIV          "priviledges"

//! Stored procesures
#define FCN_USER_LOGIN          "user_login"
#define FCN_USER_LOGOUT         "user_logout"
#define FCN_USER_GET_ACC        "user_getaccountdata"
#define FCN_USER_REGISTER       "user_register"
#define FCN_USER_DATA           "user_getdata"
#define FCN_USER_INV            "user_getinventory"

#define F_USERDATA_DATA_ID      "user_data_id"
#define F_USERDATA_INV_ID       "user_inventory_id"
#define F_USERDATA_MAILBOX_ID   "user_mailbox_id"
#define F_USERDATA_SKILLS_ID    "user_skills_id"

#define F_INV_ID                "inventory_id"
#define F_INV_DATA              "inv_data"
#define F_INV_USER_DATA_ID      "user_data_id"


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

    return true;
}

void StoragePostgreSQL::release()
{
    if ( !_p_databaseConnection )
        return;

    delete _p_databaseConnection;
    _p_databaseConnection = NULL;
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
        query = std::string( "SELECT " FCN_USER_LOGIN "( '" + login + "', '" + enc_passwd + "' );" );

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

            default:
                ;
        }

        // retrieve the account data
        query = std::string( "SELECT * FROM " FCN_USER_GET_ACC "( '" + login + "' );" );
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
        res[ 0 ][ F_USERACC_LASTLOGIN ].to ( acc._lastLogin );
        res[ 0 ][ F_USERACC_ONLINETIME ].to ( acc._onlineTime );

        acc._nickName =  login;

        // commit the transaction
        transaction.commit();
    }
    catch( const std::exception& e )
    {
        log_info << "PostgreSQL: problem on logging in user " << login << ", reason: " << e.what()  << std::endl;
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
        query = std::string( "SELECT " FCN_USER_LOGOUT "( '" + login + "' );" );
        pqxx::result res = transaction.exec( query );

        if ( res.size() < 1 )
        {
            log_info << "PostgreSQL: 1) internal error on loggin out user " << login << std::endl;
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

        // logout the user
        query = std::string( "SELECT " FCN_USER_REGISTER "( '" + login + "', '" + enc_passwd + "', '" + name + "', '" + email + "' );" );
        pqxx::result res = transaction.exec( query );

        if ( res.size() < 1 )
        {
            log_info << "PostgreSQL: internal error, could register user " << login << std::endl;
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

bool StoragePostgreSQL::updateUserAccount( const UserAccount& /*acc*/ )
{
    //! TODO ...

    return false;
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
