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

#include <ctd_frbase.h>
#include "ctd_membernw.h"
#include "ctd_player.h"

using namespace std;
using namespace CTD;
using namespace Ogre;


CTD_MemberNw::CTD_MemberNw( Player* pkMember )
{
    
    //m_bIsRemoteClient = IsMaster();
    m_bIsRemoteClient = false;
    if ( !pkMember ) {

        m_bIsRemoteClient = true;

        // create new entity for remote client
        pkMember = new Player;
//        pkMember->SetPlayerNetworking( this );
        pkMember->Initialize();

        m_pcPlayerName[ 0 ]   = 0;
        m_pcAnimFileName[ 0 ] = 0;

    }
    m_uiCmdAnimFlags = 0;
    m_pkMember       = pkMember;

}

CTD_MemberNw::~CTD_MemberNw()
{

    // remove ghost from simulation ( server and client )
    if ( IsRemoteClient() ) {
    
        Framework::Get()->RemoveEntiy( ( CTD::BaseEntity* )m_pkMember );
        delete m_pkMember;

    }

}

void CTD_MemberNw::PostObjectCreate()
{

    if ( IsRemoteClient() && Framework::Get()->GetGameMode() == stateCLIENT ) {

//        m_pkMember->SetPlayerName( m_pcPlayerName );
//        m_pkMember->SetAnimConfig( m_pcAnimFileName );

        // integrate the entity into scene     
//        assert ( Framework::Get()->AddEntiy( GetLevelSet(), ( CTD::BaseEntity* )m_pkMember ) == true );
        m_pkMember->PostInitialize();
    
    }

}

void CTD_MemberNw::Initialize( const Ogre::Vector3& kPos, const string& strPlayerName, const string& strMeshFileName )
{

    m_fPositionX = kPos.x;
    m_fPositionY = kPos.y;
    m_fPositionZ = kPos.z;
    strcpy( m_pcPlayerName, strPlayerName.c_str() );
    strcpy( m_pcAnimFileName, strMeshFileName.c_str() );

}

void CTD_MemberNw::PutChatText( const std::string& strText )
{

    static tChatMsg s_pcTextBuffer;
    strcpy( s_pcTextBuffer.m_pcText, strText.c_str() );
    ALL_REPLICAS_FUNCTION_CALL( RPC_AddChatText( s_pcTextBuffer ) );
}

void CTD_MemberNw::RPC_AddChatText( tChatMsg pcText )
{
    // a server could direct all messages into a log file!
    if ( Framework::Get()->GetGameMode() == stateCLIENT ) {

        pcText.m_pcText[ 255 ] = 0;
//        m_pkMember->PostChatText( pcText.m_pcText );

    }
}

void CTD_MemberNw::Update()
{
}

void CTD_MemberNw::UpdateAnimationFlags( unsigned int uiCmdFlag )
{
    m_uiCmdAnimFlags = uiCmdFlag;
}

void CTD_MemberNw::GetAnimationFlags( unsigned int& uiCmdFlag )
{
    uiCmdFlag = m_uiCmdAnimFlags;
}

void CTD_MemberNw::UpdatePosition( float fPosX, float fPosY, float fPosZ )
{

    m_fPositionX = fPosX;
    m_fPositionY = fPosY;
    m_fPositionZ = fPosZ;

}

void CTD_MemberNw::GetPosition( float& fPosX, float& fPosY, float& fPosZ )
{

    fPosX = m_fPositionX;
    fPosY = m_fPositionY;
    fPosZ = m_fPositionZ;

}

void CTD_MemberNw::UpdateRotation( float fYaw )
{

    m_fYaw = fYaw;

}

void CTD_MemberNw::GetRotation( float& fYaw )
{

    fYaw = m_fYaw;

}
