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
 # neoengine, father class for all kinds of menu items
 #  game settings are represented by these items. the instance name 
 #  of item entities identify the parameter names.
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
 #  05/14/2004 boto       creation of CTDMenuItem
 #
 ################################################################*/

#ifndef _CTD_MENUITEM_H_
#define _CTD_MENUITEM_H_


#include "../base.h"
#include "../ctd_menucontrol.h"
#include <string>

namespace CTD_IPluginMenu
{

class CTDMenuItem : public CTD::BaseEntity
{

    public:

                                                    CTDMenuItem();
        
        virtual                                     ~CTDMenuItem();


        /**
        * Type of menu item.
        */
        enum  CTDItemType { eInput, eOutput, eAction, eChangeGroup, eUnknown };

        /**
        * Set item type.
        */
        void                                        SetType( CTDMenuItem::CTDItemType eType ) { m_eType = eType; }

        /**
        * Get item type.
        */
        CTDMenuItem::CTDItemType                    GetType() { return m_eType; }

        /**
        * Return true if item is in focus.
        */
        bool                                        IsFocused() { return m_bIsFocused; }

        /**
        * Set group id.
        */
        void                                        SetGroupID( unsigned int uiGroupID ) { m_uiGroupID = uiGroupID; }

        /**
        * Get group id.
        */
        unsigned int                                GetGroupID() { return m_uiGroupID; }

        /**
        * Call back funtion for beginning with focus state. This function is called when 
        *  the user activates an item.
        */
        virtual void                                OnBeginFocus() { m_bIsFocused = true; }

        /**
        * Call back funtion for ending with focus state. This function is called when 
        *  the user changes the focus to another item.
        */
        virtual void                                OnEndFocus() { m_bIsFocused = false; }

        /**
        * Call back funtion for getting activated. This function is called when 
        *  the user activates an item, e.g. by pressing enter for a focused item.
        */
        virtual void                                OnActivate() {}

        /**
        * Call back funtion for input event. This function is called when 
        *  the user presses a key. iKeyData contains the key code and cKeyData contains the ASCII character.
        */
        virtual void                                OnInput( CTDMenuControl::CTDCtrlKeys eCtrlKey, unsigned int iKeyData, char cKeyData ) {}


        //******************************* override some functions **********************************//

        /**
        * Initializing function
        */
        virtual void                                Initialize();

        /**
        * Post-initializing function, this is called after all plugins' entities are initilized.
        * One important usage of this function is to search and attach entities to eachother, after all entities are initialized.
        */
        virtual void                                PostInitialize() {}


        /**
        * Render object
        * \param pkFrustum                          Current view frustum (if any)
        * \param bForce                             Render even if rendered previously this frame or deactivated (default false)
        * \return                                   true if we were rendered, false if not (already rendered, not forced)
        */
        virtual bool                                Render( NeoEngine::Frustum *pkFrustum = 0, bool bForce = false );

        //*******************************************************************************************//

        virtual int                                 ParameterDescription( int iParamIndex, CTD::ParameterDescriptor *pkDesc );

        //******************************************************************************************//

        // this name correspond to one of game setting's parameter names ( e.g. player name or server's ip address )
        std::string                                 m_strParameterName;

        // item type, e.g. input, output, change group, etc.
        CTDItemType                                 m_eType;
    
        // group affiliation ( every menu section has an unique group id )
        unsigned int                                m_uiGroupID;

        // this number determines the order for selecting menu items when using left right keys instead of mouse
        unsigned int                                m_uiSelectionOrder;

        // general item parameters
        NeoEngine::Vector3d                         m_kPosition;

        NeoEngine::Vector3d                         m_kRotation;
        
        NeoEngine::MeshEntity                       *m_pkMesh;

    private:

        // this flas shows whether the item is in focus
        bool                                        m_bIsFocused;
        
};

}

#endif //_CTD_MENUITEM_H_
