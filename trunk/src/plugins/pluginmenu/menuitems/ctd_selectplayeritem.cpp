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
 # neoengine, select player item for 3d menu
 #
 #
 #   date of creation:  10/22/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>
#include "ctd_selectplayeritem.h"
#include <ctd_printf.h>
#include <ctd_settings.h>

using namespace std;
using namespace CTD;
using namespace NeoEngine;
using namespace NeoChunkIO;

namespace CTD_IPluginMenu
{

// plugin global entity descriptor for select player item
CTDMenuSelectPlayerItemDesc g_pkCTDMenuSelectPlayerItemEntity_desc;
//-------------------------------------------//

#define CTD_PLAYER_CFG_PREVIEWMESH "previewmesh"
#define CTD_PLAYER_CFG_CONFIGFILE  "configfile"


CTDMenuSelectPlayerItem::CTDMenuSelectPlayerItem()
{

    CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Menu) entity ' MenuSelectPlayerItem ' created " );

    CTDMenuItem::m_kPosition            = Vector3d( 0, 0, 0 );
    CTDMenuItem::m_kRotation            = Vector3d( 0, 0, 0 );
    CTDMenuItem::m_pkMesh               = NULL;
    m_pkSelectedPlayerMesh              = NULL;
    m_uiSelectedPlayer                  = 0;
    m_fPosVar                           = 0;
    m_bSelecting                        = false;
    m_ePlayerSelect                     = stateIdle;
    m_fFadingTime                       = 0.5f;

    // set item type
    CTDMenuItem::SetType( CTDMenuItem::eAction );

}

CTDMenuSelectPlayerItem::~CTDMenuSelectPlayerItem()
{

    CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Menu) entity ' MenuSelectPlayerItem ' destroyed " );

}

// init entity
void CTDMenuSelectPlayerItem::Initialize() 
{ 

    // load all given players
    CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Menu) entity ' MenuSelectPlayerItem ', creating player list... " );

    Explode( " ", m_strPlayerList, &m_vstrPlayers );
    for ( size_t uiCnt = 0; uiCnt < m_vstrPlayers.size(); uiCnt++ ) {

        Settings kSettings;
        string   strMesh;
        string   strConfigFile;
        // register the settings to be read
        kSettings.RegisterSetting( string( CTD_PLAYER_CFG_PREVIEWMESH ), strMesh );
        kSettings.RegisterSetting( string( CTD_PLAYER_CFG_CONFIGFILE ),  strConfigFile );

        CTDCONSOLE_PRINT( LogLevel( INFO ), "   loading player settings: " +  m_vstrPlayers[ uiCnt ] + ".cfg" );

        if ( kSettings.Load( m_vstrPlayers[ uiCnt ] + ".cfg" ) == false ) {
        
            CTDCONSOLE_PRINT( LogLevel( INFO ), "    *** error loading player settings file: " +  m_vstrPlayers[ uiCnt ] + ".cfg" );
            continue;

        }

        if ( kSettings.GetValue( CTD_PLAYER_CFG_PREVIEWMESH, strMesh ) == false ) {
            CTDCONSOLE_PRINT( LogLevel( INFO ), "    *** error, cannot find parameter '" + string ( CTD_PLAYER_CFG_PREVIEWMESH ) + 
                "' in player settings file: " +  m_vstrPlayers[ uiCnt ] + ".cfg" );
            continue;
        }
        if ( kSettings.GetValue( CTD_PLAYER_CFG_CONFIGFILE, strConfigFile ) == false ) {
            CTDCONSOLE_PRINT( LogLevel( INFO ), "    *** error, cannot find parameter '" + string( CTD_PLAYER_CFG_CONFIGFILE ) + "' in player settings file: " +  
                m_vstrPlayers[ uiCnt ] + ".cfg" );
            continue;
        }

        // load the preview mesh
        Scene kScene;
        File *pkFile = NeoEngine::Core::Get()->GetFileManager()->GetByName( strMesh );
        if ( !pkFile ) {

            CTDCONSOLE_PRINT( LogLevel( INFO ), "    *** error, cannot find preview mesh file '" + strMesh + "' in player settings file: " +  
                m_vstrPlayers[ uiCnt ] + ".cfg" );
            continue;

        }

        if ( kScene.Load( pkFile ) == false ) {

            CTDCONSOLE_PRINT( LogLevel( INFO ), "    *** error, cannot load preview mesh '" + strMesh + "' in player settings file: " +  
                m_vstrPlayers[ uiCnt ] + ".cfg" );
            continue;

        }
        MeshEntity *pkMesh = new MeshEntity( kScene.GetMeshes()[0] );
        pkMesh->GenerateBoundingVolume();
        pkMesh->GetMesh()->SetName( "__menu_preview_" + strMesh );

        // create a new player and push it into list
        Player *pkPlayer = new Player( pkMesh, strConfigFile );
        m_vpkPlayers.push_back( pkPlayer );

        kSettings.Shutdown();

        // prepare the meshes for alpha blending
        for ( size_t uiSubMesh = 0; uiSubMesh < pkMesh->GetSubMeshes().size(); uiSubMesh++ ) {

            Material* pkMaterial = pkMesh->GetSubMeshes()[ uiSubMesh ]->m_pkMaterial;
            pkMaterial->m_kBlendMode.Set( BlendMode::DECAL );
            pkPlayer->m_vpkDiffuseColor.push_back( &pkMaterial->m_kDiffuse );

        }

    }

    if ( m_vpkPlayers.size() == 0 ) {

        CTDCONSOLE_PRINT( LogLevel( INFO ), "    *** error, no players are defined! entity deactivated." );
        Deactivate();
        return;

    }

    // take the first player's mesh as default selected one
    m_uiSelectedPlayer      = 0;
    m_pkSelectedPlayerMesh  = m_vpkPlayers[ m_uiSelectedPlayer ]->m_pkPreviewMesh;
  	SetEntity( m_pkSelectedPlayerMesh );

    // set initial position and orientation
    SetTranslation( m_kPosition );
    m_kRotationQ = Quaternion( EulerAngles( m_kRotation.x * PI / 180.0f, m_kRotation.y * PI / 180.0f, m_kRotation.z * PI / 180.0f ) );
    SetRotation( m_kRotationQ );

}

