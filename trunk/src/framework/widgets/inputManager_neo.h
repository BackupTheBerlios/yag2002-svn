#ifndef _INPUTMANAGER_PC_H_
#define _INPUTMANAGER_PC_H_

#include "Utility\utlSingleton.h"
#include "Utility\utlPulseUser.h"

class   inputCPointer;
class   inputCKeyboard; 

namespace CTD
{
    
class WidgetInput;

class inputManager_Neo :  public NeoEngine::InputEntity
{

    public:

                                                    inputManager_Neo( NeoEngine::InputGroup *pkGroup );

                                                    ~inputManager_Neo(){};

        void                                        Input( const NeoEngine::InputEvent *pkEvent );



        /***************************************
        Pulse functions
        ***************************************/
        bool                                        Init       (int a_Value);
        bool                                        ShutDown   (int a_Value);
        int                                         Pulse();

        /**************************************************
        Configuration
        **************************************************/    
        void                                        Configure(HWND a_hWnd);
        
        /**************************************************    
        Processing    
        **************************************************/
        
        void                                        ProcessCharInput(int a_KeyState, int a_Key);

        void                                        ProcessKeyInput (int a_KeyState, int a_lParam, int a_Key);

        /**
        * Add an input callback object. Its class must inherit from WidgetInput
        */
        void                                        AddInputCallback( WidgetInput* pkCallbackObject );

        /**
        * Remove an input callback function.
        */
        void                                        RemoveInputCallback( WidgetInput* pkCallbackObject );


        inputCPointer*                              m_pMouse;

        inputCKeyboard*                             m_pKeyboard;

    protected:

        utlPulseUser  <inputManager_Neo>            m_Pulse;

        NeoEngine::RenderDevice                     *m_pkRenderDevice;

        std::vector< WidgetInput* >                 m_vpkInputCallbacks;


        float                                       m_fScreenWidth;

        float                                       m_fScreenHeight;

};

extern inputManager_Neo *g_pkInputManager;

} // namespace CTD

#endif