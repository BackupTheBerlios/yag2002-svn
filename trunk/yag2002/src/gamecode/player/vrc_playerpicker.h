/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2006, A. Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU Lesser General Public 
 *  License version 2.1 as published by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public 
 *  License along with this program; if not, write to the Free 
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 * 
 ****************************************************************/

/*###############################################################
 # Entiy for player's picking funtion
 #
 #   date of creation:  07/16/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 ################################################################*/

#ifndef _VRC_PLAYERPICKER_H_
#define _VRC_PLAYERPICKER_H_

#include <vrc_main.h>
#include "../storage/vrc_storageclient.h"

namespace vrc
{

//! Entity name
#define ENTITY_NAME_PLAYERPICKER            "PlayerPicker"

//! Period for picking check
#define PLAYERPICKING_UPDATE_PERIOD         0.2f

//! Forward declarations
class EnPlayer;

//! Player picker entity
class EnPlayerPicker :
    public yaf3d::BaseEntity,
    public gameutils::PlayerUtils::CallbackPlayerListChange,
    public StorageClient::AccountInfoResult
{
    public:


                                                    EnPlayerPicker();

        virtual                                     ~EnPlayerPicker();


        //! Initializing function, this is called after all engine modules are initialized and a map is loaded.
        void                                        initialize();

    protected:

        //! Update entity
        void                                        updateEntity( float deltaTime );

        //! Override this method of yaf3d::BaseEntity to get notifications (from menu system)
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        //! Check for players to pick
        void                                        checkPicking();

        //! Called by input handler when a player has been picked.
        void                                        playerPicked( EnPlayer* p_player );

        //! Called when the player leave or join, used for maintaining the pickable players.
        virtual void                                onPlayerListChanged( bool localplayer, bool joining, yaf3d::BaseEntity* p_entity );

        //! Called by StorageClient when account information is available after requesting it.
        virtual void                                accountInfoResult( tAccountInfoData& info );

        //! Input handler for object used for picking
        class PickerInputHandler : public vrc::gameutils::GenericInputHandler< PickerInputHandler >
        {
            public:

                explicit                            PickerInputHandler( EnPlayerPicker* p_entity );

                virtual                             ~PickerInputHandler() {}

                //! Handle input events.
                bool                                handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

                //! Enable / disable input handling
                void                                enable( bool enable )
                                                    {
                                                        _enable = enable;
                                                    }

            protected:

                //! Picker entity
                EnPlayerPicker*                     _p_entity;

                //! Enable / disable flag for handler
                bool                                _enable;
        };

        PickerInputHandler*                         _p_inputHandler;

        //! Maximal distance for considering another player as pickable
        float                                       _maxPickDistance;

        //! Highlight pickable player by mesh or color?
        bool                                        _highlightByMesh;

        //! Mesh file for the picking sign
        std::string                                 _signMeshFile;

        //! Player detection cone angle
        float                                       _detectionAngle;

        //! Detection view
        float                                       _viewAngle;

        //! Picking check timer
        float                                       _pickCheckTimer;

        //! Square of picking max distance
        float                                       _maxPickDistance2;

        //! Sign mesh node
        osg::ref_ptr< osg::Node >                   _signMesh;

        //! Currently picked player
        EnPlayer*                                   _p_pickedPlayer;

        // GUI related methods and variables
        // #################################

        //! Setup the gui which is used for showing up information on picked player.
        void                                        setupGui();

        //! Update the gui display with new data.
        void                                        updateGui( tAccountInfoData& info );

        //! Callback for on-close event
        bool                                        onClickedClose( const CEGUI::EventArgs& arg );

        //! Frame object
        CEGUI::FrameWindow*                         _p_frame;

        //! Profile pane: nick name
        CEGUI::Editbox*                             _p_editboxNickName;

        //! Profile pane: member since
        CEGUI::Editbox*                             _p_editboxMemberSince;

        //! Profile pane: total online time
        CEGUI::Editbox*                             _p_editboxOnlineTime;

        //! Profile pane: status
        CEGUI::Editbox*                             _p_editboxStatus;

        //! Profile pane: about me text
        CEGUI::MultiLineEditbox*                    _p_editboxAboutMe;
};

//! Entity type definition used for type registry
class PlayerPickerEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    PlayerPickerEntityFactory() : 
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_PLAYERPICKER, yaf3d::BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~PlayerPickerEntityFactory() {}

        Macro_CreateEntity( EnPlayerPicker );
};

} // namespace vrc

#endif // _VRC_PLAYERPICKER_H_
