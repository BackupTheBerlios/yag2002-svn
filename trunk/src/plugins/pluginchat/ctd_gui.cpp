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
 # neoengine, chat's gui system
 #
 # this class implements the gui for chat plugin
 #
 #
 #   date of creation:  08/22/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>

#include "ctd_gui.h"
#include <ctd_printf.h>
#include <ctd_settings.h>

using namespace std;
using namespace NeoEngine;
using namespace CTD;


namespace CTD_IPluginChat {


// plugin global entity descriptor for gui system
CTDGuiDesc g_pkGuiEntity_desc;
//-------------------------------------------//

// instance counter for CTDGui, there must be one single instance for the gui entity!
static unsigned int s_uiGuiEntityCnt = 0;

CTDGui::CTDGui()
{

    // check whether there is more than one single instance of this entity
    s_uiGuiEntityCnt++;
    assert( ( s_uiGuiEntityCnt == 1 ) && "CTDGui: there must be one single instance of Gui entity!" );

    CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Chat) entity ' Gui ' created " );

    // activate networking as client object
    SetNetworkingType( CTD::stateNONE );

    // set the entity ( node ) name
    //  for client objects it is essential that the entity name is set in constructor
    SetName( CTD_ENTITY_NAME_Gui );
    
    m_kConfigFile   = "index.xml";

    m_pkChatMember  = NULL;

    m_pcPlayerName  = NULL;

}

CTDGui::~CTDGui()
{

    CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Chat) entity ' Gui ' destroyed " );

    s_uiGuiEntityCnt--;

}

// init entity
void CTDGui::Initialize() 
{

    // set a unique instance name, so chat member entity can find us later. we take the same name as entity name
    SetInstanceName( CTD_ENTITY_NAME_Gui );

    // the server needs no gui system!
    if ( Framework::Get()->GetGameMode() == stateSERVER ) {
    
        Deactivate();

        CTDCONSOLE_PRINT( LogLevel( INFO ), " (Plugin Chat) entity ' Gui ' in chat server we don't need a gui system, entity deactivated. " );
        
    }

}

void CTDGui::PostInitialize()
{

}

