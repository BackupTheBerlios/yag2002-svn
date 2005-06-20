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
class EnCamera;
class EnSkyBox;
class EnFog;

//! The menu system is controlled by this entity
class EnMenu :  public BaseEntity
{
    public:
                                                    EnMenu();

        virtual                                     ~EnMenu();

        //! Initializing function, this is called after all engine modules are initialized and a map is loaded.
        void                                        initialize();

        //! This entity needs updating
        void                                        updateEntity( float deltaTime );

    protected:

        //! This entity does not need a transform node, which would be created by level manager on loading
        const bool                                  isTransformable() const { return false; }

        //! Make this entity persistent, thus on level changing this entity will not be deleted by entity manager
        //  and update and notification registries will be kept.
        const bool                                  isPersistent() const { return true; }

        // Interface methods for intro and settings dialog
        //-----
        //! Enter the menu
        void                                        enter();

        //! Leave menu
        void                                        leave();

        //! Leave current level and return to menu
        void                                        leaveLevel();

        //! This method is called by DialogLevelSelect. p_img is an optional background image for the choosen level.
        void                                        onLevelSelected( std::string levelfile, CEGUI::Image* p_img = NULL );

        //! This method is called by DialogLevelSelect when canceling a level selection process.
        void                                        onLevelSelectCanceled();
        //-----

        //! Override this method of BaseEntity to get notifications (from menu system)
        void                                        handleNotification( const EntityNotification& notify );

        //! Load a new level, optionally a background image can be shown during loading
        void                                        loadLevel( std::string levelfile, CEGUI::Image* p_img = NULL );

        //! Entity parameters

        std::string                                 _menuConfig;

        std::string                                 _settingsDialogConfig;

        std::string                                 _levelSelectDialogConfig;

        std::string                                 _introTexture;

        std::string                                 _loadingOverlayTexture;

        std::string                                 _buttonClickSound;

        std::string                                 _buttonHoverSound;

        std::string                                 _introductionSound;

        std::string                                 _skyboxImages[ 6 ];

        std::string                                 _menuSceneFile;

        std::string                                 _menuCameraPathFile;

    protected:

        //! Interface method used by settings dialog
        //---
        void                                        onSettingsDialogClose();
        //---

        //! Begin intro
        void                                        beginIntro();

        //! Stop intro
        void                                        stopIntro();

        //! Ued to turn on / off skybox when entering / leaving menu
        void                                        enableSkybox( bool en );

        //! Ued to turn on / off menu fog when entering / leaving menu
        void                                        enableFog( bool en );

        //! Callback for button click "game settings"        
        bool                                        onClickedGameSettings( const CEGUI::EventArgs& arg );

        //! Callback for button click "return to level"
        bool                                        onClickedReturnToLevel( const CEGUI::EventArgs& arg );

        //! Callback for button click "quit"
        bool                                        onClickedQuit( const CEGUI::EventArgs& arg );

        //! Callback for button click "leave"
        bool                                        onClickedLeave( const CEGUI::EventArgs& arg );

        //! Callback for button click "join to chat"  
        bool                                        onClickedJoin( const CEGUI::EventArgs& arg );

        //! Callback for button click "start server"  
        bool                                        onClickedServer( const CEGUI::EventArgs& arg );
        
        //! Callback for button click "start walk-through"  
        bool                                        onClickedWT( const CEGUI::EventArgs& arg );

        //! Callback for mouse hover ( over a button ), used for playing sound
        bool                                        onButtonHover( const CEGUI::EventArgs& arg );

        //! Creates a sound entity with given filename
        EnAmbientSound*                             setupSound( const std::string& filename, float volume );

        //! Create a scene for menu
        void                                        createMenuScene();

        //! Switch the menu and game scenes
        void                                        switchMenuScene( bool tomenu );

        //! Menu states
        enum
        {
            None,
            BeginIntro,
            Intro,
            BeginLoadingLevel,
            LoadingLevel,
            UnloadLevel,
            Visible,
            Hidden
        }                                           _menuState;

        //! Internal state used for loading a level for server or standalone mode
        enum
        {
            ForServer,
            ForStandalone
        }                                           _levelSelectionState;

        osg::ref_ptr< osg::Group >                  _menuScene;

        osg::ref_ptr< osg::Node >                   _levelScene;

        osg::ref_ptr< osg::Group >                  _menuAnimationPath;

        EnCamera*                                   _p_cameraControl;

        EnSkyBox*                                   _p_skyBox;

        EnFog*                                      _p_fog;

        std::auto_ptr< EnAmbientSound >             _clickSound;

        std::auto_ptr< EnAmbientSound >             _hoverSound;

        std::auto_ptr< EnAmbientSound >             _introSound;

        std::auto_ptr< DialogGameSettings >         _settingsDialog;

        std::auto_ptr< DialogLevelSelect >          _levelSelectDialog;

        std::auto_ptr< IntroControl >               _intro;

        CEGUI::Window*                              _p_menuWindow;

        CEGUI::Window*                              _p_loadingWindow;

        CEGUI::PushButton*                          _p_btnStartJoin;

        CEGUI::PushButton*                          _p_btnStartServer;

        CEGUI::PushButton*                          _p_btnStartWT;

        CEGUI::PushButton*                          _p_btnReturn;

        CEGUI::PushButton*                          _p_btnLeave;

        CEGUI::StaticImage*                         _p_loadingOverly;

        CEGUI::Image*                               _p_loadingOverlayImage;

        CEGUI::StaticImage*                         _p_loadingLevelPic;

        std::string                                 _queuedLevelFile;

        MenuInputHandler*                           _p_inputHandler;

        //! Shows that a level is already loaded
        bool                                        _levelLoaded;

    friend class DialogGameSettings;
    friend class DialogLevelSelect;
    friend class MenuInputHandler;
    friend class IntroControl;
};

//! Entity type definition used for type registry
class MenuEntityFactory : public BaseEntityFactory
{
    public:
                                                    MenuEntityFactory() : 
                                                     BaseEntityFactory( ENTITY_NAME_MENU, BaseEntityFactory::Standalone | BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~MenuEntityFactory() {}

        Macro_CreateEntity( EnMenu );
};

}

#endif // _CTD_MENU_H_
