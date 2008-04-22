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
 # player implementation for client mode
 #
 #   date of creation:  05/31/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  05/31/2005 boto       creation of PlayerImplClient
 #
 ################################################################*/

#ifndef _VRC_PLAYERIMPLCLIENT_H_
#define _VRC_PLAYERIMPLCLIENT_H_

#include <vrc_main.h>
#include "vrc_playerimpl.h"

//! Class declaration for player networking
class PlayerNetworking;

namespace vrc
{

template< class PlayerImplT > class PlayerIHCharacterCameraCtrl;

//! Player implementation for game mode Client ( see framework class yaf3d::GameState )
class PlayerImplClient : public BasePlayerImplementation, public yaf3d::SessionNotifyCallback
{
    public:

        explicit                                    PlayerImplClient( EnPlayer* player );

        virtual                                     ~PlayerImplClient();

        //! Initialize
        void                                        initialize();

        //! Post-initialize
        void                                        postInitialize();

        //! Update
        void                                        update( float deltaTime );

        //! Implementation's notification callback
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        //! Networking callback for getting notified when server dicsonnection occures because of network problems or server shutdown
        void                                        onServerDisconnect( int sessionID );

        //! Overriden method for enable / disable voice chat; the client implementation supports this feature;
        void                                        enableVoiceChat( bool en );

    protected:

        //! Get the configuration settings
        void                                        getConfiguration();

        //! Set the networking status, used by networking class.
        void                                        setNetworkInitialized( bool con );

        //! Flag indicating that the player is connected to server and has received its initial data
        bool                                        _isNwInitialized;

        //! Indicated remote / local client
        bool                                        _isRemoteClient;

        //! Input handler
        PlayerIHCharacterCameraCtrl< PlayerImplClient >* _p_inputHandler;

    friend class PlayerIHCharacterCameraCtrl< PlayerImplClient >;
    friend class ::PlayerNetworking;
};

} // namespace vrc

#endif // _VRC_PLAYERIMPLCLIENT_H_
