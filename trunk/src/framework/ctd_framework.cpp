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
 # neoengine, framework interface class
 #
 # this interface is used by game plugins
 #
 #
 #   date of creation:  09/24/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 #
 ################################################################*/

#include <ctd_frbase.h>
#include "ctd_framework.h"
#include "ctd_framework_impl.h"
#include <ctd_network.h>

using namespace std;
using namespace NeoEngine;

namespace CTD
{

Framework* Framework::s_pkSingletonFramework = NULL;

Framework::Framework( FrameworkImpl *pkFrameworkImpl )
{

    m_pkImpl = pkFrameworkImpl;

}

Framework::~Framework()
{
}

LevelSet* Framework::LoadLevel( const std::string &strLevelFile ) 
{ 
    // load a level, create a new plugin manager for new level
    return m_pkImpl->LoadLevel( strLevelFile ); 
}

void Framework::StartGame()
{
    return m_pkImpl->StartGame();
}

void Framework::ExitGame()
{
    return m_pkImpl->ExitGame();
}

bool Framework::GetKeyCode( const std::string &strKeyName , int &iKeyCode ) 
{
    return m_pkImpl->GetKeyCode( strKeyName, iKeyCode );
}

BaseEntity* Framework::FindEntity( const std::string &strInstanceName, const std::string &strPluginName )
{
    return m_pkImpl->FindEntity( strInstanceName, strPluginName );
}

void Framework::SendPluginMessage( int iMsgId, void *pMsgStruct, const string &strPluginName )
{
    m_pkImpl->SendPluginMessage( iMsgId, pMsgStruct, strPluginName );
}

void Framework::SendEntityMessage( int iMsgId, void *pMsgStruct, const string &strPluginName )
{
    m_pkImpl->SendEntityMessage( iMsgId, pMsgStruct, strPluginName, false );
}

GameMode Framework::GetGameMode()
{ 
    return m_pkImpl->m_eGameMode; 
}

const string& Framework::GetRootPath()
{ 
    return m_pkImpl->m_strRootPath; 
}

NeoEngine::RenderDevice* Framework::GetRenderDevice()
{ 
    return m_pkImpl->m_pkRenderDevice; 
}

const std::string& Framework::GetRenderDeviceString() 
{ 
    return m_pkImpl->m_strRenderDevice; 
}

NeoEngine::RenderCaps&  Framework::GetRenderCaps()
{ 
    return m_pkImpl->m_kRenderCaps; 
}

bool Framework::UseStencilBuffer()
{ 
    return m_pkImpl->m_bUseStencilBuffer; 
}

NeoEngine::AudioDevice* Framework::GetAudioDevice()     
{ 
    return m_pkImpl->m_pkAudioDevice; 
}

const std::string& Framework::GetAudioDeviceString() 
{ 
    return m_pkImpl->m_strAudioDevice; 
}

LevelManager* Framework::GetLevelManager()
{ 
    return m_pkImpl->m_pkLevelManager; 
}

void Framework::SwitchToLevelSet( LevelSet* pkLevelSet )
{
    m_pkImpl->m_pkCurrentLevelSet = pkLevelSet; 
}

LevelSet* Framework::GetCurrentLevelSet()
{
    return m_pkImpl->m_pkCurrentLevelSet;
}

void Framework::SetBackgroundColor( NeoEngine::Color kColor ) 
{ 
    m_pkImpl->m_kBackgoundColor = kColor; 
}

NeoEngine::Color Framework::GetBackgroundColor()
{ 
    return m_pkImpl->m_kBackgoundColor;
}

NetworkDevice* Framework::GetNetworkDevice()
{ 
    return m_pkImpl->m_pkNetworkDevice;
}

bool Framework::InitiateClientNetworking( int iClientPort, int iServerPort, const string& strServerIP, const string& strNodeName, string& strLevelName )
{
    bool bResuslt = m_pkImpl->InitiateClientNetworking( iClientPort, iServerPort, strServerIP, strNodeName );
    strLevelName  = m_pkImpl->m_strLevelName;
    return bResuslt;
}

bool Framework::StartClientNetworking()
{
    return m_pkImpl->StartClientNetworking();
}

void Framework::ShutdownClientNetworking()
{
    m_pkImpl->ShutdownClientNetworking();
}

void Framework::EmergencyShutdownNetworkDevice()
{
    m_pkImpl->EmergencyShutdownNetworkDevice();
}

void Framework::RegisterPreRenderEntity( CTD::BaseEntity *pkEntity )
{ 
    m_pkImpl->m_vpkPreRenderEntities.push_back( pkEntity ); 
}

void Framework::Register2DRendering( BaseEntity *pkEntity )
{ 
    m_pkImpl->m_vpk2DRenderEntities.push_back( pkEntity ); 
}

const float& Framework::GetFPS()
{ 
    return m_pkImpl->m_fFPS; 
}

BaseEntity* Framework::GetPlayer()
{
    return m_pkImpl->m_pkPlayer; 
}

void Framework::SetPlayer( BaseEntity *pkPlayer )
{ 
    m_pkImpl->m_pkPlayer = pkPlayer; 
}

} // namespace CTD
