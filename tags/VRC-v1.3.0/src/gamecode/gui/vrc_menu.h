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
 # entity game menu
 #
 #   date of creation:  04/24/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
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
class DialogLogin;
class MenuInputHandler;
class En2DSound;
class IntroControl;
class EnCamera;

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

        std::string                                 _loginDialogConfig;

        std::string                                 _levelSelectDialogConfig;

        std::string                                 _introTexture;

        std::string                                 _loadingOverlayTexture;

        std::string                                 _introductionSound;

        float                                       _introductionSoundVolume;

        std::string                                 _backgroundSound;

        float                                       _backgroundSoundVolume;

        std::string                                 _menuCameraPathFile;

        float                                       _menuIdleTimeout;

        osg::Vec3f                                  _cameraBackgroundColor;

        float                                       _cameraFov;
        
        float                                       _cameraNearClip;
        
        float                                       _cameraFarClip;

    protected:

        //! Interface method used by settings dialog
        //---

        //! This is called when the setings dialog closes.
        void                                        onSettingsDialogClose();

        //! This is called when the login dialog closes. 'login' is false if cancel button has been clicked.
        void                                        onLoginDialogClose( bool btnlogin );

        //! Enable / disable background music
        void                                        setBkgMusicEnable( bool en );

        //! Set background music volume
        void                                        setBkgMusicVolume( float volume );
        //---

        //! Begin intro
        void                                        beginIntro();

        //! Stop intro
        void                                        stopIntro();

        //! Called whenever an user interaction with gui occured, this is used for detecting idling
        void                                        triggerUserInteraction();

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

        //! Callback for button click "start walk-through"  
        bool                                        onClickedWT( const CEGUI::EventArgs& arg );

        //! Callback for mouse hover ( over a button ), used for playing sound
        bool                                        onButtonHover( const CEGUI::EventArgs& arg );

        //! Creates a sound entity with given filename
        En2DSound*                                  setupSound( const std::string& filename, float volume, bool loop ) const;

        //! Create a scene for menu
        bool                                        createMenuScene();

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
            FadeIntoLevel,
            PrepareUnloadLevel,
            UnloadLevel,
            Quitting,
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
            ForStandalone
        }                                           _levelSelectionState;

        osg::ref_ptr< osg::Node >                   _menuScene;

        osg::ref_ptr< osg::Node >                   _levelScene;

        osg::ref_ptr< osg::Group >                  _menuAnimationPath;

        EnCamera*                                   _p_cameraControl;

        En2DSound*                                  _p_introSound;

        float                                       _introTimeScale;

        En2DSound*                                  _p_backgrdSound;

        float                                       _backgrdSoundVolume;

        std::auto_ptr< DialogGameSettings >         _settingsDialog;

        std::auto_ptr< DialogLogin >                _loginDialog;

        std::auto_ptr< DialogLevelSelect >          _levelSelectDialog;

        std::auto_ptr< IntroControl >               _intro;

        CEGUI::Window*                              _p_menuWindow;

        CEGUI::Window*                              _p_loadingWindow;

        CEGUI::PushButton*                          _p_btnStartJoin;

        CEGUI::PushButton*                          _p_btnStartWT;

        CEGUI::PushButton*                          _p_btnReturn;

        CEGUI::PushButton*                          _p_btnLeave;

        CEGUI::StaticImage*                         _p_loadingOverly;

        CEGUI::StaticImage*                         _p_loadingLevelPic;

        gameutils::LevelFiles*                      _p_clientLevelFiles;

        std::string                                 _queuedLevelFile;

        MenuInputHandler*                           _p_inputHandler;

        float                                       _soundFadingCnt;

        float                                       _idleCounter;

        //! Shows that a level is already loaded
        bool                                        _levelLoaded;

        //! Menu fade out timer used after loading a level
        float                                       _fadeIntoLevelTimer;

    friend class DialogGameSettings;
    friend class DialogLevelSelect;
    friend class DialogLogin;
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
