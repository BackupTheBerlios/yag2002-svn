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
 # entity for displaying player information
 #
 #   date of creation:  06/28/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_PLAYERINFODISPLAY_H_
#define _CTD_PLAYERINFODISPLAY_H_

#include <ctd_main.h>

namespace CTD
{

#define ENTITY_NAME_PLAYERINFODISPLAY    "PlayerInfoDisplay"

class EnPlayer;

//! Player information display entity
class EnPlayerInfoDisplay :  public BaseEntity
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

        osg::Vec3f                                  _position;

    protected:

        //! Override notification callback
        void                                        handleNotification( const EntityNotification& notification );

        //! Override this method and return true to get a persisten entity
        const bool                                  isPersistent() const { return true; }

        //! This entity needs no transformation
        const bool                                  isTransformable() const { return false; }

        //! Internal usage
        bool                                        getPlayerEntity();

        //! Enable flag
        bool                                        _enable;

        //! Player entity
        EnPlayer*                                   _p_playerEntity;

        // GUI window
        CEGUI::FrameWindow*                         _p_wnd;

        CEGUI::StaticText*                          _p_outputText;
};

//! Entity type definition used for type registry
class PlayerInfoDisplayEntityFactory : public BaseEntityFactory
{
    public:
                                                    PlayerInfoDisplayEntityFactory() : 
                                                     BaseEntityFactory( ENTITY_NAME_PLAYERINFODISPLAY, BaseEntityFactory::Standalone | BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~PlayerInfoDisplayEntityFactory() {}

        Macro_CreateEntity( EnPlayerInfoDisplay );
};

} // namespace CTD

#endif // _CTD_PLAYERINFODISPLAY_H_
