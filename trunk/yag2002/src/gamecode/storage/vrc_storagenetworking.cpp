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
 # networking for storage
 #
 # this class implements the networking functionality for storage
 #
 #
 #   date of creation:  09/25/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_storagenetworking.h"
#include "vrc_storageclient.h"
#include "vrc_storageserver.h"
#include <RNReplicaNet/Inc/DataBlock_Function.h>

using namespace RNReplicaNet;

namespace vrc
{

StorageNetworking::StorageNetworking() :
 _p_accountInfoCallback( NULL ),
 _p_accountPublicInfoCallback( NULL ),
 _p_contactsCallback( NULL )
{
}

StorageNetworking::~StorageNetworking()
{
    // unset the networking object in storage client
    StorageClient::get()->setNetworking( NULL );
}

void StorageNetworking::PostObjectCreate()
{
    // set the networking object in storage client
    StorageClient::get()->setNetworking( this );
}

void StorageNetworking::requestAccountInfo( unsigned int userID, CallbackAccountInfoResult* p_callback )
{ // used by connecting client

    assert( p_callback && "a valid callback for account request is needed!" );
    assert( ( _p_accountInfoCallback == NULL ) && "only one request for account info can be handled at the same time!" );

    _p_accountInfoCallback = p_callback;

    // call the account info rpc on server
    tAccountInfoData info;
    memset( &info, 0, sizeof( info ) );
    info._userID        = userID;
    info._sessionCookie = yaf3d::NetworkDevice::get()->getSessionID();

    MASTER_FUNCTION_CALL( RPC_RequestAccountInfo( info ) );
}

void StorageNetworking::requestPublicAccountInfo( const std::string& username, class CallbackAccountInfoResult* p_callback )
{
    assert( p_callback && "a valid callback for account request is needed!" );

    // is any request pending?
    if ( _p_accountPublicInfoCallback )
        return;

    _p_accountPublicInfoCallback = p_callback;

    // call the account info rpc on server
    tAccountInfoData info;
    memset( &info, 0, sizeof( info ) );
    info._userID        = 0;            // this identifies that we request public info
    info._sessionCookie = yaf3d::NetworkDevice::get()->getSessionID();
    strcpy_s( info._p_nickName, sizeof( info._p_nickName ) - 1, username.c_str() );
    info._p_nickName[ sizeof( info._p_nickName ) - 1 ] = 0;

    MASTER_FUNCTION_CALL( RPC_RequestAccountInfo( info ) );
}

void StorageNetworking::updateAccountInfo( unsigned int userID, const tAccountInfoData& info )
{
    tAccountInfoData updateinfo;
    memset( &updateinfo, 0, sizeof( updateinfo ) );
    updateinfo._sessionCookie = yaf3d::NetworkDevice::get()->getSessionID();
    updateinfo._userID        = userID;
    strcpy_s( updateinfo._p_userDescription, sizeof( updateinfo._p_userDescription ) - 1, info._p_userDescription );
    updateinfo._p_userDescription[ sizeof( updateinfo._p_userDescription ) - 1 ]   = 0;

    MASTER_FUNCTION_CALL( RPC_RequestUpdateAccountInfo( updateinfo ) );
}

void StorageNetworking::requestContacts( unsigned int userID, CallbackAccountInfoResult* p_callback )
{
    _p_contactsCallback = p_callback;
    //! TODO
    tUserContacts   conts;
    conts._sessionCookie = yaf3d::NetworkDevice::get()->getSessionID();
    conts._userID        = userID;
    conts._cmd           = tUserContacts::eRequestContacts;
    conts._dataLen       = 0;

    MASTER_FUNCTION_CALL( RPC_RequestContacts( conts ) );
}

void StorageNetworking::updateContacts( unsigned int userID, const std::vector< std::string >& contacts )
{
    // convert the contacts list into a comma separated string of unique names
    std::string             contlist;
    std::set< std::string > names;
    std::vector< std::string >::const_iterator p_contact = contacts.begin(), p_end = contacts.end();
    for ( ; p_contact != p_end; ++p_contact )
    {
        // skip non-unique names
        if ( names.find( *p_contact ) != names.end() )
            continue;

        names.insert( *p_contact );
        contlist += *p_contact;
        if ( p_contact != p_end )
            contlist += ",";
    }

    tUserContacts   conts;
    conts._sessionCookie = yaf3d::NetworkDevice::get()->getSessionID();
    conts._userID        = userID;
    conts._cmd           = tUserContacts::eUpdateContacts;
    conts._dataLen       = ( contlist.length() < VRC_SNDATA_MAXLEN ) ? contlist.length() : VRC_SNDATA_MAXLEN;
    strncpy( conts._p_data, contlist.c_str(), sizeof( conts._p_data ) );

    MASTER_FUNCTION_CALL( RPC_RequestContacts( conts ) );
}

void StorageNetworking::RPC_RequestAccountInfo( tAccountInfoData info )
{ // this method is called only on server

    info._p_lastLogin[ 0 ]        = 0;
    info._p_onlineTime[ 0 ]       = 0;
    info._p_registrationDate[ 0 ] = 0;
    info._p_userDescription[ 0 ]  = 0;
    info._priviledges             = static_cast< unsigned int >( -1 );

    UserAccount acc;
    // requesting for private or public account information?
    if ( info._userID )
    {
        log_debug << "StorageNetworking: user ID '" << info._userID << "' requests for account info ..." << std::endl;

        info._p_nickName[ 0 ] = 0;

        if ( !StorageServer::get()->getUserAccount( info._userID, info._sessionCookie, acc ) )
        {
            log_warning << "StorageNetworking: could not retrieve account info for user: " << info._userID << std::endl;
        }
        else
        {
            // fill in the account info
            memset( info._p_lastLogin, 0, sizeof( info._p_lastLogin ) );
            memset( info._p_onlineTime, 0, sizeof( info._p_onlineTime ) );
            memset( info._p_nickName, 0, sizeof( info._p_nickName ) );
            memset( info._p_registrationDate, 0, sizeof( info._p_registrationDate ) );
            memset( info._p_userDescription, 0, sizeof( info._p_userDescription ) );

            strcpy_s( info._p_lastLogin, sizeof( info._p_lastLogin ) - 1, acc.getLastLogin().c_str() );
            strcpy_s( info._p_onlineTime, sizeof( info._p_onlineTime ) - 1, acc.getOnlineTime().c_str() );
            strcpy_s( info._p_nickName, sizeof( info._p_nickName ) - 1, acc.getNickname().c_str() );
            strcpy_s( info._p_registrationDate, sizeof( info._p_registrationDate ) - 1, acc.getRegistrationDate().c_str() );
            strcpy_s( info._p_userDescription, sizeof( info._p_userDescription ) - 1, acc.getUserDescription().c_str() );

            info._p_lastLogin[ sizeof( info._p_lastLogin ) - 1 ]               = 0;
            info._p_onlineTime[ sizeof( info._p_onlineTime ) - 1 ]             = 0;
            info._p_nickName[ sizeof( info._p_nickName ) - 1 ]                 = 0;
            info._p_registrationDate[ sizeof( info._p_registrationDate ) - 1 ] = 0;
            info._p_userDescription[ sizeof( info._p_userDescription ) - 1 ]   = 0;

            info._priviledges = acc.getPriviledges();
            info._status      = acc.getStatus();
        }
    }
    else
    {
        info._p_nickName[ sizeof ( info._p_nickName ) - 1 ] = 0;

        log_debug << "StorageNetworking: requesting public info of '" << info._p_nickName << "' from sid " << info._sessionCookie << std::endl;

        acc._nickName = info._p_nickName;
        if ( !StorageServer::get()->getPublicUserAccountInfo( acc ) )
        {
            log_warning << "StorageNetworking: could not retrieve public account info for user: " << info._p_nickName << std::endl;
        }
        else
        {
            memset( info._p_lastLogin, 0, sizeof( info._p_lastLogin ) );
            memset( info._p_onlineTime, 0, sizeof( info._p_onlineTime ) );
            memset( info._p_nickName, 0, sizeof( info._p_nickName ) );
            memset( info._p_registrationDate, 0, sizeof( info._p_registrationDate ) );
            memset( info._p_userDescription, 0, sizeof( info._p_userDescription ) );

            strcpy_s( info._p_nickName, sizeof( info._p_nickName ) - 1, acc.getNickname().c_str() );
            strcpy_s( info._p_registrationDate, sizeof( info._p_registrationDate ) - 1, acc.getRegistrationDate().c_str() );
            strcpy_s( info._p_onlineTime, sizeof( info._p_onlineTime ) - 1, acc.getOnlineTime().c_str() );
            strcpy_s( info._p_userDescription, sizeof( info._p_userDescription ) - 1, acc.getUserDescription().c_str() );

            info._p_nickName[ sizeof( info._p_nickName ) - 1 ]                 = 0;
            info._p_registrationDate[ sizeof( info._p_registrationDate ) - 1 ] = 0;
            info._p_onlineTime[ sizeof( info._p_onlineTime ) - 1 ]             = 0;
            info._p_userDescription[ sizeof( info._p_userDescription ) - 1 ]   = 0;

            info._priviledges = acc.getPriviledges();
            info._status      = acc.getStatus();
        }
    }

    // sent out the account info result
    ALL_REPLICAS_FUNCTION_CALL( RPC_AccountInfoResult( info ) );
}

void StorageNetworking::RPC_RequestUpdateAccountInfo( tAccountInfoData info )
{
    //! NOTE: currently, only the user description can be updated!
    UserAccount acc;
    acc._userDescription = info._p_userDescription;
    acc._userID          = info._userID;
    if ( !StorageServer::get()->updateUserAccount( info._userID, info._sessionCookie, acc ) )
    {
        log_warning << "StorageNetworking: could not update account info for user: " << info._userID << std::endl;
    }
}

void StorageNetworking::RPC_AccountInfoResult( tAccountInfoData info )
{// this method is called on client ( also on remote clients )

    // public or private info?
    if ( info._userID ) // userID == 0 means public info
    {
        // notify about authentification result
        if ( _p_accountInfoCallback )
            _p_accountInfoCallback->accountInfoResult( info );

        // reset the callback
        _p_accountInfoCallback = NULL;
    }
    else
    {
        if ( _p_accountPublicInfoCallback )
            _p_accountPublicInfoCallback->accountInfoResult( info );

        // reset the callback
        _p_accountPublicInfoCallback = NULL;
    }
}

void StorageNetworking::RPC_RequestContacts ( tUserContacts data )
{// called on server

    int          sessionID = GetProcessingDataBlocksFromSessionID();
    unsigned int userID    = StorageServer::get()->getUserID( sessionID );

    switch ( data._cmd )
    {
        case tUserContacts::eRequestContacts:
        {
            std::string   contacts;
            tUserContacts response;

            response._cmd = tUserContacts::eRestuls;

            // if the call fails the reason can be picked up in 'contacts'
            if ( !StorageServer::get()->getUserContacts( data._userID, data._sessionCookie, contacts ) )
            {
                log_error << "StorageNetworking: cannot get contacts, " << data._userID << ", reason: " << contacts << std::endl;
                response._cmd |= tUserContacts::eError;
            }

            strncpy( response._p_data, contacts.c_str(), sizeof( response._p_data ) - 1 );
            response._dataLen = ( contacts.length() > VRC_SNDATA_MAXLEN ) ? contacts.length() : VRC_SNDATA_MAXLEN;

            NOMINATED_REPLICAS_FUNCTION_CALL( 1, &sessionID, RPC_ContactsResult( response ) );
        }
        break;

        case tUserContacts::eUpdateContacts:

            //! TODO
            break;

        default:
            log_error << "StorageNetworking: unknown contacts request" << std::endl;
    }
}

void StorageNetworking::RPC_ContactsResult( tUserContacts data )
{ // called on client

    // call the callback method
    if ( !_p_contactsCallback )
    {
        log_error << "StorageNetworking: received unexpected contacts results" << std::endl;
        return;
    }

    std::vector< std::string > contacts;
    if ( data._cmd & tUserContacts::eError )
    {
        log_error << "StorageNetworking: error on receiving contacts data" << std::endl;
        _p_contactsCallback->contactsResult( false, contacts );
        return;
    }

    // fill the contact list
    data._p_data[ ( data._dataLen > sizeof( data._p_data ) - 1 ) ? sizeof( data._p_data ) - 1 : data._dataLen ] = 0;
    yaf3d::explode( data._p_data, ",", &contacts );

    _p_contactsCallback->contactsResult( true, contacts );
    _p_contactsCallback = NULL;
}

} // namespace vrc
