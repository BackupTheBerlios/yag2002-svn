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
 # neoengine, menu control
 #
 # this class implements the total control of 3d menu
 #
 #
 #   date of creation:  05/14/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include "base.h"
#include "ctd_menucontrol.h"
#include "menuitems/ctd_menuitem.h"

#include <list>

using namespace std;
using namespace CTD;
using namespace NeoEngine;

namespace CTD_IPluginMenu
{

// plugin global entity descriptor for menu control
CTDMenuControlDesc g_pkCTDMenuControlEntity_desc;
//-------------------------------------------//
extern CTDPrintf    g_CTDPrintfBuffered;
extern CTDPrintf    g_CTDPrintf;

// one single instance of menu control entity must exist. this varable is used to check this.
static bool s_bMenuCtrlCreated = false;

// input listener
void CTDMenuControlListener::Input( const InputEvent *pkEvent )
{

    if ( NeoEngine::Core::Get()->GetConsole()->IsActive() ) {

        return;

    }


    int iKeyData = pkEvent->m_aArgs[0].m_iData;
    m_pkMenuControl->m_iKeyData = iKeyData;
    m_pkMenuControl->m_cKeyData = pkEvent->m_aArgs[1].m_iData;

    // handle menu activation key
    if ( pkEvent->m_iType == IE_KEYDOWN ) {
        
        // ESC means activation of menu
        if ( iKeyData == KC_ESCAPE ) {

            // switch to game only if one is loaded
            if ( Framework::Get()->GetCurrentLevelSet() == GetMenuLevelSet() ) {

                if ( GetGameLevelSet() ) {

                    // switch to game
                    Framework::Get()->SwitchToLevelSet( GetGameLevelSet() );
                    GetMenuLevelSet()->SetUpdateFlag( false );
                    GetMenuLevelSet()->SetRenderFlag( false );

                    // notify all entities
                    Framework::Get()->SendEntityMessage( CTD_ENTITY_EXIT_MENU, ( void* )NULL );

                }

            } else {

                // switch to menu
                Framework::Get()->SwitchToLevelSet( GetMenuLevelSet() );
                GetMenuLevelSet()->SetUpdateFlag( true );
                GetMenuLevelSet()->SetRenderFlag( true );

                // notify all entities
                Framework::Get()->SendEntityMessage( CTD_ENTITY_ENTER_MENU, ( void* )NULL );

            }

        }
        
        // check and set control key
        if ( ( iKeyData == KC_LSHIFT ) || ( iKeyData == KC_RSHIFT ) ) {

            m_pkMenuControl->m_eCtrlKey = CTDMenuControl::eSHIFT;

        } else {

            if ( ( iKeyData == KC_LALT ) || ( iKeyData == KC_RALT ) ) {

                m_pkMenuControl->m_eCtrlKey = CTDMenuControl::eALT;

            } else {

                // process ctrl keys
                if ( iKeyData == KC_RIGHT ) {

                    m_pkMenuControl->m_eKeyPressed = CTDMenuControl::eNext;

                } else {

                    if ( iKeyData == KC_LEFT ) {

                        m_pkMenuControl->m_eKeyPressed = CTDMenuControl::ePrev;

                    } else {

                        if ( iKeyData == KC_RETURN ) {

                            m_pkMenuControl->m_eKeyPressed = CTDMenuControl::eEnter;

                        } else {

                            // process input letters
                            m_pkMenuControl->m_eKeyPressed = CTDMenuControl::eInput;

                        }

                    }

                }

            }
        
        }

    } else {

        if ( pkEvent->m_iType == IE_KEYUP ) {

            if ( ( iKeyData == KC_LSHIFT ) || ( iKeyData == KC_RSHIFT ) || 
                 ( iKeyData == KC_LALT ) || ( iKeyData == KC_RALT ) ) {

                m_pkMenuControl->m_eCtrlKey = CTDMenuControl::eNONE;

            } 

        }

    }

}


CTDMenuControl::CTDMenuControl()
{

    m_pkInputGroup              = NULL;
    m_pkInputListener           = NULL;
    m_pkActiveGroup             = NULL;
    m_uiActiveItem              = 0;
    m_cKeyData                  = 0;
    m_iKeyData                  = 0;
    m_bItemInFocus              = false;

    m_bLockInput                = false;
    m_pkChangeGrpEntity         = NULL;

    CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Menu) entity ' MenuControl ' created " );