void CTDGui::SetupGuiSystem() 
{

    // configure the widget system
    Widgets::Get()->Initialize();
    Widgets::Get()->Activate();
    Widgets::Get()->Load( m_kConfigFile );

    m_bEditText = false;

    // set a callback for walk mode button
    guiCButton*  pkWalkModeBtn   = ( guiCButton* )dataManager_S::SearchWidget("WalkMode");
    assert( ( pkWalkModeBtn != NULL ) && "'WalkMode' button is not defined in gui definition file!" );

    CB_Imp_GuiC_Callback* pkGLOActivateWalkModeButtonCallback = new GuiC_Callback< CTDGui >( this, OnActivateWalkMode, CM_GUI );
    pkWalkModeBtn->AddCallback( pkGLOActivateWalkModeButtonCallback, eGuiCBase::CB_SELECT );

    // set a callback for chat message editbox
    guiCWindow*  pkEditBoxWindow   = ( guiCWindow* )dataManager_S::SearchWidget("EditBox");
    assert( ( pkEditBoxWindow != NULL ) && "'EditBox' is not defined in gui definition file!" );
    
    CB_Imp_GuiC_Callback* pkGLOActivateEditboxCallback = new GuiC_Callback< CTDGui >( this, OnActivateEditText, CM_GUI );
    pkEditBoxWindow->AddCallback( pkGLOActivateEditboxCallback, eGuiCBase::CB_SELECT );


    m_pkEditBox   = ( guiCEdit* )dataManager_S::SearchWidget("EditArea");
    assert( ( m_pkEditBox != NULL ) && "'EditBox::EditArea' is not defined in gui definition file!" );

    // we allow also activation of edit box when one clicks on text area
    CB_Imp_GuiC_Callback* pkGLOActivateEditAreaCallback = new GuiC_Callback< CTDGui >( this, OnActivateEditText, CM_GUI );
    m_pkEditBox->AddCallback( pkGLOActivateEditAreaCallback, eGuiCBase::CB_SELECT );

    m_pkMsgBox   = ( guiCEdit* )dataManager_S::SearchWidget("MsgBox");
    assert( ( m_pkMsgBox != NULL ) && "'MsgWindow::MsgBox' is not defined in gui definition file!" );
    
    // setup the text buffer of message window
    //--------------------------------------------------//
    // allocate text buffer
    int iMaxStringLen = m_pkMsgBox->LengthMax();
    // if no length given in xml file then assume the default size
    if ( iMaxStringLen <= 0 ) {
        iMaxStringLen = 4096;
    }
    wchar_t *pcTextBuffer = new wchar_t[ iMaxStringLen ];
    pcTextBuffer[ iMaxStringLen - 1 ] = 0;
    m_pkMsgBox->StringRef() =  pcTextBuffer;
    
    // reset the text buffer
    pcTextBuffer        = m_pkMsgBox->GetString();
    pcTextBuffer[ 0 ]   = 0;
    //--------------------------------------------------//

    // calculate the available lines in message window
    //--------------------------------------------------//
    m_uiMsgTotalLines   = 0;

    // get  message window font size
    utlVector kSize;
    m_pkMsgFont         = ( dataFont* )dataManager_S::SearchDataFont( "MsgFont" );
    m_pkMsgFont->GetSize ( "A", 1, kSize );
    m_fMsgFontHeight    = kSize.y;

    // add this object to callback list so the widget system will notify us on new inputs
    //-------------------------------//
    Widgets::Get()->AddInputCallback( this );

    // get local chat member name
    //-------------------------------//
    string strPlayerName;
    Framework::Get()->GetGameSettings()->GetValue( CTD_STOKEN_PLAYERNAME, strPlayerName );
    strPlayerName       = "<" + strPlayerName + ">";
    m_pcPlayerName      = utlStringClone_C( ( char* )strPlayerName.c_str() );

}

void CTDGui::OnInput( const NeoEngine::InputEvent *pkEvent )
{

    // proceed only when edit box is active
    if ( m_bEditText == false ) {

        return;

    }

    int     iKeyData     = pkEvent->m_aArgs[0].m_iData;
    int     iCharKeyData = pkEvent->m_aArgs[0].m_iData;

    // handle key down/up events
    if ( ( pkEvent->m_iType == IE_KEYDOWN ) && ( iCharKeyData == KC_RETURN ) ) {

        // don't print empty text into message window
        if ( ( !m_pkEditBox->StringRef() ) || ( m_pkEditBox->StringRef()[ 0 ] == L'' ) ) {

            return;

        }

        // get the text in edit box and set the player name at begin of text
        static wchar_t s_pcText[ 256 ]; 
        utlStringCopy( s_pcText, m_pcPlayerName );
        utlStringConcat( s_pcText, m_pkEditBox->StringRef() );

        // send message over network
        m_pkChatMember->Message( CTD_ENTITY_GUI_SEND_TEXT, s_pcText );

        // set caret to position 0
        int     iLen = utlStringLength( s_pcText );
        m_pkEditBox->MoveCaret( -iLen );

        // add the message to message window
        AddMessage( s_pcText );

        // reset the text in editbox
        m_pkEditBox->StringRef()[ 0 ] = 0;

    }

}

