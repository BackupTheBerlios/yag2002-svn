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
 # entity for displaying player information
 #
 #   date of creation:  06/28/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _VRC_PLAYERINFODISPLAY_H_
#define _VRC_PLAYERINFODISPLAY_H_

#include <vrc_main.h>
#include <vrc_gameutils.h>

namespace vrc
{

#define ENTITY_NAME_PLAYERINFODISPLAY    "PlayerInfoDisplay"

class EnPlayer;

//! Player information display entity
class EnPlayerInfoDisplay :  public yaf3d::BaseEntity, public gameutils::PlayerUtils::CallbackPlayerListChange
{
    public:
                                                    EnPlayerInfoDisplay();

        virtual                                     ~EnPlayerInfoDisplay();

        //! Initialize entity
        void                                        initialize();

        //! Update entity
        void                                        updateEntity( float deltaTime );

        //! Enable / disable statistics rendering
        void                                        enable( bool en );

    protected:

        //! Override notification callback
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        //! Override this method and return true to get a persisten entity
        bool                                        isPersistent() const { return true; }

        //! This entity needs no transformation
        bool                                        isTransformable() const { return false; }

        //! Internal usage
        bool                                        getPlayerEntity();

        //! Functor for changed player list
        void                                        onPlayerListChanged( bool localplayer, bool joining, yaf3d::BaseEntity* p_entity );

        //! Positon on screen
        osg::Vec2f                                  _position;

        //! Enable flag
        bool                                        _enable;

        //! Player entity
        EnPlayer*                                   _p_playerEntity;

        // GUI window
        CEGUI::FrameWindow*                         _p_wnd;

        CEGUI::StaticText*                          _p_outputText;
};

//! Entity type definition used for type registry
class PlayerInfoDisplayEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    PlayerInfoDisplayEntityFactory() : 
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_PLAYERINFODISPLAY, yaf3d::BaseEntityFactory::Standalone | yaf3d::BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~PlayerInfoDisplayEntityFactory() {}

        Macro_CreateEntity( EnPlayerInfoDisplay );
};

} // namespace vrc

#endif // _VRC_PLAYERINFODISPLAY_H_
