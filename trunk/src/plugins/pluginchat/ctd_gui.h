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
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  08/22/2004 boto       creation of CTDGui
 #
 ################################################################*/

#ifndef _CTD_GUI_H_
#define _CTD_GUI_H_

#include <widgets/ctd_widgets.h>

#include <string>


namespace CTD_IPluginChat {

// this is the printf object
extern CTDPrintf    g_CTDPrintf;
extern CTDPrintf    g_CTDPrintfNwStats;

class CTDGui : public CTD::BaseEntity, public CTD::WidgetInput
{

    public:

                                                    CTDGui();

                                                    ~CTDGui();

        //******************************* override some functions **********************************//

        /**
        * Get network type of this entity ( see BaseEntity for more details ).
        * Return always the type using BaseEntity::GetNetworkingType() and set
        * your desired type in ctor ( as a client object can get remote client object through the framework ).
        */
        CTD::tCTDNetworkingType                     GetNetworkingType() 
                                                    { 
                                                        return BaseEntity::GetNetworkingType();
                                                    }

        /**
        * Initializing function
        */
        void                                        Initialize();

        /**
        * Post-initializing function, this is called after all plugins' entities are initilized.
        * One important usage of this function is to search and attach entities to eachother, after all entities are initialized.
        */
         void                                       PostInitialize();

        /**
        * Update object
        * \param fDeltaTime                         Time passed since last update
        */
        void                                        UpdateEntity( float fDeltaTime );       
        
        /**
        * Messaging function.
        * \param  iMsgId                            Message ID
        * \param  pMsgStruct                        Message specific data structure
        * \return                                   This return value is message specific.
        */
        int                                         Message( int iMsgId, void *pMsgStruct );


        /**
        * Messaging function for incomming network pakets. This function is called by framework.
        * \param   iMsgId                           Message ID
        * \param   pMsgStruct                       Message specific data structure
        */
        void                                        NetworkMessage( int iMsgId, void *pMsgStruct );

        /**
        * Entity parameter description function.
        */
        int                                         ParameterDescription( int iParamIndex, CTD::ParameterDescriptor *pkDesc );

        //******************************************************************************************//

        // entity parameters
        //------------------------------------------------------------------------------------------//

        // config file ( xml format )
        std::string                                 m_kConfigFile;

        // GLO related functions and variables
        //------------------------------------------------------------------------------------------//

        // callback functions

        // is called when the text area is clicked
        void                                        OnActivateEditText( CB_Imp_GuiC_Callback* pkCallback, guiCBase* pkControl, void* pkData );
        // is called when the check box for walk mode is clicked
        void                                        OnActivateWalkMode( CB_Imp_GuiC_Callback* pkCallback, guiCBase* pkControl, void* pkData );

        // is called when a key is pressed
        void                                        OnInput( const NeoEngine::InputEvent *pkEvent );

    protected:

        // setup the gui system when a connection with server has been established
        void                                        SetupGuiSystem();

        // add message to message window
        void                                        AddMessage( wchar_t *pcMsg );

        CTD::BaseEntity                             *m_pkChatMember;

        wchar_t                                     *m_pcPlayerName;

        // true if in message editing mode
        bool                                        m_bEditText;

        guiCEdit                                    *m_pkEditBox;
        
        guiCEdit                                    *m_pkMsgBox;
        
        dataFont                                    *m_pkMsgFont;

        float                                       m_fMsgFontHeight;

        unsigned int                                m_uiMsgMaxLines;

        unsigned int                                m_uiMsgTotalLines;

};


// descriptor for gui system entity
class CTDGuiDesc : public CTD::EntityDescriptor
{

    public:
                                                    CTDGuiDesc() { };
                                                    ~CTDGuiDesc() { }
            
        const std::string&                          GetEntityName() { CTD_RETURN_ENTITYNAME( CTD_ENTITY_NAME_Gui ) }
        CTD::BaseEntity*                            CreateEntityInstance() { return (CTD::BaseEntity*) new CTDGui; };


};

// global instance of gui system entity is used in dll interface
extern CTDGuiDesc g_pkGuiEntity_desc;

}

#endif //_CTD_GUI_H_