void CTDGui::AddMessage( wchar_t *pcMsg )
{

    // add new text to message window
    wchar_t *pcMsgText = m_pkMsgBox->StringRef();
    utlStringConcat( pcMsgText, pcMsg );
    utlStringConcat( pcMsgText, ( wchar_t* )"\n" );

    // get message window height
    utlVector kSize;
    rendCBase   *pkRenderPrimitive = m_pkMsgBox->GetRender();
    pkRenderPrimitive->GetCurrentOrNormalDraw()->GetSize( kSize );
    m_uiMsgMaxLines     = ( unsigned int )( kSize.y / m_fMsgFontHeight );

    // fit the message lines into message box
    if ( m_uiMsgTotalLines > m_uiMsgMaxLines ) {

        // find the end of first line in message buffer
        int iLen   = utlStringLength( pcMsgText );
        int iCRPos = 0;
        for ( ; iCRPos < iLen; iCRPos ++ ) {

            if ( ( wchar_t )pcMsgText[ iCRPos ] == ( wchar_t )'\n' ) {

                break;

            }

        }

        // cut the first line from msg buffer
        if ( iCRPos < iLen ) {

            // skip CR
            iCRPos++;

            // FIXME: i don't know why memmove does not work here!?
            //memmove( pcMsgText, ( void* )( pcMsgText + iCRPos ), iLen - iCRPos + 1 );

            // do the text moving by hand :-(
            for ( int i = 0; i <= ( iLen - iCRPos ); i++ ) {

                pcMsgText[ i ] = pcMsgText[ i + iCRPos ];
            }

        }

    } else {

        // increase total count of used lines if not the maximum reached
        m_uiMsgTotalLines++;

    }

}

void CTDGui::OnActivateEditText( CB_Imp_GuiC_Callback* a_pCallback, guiCBase* a_pControl, void* a_pData )
{

    ClearMovementFlags();
    LockMovementInput( true );
    m_bEditText = true;

}

void CTDGui::OnActivateWalkMode( CB_Imp_GuiC_Callback* a_pCallback, guiCBase* a_pControl, void* a_pData )
{

    LockMovementInput( false );
    m_bEditText = false;

}

void CTDGui::UpdateEntity( float fDeltaTime ) 
{ 
    
}

// incomming network messages
void CTDGui::NetworkMessage( int iMsgId, void *pkMsgStruct )
{
    
    // wait until we have been integrated into network session
    switch ( iMsgId ) {
        
        case CTD_NM_SYSTEM_NOTIFY_CONNECTION_ESTABLISHED:
        {

            // get the chat member entity, we need it for sending messages over network, etc.
            //-------------------------------//
            m_pkChatMember = Framework::Get()->GetPlayer();
            assert( m_pkChatMember && " 'CTDGui::Initialize' player is not set in framework!" );

            // setup the gui system
            SetupGuiSystem();

        }
        break;

        case CTD_NM_SYSTEM_NOTIFY_CONNECTION_LOST:
        {
        }
        break;

        case CTD_NM_SYSTEM_NOTIFY_CONNECTION_RESUMED:
        {
        }
        break;

        case CTD_NM_SYSTEM_NOTIFY_DISCONNECTED:
        {
        }
        break;

        default:
            // unexpected message type!
            ;
    }

}

int CTDGui::Message( int iMsgId, void *pMsgStruct ) 
{ 

    switch ( iMsgId ) {

        // handle entering the menu, disable widgets drawing
        case CTD_ENTITY_ENTER_MENU:

            Widgets::Get()->Deactivate();

            break;

        // handle exiting the menu, enable widgets drawing
        case CTD_ENTITY_EXIT_MENU:

            Widgets::Get()->Activate();

            break;

        // add a new chat message to message console
        case CTD_ENTITY_CHATMEMBER_ADD_MSG:
        {

            char            *pcMsg = ( char* )pMsgStruct;
            // convert the multi-byte character sting to wide characher string
            static wchar_t  s_pcWMsg[ 256 ];
            mbstowcs( s_pcWMsg, pcMsg, 256 );

            AddMessage( s_pcWMsg );

        }
        break;

        default:
            break;

    }

    return 0; 
}   


int CTDGui::ParameterDescription( int iParamIndex, ParameterDescriptor *pkDesc )
{

    int iParamCount = 1;

    if ( pkDesc == NULL ) {

        return iParamCount;
    }

    switch( iParamIndex ) 
    {

    case 0:
        pkDesc->SetName( "ConfigFile" );
        pkDesc->SetType( ParameterDescriptor::CTD_PD_STRING );
        pkDesc->SetVar( &m_kConfigFile );
        
        break;

    default:
        return -1;
    }

    return iParamCount;

}

} // namespace CTD_IPluginChat

