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
 # networking for chat member
 #
 # this class implements the networking functionality for chat member
 #
 #
 #   date of creation:  12/25/2004
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  02/20/2005 boto       creation of PlayerNetworking
 #
 ################################################################*/

#ifndef _VRC_PLAYERNETWORKING_H_
#define _VRC_PLAYERNETWORKING_H_

#include <vrc_main.h>
#include "networkingRoles/_RO_PlayerNetworking.h"

namespace vrc
{
    class EnPlayer;
    class yaf3d::BaseEntity;
    class BasePlayerImplementation;
}

class PlayerNetworking : _RO_DO_PUBLIC_RO( PlayerNetworking )
{

    public:

        explicit                                   PlayerNetworking( vrc::BasePlayerImplementation* p_playerImp = NULL );

        virtual                                    ~PlayerNetworking();

        //! Initialize
        void                                        initialize(
                                                                const osg::Vec3f&  pos,
                                                                const std::string& playerName,
                                                                const std::string& cfgFile
                                                              );

        //! Returns true if the networking is for local client
        bool                                        isRemoteClient() const { return _remoteClient; }

        //! Change the player name
        void                                        updatePlayerName( const std::string& name );

        //! Update position in network, use this for client
        void                                        updatePosition( float x, float y, float z );

        //! Update rotation in network, use this for client
        void                                        updateRotation( float yaw );

        //! Get position, use this for remote clients
        void                                        getPosition( float& x, float& y, float& z );

        //! Get rotation, use this for remote clients
        void                                        getRotation( float& yaw );

        //! Update animation flags
        void                                        updateAnimationFlags( unsigned char cmdFlag );

        //! Get animation flags
        unsigned char                               getAnimationFlags();

        //! Put new chat text
        void                                        putChatText( const CEGUI::String& text );

        //! Enable / disable voice chat, use only for local client
        void                                        enableVoiceChat( bool en );

        //! Is voice chat enabled? This can be used on local and remote clients.
        bool                                        isEnabledVoiceChat();


        // Internal RN Overrides, do not use these methods!
        //-----------------------------------------------------------------------------------//
        //! Object can now be initialized in scene
        void                                        PostObjectCreate();

        //! Hook into player name changes
        void                                        DataBlockPacketDataReceived( const RNReplicaNet::DataBlock* p_datablock );

        //! Granting function called by server on new connected client for getting initial data etc.
        void                                        RPC_ServerGrantsAccess( tInitializationData initData );

        //! Request initialization data from server, used by remote clients.
        void                                        RPC_RequestInitialization();

        //! Initialization function called on a new connected client and its ghosts
        void                                        RPC_Initialize( tInitializationData initData );

        //! Enable / disable voice chat
        void                                        RPC_EnableVoiceChat( bool en );

    protected:

        //! Create all player related entities
        void                                        createPlayer();

        // network-shared attributes
        float                                       _positionX;

        float                                       _positionY;

        float                                       _positionZ;

        float                                       _yaw;

        unsigned char                               _cmdAnimFlags;

        char                                        _p_playerName[ 32 ];

        char                                        _p_configFile[ 64 ];

        bool                                        _voiceChat;

        char                                        _ip[ 24 ];

        //-------

        bool                                        _voiceChatEnable;

        bool                                        _remoteClient;

        bool                                        _remoteClientInitialized;

        vrc::BasePlayerImplementation*              _p_playerImpl;

        std::vector< yaf3d::BaseEntity* >           _loadedEntities;

        vrc::EnPlayer*                              _loadedPlayerEntity;

    friend class _MAKE_RO( PlayerNetworking );
};


#endif //_VRC_PLAYERNETWORKING_H_
