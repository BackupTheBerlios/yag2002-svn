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
 # storage interface used by server for authentification and
 #  user data exchange.
 #
 #   date of creation:  09/29/2007
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_storageserver.h"
#include "vrc_storagenetworking.h"
#include "database/vrc_mysqlstorage.h"
#include "database/vrc_account.h"
#include "database/vrc_connectiondata.h"

//! Implement the singleton
YAF3D_SINGLETON_IMPL( vrc::StorageServer )

//TODO: tbd along with configuration file
// default values for database connection
#define SERVER               "localhost"
#define PORT                 3306
#define SCHEMA               "demo_account_mgm"
#define USER                 "demouser"
#define PASSWD               "demo2007"

//! Actual DB table names
#define DBTBL_ACCOUNTS       "accounts"
#define DBTBL_GAMEWORLD      "game_world"
#define DBTBL_GAMEITEMS      "game_items"
#define DBTBL_USERINVENTORY  "user_inventory"

namespace vrc
{

StorageServer::StorageServer() :
 _connectionEstablished( false ),
 _p_networking( NULL ),
 _p_storage( NULL ),
 _p_account( NULL )
{
}

StorageServer::~StorageServer()
{
    if ( _p_networking )
        delete _p_networking;

    if ( _p_storage )
    {
        _p_storage->release();
        delete _p_storage;
    }

    if ( _p_account )
        delete _p_account;
}

void StorageServer::shutdown()
{
    log_info << "StorageServer: shutting down" << std::endl;

    // destroy the singleton
    destroy();
}

void StorageServer::initialize() throw ( StorageServerException )
{
    // this is created only on server
    assert( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server );

    if ( _p_networking )
        throw StorageServerException( "Storage server is already initialized." );

    ConnectionData connData;
    connData._server = SERVER;
    connData._port   = PORT;
    connData._schema = SCHEMA;
    connData._user   = USER;
    connData._passwd = PASSWD;
    connData._content[ TBL_NAME_USERACCOUNTS  ] = DBTBL_ACCOUNTS;
    connData._content[ TBL_NAME_GAMEWORLD     ] = DBTBL_GAMEWORLD;
    connData._content[ TBL_NAME_GAMEITEMS     ] = DBTBL_GAMEITEMS;
    connData._content[ TBL_NAME_USERINVENTORY ] = DBTBL_USERINVENTORY;

    // setup the storage
    _p_storage = new MysqlStorage();

    // initialize the storage
    if ( !_p_storage->initialize( connData ) )
        throw StorageServerException( "Storage backend could not be initialized." );

    // create the account object
    _p_account = new Account( _p_storage );

    log_info << "setup storage ..." << std::endl;

    // create and publish the storage network object
    _p_networking = new StorageNetworking();
    _p_networking->Publish();

    // set the authentification callback in network device, whenever a client connects then 'authentify' is called.
    yaf3d::NetworkDevice::get()->setAuthCallback( this );

    _connectionEstablished = true;

    log_info << "storage successfully initialized" << std::endl;
}

bool StorageServer::authentify( const std::string& login, const std::string& passwd, unsigned int& userID )
{
    if ( !_connectionEstablished )
        return false;

    assert ( _p_networking && "storage server is not initialized!" );

    // guest login
    if ( !login.length() )
    {
        userID = static_cast< unsigned int >( -1 );
        return true;
    }

    // check the login
    AccountData acc;
    acc.setUsername( login );
    if ( !_p_account->loginUser( acc, passwd ) )
    {
        log_verbose << "user '" << login << "' failed to login" << std::endl;
        return false;
    }

    userID = acc.getUserId();

    return true;
}


} // namespace vrc