    // set the entity ( node ) name
    SetName( CTD_ENTITY_NAME_MenuControl );

}

CTDMenuControl::~CTDMenuControl()
{

    CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Menu) entity ' MenuControl ' destroyed " );

    delete m_pkInputGroup;
    delete m_pkInputListener;

}

// init entity
void CTDMenuControl::Initialize() 
{ 


    if ( s_bMenuCtrlCreated == true ) {

        CTDCONSOLE_PRINT( LogLevel( WARNING ), " (Plugin Menu) entity ' MenuControl ' only one instance of this entity can exist! entity deactivated." );
        Deactivate();
        return;

    }

    s_bMenuCtrlCreated  = true;

#ifdef _DEBUG
    m_bCamAdjustMode    = false;
#endif

    // set the instance name as entity name in order to be able to search for it in other menu entity types such as menu item or group
    SetInstanceName( CTD_ENTITY_NAME_MenuControl );

    // setup input listener for menu activation
    m_pkInputGroup      = new InputGroup();
    m_pkInputListener   = new CTDMenuControlListener( m_pkInputGroup, this );
    m_pkInputGroup->AttachEntity( ( InputEntity* )NeoEngine::Core::Get()->GetConsole() );

}

// post-init entity
void CTDMenuControl::PostInitialize()
{

}


void CTDMenuControl::UpdateEntity( float fDeltaTime ) 
{ 
    
#ifdef _DEBUG
    // allow path tweeking
    if ( m_bCamAdjustMode == true ) {

        if ( m_eKeyPressed == eInput ) {

            switch ( m_iKeyData ) {
        
                case KC_X:

                    if ( m_eCtrlKey == eSHIFT ) {

                        m_kRotationOffset.x -= 1.0f;

                    } else {

                        m_kRotationOffset.x += 1.0f;
                    
                    }
                    break;

                case KC_Y:

                    if ( m_eCtrlKey == eSHIFT ) {

                        m_kRotationOffset.y -= 1.0f;

                    } else {

                        m_kRotationOffset.y += 1.0f;
                    
                    }
                    break;

                case KC_Z:

                    if ( m_eCtrlKey == eSHIFT ) {

                        m_kRotationOffset.z -= 1.0f;

                    } else {

                        m_kRotationOffset.z += 1.0f;
                    
                    }
                    break;

                case KC_R:

                    m_kRotationOffset.Reset();
                    break;

                default:
                    ;

            }

        }


        char    pcBuff[ 64 ];
        string  kMsg( "  camera adjusting mode activated ( press F11 to toggle mode, use following keys: SHIFT/X/Y/Z or R for resetting offset ) " );
        CTDPRINTF_PRINT( g_CTDPrintf, kMsg );
        kMsg = "  current camera offset: ";
        sprintf( pcBuff, "x(%f)  y(%f)  z(%f)", m_kRotationOffset.x, m_kRotationOffset.y, m_kRotationOffset.z );
        kMsg += pcBuff;
        CTDPRINTF_PRINT( g_CTDPrintf, kMsg );

        // set the rotation offset
        m_kRotOffset = Quaternion( EulerAngles( m_kRotationOffset.x * PI / 180.0f, m_kRotationOffset.y * PI / 180.0f, m_kRotationOffset.z * PI / 180.0f ) );

        // add the offset to camera rotation, take care that the camera entity is placed before the menu control entity
        //  as otherwise this offset will be overriden by camera entity in each frame!
        Quaternion kCamRot = GetMenuLevelSet()->GetCamera()->GetRotation();
        kCamRot = m_kRotOffset * kCamRot;
        GetMenuLevelSet()->GetCamera()->SetRotation( kCamRot );

    }
#endif

    // we lock the input processing during camera path animation
    if ( m_bLockInput == true ) {

        if ( m_pkChangeGrpEntity->Message( CTD_CHANGEGROUP_CAM_IN_POS, NULL ) != 0 ) {

            m_bLockInput = false;

        }

    } else {

        ProcessKeys();

    }

}

