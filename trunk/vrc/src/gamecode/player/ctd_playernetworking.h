/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2002-2004, Ali Botorabi
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
 # networking for chat member
 #
 # this class implements the networking functionality for chat member
 #
 #
 #   date of creation:  12/25/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  02/20/2005 boto       creation of PlayerNetworking
 #
 ################################################################*/

#ifndef _CTD_PLAYERNETWORKING_H_
#define _CTD_PLAYERNETWORKING_H_

#include <ctd_base.h>
#include <ctd_network.h>
#include "networkingRoles/_RO_PlayerNetworking.h"

//typedef struct _tChatMsg 
//{
//    char    m_pcText[ 256 ];
//} tChatMsg;

namespace CTD
{
    class BaseEntity;
}

class PlayerNetworking : _RO_DO_PUBLIC_RO( PlayerNetworking )
{

    public:

                                                    PlayerNetworking( CTD::BaseEntity* p_player = NULL );
                                                        
        virtual                                     ~PlayerNetworking();

        //! Initialize
        void                                        initialize(             
                                                                const osg::Vec3f&  pos,
                                                                const std::string& playerName,
                                                                const std::string& meshFileName
                                                              );

        //! Update distributed object over network, call this in every game loop step
        void                                        update();

        //! Returns true if the networking is for local client
        bool                                        isRemoteClient() { return _remoteClient; }

        //! Update position in network, use this for client
        void                                        updatePosition( float x, float y, float z );

        //! Update rotation in network, use this for client
        void                                        updateRotation( float yaw );

        //! Get position, use this for remote clients
        void                                        getPosition( float& x, float& y, float& z );

        //! Get rotation, use this for remote clients
        void                                        getRotation( float& yaw );

        //! Update animation flags
        void                                        updateAnimationFlags( unsigned int cmdFlag );

        //! Get animation flags
        void                                        getAnimationFlags( unsigned int& cmdFlag );

        //! Put new chat text
        void                                        putChatText( const std::string& text );

        // Overrides
        //-----------------------------------------------------------------------------------//
        //! Object can now be initialized in scene
        void                                        PostObjectCreate();

        // RPCs
        //-----------------------------------------------------------------------------------//
        //! New incomming chat text
        void                                        RPC_AddChatText( tChatMsg chatMsg );

    protected:

        float                                       _positionX;
        float                                       _positionY;
        float                                       _positionZ;

        float                                       _yaw;

        unsigned int                                _cmdAnimFlags;

        bool                                        _remoteClient;

        CTD::BaseEntity*                            _p_member;

        char                                        _p_playerName[ 32 ];

        char                                        _p_animFileName[ 64 ];

    friend class _MAKE_RO( PlayerNetworking );
};


#endif //_CTD_PLAYERNETWORKING_H_