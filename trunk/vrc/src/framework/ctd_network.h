/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2004-2006, Ali Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU General Public License 
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public 
 *  License along with this program; if not, write to the Free 
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 * 
 ****************************************************************/

/*###############################################################
 # networking core
 #
 # this core uses RakNet and provides networking services
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
 #  04/08/2004 boto       creation of networking core
 #
 #  09/24/2004 boto       redesigned
 #
 #  02/20/2005 boto       adapted to new environment (osg)
 #
 ################################################################*/

#ifndef _CTD_NETWORK_H_
#define _CTD_NETWORK_H_

#include <RNReplicaNet/Inc/ReplicaNet.h>
#include <RNPlatform/Inc/Threadclass.h>
#include <RNPlatform/Inc/PlatformHeap.h>
#include <RNXPURL/Inc/XPURL.h>

#include "ctd_singleton.h"
#include "ctd_network_defs.h"

namespace CTD
{

class Application;
class NetworkDevice;

//! Server information
class NodeInfo
{

    public:

        /**
        * Get level name, this is relevant for clients
        * \return                                       Level name
        */
        const std::string&                              getLevelName() { return _levelName; }

        /**
        * Get Node name, for server it is the connecting client name, for client it is the server name
        * \return                               Node name
        */
        const std::string&                              getNodeName() { return _nodeName; }

    protected:

        //! Level name
        std::string                                     _levelName;

        // Node name
        std::string                                     _nodeName;

    friend  class NetworkDevice;
    friend  class Application;

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
        * Get networking mode: NONE, CLIENT, SERVER
        */
        NetworkingMode                  getMode() { return _mode; }

        /**
        * Get server's / client's node info, call this after a a session is established.
        * \return                       Node information. NULL if the session is not stable.
        */
        NodeInfo*                       getNodeInfo();

    protected:

        /**
        * Construction
        */
                                        NetworkDevice();
        /**
        * Avoid the usage of copy constructor.
        */
                                        NetworkDevice( NetworkDevice &kNetWorkDevice );

        /**
        * Avoid assigning this singleton
        */
        NetworkDevice&                  operator = ( const NetworkDevice& );

        /**
        * Destruction
        */
        virtual                        ~NetworkDevice();

        /**
        * Shutdown the network device.
        */
        void                            shutdown();

        /**
        * Setup a server session
        * \param channel                Channel
        * \param nodeInfo               Server information such as server and level name
        * \return                       true if successful
        */
        bool                            setupServer( int channel, const NodeInfo& nodeInfo );

        /**
        * Setup a client session joining to a server
        * \param URL                    Server URL
        * \param channel                Channel
        * \param nodeInfo               Client information
        * \return                       true if successful
        */
        bool                            setupClient( const std::string& URL, int channel, const NodeInfo& nodeInfo );

        /**
        * Start the client processing. Call this after SetupClient.
        * \return                       true if successfully started.
        */
        bool                            startClient();

        /**
        * Update server session
        * \param deltaTime              Time past since last update
        */
        void                            updateServer( float deltaTime );

        /**
        * Update client session
        * \param deltaTime              Time past since last update
        */
        void                            updateClient( float deltaTime );

        /** 
        * Lock object creation and deletion
        */
        void                            lockObjects();

        /**
        * Unlock object creation and deletion
        */
        void                            unlockObjects();

        //! Mode
        NetworkingMode                  _mode;

        //! Session instance
        RNReplicaNet::ReplicaNet*        _p_session;

        //! Server's / client's node information
        NodeInfo                        _nodeInfo;

        //! Flag for indicating a stable client session
        bool                            _clientSessionStable;

        //! Flag for indicating a stable server session
        bool                            _serverSessionStable;

    friend class Singleton< NetworkDevice >;
};

} // namespace CTD

#endif //_CTD_NETWORK_H_
