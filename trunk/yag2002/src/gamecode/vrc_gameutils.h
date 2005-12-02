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
 # common game utilities
 #
 #   date of creation:  06/16/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_GAMEUTILS_H_
#define _VRC_GAMEUTILS_H_

#include <base.h>

namespace vrc
{
namespace gameutils
{

//! VRC version
#define VRC_VERSION                         "0.5.0"

//! Game code's configuration setting names
#define VRC_GS_PLAYER_NAME                  "playerName"
#define VRC_GS_PLAYER_CONFIG_DIR            "playerConfigDir"
#define VRC_GS_PLAYER_CONFIG                "playerConfig"
#define VRC_GS_KEY_MOVE_FORWARD             "moveForward"
#define VRC_GS_KEY_MOVE_BACKWARD            "moveBackward"
#define VRC_GS_KEY_MOVE_LEFT                "moveLeft"
#define VRC_GS_KEY_MOVE_RIGHT               "moveRight"
#define VRC_GS_KEY_JUMP                     "jump"
#define VRC_GS_KEY_CAMERAMODE               "cameraMode"
#define VRC_GS_KEY_CHATMODE                 "chatMode"
#define VRC_GS_MOUSESENS                    "mouseSensitivity"
#define VRC_GS_INVERTMOUSE                  "mouseInverted"
//! Game code settings' defaults
#define VRC_GS_MAX_MOUSESENS                3.0f                // maximal mouse sensitivity

//! GUI's standard sounds
#define GUI_SND_NAME_CLICK                  "cgui_cli"
#define GUI_SND_NAME_HOVER                  "cgui_hov"
#define GUI_SND_NAME_SCROLLBAR              "cgui_scr"
#define GUI_SND_NAME_ATTENTION              "cgui_att"


//! This class is responsible for registration of all game code (VRC) related configuration settings
class VRCConfigRegistry : public yaf3d::GameState::CallbackStateChange
{
    public:
                                                    VRCConfigRegistry();

        virtual                                     ~VRCConfigRegistry();

        //! Callback method ( we register the settings during 'Initializing' state )
        void                                        onStateChange( unsigned int state );

    protected:

        std::string                                 _playerName;

        std::string                                 _playerConfig;

        std::string                                 _playerConfigDir;

        float                                       _mouseSensitivity;

        bool                                        _mouseInverted;

        std::string                                 _moveForward;

        std::string                                 _moveBackward;
        
        std::string                                 _moveLeft;
        
        std::string                                 _moveRight;

        std::string                                 _jump;

        std::string                                 _cameramode;

        std::string                                 _chatmode;
};

//! Single instance providing player-related utility services
class PlayerUtils : public yaf3d::Singleton< vrc::gameutils::PlayerUtils >
{
    public:

                                                    PlayerUtils();

        //! Retrieve player configuration file path depending on game settings and given game mode ( Server, Client, Standalone ) and
        //! in case of Client the remote flag determines local or remote client.
        //! Returns false if something went wrong.
        bool                                        getPlayerConfig( unsigned int mode, bool remote, std::string& cfgfile );

        //! Stores a pointer to local player entity
        void                                        setLocalPlayer( yaf3d::BaseEntity* p_entity );

        //! Return the previousely set local player entity
        yaf3d::BaseEntity*                          getLocalPlayer();

        //! Add a new remote player ( ghost ) into internal list
        void                                        addRemotePlayer( yaf3d::BaseEntity* p_entity );

        //! Remove a remote player from internal list
        void                                        removeRemotePlayer( yaf3d::BaseEntity* p_entity );

        //! Return the list of remote players
        inline std::vector< yaf3d::BaseEntity* >&   getRemotePlayers();

    protected:

        yaf3d::BaseEntity*                          _p_localPlayer;
    
        std::vector< yaf3d::BaseEntity* >           _remotePlayers;

    friend public yaf3d::Singleton< vrc::gameutils::PlayerUtils >;
};

inline std::vector< yaf3d::BaseEntity* >& PlayerUtils::getRemotePlayers()
{                                                     
    return _remotePlayers;
}

//! Single instance providing GUI-related utility services
class GuiUtils : public yaf3d::Singleton< vrc::gameutils::GuiUtils >
{
    public:

                                                    GuiUtils();

        virtual                                     ~GuiUtils();

        //! Returns the main GUI window.
        //! Note: all other GUIs in a level must be attached to this one.
        CEGUI::Window*                              getMainGuiWindow();

        //! Show / hide main window
        void                                        showMainWindow( bool show );

        //! Destroy main window
        void                                        destroyMainWindow();

        //! Hide mouse pointer
        void                                        hidePointer();

        //! Show mouse pointer. Pass 'true' in order to show, otherwise hide the pointer.
        void                                        showMousePointer( bool show );

        //! Create a non-looped sound given a sound file (wave). Later the sound can be accessed via its 'name'.
        osg::ref_ptr< osgAL::SoundState >           createSound( const std::string& name, const std::string& filename, float volume = 0.2f );

        //! Given a sound name return its sound object. Returns an empty ret_ptr if sound name does not exist.
        osg::ref_ptr< osgAL::SoundState >           getSound( const std::string& name );

        //! Play a previousely created sound.
        void                                        playSound( const std::string& name );

    protected:

        //! The main window instance
        CEGUI::Window*                              _p_mainWindow;        
        
        //! yaf3d::Application's root window
        CEGUI::Window*                              _p_rootWindow;

        typedef std::map< std::string, osg::ref_ptr< osgAL::SoundState > > MapSound;

        //! A map of sound object and its associated name
        MapSound                                    _soundMap;

    friend public yaf3d::Singleton< vrc::gameutils::GuiUtils >;
};

//! Helper class for getting a lookup table with level files and their preview images
class LevelFiles
{
    public:

        //! Given a directory all preview images are gathered in a lookup table
                                                    LevelFiles( const std::string& dir );

        virtual                                     ~LevelFiles();

        //! Given a file name return its preview image. NULL if the file or preview pic does not exist.
        CEGUI::Image*                               getImage( const std::string& file );

        //! Get look up table
        std::map< std::string, CEGUI::Image* >&     getAllFiles() { return _files; }

        //! Get count of level files
        unsigned int                                count() { return _files.size(); }

    protected:

        std::map< std::string, CEGUI::Image* >      _files;
};

} // namespace gameutils
} // namespace vrc

#endif //_VRC_GAMEUTILS_H_
