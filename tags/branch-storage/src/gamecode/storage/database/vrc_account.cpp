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
 # functionalities for account requests from storage
 #
 #   date of creation:  10/03/2007
 #
 #   author:            mpretz 
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_account.h"
#include "vrc_basestorage.h"

// openssl headers
//#include <e_os.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

#undef  SHA_0 /* FIPS 180 */
#define SHA_1 /* FIPS 180-1 */

//! Account tokens in db
#define TBL_ACCOUNT_ID              "ID"
#define TBL_ACCOUNT_USERNAME        "username"
#define TBL_ACCOUNT_PASSWORD        "password"
#define TBL_ACCOUNT_FIRSTNAME       "firstname"
#define TBL_ACCOUNT_LASTNAME        "lastname"
#define TBL_ACCOUNT_NICKNAME        "nickname"
#define TBL_ACCOUNT_LASTLOGIN       "lastlogin"
#define TBL_ACCOUNT_EMAIL           "email"

namespace vrc
{

//! Implementation of class AccountData
AccountData::AccountData() :
_userName( "" ),
_firstName( "" ),
_lastName( "" ),
_nickName( "" ),
_email( "" ),
_lastLogin( 0 ),
_userID( 0 )
{
}

AccountData::~AccountData()
{
}

const std::string& AccountData::getUsername() const
{
    return _userName;
}

const std::string& AccountData::getFirstname() const
{
    return _firstName;
}

const std::string& AccountData::getLastname() const
{
    return _lastName;
}

const std::string& AccountData::getNickname() const
{
    return _nickName;
}

const std::string& AccountData::getEmail() const
{
    return _email;
}

unsigned int AccountData::getLastLogin() const
{
    return _lastLogin;
}

unsigned int AccountData::getUserId() const
{
    return _userID;
}

void AccountData::setUsername( const std::string& username )
{
    _userName = username;
}

void AccountData::setFirstname( const std::string& firstname )
{
    _firstName = firstname;
}

void AccountData::setLastname( const std::string& lastname )
{
    _lastName = lastname;
}

void AccountData::setNickname( const std::string& nickname )
{
    _nickName = nickname;
}

void AccountData::setEmail( const std::string& email )
{
    _email = email;
}

void AccountData::setLastLogin( const unsigned int lastlogin )
{
    _lastLogin = lastlogin;
}

void AccountData::setUserId( const unsigned int userID )
{
    _userID = userID;
}


//! Implementation of class Account
Account::Account( BaseStorage* p_storage ) :
_p_storage( p_storage )
{
}

Account::~Account()
{
}

bool Account::loginUser( AccountData& account, const std::string& password )
{
    assert( _p_storage && "no valid storage object exists!" );

    std::vector< std::string > cond;
    cond.push_back( TBL_ACCOUNT_USERNAME "='" + account.getUsername() + "'" );
    std::vector< BaseStorage::col_pair > data = _p_storage->getData( "ID", TBL_NAME_USERACCOUNTS, &cond );

    if ( data.size() > 1 )    // !!! THIS CASE IS NOT ALLOWED !!!
    {
        // more than one user found with the same name
        log_error << "Account: semantic error in database (more than one user has been found)" << std::endl;
        return false;
    }
    else if ( data.size() == 0 )
    {
        // no user (with this name) has been found
        log_error << "Account: user does not exist" << std::endl;
        return false;
    }

    account.setUserId( ( unsigned int )atoi( data.at( 0 )[ "id" ].c_str() ) );

    // now check if user entered the correct password
    std::string mdpasswd;
    if ( password.compare( "" ) != 0 )
    {
        unsigned char md[ SHA_DIGEST_LENGTH ];
        char          p_buf[ 80 ];
        EVP_MD_CTX    c;

        EVP_MD_CTX_init( &c );
        EVP_Digest( password.c_str(), password.length(), md, NULL, EVP_sha1(), NULL );

        for ( int i = 0; i < SHA_DIGEST_LENGTH; ++i )
            sprintf_s( &( p_buf[ i * 2 ] ), sizeof( p_buf ) - i * 2, "%02x", md[ i ] );

        EVP_MD_CTX_cleanup( &c );
        mdpasswd = p_buf;

        memset( p_buf, 0, sizeof( p_buf ) );
    }

    cond.clear();
    cond.push_back( TBL_ACCOUNT_USERNAME "='" + account.getUsername() + "'" );
    cond.push_back( TBL_ACCOUNT_PASSWORD "='" + mdpasswd + "'" );

    mdpasswd.erase();

    data = _p_storage->getData( TBL_ACCOUNT_ID, TBL_NAME_USERACCOUNTS, &cond );

    if ( ( data.size() == 1 ) && ( account.getUserId() == ( unsigned int )atoi( data.at( 0 )[ "id" ].c_str() ) ) )
    {
        // get all user-relevant infomation

        std::stringstream uid;
        uid << account.getUserId();
        cond.clear();
        cond.push_back( TBL_ACCOUNT_ID "=" + uid.str() );
        //TODO: kein '*', sondern nur die notwendigen spalten abfragen, um zu verhindern,
        // das das passwort nich mit ausgelesen wird
        data = _p_storage->getData( "*", TBL_NAME_USERACCOUNTS, &cond );

        account.setFirstname( data.at( 0 )[ TBL_ACCOUNT_FIRSTNAME ] );
        account.setLastname( data.at( 0 )[ TBL_ACCOUNT_LASTNAME ] );
        account.setNickname( data.at( 0 )[ TBL_ACCOUNT_NICKNAME ] );
        account.setEmail( data.at( 0 )[ TBL_ACCOUNT_EMAIL ] );
        account.setLastLogin( ( unsigned int )atoi( data.at( 0 )[ TBL_ACCOUNT_LASTLOGIN ].c_str() ) );
        return true;
    }

    // incorrect password for current user
    log_error << "Account: incorrect password for current user" << std::endl;

    // clean up
    cond.clear();
    data.clear();

    return false;
}

bool Account::logoutUser( const AccountData& account )
{
    // update all relevant information about user

    std::vector< std::string > cond;

    // assemble the user ID field
    std::stringstream uid;
    uid << account.getUserId();

    cond.push_back( "id=" + uid.str());

    // assemble the last login field
    time_t logout_time;
    time( &logout_time );
    std::stringstream ll;
    ll << ( int )logout_time;

    if ( !_p_storage->setData( std::string( "lastlogin=" ) + ll.str(), TBL_NAME_USERACCOUNTS, &cond ) )
        log_warning << "Account: cannot save last login for current user" << std::endl;

    return true;
}

bool Account::updateAccount( const AccountData& account, const std::string& password )
{
    // we don't need to check if user exists, because updating
    // user-information is always done on existing user

    std::string mdpasswd;

    if ( password.compare( "" ) != 0 )
    {
        unsigned char md[ SHA_DIGEST_LENGTH ];
        char          p_buf[ 80 ];
        EVP_MD_CTX    c;

        EVP_MD_CTX_init( &c );
        EVP_Digest( password.c_str(), password.length(), md, NULL, EVP_sha1(), NULL );

        for ( int i = 0; i < SHA_DIGEST_LENGTH; ++i )
            sprintf_s( &( p_buf[ i * 2 ] ), sizeof( p_buf ) - i * 2, "%02x", md[ i ] );

        EVP_MD_CTX_cleanup( &c );
        mdpasswd = p_buf;

        memset( p_buf, 0, sizeof( p_buf ) );
    }

    std::stringstream uid;
    uid << account.getUserId();

    std::vector< std::string > cond;
    cond.push_back( "id=" + uid.str() );

    std::string tmp;

    // get all new user informations
    tmp.append( "firstname='" ); tmp.append( account.getFirstname() ); tmp.append( "'" );
    tmp.append( ",lastname='" ); tmp.append( account.getLastname()  ); tmp.append( "'" );
    tmp.append( ",nickname='" ); tmp.append( account.getNickname()  ); tmp.append( "'" );
    tmp.append( ",email='"    ); tmp.append( account.getEmail()     ); tmp.append( "'" );

    // check if we have a new password
    if ( password.compare( "" ) != 0 )
    {
        tmp.append( ",password='" ); tmp.append( mdpasswd ); tmp.append( "'" );
    }

    mdpasswd.erase();

    if ( !_p_storage->setData( tmp, TBL_NAME_USERACCOUNTS, &cond ) )
    {
        tmp.clear();
        log_error << "Account: cannot save new informations about current user" << std::endl;

        return false;
    }

    return true;
}

} // namespace vrc