void CTDMenuControl::ProcessKeys()
{

    switch( m_eKeyPressed ) {

        case eNext:

            SelectNextItem( m_pkActiveGroup, m_uiActiveItem );
            break;

        case ePrev:

            SelectPrevItem( m_pkActiveGroup, m_uiActiveItem );
            break;

        case eInput:

#ifdef _DEBUG
            // 'F11' toggles the camera adjusting mode used for tweeking cam paths
            if ( m_iKeyData == KC_F11 ) {

                m_bCamAdjustMode = !m_bCamAdjustMode;

            }
#endif

            m_pkActiveGroup->m_vpkMenuItems[ m_uiActiveItem ]->OnInput( m_eCtrlKey, m_iKeyData, m_cKeyData );
            break;

        case eEnter:

            ActivateItem( m_pkActiveGroup, m_uiActiveItem );
            break;

        default:

            break;

    }

    m_eKeyPressed = eUnknown;

}

void CTDMenuControl::SelectNextItem( CTDItemGroup *pkActiveGroup, unsigned int uiActiveItem )
{

    if ( ( m_bItemInFocus == true ) || ( pkActiveGroup == NULL ) ) {

        return;

    }

    pkActiveGroup->m_vpkMenuItems[ uiActiveItem ]->OnEndFocus();

    m_uiActiveItem++;
    // wrap to first item in group if index > last item in group
    if ( m_uiActiveItem >= pkActiveGroup->m_vpkMenuItems.size() ) {

        m_uiActiveItem = 0;

    }

    pkActiveGroup->m_vpkMenuItems[ m_uiActiveItem ]->OnBeginFocus();

}

void CTDMenuControl::SelectPrevItem( CTDItemGroup *pkActiveGroup, unsigned int uiActiveItem )
{

    if ( ( m_bItemInFocus == true ) || ( pkActiveGroup == NULL ) ) {

        return;

    }

    pkActiveGroup->m_vpkMenuItems[ uiActiveItem ]->OnEndFocus();

    // wrap to first item in group if index > last item in group
    if ( m_uiActiveItem == 0 ) {

        m_uiActiveItem = ( unsigned int )pkActiveGroup->m_vpkMenuItems.size() - 1;

    } else {

        m_uiActiveItem--;

    }

    pkActiveGroup->m_vpkMenuItems[ m_uiActiveItem ]->OnBeginFocus();
    
}

void CTDMenuControl::ActivateItem( CTDItemGroup *pkActiveGroup, unsigned int uiActiveItem )
{

    if ( pkActiveGroup == NULL ) {

        return;

    }

    pkActiveGroup->m_vpkMenuItems[ uiActiveItem ]->OnActivate();
    
    // check for group change items ( changig to a menu sub-section )
    if ( pkActiveGroup->m_vpkMenuItems[ uiActiveItem ]->GetType() == CTDMenuItem::eChangeGroup ) {

        // find the new group and activate it
        //----------------------------------------------------------//

        // get the destination group id
        size_t uiGroupID            = pkActiveGroup->m_vpkMenuItems[ uiActiveItem ]->Message( CTD_CHANGEGROUP_GET_ID, NULL );
        size_t uiGroups             = m_vpkMenuGroups.size();
        CTDItemGroup *pkFindGroup   = NULL;
        // search the destination group
        for ( size_t uiGrpCnt = 0; uiGrpCnt < uiGroups; uiGrpCnt++ ) {

            if ( m_vpkMenuGroups[ uiGrpCnt ]->m_uiGroupID == uiGroupID ) {

                pkFindGroup = m_vpkMenuGroups[ uiGrpCnt ];
                break;

            }

        }
        // activate the group if found
        if ( pkFindGroup == NULL ) {

            CTDPRINTF_PRINT( g_CTDPrintfBuffered, "CTDMenuControl::" + GetInstanceName() + " warning, cannot find desired group id to change to!" );

        } else {

            m_pkActiveGroup     = pkFindGroup;
            m_uiActiveItem      = 0;
            m_bLockInput        = true;     // lock the input processing during camera path animation
            m_pkChangeGrpEntity = pkActiveGroup->m_vpkMenuItems[ uiActiveItem ];

        }

        return;

    }

}

