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

#ifndef _VRC_MENU_H_
#define _VRC_MENU_H_

#include <vrc_main.h>

namespace vrc
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

namespace gameutils
{
    class LevelFiles;
}

//! The menu system is controlled by this entity
class EnMenu :  public yaf3d::BaseEntity
{
    public:
                                                    EnMenu();

        virtual                                     ~EnMenu();

        //! Initializing function, this is called after all engine modules are initialized and a map is loaded.
        void                                        initialize();

        //! This entity does not need a transform node, which would be created by level manager on loading
        const bool                                  isTransformable() const { return false; }

        //! Make this entity persistent, thus on level changing this entity will not be deleted by entity manager
        //  and update and notification registries will be kept.
        const bool                                  isPersistent() const { return true; }

    protected:

        //! This entity needs updating
        void                                        updateEntity( float deltaTime );

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

        //! Override this method of yaf3d::BaseEntity to get notifications (from menu system)
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        //! Load a new level, optionally a background image can be shown during loading
        void                                        loadLevel( std::string levelfile, CEGUI::Image* p_img = NULL );

        //! Entity parameters

        std::string                                 _menuConfig;

        std::string                                 _settingsDialogConfig;

        std::string                                 _levelSelectDialogConfig;

        std::string                                 _introTexture;

        std::string                                 _loadingOverlayTexture;

        std::string                                 _buttonClickSound;

        float                                       _buttonClickSoundVolume;

        std::string                                 _buttonHoverSound;

        float                                       _buttonHoverSoundVolume;

        std::string                                 _introductionSound;

        float                                       _introductionSoundVolume;

        std::string                                 _backgroundSound;

        float                                       _backgroundSoundVolume;

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
        EnAmbientSound*                             setupSound( const std::string& filename, float volume, bool loop ) const;

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
            PrepareLoadingLevel,
            LoadingLevel,
            UnloadLevel,
            Visible,
            Hidden
        }                                           _menuState;

        //! Menu sound states
        enum
        {
            SoundStopped,
            SoundFadeIn,
            SoundFadingIn,
            SoundFadeOut
        }                                           _menuSoundState;

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

        EnFog*                                      _p_menuFog;

        EnFog*                                      _p_sceneFog;

        EnAmbientSound*                             _p_clickSound;

        EnAmbientSound*                             _p_hoverSound;

        EnAmbientSound*                             _p_introSound;

        EnAmbientSound*                             _p_backgrdSound;

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

        CEGUI::StaticImage*                         _p_loadingLevelPic;

        gameutils::LevelFiles*                      _p_clientLevelFiles;

        std::string                                 _queuedLevelFile;

        MenuInputHandler*                           _p_inputHandler;

        float                                       _soundFadingCnt;

        //! Shows that a level is already loaded
        bool                                        _levelLoaded;

    friend class DialogGameSettings;
    friend class DialogLevelSelect;
    friend class MenuInputHandler;
    friend class IntroControl;
};

//! Entity type definition used for type registry
class MenuEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    MenuEntityFactory() : 
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_MENU, yaf3d::BaseEntityFactory::Standalone | yaf3d::BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~MenuEntityFactory() {}

        Macro_CreateEntity( EnMenu );
};

} // namespace vrc

#endif // _VRC_MENU_H_
