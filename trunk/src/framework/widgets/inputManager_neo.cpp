
#include <ctd_frbase.h>

#include "inputManager_Neo.h"

#include <Data/dataManager.h>
#include <Data/dataTypes.h>
#include <Utility/utlPulseTypes.h>
#include <Utility/utlError.h>
#include <Utility/utlMacros.h>
#include <Utility/utlString.h>
#include <Input/inputManager.h>
#include <Input/inputCButton.h>
#include <Input/inputCKeyboard.h>
#include <Input/inputCPointer.h>

#include <ctd_widgets.h>

static const int RIGHT_MOUSE_KEY = 0x0001FFFF;
static const int LEFT_MOUSE_KEY  = 0x0002FFFF;

using namespace NeoEngine;

namespace CTD
{

/**************************************************
Constructor - Construct
**************************************************/
inputManager_Neo::inputManager_Neo( NeoEngine::InputGroup *pkGroup ) : InputEntity( pkGroup )
{
    m_Pulse.Construct         (this, PS_INPUT_HW,  PG_SYSTEM, "inputManager_Neo");
    m_pMouse            = NULL;
    m_pKeyboard         = NULL;

    m_fScreenWidth      = ( float )NeoEngine::Core::Get()->GetRenderDevice()->GetWidth();
    m_fScreenHeight     = ( float )NeoEngine::Core::Get()->GetRenderDevice()->GetHeight();


}

// returns true if given key code is a character
bool IsChar( int KeyCode ) 
{


    bool bChar =    ( ( KeyCode >= KC_0 )   && ( KeyCode <= KC_9  ) ) ||
                    ( ( KeyCode >= KC_A )   && ( KeyCode <= KC_Z  ) ) ||
                    ( ( KeyCode >= KC_DOT ) && ( KeyCode <= KC_AT ) ) ||
                    ( KeyCode == KC_SPACE )                         ;

    return bChar;

}

/**************************************************
Adding / removing input callbacks
**************************************************/
void inputManager_Neo::AddInputCallback( WidgetInput* pkCallbackObject )
{

    // check whether the callback object is already in list
    size_t uiCallbackNum =  m_vpkInputCallbacks.size();
    for ( size_t uiCallbacks = 0; uiCallbacks < uiCallbackNum; uiCallbacks++ ) {

        if ( pkCallbackObject == m_vpkInputCallbacks[ uiCallbacks ] ) {

            return;

        }

    }

    m_vpkInputCallbacks.push_back( pkCallbackObject );

}

void inputManager_Neo::RemoveInputCallback( WidgetInput* pkCallbackObject )
{

    // check whether the callback object is already in list
    vector< WidgetInput* >::iterator    pkCalback    = m_vpkInputCallbacks.begin();
    vector< WidgetInput* >::iterator    pkCalbackEnd = m_vpkInputCallbacks.end();
    
    while ( pkCalback != pkCalbackEnd ) {

        if ( pkCallbackObject == *pkCalback ) {

            break;

        }
        pkCalback++;

    }

    if ( pkCalback != pkCalbackEnd ) {

        m_vpkInputCallbacks.erase( pkCalback );
        delete *pkCalback;

    }

}

/**************************************************
Input processing
**************************************************/
void inputManager_Neo::Input( const NeoEngine::InputEvent *pkEvent )
{

    int     iKeyCode     = pkEvent->m_aArgs[ 0 ].m_iData;
    int     iCharKeyData = pkEvent->m_aArgs[ 1 ].m_iData;

    // handle key down events 
    if ( pkEvent->m_iType == IE_KEYDOWN ) {

        // process input if a valid ascii code available
        if ( ( iCharKeyData != 0 ) && IsChar( iKeyCode ) ) {

            ProcessCharInput( 0, iCharKeyData );

        } else {

            ProcessKeyInput( WM_KEYDOWN, iKeyCode, iCharKeyData );

        }

        // process callbacks
        vector< WidgetInput* >::iterator    pkCalback    = m_vpkInputCallbacks.begin();
        vector< WidgetInput* >::iterator    pkCalbackEnd = m_vpkInputCallbacks.end();
        
        while ( pkCalback != pkCalbackEnd ) {
        
            ( *pkCalback )->OnInput( pkEvent );
            pkCalback++;

        }

    } 

    if( pkEvent->m_iType == IE_MOUSEDOWN )
    {
        if( iKeyCode == MB_LEFT ) {

            ProcessKeyInput( WM_KEYDOWN, 0, LEFT_MOUSE_KEY );

        } else {
            
            if( iKeyCode == MB_RIGHT ) {

                ProcessKeyInput( WM_KEYDOWN, 0, RIGHT_MOUSE_KEY );

            }

        }
    }
    else if( pkEvent->m_iType == IE_MOUSEUP )
    {
        if( iKeyCode == MB_LEFT ) {

            ProcessKeyInput( WM_KEYUP, 0, LEFT_MOUSE_KEY );

        } else {
            
            if( iKeyCode == MB_RIGHT ) {
            
                ProcessKeyInput( WM_KEYUP, 0, RIGHT_MOUSE_KEY );

            }

        }

    }

}


/***************************************
Pulse functions - Init
***************************************/
bool inputManager_Neo::Init(int a_Value)
{

    m_pkRenderDevice = NeoEngine::Core::Get()->GetRenderDevice();

    if (a_Value == PT_SYSTEM) {
        
        m_pMouse    = new inputCPointer ("Mouse_0");
        m_pKeyboard = new inputCKeyboard("Keyboard_0");

        new inputCButton("Button_0",  'Q');
        new inputCButton("RM_Button", RIGHT_MOUSE_KEY);
        new inputCButton("LM_Button", LEFT_MOUSE_KEY);
        
        // FIXME
        m_pkRenderDevice->ShowCursor( false );

    }

    return true;
}


bool inputManager_Neo::ShutDown   (int a_Value)
{

    m_pkRenderDevice->ShowCursor( true );

    return true;
}

void inputManager_Neo::Configure(HWND a_hWnd)
{

}


/***************************************
Pulse functions - Process Key States
***************************************/
void inputManager_Neo::ProcessKeyInput(int a_KeyState, int a_lParam, int a_Key)
{
    struct VKMap
    {
        int m_VK;
        int m_Ascii;
    };

    static VKMap m_VKMap[] = {
        {KC_DELETE,     GK_DEL},
        {KC_RETURN,     GK_CR},
        {KC_BACKSPACE,  GK_BS},
        {KC_LEFT,       GK_LEFT},
        {KC_UP,         GK_UP},
        {KC_RIGHT,      GK_RIGHT},
        {KC_DOWN,       GK_DOWN}};

   int buttonState = -1;

    // Checked Mapped keys to char keys first
    if ((a_KeyState == WM_KEYDOWN))
        {
        for (int i=0; i<UTL_ARRAY_LEN(m_VKMap); i++)
            {
            if (m_VKMap[i].m_VK == a_lParam)
                {
                ProcessCharInput(WM_CHAR, m_VKMap[i].m_Ascii);
                return;
                }
            }
        }

    // These keys don't auto repeat
    if ((a_lParam & (1 << 30)))
        return;

    // Map PC States to internal states
    if (a_KeyState == WM_KEYDOWN)
        {
        buttonState = eGuiCBase::CB_SELECT;
        }
    else
        {
        buttonState = eGuiCBase::CB_NORMAL;
        }

    inputManager::p().ProcessInputButtons(a_Key , buttonState);
}

void inputManager_Neo::ProcessCharInput(int a_KeyState, int a_Key)
{
    // Process Keyboard
    if ( m_pKeyboard ) {
    
        // Set the key information and tag the change
        m_pKeyboard->HardwareId() = a_Key;
        m_pKeyboard->ChangeState(eGuiCBase::CB_SELECT);
        
    }
}

/***************************************
Pulse functions - Pulse
***************************************/
int  inputManager_Neo::Pulse()
{

    // Position in screen space
    if ( m_pMouse ) {

        HWND hWnd = ( HWND ) m_pkRenderDevice->GetWindow().m_hWnd;

        POINT point;
        RECT rectClient;
        GetCursorPos((LPPOINT)&point);
        GetClientRect( hWnd, &rectClient );

        //Convert to window position
        RECT rect;
        GetWindowRect( hWnd, &rect );

        // Figure out width of side offsets
        int borderOffset  = (((rect.right-rect.left) - rectClient.right)/2);
        point.x-= rect.left+borderOffset;
        point.y-= (rect.bottom-borderOffset) - rectClient.bottom;

        m_pMouse->SetPosition( ( ( float )point.x ) / m_fScreenWidth, ( ( float )point.y ) / m_fScreenHeight );
 
    }


    return 0;
}

} // namespace CTD
