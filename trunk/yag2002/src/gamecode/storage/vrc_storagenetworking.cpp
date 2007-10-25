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
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_storagenetworking.h"
#include "vrc_storageclient.h"
#include <RNReplicaNet/Inc/DataBlock_Function.h>

using namespace RNReplicaNet;

namespace vrc
{

StorageNetworking::StorageNetworking() :
 _p_accountInfoCallback( NULL )
{
}

StorageNetworking::~StorageNetworking()
{
    // unset the networking object in storage client
    StorageClient::get()->setNetworking( NULL );
}

void StorageNetworking::PostObjectCreate()
{ // a new client has joined

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
    info._userID = userID;
    MASTER_FUNCTION_CALL( RPC_RequestAccountInfo( info ) );
}

void StorageNetworking::RPC_RequestAccountInfo( tAccountInfoData info )
{ // this method is called only on server

    log_debug << "storage: user ID '" << info._userID << "' requests for account info ..." << std::endl;

    //! TODO: use the storage server for retrieving account info
    info._registrationDate = -1;
    info._onlineTime       = -1;
    info._priviledges      = -1;

    // sent out the account info result
    ALL_REPLICAS_FUNCTION_CALL( RPC_AccountInfoResult( info ) );
}

void StorageNetworking::RPC_AccountInfoResult( tAccountInfoData info )
{// this method is called on client ( also on remote clients )

    // notify about authentification result
    if ( _p_accountInfoCallback )
        _p_accountInfoCallback->accountInfoResult( info );

    // reset the callback
    _p_accountInfoCallback = NULL;
}

} // namespace vrc
