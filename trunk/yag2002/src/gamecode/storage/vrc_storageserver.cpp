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
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_userinventory.h"
#include "vrc_storageserver.h"
#include "vrc_storagenetworking.h"
#include "vrc_mailboxnetworking.h"
#include "database/vrc_storagepostgres.h"
#include "database/vrc_mailboxpostgres.h"
#include "database/vrc_account.h"
#include "database/vrc_connectiondata.h"


//! Multi-platform '_getch()' for reading the db password from console
#ifdef WIN32
    #include <conio.h>

    #define LINE_TERM   0xd

#else
    #include <termios.h>
    int _getch()
    {
       int ch;
       struct termios oldt, newt;

       tcgetattr( STDIN_FILENO, &oldt );
       newt = oldt;
       newt.c_lflag &= ~( ICANON | ECHO );
       tcsetattr( STDIN_FILENO, TCSANOW, &newt );
       ch = getchar();
       tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
       return ch;
    }

    #define LINE_TERM   0xa

#endif

//! Implement the singleton
YAF3D_SINGLETON_IMPL( vrc::StorageServer )

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
 _p_mailboxNetworking( NULL ),
 _p_mailbox( NULL )
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

    // delete the user cache entries
    std::map< int, UserState* >::iterator p_beg = _userCache.begin(), p_end = _userCache.end();
    for ( ; p_beg != p_end; ++p_beg )
        delete p_beg->second;
}

void StorageServer::shutdown()
{
    log_info << "StorageServer: shutting down" << std::endl;

    // destroy the singleton
    destroy();
}

void StorageServer::initialize() throw ( StorageServerException )
{
    log_info << "StorageServer: initializing" << std::endl;

    // this is created only on server
    assert( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server );

    if ( _p_networking )
        throw StorageServerException( "Storage server is already initialized." );

    std::string  dbip;
    std::string  dbname;
    std::string  dbuser;
    std::string  dbpasswd;
    unsigned int dbport = 0;

    yaf3d::Configuration::get()->getSettingValue( VRC_GS_DB_IP, dbip );
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_DB_NAME, dbname );
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_DB_USER, dbuser );
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_DB_PORT, dbport );

    ConnectionData connData;
    connData._server = dbip;
    connData._port   = dbport;
    connData._dbname = dbname;
    connData._user   = dbuser;

    // setup the storage
    _p_storage = new StoragePostgreSQL();

    // retry the db connection up to 3 times
    bool         dbinitialized = false;
    unsigned int retrycnt      = 3;
    while ( retrycnt-- > 0 )
    {
        // get the password from console
        std::cout << "################################" << std::endl;
        std::cout << ">>> enter the database password: ";
        int c = 0;
        do
        {
            c = _getch();
            if ( c != LINE_TERM )
                dbpasswd += c;
        }
        while ( c != LINE_TERM );
        std::cout << std::endl;
        std::cout << "################################" << std::endl;
        std::cout << std::endl;

        connData._passwd = dbpasswd;

        // try to initialize the storage
        if ( !_p_storage->initialize( connData ) )
        {
            _p_storage->release();
            dbpasswd = "";
            continue;
        }
        else
        {
            dbinitialized = true;
            break;
        }
    }

    // remove sensitive data from memory
    for ( std::size_t cnt = 0; cnt < connData._passwd.size(); cnt++ )
        connData._passwd[ cnt ] = 0;

    for ( std::size_t cnt = 0; cnt < dbpasswd.size(); cnt++ )
        dbpasswd[ cnt ] = 0;

    if ( !dbinitialized )
    {
        throw StorageServerException( "Storage backend could not be initialized." );
    }

    log_info << "setup storage ..." << std::endl;

    // create and publish the storage network object
    _p_networking = new StorageNetworking();
    _p_networking->Publish();

    // setup the mailbox
    _p_mailbox = new MailboxPostgreSQL( ( dynamic_cast< StoragePostgreSQL* >( _p_storage ) )->_p_databaseConnection );
    _p_mailboxNetworking = new MailboxNetworking( _p_mailbox );
    _p_mailboxNetworking->Publish();

    // set the authentification callback in network device, whenever a client connects then 'authentify' is called.
    yaf3d::NetworkDevice::get()->setAuthCallback( this );

    // register for getting network session notifications ( client left notification )
    yaf3d::NetworkDevice::get()->registerSessionNotify( this );

    _connectionEstablished = true;

    log_info << "storage successfully initialized" << std::endl;
}

bool StorageServer::registerUser( const std::string& name, const std::string& nickname, const std::string& passwd, const std::string& email )
{
    if ( !_connectionEstablished )
        return false;

    if ( !_p_storage->registerUser( name, nickname, passwd, email ) )
    {
        log_verbose << "user '" << nickname << "' already exists" << std::endl;
        return false;
    }

    return true;
}

