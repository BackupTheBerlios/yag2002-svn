/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2007, A. Botorabi
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
 # entity for displaying name of player in front
 #
 #   date of creation:  11/02/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_PLAYERNAMEDISP_H_
#define _CTD_PLAYERNAMEDISP_H_

#include <ctd_main.h>

namespace vrc
{

//! Period for updating name
#define PLAYERNAME_UPDATE_PERIOD        0.5f

//! Entity name
#define ENTITY_NAME_PLAYERNAMEDISP      "PlayerNameDisplay"

//! Entity for displaying player name in front of local player
class EnPlayerNameDisplay : public yaf3d::BaseEntity
{
    public:


                                                    EnPlayerNameDisplay();

        virtual                                     ~EnPlayerNameDisplay();


        //! This entity does not need a transform node
        const bool                                  isTransformable() const { return false; }

        
        //! Initializing function, this is called after all engine modules are initialized and a map is loaded.
        void                                        initialize();

       
        //! Post-initializing function, this is called after all plugins' entities are initilized.
        void                                        postInitialize();

    protected:

        //! Update entity
        void                                        updateEntity( float deltaTime );

        //! Override this method of yaf3d::BaseEntity to get notifications (from menu system)
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        //! Determine the name of player in front and update the name gui
        void                                        updateName();

        //! Relative display gui position screen [ 0..1, 0..1 ] ( only x and y coords are used )
        osg::Vec3f                                  _position;

        //! Player detection cone angle
        float                                       _detectionAngle;

        //! Detection view
        float                                       _viewAngle;

        //! List of player entities
        std::vector< yaf3d::BaseEntity* >                  _players;

        //! Internal timer for updating the name
        float                                       _updateTimer;

        //! Name field
        CEGUI::StaticText*                          _nameBox;
};

//! Entity type definition used for type registry
class PlayerNameDisplayEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    PlayerNameDisplayEntityFactory() : 
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_PLAYERNAMEDISP, yaf3d::BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~PlayerNameDisplayEntityFactory() {}

        Macro_CreateEntity( EnPlayerNameDisplay );
};

} // namespace vrc

#endif // _CTD_PLAYERNAMEDISP_H_
