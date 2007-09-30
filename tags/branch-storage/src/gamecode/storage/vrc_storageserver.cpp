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


//! Implement the singleton
YAF3D_SINGLETON_IMPL( vrc::StorageServer )


namespace vrc
{

StorageServer::StorageServer() :
 _p_networking( NULL )
{
}

StorageServer::~StorageServer()
{
    if ( _p_networking )
        delete _p_networking;
}

void StorageServer::shutdown()
{
    // destroy the singleton
    destroy();
}

void StorageServer::initialize() throw ( StorageServerException )
{
    // this is created only on server
    assert( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server );

    if ( _p_networking )
        throw StorageServerException( "Storage server is already initialized." );

    log_info << "  setup storage ..." << std::endl;

    // create and publish the storage network object
    _p_networking = new StorageNetworking();
    _p_networking->Publish();

    // set the authentification callback in network device, whenever a client connects then 'authentify' is called.
    yaf3d::NetworkDevice::get()->setAuthCallback( this );


    //! TODO: setup the sql stuff


    log_info << "  storage successfully initialized" << std::endl;
}

bool StorageServer::authentify( const std::string& login, const std::string& passwd, unsigned int& userID )
{
    assert ( _p_networking && "storage server is not initialized!" );

    // guest login
    if ( !login.length() )
    {
        userID = static_cast< unsigned int >( -1 );
        return true;
    }

    //! TODO: request the account manager for auth!

    // test account
    if ( ( login == std::string( "guest" ) ) && ( passwd == std::string( "1234" ) ) )
    {
        userID = static_cast< unsigned int >( 100 );
        return true;
    }

    return false;
}


} // namespace vrc
