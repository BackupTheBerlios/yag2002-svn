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
 #  12/25/2004 boto       creation of CTD_MemberNw
 #
 ################################################################*/

#ifndef _CTD_MEMBERNW_H_
#define _CTD_MEMBERNW_H_

#include <ctd_framework.h>
#include <ctd_network.h>
#include "networkingRoles/_RO_PlayerNetworking.h"
#include "ctd_player.h"

//typedef struct _tChatMsg 
//{
//    char    m_pcText[ 256 ];
//} tChatMsg;

class CTD_MemberNw : _RO_DO_PUBLIC_RO( CTD_MemberNw )
{

    public:

                                                    CTD_MemberNw( CTD::Player* pkMember = NULL );
                                                        
        virtual                                     ~CTD_MemberNw();

        //! Initialize
        void                                        Initialize(             
                                                                const Ogre::Vector3& kPos,
                                                                const std::string& strPlayerName,
                                                                const std::string& strMeshFileName
                                                              );

        //! Update distributed object over network, call this in every game loop step
        void                                        Update();

        //! Returns true if the networking is for local client
        bool                                        IsRemoteClient() { return m_bIsRemoteClient; }

        //! Update position in network, use this for client
        void                                        UpdatePosition( float fPosX, float fPosY, float fPosZ );

        //! Update rotation in network, use this for client
        void                                        UpdateRotation( float fYaw );

        //! Get position, use this for remote clients
        void                                        GetPosition( float& fPosX, float& fPosY, float& fPosZ );

        //! Get rotation, use this for remote clients
        void                                        GetRotation( float& fYaw );

        //! Update animation flags
        void                                        UpdateAnimationFlags( unsigned int uiCmdFlag );

        //! Get animation flags
        void                                        GetAnimationFlags( unsigned int& uiCmdFlag );

        //! Put new chat text
        void                                        PutChatText( const std::string& strText );

        // Overrides
        //-----------------------------------------------------------------------------------//
        //! Object can now be initialized in scene
        void                                        PostObjectCreate();

        // RPCs
        //-----------------------------------------------------------------------------------//
        //! New incomming chat text
        void                                        RPC_AddChatText( tChatMsg pcText );

    protected:

        float                                       m_fPositionX;
        float                                       m_fPositionY;
        float                                       m_fPositionZ;

        float                                       m_fYaw;

        unsigned int                                m_uiCmdAnimFlags;

        bool                                        m_bIsRemoteClient;

        CTD::Player                                *m_pkMember;

        char                                        m_pcPlayerName[ 32 ];

        char                                        m_pcAnimFileName[ 64 ];

    friend class _RO_CTD_MemberNw;

};


#endif //_CTD_MEMBERNW_H_
