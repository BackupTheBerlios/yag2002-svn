/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2006, A. Botorabi
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
 # networking core
 #
 # this core uses ReplicaNet and provides networking services
 #
 #
 #   date of creation:  04/08/2004
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  04/08/2004 boto       creation of networking core basing on RakNet
 #
 #  09/24/2004 boto       redesigned
 #
 #  02/20/2005 boto       adapted to new environment (osg, ReplicaNet)
 #
 ################################################################*/

#ifndef _NETWORK_H_
#define _NETWORK_H_

#include <RNReplicaNet/Inc/ReplicaNet.h>
#include <RNPlatform/Inc/ThreadClass.h>
#include <RNPlatform/Inc/PlatformHeap.h>
#include <RNXPURL/Inc/XPURL.h>

#include "singleton.h"
#include "network_defs.h"


namespace yaf3d
{

//! Network protocol version, use getProtocolVersionAsString to convert it to a string
// Format: 0 x 00 - Current - Revision - Age
#define YAF3D_NETWORK_PROT_VERSION      0x00010400
//! Helper function for converting the protocol version to a string
std::string getProtocolVersionAsString( unsigned int version );

//! Given a host return its IP address ( the host can also be an IP )
std::string resolveHostName( const std::string& host );


class Application;
class NetworkDevice;

//! Network node information
class NodeInfo
{
    public:
                                                    NodeInfo() :
                                                     _needAuthenfication( false ),
                                                     _accessGranted( false ),
                                                     _protocolVersion( YAF3D_NETWORK_PROT_VERSION )
                                                    {}

                                                    NodeInfo( const std::string& levelname, const std::string& nodename ) :
                                                     _levelName( levelname ),
                                                     _nodeName( nodename ),
                                                     _needAuthenfication( false ),
                                                     _accessGranted( false ),
                                                     _protocolVersion( YAF3D_NETWORK_PROT_VERSION )
                                                    {}

        /**
        * Get level name, this is relevant for clients
        * \return                                   Level name
        */
        const std::string&                          getLevelName() const { return _levelName; }

        /**
        * Get Node name, for server it is the connecting client name, for client it is the server name
        * \return                                   Node name
        */
        const std::string&                          getNodeName() const { return _nodeName; }

        /**
        * Should the server request connecting clients for authentification?
        * \needAuth                                 If true, then the server requests clients for authentification.
        */
        void                                        setNeedAuthentification( bool needAuth ) { _needAuthenfication = needAuth; }

        /**
        * Connection to server needs an authentification. Use this method on client.
        * \return                                   If true then the server requests for authentification.
        */
        bool                                        needAuthentification() const { return _needAuthenfication; }

        /**
        * Returns true if server grants access, otherwise false. Use this method on client.
        */
        bool                                        getAccessGranted() const { return _accessGranted; }

    protected:

        //! Level name
        std::string                                 _levelName;

        //! Node name
        std::string                                 _nodeName;

        //! Server requests clients for authentification
        bool                                        _needAuthenfication;

        //! True if the server grants access, otherwise false.
        bool                                        _accessGranted;

        //! Network protocol version
        unsigned int                                _protocolVersion;

    friend  class NetworkDevice;
    friend  class Application;
};

//! Base callback class used during authentification when a client connects to the server.
/* This callback is used on server and client. It is also used for register new users. */
class CallbackAuthentification
{
    public:

	    virtual                                    ~CallbackAuthentification() {}

        /**
        * This method is called on server when a client requests for authentification on connecting.
        * \sessionID                                Unique session ID of connecting client.
        * \login                                    Login name
        * \passwd                                   Password
        * \userID                                   User's ID which is also sent to client.
        * \return                                   True if the authentification was succesfull, then the userID has a valid value, otherwise false.
        */
        virtual bool                                authentify( int /*sessionID*/, const std::string& /*login*/, const std::string& /*passwd*/, unsigned int& /*userID*/ ) { return false; }

        /**
        * Register a new user.
        * \name                                     Real name
        * \login                                    Login name
        * \passwd                                   Password
        * \email                                    E-Mail address
        * \return                                   Return false if the registration went wrong.
        */
        virtual bool                                registerUser( const std::string& /*name*/, const std::string& /*login*/, const std::string& /*passwd*/, const std::string& /*email*/ ) { return false; }

        /**
        * This method is called on client when successfully authentified on server.
        * \userID                                   User's unique ID.
        */
        virtual void                                authentificationResult( unsigned int /*userID*/ ) {}
};

//! Class for registering a callback in order to get notification when clients join / leave the network session.
class SessionNotifyCallback
{
    public:

                                                    SessionNotifyCallback() {}

        virtual                                     ~SessionNotifyCallback() {}

        //! Override this method for getting notification when a client joins to the network
        virtual void                                onSessionJoined( int /*sessionID*/ ) {}

        //! Override this method for getting notification when a client leaves the network
        virtual void                                onSessionLeft( int /*sessionID*/ ) {}

        //! Override this method for getting notification when disconnected from server ( server shutdown, network problems )
        virtual void                                onServerDisconnect( int /*sessionID*/ ) {}
};

//! Derived class from RNReplicaNet::ReplicaNet, this class allows tracking of joining / leaving sessions
class Networking: public RNReplicaNet::ReplicaNet
{
    public:

                                                    Networking();

        virtual                                     ~Networking();

        //! Use this method in order to register a callback object in order to get notification when a client joins to or leaves the network
        void                                        registerSessionNotify( SessionNotifyCallback* p_cb );

        //! Deregister the callback object
        void                                        deregisterSessionNotify( SessionNotifyCallback* p_cb );

        //! Get all currently active session IDs
        void                                        getSessionIDs( std::vector< int >& ids );