int CTDMenuControl::Message( int iMsgId, void *pMsgStruct ) 
{ 

    switch( iMsgId ) {

        case CTD_MENUCTRL_REGISTER_GROUP:
        {

            tCTDMenuCtrlRegisterGroup   *pkRegData = ( tCTDMenuCtrlRegisterGroup* )pMsgStruct;
            RegisterGroup( pkRegData->m_pkItemGroupPtr );

        }
        break;

        // this message is sent by framework when entering menu
        case CTD_ENTITY_ENTER_MENU:
        {

            m_uiActiveItem  = 0;
            m_pkActiveGroup = m_vpkMenuGroups[ 0 ];
            m_pkActiveGroup->m_vpkMenuItems[ 0 ]->OnBeginFocus();

        }
        break;

        // reset menu when going back to game
        case CTD_ENTITY_EXIT_MENU:

            m_pkActiveGroup->m_vpkMenuItems[ m_uiActiveItem ]->OnEndFocus();

            m_uiActiveItem  = 0;
            m_pkActiveGroup = m_vpkMenuGroups[ 0 ];
            m_bItemInFocus  = false;

            break;

        default:
            break;

    }

    return 0; 

}   
        
void CTDMenuControl::RegisterGroup( CTDItemGroup *pkItemGroup )
{

    // consistence check
    size_t uiMenuItems = m_vpkMenuGroups.size();
    for ( size_t uiMenuItemCnt = 0; uiMenuItemCnt < uiMenuItems; uiMenuItemCnt++ ) {

        if ( m_vpkMenuGroups[ uiMenuItemCnt ] == pkItemGroup ) {

            string strMsg = " (Plugin Menu) entity ' MenuControl ' menu group ' " + pkItemGroup->m_strGroupName + " ' already registered, skipping!";
            CTDCONSOLE_PRINT( LogLevel( WARNING ), strMsg );
            return;

        }

    }

    // check whether this group is the initial one appearing at menu entrance
    if ( m_uiActiveGroup == pkItemGroup->m_uiGroupID ) {

        pkItemGroup->m_bActive  = true;
        m_pkActiveGroup         = pkItemGroup;

    }

    m_vpkMenuGroups.push_back( pkItemGroup );

    CTDCONSOLE_PRINT( LogLevel( INFO ), " item ordering: " );
    vector< CTDMenuItem* >  &rvpkMenuItems = pkItemGroup->m_vpkMenuItems;
    for ( size_t uiMenuItemCnt = 0; uiMenuItemCnt < rvpkMenuItems.size(); uiMenuItemCnt++ ) {

        neolog << LogLevel( INFO ) << "   item name: " << rvpkMenuItems[ uiMenuItemCnt ]->GetInstanceName() << ", ordering: " << rvpkMenuItems[ uiMenuItemCnt ]->m_uiSelectionOrder << endl;

    }

}

int CTDMenuControl::ParameterDescription( int iParamIndex, ParameterDescriptor *pkDesc )
{

    int iParamCount = 1;

    if ( pkDesc == NULL ) {

        return iParamCount;
    }

    switch( iParamIndex ) 
    {
    case 0:
        pkDesc->SetName( "InitialGroup" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_INTEGER );
        pkDesc->SetVar( &m_uiActiveGroup );
        
        break;

    default:
        return -1;
    }

    return iParamCount;

}

}
