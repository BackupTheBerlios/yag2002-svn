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

#ifndef _VRC_STORAGESERVER_H_
#define _VRC_STORAGESERVER_H_

#include <vrc_main.h>
#include "vrc_storagenetworking.h"
#include "vrc_userinventory.h"
#include "database/vrc_account.h"

namespace vrc
{

//! Class declarations
class StorageNetworking;
class BaseStorage;
class Account;

namespace gameutils
{
    class VRCStateHandler;
}


//! Class for storage server related exceptions
class StorageServerException : public std::runtime_error
{
    public:
                                                    StorageServerException( const std::string& reason ) :
                                                     std::runtime_error( reason )
                                                    {
                                                    }

        virtual                                     ~StorageServerException() throw() {}

                                                    StorageServerException( const StorageServerException& e ) :
                                                     std::runtime_error( e )
                                                    {
                                                    }

    protected:

                                                    StorageServerException();

        StorageServerException&                     operator = ( const StorageServerException& );
};


//! Storage server
class StorageServer : public yaf3d::Singleton< vrc::StorageServer >, public yaf3d::CallbackAuthentification, public yaf3d::SessionNotifyCallback
{
    public:

        //! Returns true if the user ID and session ID match.
        bool                                        validateClient( unsigned int userID, int sessionID );

        //! Get the user account information. The userID and sessionID must both match for any network connection!
        bool                                        getUserAccount( unsigned int userID, int sessionID, UserAccount* p_account );

        //! Get the inventory of given user. The userID and sessionID must both match for any network connection! The inventory is stored in p_inv.
        bool                                        getUserInventory( unsigned int userID, int sessionID, UserInventory* p_inv );

    protected:

                                                    StorageServer();

        virtual                                     ~StorageServer();

        //! Initialize the storage server
        void                                        initialize() throw ( StorageServerException );

        //! Shutdown the storage server
        void                                        shutdown();

        //! Override of networking's CallbackAuthentification method for authentification when a client connects
        bool                                        authentify( int sessionID, const std::string& login, const std::string& passwd, unsigned int& userID );

        //! Override of networking's CallbackAuthentification method for registeration of a new user. Returns false if the nickname already exists.
        bool                                        registerUser( const std::string& name, const std::string& nickname, const std::string& passwd, const std::string& email );

        //! Override of method for getting notification when a client leaves the network
        void                                        onSessionLeft( int sessionID );

        //! Flag indicating that a connection to storage data base has been established
        bool                                        _connectionEstablished;

        //! Networking for storage
        StorageNetworking*                          _p_networking;

        //! Data base storage
        BaseStorage*                                _p_storage;

        //! Struct used for user cache
        class UserState
        {
            public:
                                                UserState() :
                                                 _sessionID( -1 ),
                                                 _p_userInventory( NULL )
                                                {
                                                }

                                                ~UserState()
                                                {
                                                    if ( _p_userInventory )
                                                        delete _p_userInventory;
                                                }

                //! Client session ID
                int                             _sessionID;

                //! User accout
                UserAccount                     _userAccount;

                //! User data containing the references to inventory, mailbox, skills, etc.
                UserData                        _userData;

                //! User inventory
                UserInventory*                  _p_userInventory;
        };

        //! User cache < session ID, user state >
        std::map< int, UserState* >             _userCache;

    friend class StorageNetworking;
    friend class gameutils::VRCStateHandler;
    friend class yaf3d::Singleton< vrc::StorageServer >;
};

} // namespace vrc

#endif // _VRC_STORAGESERVER_H_