void CTDMenuSelectPlayerItem::OnBeginFocus() 
{

    CTDMenuItem::OnBeginFocus();
    m_bSelecting    = true;
    m_ePlayerSelect = stateIdle;
    SetTranslation( m_kPosition );
    SetRotation( m_kRotationQ );

}

void CTDMenuSelectPlayerItem::OnEndFocus() 
{

    CTDMenuItem::OnEndFocus();
    SetTranslation( m_kPosition );
    SetRotation( m_kRotationQ );
    m_bSelecting    = false;
    m_ePlayerSelect = stateIdle;

}

void CTDMenuSelectPlayerItem::OnInput( CTDMenuControl::CTDCtrlKeys eCtrlKey, unsigned int iKeyData, char cKeyData ) 
{

    if ( IsActive() == false ) {

        return;

    }

    m_bSelecting = true;

    // select next player
    if ( ( iKeyData == KC_UP ) && ( m_ePlayerSelect == stateIdle ) ) {

        m_uiPrevSelectedPlayer = m_uiSelectedPlayer;
        m_uiSelectedPlayer++;
        if ( m_uiSelectedPlayer >= m_vpkPlayers.size() ) {
            m_uiSelectedPlayer = 0;
        }
        m_ePlayerSelect = stateChangePlayer;

    } 
    else 
    if ( ( iKeyData == KC_DOWN ) && ( m_ePlayerSelect == stateIdle ) ) {

        m_uiPrevSelectedPlayer = m_uiSelectedPlayer;
        if ( m_uiSelectedPlayer == 0 ) {

            m_uiSelectedPlayer = m_vpkPlayers.size();

        } 
        m_uiSelectedPlayer--;
        m_ePlayerSelect = stateChangePlayer;

    } 

}

void CTDMenuSelectPlayerItem::OnActivate() 
{

    if ( IsActive() == false ) {

        return;

    }
        
    SetTranslation( m_kPosition );
    SetRotation( m_kRotationQ );
    m_bSelecting    = false;
    m_ePlayerSelect = stateIdle;

}

bool CTDMenuSelectPlayerItem::Render( Frustum *pkFrustum, bool bForce ) 
{ 

    // render the selected player's mesh
    m_pkSelectedPlayerMesh->Render();
    return true;

}

