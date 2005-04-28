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

#include <ctd_main.h>

namespace CTD
{

#define ENTITY_NAME_MENU    "Menu"

class DialogGameSettings;
class DialogLevelSelect;
class MenuInputHandler;
class EnAmbientSound;
class IntroControl;

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

        //! Override this method of BaseEntity to get notifications (from menu system)
        void                                        handleNotification( EntityNotification& notify );

        //! Entity parameters

        std::string                                 _menuConfig;

        std::string                                 _settingsDialogConfig;

        std::string                                 _levelSelectDialogConfig;

        std::string                                 _introTexture;

        std::string                                 _buttonClickSound;

        std::string                                 _buttonHoverSound;

        std::string                                 _introductionSound;

    protected:

        //! Begin intro
        void                                        beginIntro();

        //! Stop intro
        void                                        stopIntro();

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

        //! Menu states
        enum
        {
            None,
            BeginIntro,
            Intro,
            Visible,
            Hidden
        }                                           _menuState;

        std::auto_ptr< EnAmbientSound >             _clickSound;

        std::auto_ptr< EnAmbientSound >             _hoverSound;

        std::auto_ptr< EnAmbientSound >             _introSound;

        std::auto_ptr< DialogGameSettings >         _settingsDialog;

        std::auto_ptr< DialogLevelSelect >          _levelSelectDialog;

        std::auto_ptr< IntroControl >               _intro;

        CEGUI::Window*                              _p_menuWindow;

        bool                                        _beginHover;

        MenuInputHandler*                           _p_inputHandler;

    friend class MenuInputHandler;
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
