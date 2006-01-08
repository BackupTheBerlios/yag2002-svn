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
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
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

//! Network protocol version, use getProtocolVersionAsString to convert it to a string
// Format: 0 x 00 - Current - Revision - Age
#define YAF3D_NETWORK_PROT_VERSION      0x00010002
//! Helper function for converting the protocol version to a string
std::string getProtocolVersionAsString( unsigned int version );

namespace yaf3d
{

class Application;
class NetworkDevice;

//! Network node information
class NodeInfo
{
    public:
                                                    NodeInfo() :
                                                     _protocolVersion( YAF3D_NETWORK_PROT_VERSION )
                                                    {}

                                                    NodeInfo( const std::string& levelname, const std::string& nodename ) :
                                                     _levelName( levelname ),
                                                     _nodeName( nodename ),
                                                     _protocolVersion( YAF3D_NETWORK_PROT_VERSION )
                                                    {}

        /**
        * Get level name, this is relevant for clients
        * \return                                       Level name
        */
        const std::string&                          getLevelName() const { return _levelName; }

        /**
        * Get Node name, for server it is the connecting client name, for client it is the server name
        * \return                               Node name
        */
        const std::string&                          getNodeName() const { return _nodeName; }

    protected:

        //! Level name
        std::string                                 _levelName;

        //! Node name
        std::string                                 _nodeName;

        //! Network protocol version
        unsigned int                                _protocolVersion;

    friend  class NetworkDevice;
    friend  class Application;
};

//! Class for registering a callback in order to get notification when clients join / leave the network session.
class SessionNotifyCallback
{
    public:

                                                    SessionNotifyCallback() {}

        virtual                                     ~SessionNotifyCallback() {}

        //! Override this method for getting notification when a client joins to the network
        virtual void                                onSessionJoined( int sessionID ) {}

        //! Override this method for getting notification when a client leaves the network
        virtual void                                onSessionLeft( int sessionID ) {}

        //! Override this method for getting notification when disconnected from server ( server shutdown, network problems )
        virtual void                                onServerDisconnect( int sessionID ) {}
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

        //! Number of joined sessions
        int                                         _numSessions;

        //! A list of joined sessions ids
        std::vector< int >                          _sessionIDs;

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
        typedef enum NetworkingMode
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
        * Setup a client session joining to a server ( throws exception )
        * \param ServerIp                           Server IP address
        * \param channel                            Channel
        * \param nodeInfo                           Client information
        */
        void                                        setupClient( const std::string& serverIp, int channel, const NodeInfo& nodeInfo ) throw ( NetworkException );

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
        * Get server's / client's node info, call this after a a session is established.
        * \return                                   Node information. NULL if the session is not stable.
        */
        NodeInfo*                                   getNodeInfo();

        /**
        * Get the session ID, call this after a successful connection
        * \return                                   Session ID
        */
        int                                         getSessionID();

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

        //! Server's / client's node information
        NodeInfo                                    _nodeInfo;

        //! Flag for indicating a stable client session
        bool                                        _clientSessionStable;

        //! Flag for indicating a stable server session
        bool                                        _serverSessionStable;

    friend class Singleton< NetworkDevice >;
    friend class Application;
};

} // namespace yaf3d

#endif //_NETWORK_H_
