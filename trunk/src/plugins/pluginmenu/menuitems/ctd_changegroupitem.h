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
 # neoengine, item for changing to another menu section ( group )
 #
 #
 #   date of creation:  05/31/2004
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  05/31/2004 boto       creation of CTDMenuChangeGroupItem
 #
 ################################################################*/

#ifndef _CTD_MENUCHANGEGROUPITEM_H_
#define _CTD_MENUCHANGEGROUPITEM_H_


#include "../base.h"
#include "ctd_menuitem.h"
#include "../ctd_menucontrol.h"

#include <neoengine/skeletonanimator.h>
#include <neoengine/animatednode.h>
#include <neochunkio/nodeanimation.h>
#include <neochunkio/scenenode.h>
#include <neochunkio/stdstring.h>
#include <neochunkio/float.h>
#include <neochunkio/integer.h>

#include <string>

namespace CTD_IPluginMenu
{

// name of this entity
#define CTD_ENTITY_NAME_MenuChangeGroupItem  "MenuChangeGroupItem"

class CTDMenuChangeGroupItem: public CTDMenuItem
{

    public:

                                                    CTDMenuChangeGroupItem();
                                                    ~CTDMenuChangeGroupItem();

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
        * Call back funtion for beginning with focus state. This function is called when 
        *  the user activates an item.
        */
        void                                        OnBeginFocus();

        /**
        * Call back funtion for ending with focus state. This function is called when 
        *  the user changes the focus to another item.
        */
        void                                        OnEndFocus();

        /**
        * Call back funtion for getting activated. This function is called when 
        *  the user activates an item, e.g. by pressing enter for a focused item.
        */
        void                                        OnActivate();

        /**
        * Update object
        * \param fDeltaTime                         Time passed since last update
        */
        void                                        UpdateEntity( float fDeltaTime );

        /**
        * Render object
        * \param pkFrustum                          Current view frustum (if any)
        * \param bForce                             Render even if rendered previously this frame or deactivated (default false)
        * \return                                   true if we were rendered, false if not (already rendered, not forced)
        */
        bool                                        Render( NeoEngine::Frustum *pkFrustum = 0, bool bForce = false );

        /**
        * Messaging function.
        * \param  iMsgId                            Message ID
        * \param  pMsgStruct                        Message specific data structure
        * \return                                   This return value is message specific.
        */
        int                                         Message( int iMsgId, void *pkMsgStruct );

        //*******************************************************************************************//

        int                                         ParameterDescription( int iParamIndex, CTD::ParameterDescriptor *pkDesc );

        //******************************************************************************************//


        std::string                                 m_strPathAnim;

        // group id to change to
        int                                         m_iDestGroupID;

    protected:

        NeoEngine::Vector3d                         m_kCurrPosition;

        float                                       m_fPosVar;

        CTD::BaseEntity                             *m_pkCamera;

        enum { eIdle, eAnim, eFocus }               m_eState;

        NeoEngine::AnimatedNode                     *m_pkPathAnim;
        
        int                                         m_iCheckLastKey;

};


// descriptor for menu's change group item entity
class CTDMenuChangeGroupItemDesc : public CTD::EntityDescriptor
{

    public:
                                                    CTDMenuChangeGroupItemDesc() { };
                                                    ~CTDMenuChangeGroupItemDesc() { }
            
        const std::string&                          GetEntityName() { CTD_RETURN_ENTITYNAME( CTD_ENTITY_NAME_MenuChangeGroupItem ) }
        CTD::BaseEntity*                            CreateEntityInstance() { return (CTD::BaseEntity*) new CTDMenuChangeGroupItem; }


};

// global instance of change group item entity is used in dll interface
extern CTDMenuChangeGroupItemDesc g_pkCTDMenuChangeGroupItemEntity_desc;

}

#endif //_CTD_MENUCHANGEGROUPITEM_H_
