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
 # change history:
 #
 #  date       nick name  comment
 #----------------------------------------------------------
 #  10/22/2004 boto       creation of CTDMenuSelectPlayerItem
 #
 ################################################################*/

#ifndef _CTD_MENUSELECTPLAYERITEM_H_
#define _CTD_MENUSELECTPLAYERITEM_H_


#include "../base.h"
#include "ctd_menuitem.h"
#include "../ctd_menucontrol.h"
#include <string>

namespace CTD_IPluginMenu
{

// name of this entity
#define CTD_ENTITY_NAME_MenuSelectPlayerItem  "MenuSelectPlayerItem"

class CTDMenuSelectPlayerItem: public CTDMenuItem
{

    public:

                                                    CTDMenuSelectPlayerItem();

                                                    ~CTDMenuSelectPlayerItem();

        //******************************* override some functions **********************************//

        /**
        * Initializing function
        */
        void                                        Initialize();

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
        * Call back funtion for input event. This function is called when 
        *  the user presses a key.
        */
        void                                        OnInput( CTDMenuControl::CTDCtrlKeys eCtrlKey, unsigned int iKeyData, char cKeyData );

        /**
        * Call back funtion for getting activated. This function is called when 
        *  the user activates an item, e.g. by pressing enter for a focused item.
        */
        void                                        OnActivate();

        //*******************************************************************************************//

        int                                         ParameterDescription( int iParamIndex, CTD::ParameterDescriptor *pkDesc );

        //******************************************************************************************//

        //! Player list parameter
        std::string                                 m_strPlayerList;

        //! List of players extracted from m_strPlayerList
        std::vector< std::string >                  m_vstrPlayers;

        //! Fading time for changing a player
        float                                       m_fFadingTime;

    protected:

        //! Class for holding one player configuration
        class Player
        {

            public:
                                                            Player( NeoEngine::MeshEntity *pkMesh, const std::string &strConfigFile ) 
                                                            {
                                                                m_pkPreviewMesh = pkMesh;
                                                                m_strConfigFile = strConfigFile;
                                                            }

                                                            ~Player()
                                                            {                        
                                                            }

                //! Preview mesh
                NeoEngine::MeshEntity                       *m_pkPreviewMesh;

                //! Configuration file
                std::string                                 m_strConfigFile;

                //! List of submeshes' diffuse color
                std::vector< NeoEngine::Color* >            m_vpkDiffuseColor;

        };

        //! List of selectable players
        std::vector< Player* >                              m_vpkPlayers;

        NeoEngine::MeshEntity                               *m_pkSelectedPlayerMesh;


        //! Current player mesh position
        NeoEngine::Vector3d                                 m_kCurrPosition;

        //! Initial rotation in quaternion form
        NeoEngine::Quaternion                               m_kRotationQ;

        //! This is needed to animate the player mesh position
        float                                               m_fPosVar;

        //! Selecting mode
        bool                                                m_bSelecting;
        //! Current selected player index in list
        std::size_t                                         m_uiSelectedPlayer;

        //! Previously selected player index in list
        std::size_t                                         m_uiPrevSelectedPlayer;

        //! Player selection states
        enum { 
            stateIdle, 
            stateChangePlayer, 
            stateFadeOut, 
            stateFadeIn 
        }                                                   m_ePlayerSelect;

        //! Timer for changing player
        float                                               m_fChangeFadingTimer;

};


// descriptor for menu's player selector item entity
class CTDMenuSelectPlayerItemDesc : public CTD::EntityDescriptor
{

    public:
                                                    CTDMenuSelectPlayerItemDesc() { };
                                                    ~CTDMenuSelectPlayerItemDesc() { }
            
        const std::string&                          GetEntityName() { CTD_RETURN_ENTITYNAME( CTD_ENTITY_NAME_MenuSelectPlayerItem ) }
        CTD::BaseEntity*                            CreateEntityInstance() { return (CTD::BaseEntity*) new CTDMenuSelectPlayerItem; }


};

// global instance of player selector item entity is used in plugin interface
extern CTDMenuSelectPlayerItemDesc g_pkCTDMenuSelectPlayerItemEntity_desc;

}

#endif //_CTD_MENUSELECTPLAYERITEM_H_
