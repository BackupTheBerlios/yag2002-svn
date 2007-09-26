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
#include <RNReplicaNet/Inc/DataBlock_Function.h>

using namespace RNReplicaNet;

namespace vrc
{

StorageNetworking::StorageNetworking() :
 _p_authCallback( NULL ),
 _isAuthentified( false )
{
}

StorageNetworking::~StorageNetworking()
{
}

void StorageNetworking::PostObjectCreate()
{
    // a new client has joined

    //! TODO: setup the storage proxy in player utils
    //        the proxy will be implemented in player utils and provides all necessary methods for authentifying and data exchange
}

void StorageNetworking::authentify( const std::string& login, const std::string& passwd, CallbackAuthResult* p_callback )
{ // used by connecting client
    _p_authCallback = p_callback;

    tAuthData auth;
    memset( auth._login, 0, sizeof( auth._login ) );
    memset( auth._passwd, 0, sizeof( auth._passwd ) );
    strcpy_s( auth._login, login.length(), login.c_str() );
    strcpy_s( auth._passwd, passwd.length(), passwd.c_str() );

    // call the authentification rpc on server
    MASTER_FUNCTION_CALL( RPC_Authentify( auth ) );

    memset( auth._login, 0, sizeof( auth._login ) );
    memset( auth._passwd, 0, sizeof( auth._passwd ) );
}

void StorageNetworking::RPC_Authentify( tAuthData auth )
{ // this method is called only on server

    auth._login[ sizeof( auth._login ) - 1 ] = 0;
    auth._passwd[ sizeof( auth._passwd ) - 1 ] = 0;
    std::string login( auth._login );
    std::string passwd( auth._passwd );

    //! TODO: use the account manager for authentification
    std::string checklogin( "guest" );
    std::string checkpasswd( "1234" );

    // sent out the authentification result
    if ( ( login == checklogin ) && ( passwd == checkpasswd ) )
    {
        ALL_REPLICAS_FUNCTION_CALL( RPC_AuthentificationResult( true ) );
        _isAuthentified = true;
    }
    else
    {
        ALL_REPLICAS_FUNCTION_CALL( RPC_AuthentificationResult( false ) );
        _isAuthentified = false;
    }

    memset( auth._login, 0, sizeof( auth._login ) );
    memset( auth._passwd, 0, sizeof( auth._passwd ) );
}

void StorageNetworking::RPC_AuthentificationResult( bool granted )
{// this method is called on client ( also on remote clients )

    // set the authentification result flag
    _isAuthentified = granted;

    // notify about authentification result
    if ( _p_authCallback )
        _p_authCallback->authResult( granted );
}

} // namespace vrc
