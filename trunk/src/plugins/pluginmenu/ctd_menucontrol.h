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
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  05/14/2004 boto       creation of CTDMenuControl
 #
 ################################################################*/

#ifndef _CTD_MENUCONTROL_H_
#define _CTD_MENUCONTROL_H_


#include "base.h"
#include <ctd_printf.h>
#include <string>
#include <vector>

namespace CTD_IPluginMenu
{

// name of this entity
#define CTD_ENTITY_NAME_MenuControl  "MenuControl"

class CTDMenuItem;
class CTDMenuControl;

// class input listener for activating menu
class CTDMenuControlListener : public NeoEngine::InputEntity
{
    public:

                                            CTDMenuControlListener( NeoEngine::InputGroup *pkGroup, CTDMenuControl *pkMenuCtrl ) : 
                                                NeoEngine::InputEntity( pkGroup ) 
                                                { 
                                                    m_pkMenuControl = pkMenuCtrl; 
                                                }

        virtual void                        Input( const NeoEngine::InputEvent *pkEvent );

    private:

        CTDMenuControl                      *m_pkMenuControl;

};

// a class for organizing menu groups and their items
class CTDItemGroup
{

    public:

                                            CTDItemGroup() { m_bActive = false; m_uiGroupID = 0; }
                                            
                                            ~CTDItemGroup(){};

        std::string                         m_strGroupName;
        unsigned int                        m_uiGroupID;

        bool                                m_bActive;

        std::vector< CTDMenuItem* >         m_vpkMenuItems;

};

class CTDMenuControl : public CTD::BaseEntity
{

    public:

                                                    CTDMenuControl();

                                                    ~CTDMenuControl();


        //******************************* override some functions **********************************//

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

        //*******************************************************************************************//

        int                                         ParameterDescription( int iParamIndex, CTD::ParameterDescriptor *pkDesc );

        //******************************************************************************************//

        enum  CTDMenuCtrlKeys { eNext, ePrev, eInput, eEnter, eUnknown };
        CTDMenuCtrlKeys                             m_eKeyPressed;

        // there are forwarded to current active menu item
        int                                         m_iKeyData;
        char                                        m_cKeyData;
        enum CTDCtrlKeys { eNONE, eSHIFT, eALT };
        CTDCtrlKeys                                 m_eCtrlKey;

    private:

    
        // register a menu group. It is neccessary to register every menu item at start up.
        //  registering is done my menu group entities themselves using the message interface of this class.
        void                                        RegisterGroup( CTDItemGroup *pkItemGroup );

        // process input keys
        void                                        ProcessKeys();

        // select next item in active group
        void                                        SelectNextItem( CTDItemGroup *pkActiveGroup, unsigned int uiActiveItem );

        // select previous item in active group
        void                                        SelectPrevItem( CTDItemGroup *pkActiveGroup, unsigned int uiActiveItem );

        // activate / deactivate the focus on an item and retrieve its input string if it is of type input and it was on focus
        void                                        ActivateItem( CTDItemGroup *pkActiveGroup, unsigned int uiActiveItem );

        // registered menu item
        std::vector< CTDItemGroup* >                m_vpkMenuGroups;

        // this is the current active menu group
        unsigned int                                m_uiActiveGroup;

        // this is the current active item in selected menu group
        unsigned int                                m_uiActiveItem;

        // this points to current active group in the group list
        CTDItemGroup                                *m_pkActiveGroup;

        NeoEngine::InputGroup                       *m_pkInputGroup;

        CTDMenuControlListener                      *m_pkInputListener;

        // this flag is set if any item is in focus, it is used to lock further selection if an item is in focus
        bool                                        m_bItemInFocus;

        // if this is set to false then the input processing is interrupted ( is used during camera path animation while changing groups )
        bool                                        m_bLockInput;

        CTD::BaseEntity                             *m_pkChangeGrpEntity;

#ifdef _DEBUG
        // camera adjusting mode
        bool                                        m_bCamAdjustMode;
        NeoEngine::Vector3d                         m_kRotationOffset;
        NeoEngine::Quaternion                       m_kRotOffset;
#endif

};


// descriptor for menu control entity
class CTDMenuControlDesc : public CTD::EntityDescriptor
{

    public:
                                                    CTDMenuControlDesc() { };
                                                    ~CTDMenuControlDesc() { }
            
        const std::string&                          GetEntityName() { CTD_RETURN_ENTITYNAME( CTD_ENTITY_NAME_MenuControl ) }
        CTD::BaseEntity*                            CreateEntityInstance() { return (CTD::BaseEntity*) new CTDMenuControl; };


};

// global instance of menu control entity is used in dll interface
extern CTDMenuControlDesc g_pkCTDMenuControlEntity_desc;

}

#endif //_CTD_MENUCONTROL_H_