void CTDMenuSelectPlayerItem::UpdateEntity( float fDeltaTime ) 
{ 

    if ( IsFocused() == false ) {

        return;

    }

    if ( m_bSelecting ) {

        m_fPosVar += fDeltaTime;
        if ( m_fPosVar > ( 2.0f * PI ) ) {

            m_fPosVar -= ( 2.0f * PI );

        }

        float fCosVar = 0.3f * cosf( m_fPosVar );
        m_kCurrPosition = m_kPosition + Vector3d( 0, fCosVar, 0 );
        SetTranslation( m_kCurrPosition );
    
        Rotate( Quaternion( EulerAngles( 0, -fDeltaTime * 0.5f, 0 ) ) );

    }

    // handle player mesh fading on change request
    switch ( m_ePlayerSelect ) {

        case stateIdle:
            break;

        case stateChangePlayer:

            m_fChangeFadingTimer = 0;
            m_ePlayerSelect      = stateFadeOut;
            break;

        case stateFadeOut:

            m_fChangeFadingTimer += fDeltaTime;
            if ( m_fChangeFadingTimer > m_fFadingTime ) {

                // set current player mesh for rendering
                m_pkSelectedPlayerMesh  = m_vpkPlayers[ m_uiSelectedPlayer ]->m_pkPreviewMesh;
                SetEntity( m_pkSelectedPlayerMesh, false ); // don't delete the previous mesh!

                // null out the alpha value of next mesh
                std::vector< NeoEngine::Color* >& vpkColors = m_vpkPlayers[ m_uiSelectedPlayer ]->m_vpkDiffuseColor;
                for( size_t uiCols = 0; uiCols < vpkColors.size(); uiCols++ ) {

                    vpkColors[ 0 ]->a = 0;

                }

                m_ePlayerSelect      = stateFadeIn;
                m_fChangeFadingTimer = 0;
                SetTranslation( m_kPosition );
                SetRotation( m_kRotationQ );

            } else {

                std::vector< NeoEngine::Color* >& vpkColors = m_vpkPlayers[ m_uiPrevSelectedPlayer ]->m_vpkDiffuseColor;
                for( size_t uiCols = 0; uiCols < vpkColors.size(); uiCols++ ) {

                    vpkColors[ 0 ]->a = MAX( 0.0f, 1.0f - ( m_fChangeFadingTimer / m_fFadingTime ) );

                }
                    
            }
            break;

        case stateFadeIn:

            m_fChangeFadingTimer += fDeltaTime;
            if ( m_fChangeFadingTimer > m_fFadingTime ) {

                m_ePlayerSelect = stateIdle;

            } else {


                std::vector< NeoEngine::Color* >& vpkColors = m_vpkPlayers[ m_uiSelectedPlayer ]->m_vpkDiffuseColor;
                for( size_t uiCols = 0; uiCols < vpkColors.size(); uiCols++ ) {

                    vpkColors[ 0 ]->a = MIN( 1.0f, m_fChangeFadingTimer / m_fFadingTime );

                }

            }
            break;

        default:

            assert( NULL && "unknown player selection state!" );

    }
}

int CTDMenuSelectPlayerItem::Message( int iMsgId, void *pkMsgStruct ) 
{ 

    return 0;

}   

int CTDMenuSelectPlayerItem::ParameterDescription( int iParamIndex, ParameterDescriptor *pkDesc )
{

    // get the param count of father class
    int iGeneralParamCount  = CTDMenuItem::ParameterDescription( 0, NULL ) - 1; // we don't need the mesh parameter of base class, so we make it unaccessible
    int iParamCount         = iGeneralParamCount + 2; 

    if (pkDesc == NULL) {

        return iParamCount;
    }

    // general parameters are directed to father class
    if ( iParamIndex < iGeneralParamCount ) {

        CTDMenuItem::ParameterDescription( iParamIndex, pkDesc );
        return iParamCount;

    }

    // correct the parameter index
    iParamIndex -= iGeneralParamCount;

    switch( iParamIndex ) 
    {
    case 0:
        pkDesc->SetName( "PlayerList" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_STRING );
        pkDesc->SetVar( &m_strPlayerList );
        
        break;

    case 1:
        pkDesc->SetName( "FadingTime" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_FLOAT );
        pkDesc->SetVar( &m_fFadingTime );

        break;

    default:
        return -1;
    }

    return iParamCount;

}

}