    protected:

        //! Overridden method for getting notified when a session joined
        void                                        JoinerSessionIDPost( const int sessionID );

        //! Overridden method for getting notified when a session leaves
        void                                        LeaverSessionIDPost( const int sessionID );

        //! Overridden method for creating custom session IDs.
        virtual int                                 CallbackGetSessionID();

        //! Number of joined sessions
        int                                         _numSessions;

        //! A list of joined sessions ids
        std::vector< int >                          _sessionIDs;

        //! Cache used for generating unique session IDs
        std::set < int >                            _sessionIDCache;

        std::vector< SessionNotifyCallback* >       _sessionCallbacks;

        RNReplicaNet::MutexClass                    _mutex;
};

//! Class for network related exceptions
class NetworkException : public std::runtime_error
{
    public:
                                                    NetworkException( const std::string& reason ) :
                                                     std::runtime_error( reason )
                                                    {
                                                    }

        virtual                                     ~NetworkException() throw() {}

                                                    NetworkException( const NetworkException& e ) :
                                                     std::runtime_error( e )
                                                    {
                                                    }

    protected:

                                                    NetworkException();

        NetworkException&                           operator = ( const NetworkException& );
};

//! Networking device
class NetworkDevice : public Singleton< NetworkDevice >
{
    public:

        //! Networking modes
        enum NetworkingMode
        {
            NONE = 0x0,
            CLIENT,
            SERVER
        };

        /**
        * Setup a server session ( throws exception )
        * \param channel                            Channel
        * \param nodeInfo                           Server information such as server and level name
        */
        void                                        setupServer( int channel, const NodeInfo& nodeInfo ) throw ( NetworkException );

        /**
        * Set the authentification callback.
        * This callback is used on server when a client connects to server and requests for authentification.
        * It is also used on client to get the unique user ID when successfully identified.
        * \p_cb                                     Callback object
        */
        void                                        setAuthCallback( CallbackAuthentification* p_cb );

        /**
        * Setup a client session joining to a server ( throws exception ). It is also used for authentification and registration of a user.
        * \param ServerIp                           Server IP address
        * \param channel                            Channel
        * \param nodeInfo                           Client information and some information retrieved from server
        * \param login                              Login name, needed only if server needs authentification. Fill it for a registration request.
        * \param passwd                             Password, needed only if server needs authentification. Fill it for a registration request.
        * \param reguser                            Set to true if an user registration should be processed.
        * \param name                               Real name, fill it for a registration request.
        * \param email                              E-Mail address, fill it for a registration request.
        */
        void                                        setupClient( const std::string& serverIp, int channel, NodeInfo& nodeInfo, const std::string& login = "", const std::string& passwd = "", bool reguser = false, const std::string& name = "", const std::string& email = "" ) throw ( NetworkException );

        /**
        * Start the client processing. Call this after SetupClient ( throws exception ).
        */
        void                                        startClient() throw ( NetworkException );

        /**
        * Use this method in order to register a callback object in order to get notification when a client joins to or leaves the network.
        * \param p_cb                               Callback object
        */
        void                                        registerSessionNotify( SessionNotifyCallback* p_cb );

        /**
        * Deregister a previously registered callback object to getting notification when a client joins to or leaves the network..
        * \param p_cb                               Callback object
        */
        void                                        deregisterSessionNotify( SessionNotifyCallback* p_cb );

        /**
        * Get all currently active session IDs.
        * \param ids                                List of IDs which will be filled by this method.
        */
        void                                        getSessionIDs( std::vector< int >& ids );


        /**
        * Disconnect any connection and reset the networking states.
        */
        void                                        disconnect();

        /**
        * Get networking mode: NONE, CLIENT, SERVER
        */
        NetworkingMode                              getMode() const { return _mode; }

        /**
        * Get the local session ID, call this after a successful connection.
        * \return                                   Session ID
        */
        int                                         getSessionID();

        /**
        * Get IP address of given client session ID. Use this on server only.
        * \return                                   IP if a valid connection exists, otherwise empty string.
        */
        std::string                                 getClientIP( int sessionID );

        /**
        * Get server IP, this can be used in clients. Valid only after a successful connection to a server.
        * \return                                   Server IP
        */
        const std::string&                          getServerIP();

        /**
        * Get all replicated objects
        * \param objs                               List of objects
        */
        void                                        getObjects( std::vector< RNReplicaNet::ReplicaObject* >& objs );

        /**
        * Lock object creation and deletion
        */
        void                                        lockObjects();

        /**
        * Unlock object creation and deletion
        */
        void                                        unlockObjects();

    protected:

                                                    NetworkDevice();

        virtual                                    ~NetworkDevice();

        /**
        * Shutdown the network device.
        */
        void                                        shutdown();

        /**
        * Update server session
        * \param deltaTime                          Time past since last update
        */
        void                                        updateServer( float deltaTime );

        /**
        * Update client session
        * \param deltaTime                          Time past since last update
        */
        void                                        updateClient( float deltaTime );

        //! Mode
        NetworkingMode                              _mode;

        //! Session instance
        Networking*                                 _p_session;

        //! Server's node information
        NodeInfo                                    _serverNodeInfo;

        //! Flag for indicating a stable client session
        bool                                        _clientSessionStable;

        //! Flag for indicating a stable server session
        bool                                        _serverSessionStable;

        //! Server IP which can be used in clients, valid after a successful connection to a server
        std::string                                 _serverIP;

        //! Authentification callback used on server
        CallbackAuthentification*                   _p_cbAuthentification;

    friend class Singleton< NetworkDevice >;
    friend class Application;
};

} // namespace yaf3d

#endif //_NETWORK_H_