bool StorageServer::authentify( int sessionID, const std::string& login, const std::string& passwd, unsigned int& userID )
{
    if ( !_connectionEstablished )
        return false;

    assert ( _p_networking && "storage server is not initialized!" );

    // check for loging and passwd strings
    if ( !passwd.length() || !login.length() )
    {
        return false;
    }

    // check the login
    UserAccount acc;
    if ( !_p_storage->loginUser( login, passwd, acc ) )
    {
        log_verbose << "user '" << login << "' failed to login" << std::endl;
        return false;
    }

    UserState* p_state = new UserState;

    // get the user data table
    if ( !_p_storage->getUserData( acc._userID, p_state->_userData ) )
    {
        log_error << "StorageServer: could not retrieve user data, user ID " << acc._userID << std::endl;
        return false;
    }

    // cache the user login state
    p_state->_sessionID       = sessionID;
    p_state->_userAccount     = acc;
    p_state->_p_userInventory = new UserInventory( acc._userID );
    _userCache[ sessionID ]   = p_state;

    // set the valid user ID
    userID = acc._userID;

    return true;
}

void StorageServer::onSessionLeft( int sessionID )
{
    if ( !_connectionEstablished )
        return;

    std::map< int, UserState* >::iterator p_end = _userCache.end(), p_user = _userCache.find( sessionID );
    if ( p_user == p_end )
    {
        log_info << "*** StorageServer: session ID was not cached before, invalid client request!" << std::endl;
        return;
    }

    _p_storage->logoutUser( p_user->second->_userAccount.getNickname() );

    // delete and remove the cache entry
    delete p_user->second;
    _userCache.erase( p_user );
}

bool StorageServer::validateClient( unsigned int userID, int sessionID )
{
    std::map< int, UserState* >::iterator p_end = _userCache.end(), p_user = _userCache.find( sessionID );
    if ( ( p_user == p_end ) || ( p_user->second->_userAccount.getUserId() != userID ) )
    {
        log_warning << "*** StorageServer: client validation failed: user ID " << userID << ", session ID " << sessionID << std::endl;
        return false;
    }

    return true;
}

unsigned int StorageServer::getUserID( int sessionID )
{
    std::map< int, UserState* >::iterator p_end = _userCache.end(), p_user = _userCache.find( sessionID );
    if ( p_user == p_end )
    {
        log_warning << "*** StorageServer: no user logged in with session ID " << sessionID << std::endl;
        return 0;
    }

    return p_user->second->_userAccount.getUserId();
}

bool StorageServer::getUserAccount( unsigned int userID, int sessionID, UserAccount& account )
{
    if ( !_connectionEstablished )
        return false;

    std::map< int, UserState* >::iterator p_user = _userCache.find( sessionID );
    if ( p_user == _userCache.end() )
    {
        log_warning << "*** StorageServer: request for user account cannot be processed, invalid session ID " << sessionID << std::endl;
        return false;
    }

    if ( p_user->second->_userAccount._userID != userID )
    {
        log_warning << "*** StorageServer: request for user account cannot be processed, user / session ID mismatch!" << std::endl;
        return false;
    }

    account = p_user->second->_userAccount;

    return true;
}

bool StorageServer::updateUserAccount( unsigned int userID, int sessionID, const UserAccount& account )
{
    if ( !_connectionEstablished )
        return false;

    std::map< int, UserState* >::iterator p_user = _userCache.find( sessionID );
    if ( p_user == _userCache.end() )
    {
        log_warning << "*** StorageServer: request for user account update cannot be processed, invalid session ID " << sessionID << std::endl;
        return false;
    }

    if ( p_user->second->_userAccount._userID != userID )
    {
        log_warning << "*** StorageServer: request for user account update cannot be processed, user / session ID mismatch!" << std::endl;
        return false;
    }

    // update the cache
    p_user->second->_userAccount._userDescription = account._userDescription;

    // update the account in database
    _p_storage->updateUserAccount( account );

    return true;
}

bool StorageServer::getPublicUserAccountInfo( UserAccount& account )
{
    if ( !_connectionEstablished )
        return false;

    return _p_storage->getPublicUserAccountInfo( account );
}

UserInventory* StorageServer::getUserInventory( unsigned int userID, int sessionID )
{
    if ( !_connectionEstablished )
        return NULL;

    std::map< int, UserState* >::iterator p_user = _userCache.find( sessionID );
    if ( p_user == _userCache.end() )
    {
        log_warning << "*** StorageServer: request for user inventory cannot be processed; user / session ID mismatch " << sessionID << std::endl;
        return NULL;
    }

    if ( !p_user->second->_p_userInventory->isCached() )
    {
        // get user inventory
        if ( !_p_storage->getUserInventory( userID, p_user->second->_p_userInventory ) )
        {
            log_error << "*** StorageServer: could not retrieve user inventory, user ID " << userID << std::endl;
            return NULL;
        }

        p_user->second->_p_userInventory->setCached( true );
    }

    return p_user->second->_p_userInventory;
}

} // namespace vrc
