/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2004-2006, Ali Botorabi
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
 # entity game menu
 #
 #   date of creation:  04/24/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_MENU_H_
#define _CTD_MENU_H_

#include <ctd_base.h>
#include <ctd_baseentity.h>
#include <ctd_entitymanager.h>

namespace CTD
{

#define ENTITY_NAME_MENU    "Menu"

class DialogGameSettings;
class DialogLevelSelect;
class MenuInputHandler;
class EnAmbientSound;

//! The menu system is controlled by this entity
class EnMenu :  public BaseEntity
{
    public:
                                                    EnMenu();

        virtual                                     ~EnMenu();


        //! This entity does not need a transform node, which would be created by level manager on loading
        //!   We create an own one and add it into scene's root node
        bool                                        needTransformation() { return false; }

        /**
        * Initializing function, this is called after all engine modules are initialized and a map is loaded.
        */
        void                                        initialize();

        //! This entity needs updating
        void                                        updateEntity( float deltaTime );

        //! Enter the menu
        void                                        enter();

        //! Leave menu
        void                                        leave();

    protected:

        //! Entity parameters

        std::string                                 _menuConfig;

        std::string                                 _settingsDialogConfig;

        std::string                                 _levelSelectDialogConfig;

        std::string                                 _buttonClickSound;

        std::string                                 _buttonHoverSound;

    protected:


        //! Callback for button click "game settings"        
        bool                                        onClickedGameSettings( const CEGUI::EventArgs& arg );

        //! Callback for button click "quit"        
        bool                                        onClickedQuit( const CEGUI::EventArgs& arg );

        //! Callback for button click "start"        
        bool                                        onClickedStart( const CEGUI::EventArgs& arg );

        //! Callback for mouse hover ( over a button ), used for playing sound
        bool                                        onButtonHover( const CEGUI::EventArgs& arg );

        //! Callback for losing mouse hover ( over a button ), used for playing sound
        bool                                        onButtonLostHover( const CEGUI::EventArgs& arg );

        //! Creates a sound entity with given filename
        EnAmbientSound*                              setupSound( const std::string& filename );

        std::auto_ptr< EnAmbientSound >             _p_clickSound;

        std::auto_ptr< EnAmbientSound >             _p_hoverSound;

        std::auto_ptr< DialogGameSettings >         _settingsDialog;

        std::auto_ptr< DialogLevelSelect >          _levelSelectDialog;

        CEGUI::Window*                              _p_menuWindow;

        bool                                        _beginHover;

        MenuInputHandler*                           _p_inputHandler;
};

//! Entity type definition used for type registry
class MenuEntityFactory : public BaseEntityFactory
{
    public:
                                                    MenuEntityFactory() : BaseEntityFactory( ENTITY_NAME_MENU ) {}

        virtual                                     ~MenuEntityFactory() {}

        Macro_CreateEntity( EnMenu );
};

}

#endif // _CTD_MENU_H_
