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
 # common game utilities
 #
 #   date of creation:  06/16/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#ifndef _VRC_GAMEUTILS_H_
#define _VRC_GAMEUTILS_H_

#include <vrc_main.h>

namespace vrc
{

//! Class declarations in vrc namespace
class UserInventory;

namespace gameutils
{

//! Application name for server
#ifdef WIN32
    #define VRC_SERVER_APP_NAME             "vrc-server.exe"
#endif
#ifdef LINUX
    #define VRC_SERVER_APP_NAME             "vrc"
#endif

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
#define VRC_GS_KEY_OBJECTPICK               "objectPick"
#define VRC_GS_MOUSESENS                    "mouseSensitivity"
#define VRC_GS_INVERTMOUSE                  "mouseInverted"
#define VRC_GS_MUSIC_ENABLE                 "enableMusic"
#define VRC_GS_MUSIC_VOLUME                 "volumeMusic"
#define VRC_GS_FX_ENABLE                    "enableFX"
#define VRC_GS_FX_VOLUME                    "volumeFX"
#define VRC_GS_VOICECHAT_ENABLE             "voiceChatEnable"
#define VRC_GS_VOICECHAT_INPUT_DEVICE       "voiceChatInputDevice"
#define VRC_GS_VOICE_INPUT_GAIN             "voiceInputGain"
#define VRC_GS_VOICE_OUTPUT_GAIN            "voiceOutputGain"
#define VRC_GS_VOICE_PORT                   "voicePort"
#define VRC_GS_VOICE_IP_FWD                 "voiceIpForward"
#define VRC_GS_VOICE_IP_FWD_PORT            "voiceIpForwardPort"
#define VRC_GS_DB_IP                        "dbIP"
#define VRC_GS_DB_PORT                      "dbPort"
#define VRC_GS_DB_NAME                      "dbName"
#define VRC_GS_DB_USER                      "dbUser"

//! Settings default values
#define VRC_GS_DEFAULT_PLAYER_CONFIG        "default_cfg"

#define VRC_GS_DEFAULT_VOICE_PORT           31200
#define VRC_GS_DEFAULT_SOUND_VOLUME         1.0f
// Input device not ready
#define VRC_GS_VOICECHAT_INPUT_DEVICE_NA    0

//! Game code settings' defaults
#define VRC_GS_MAX_MOUSESENS                3.0f                // maximal mouse sensitivity

//! GUI's standard sounds
#define GUI_SND_NAME_CLICK                  "cgui_cli"
#define GUI_SND_NAME_HOVER                  "cgui_hov"
#define GUI_SND_NAME_SCROLLBAR              "cgui_scr"
#define GUI_SND_NAME_ATTENTION              "cgui_att"

//! VRC specific GUI imageset and elements
#define VRC_IMAGE_SET                       "VRCImageSet"
#define VRC_IMAGE_SET_FILE                  "gui/imagesets/VRCImageSet.imageset"
#define IMAGE_NAME_FOOT_NORMAL              "FootNormal"
#define IMAGE_NAME_FOOT_HOOVER              "FootHoover"
#define IMAGE_NAME_HAND_NORMAL              "HandNormal"
#define IMAGE_NAME_HAND_HOOVER              "HandHoover"
#define IMAGE_NAME_CROSSHAIR                "Crosshair"
#define IMAGE_NAME_POST                     "Post"
#define IMAGE_NAME_SOUND_ON                 "SoundOn"
#define IMAGE_NAME_SOUND_OFF                "SoundOff"


//! This class handles game state changes
class VRCStateHandler : public yaf3d::GameState::CallbackStateChange
{
    public:
                                                    VRCStateHandler();

        virtual                                     ~VRCStateHandler();

        //! Callback method for state changes
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

        std::string                                 _objectPick;

        bool                                        _musicEnable;

        float                                       _musicVolume;

        bool                                        _fxEnable;

        float                                       _fxVolume;

        bool                                        _voiceChatEnable;

        unsigned int                                _voiceChatInputDev;

        float                                       _voiceInputGain;

        float                                       _voiceOutputGain;

        unsigned int                                _voicePort;

        bool                                        _voiceIpForward;

        unsigned int                                _voiceIpForwardPort;

        std::string                                 _dbIp;

        unsigned int                                _dbPort;

        std::string                                 _dbName;

        std::string                                 _dbUser;
};

//! Single instance providing player-related utility services
class PlayerUtils : public yaf3d::Singleton< vrc::gameutils::PlayerUtils >
{
    public:

                                                    PlayerUtils();

        enum PlayerControlModes
        {
            eLockMovement       = 0x0001,
            eLockLooking        = 0x0002,
            eLockCameraSwitch   = 0x0004,
            eLockPicking        = 0x0008
        };

        //! Set the player input control modes, a combination of PlayerControlModes flags.
        inline void                                 setPlayerControlModes( unsigned int modes );

        //! Get the player input control modes, a combination of PlayerControlModes flags.
        inline unsigned int                         getPlayerControlModes() const;

        //! Is the interaction lock set?
        bool                                        isLockInteraction() const;

        //! Retrieve player configuration file path depending on game settings and given game mode ( Server, Client, Standalone ) and
        //! in case of Client the remote flag determines local or remote client. If 'cfgfile' is not empty then that file is used for getting the names of the palyer config files.
        //! Returns false if something went wrong.
        bool                                        getPlayerConfig( unsigned int mode, bool remote, std::string& levelfile, const std::string& cfgfile = "" );

        //! Store a pointer to local player entity
        void                                        setLocalPlayer( yaf3d::BaseEntity* p_entity );

        //! Return the previousely set local player entity
        yaf3d::BaseEntity*                          getLocalPlayer();

        //! Set the player inventory, used by player entity on initialization
        void                                        setPlayerInventory( UserInventory* p_inv );

        //! Get the player inventory
        UserInventory*                              getPlayerInventory();

        //! Add a new remote player ( ghost ) into internal list
        void                                        addRemotePlayer( yaf3d::BaseEntity* p_entity );

        //! Remove a remote player from internal list
        void                                        removeRemotePlayer( yaf3d::BaseEntity* p_entity );

        //! Add a new remote player ( ghost ) supporting voice chat into internal list
        void                                        addRemotePlayerVoiceChat( yaf3d::BaseEntity* p_entity );

        //! Remove a remote player supporting voice chat from internal list
        void                                        removeRemotePlayerVoiceChat( yaf3d::BaseEntity* p_entity );

        //! Return the list of remote players
        inline const std::vector< yaf3d::BaseEntity* >&   getRemotePlayers();

        //! Return the list of remote players supporting voice chat
        inline const std::vector< yaf3d::BaseEntity* >&   getRemotePlayersVoiceChat();

        //! Class used for getting notified whenever the player list ( also those supporting voice chat ) changed
        class CallbackPlayerListChange
        {
            public:
                                                        CallbackPlayerListChange() {}

                virtual                                 ~CallbackPlayerListChange() {}

                /** If 'localplayer' is true then the local player has been changed, otherwise the remote player list has been changed.
                    If 'localplayer' is true then get the entity of local player using PlayerUtil's method 'getLocalPlayer'.
                    In overridden callback method access the leaver / joiner depending on 'joining'.
                    In case a player is joining 'joining' is true otherwise it is false.
                    For a complete list of players use the PlayerUtil's methods 'getRemotePlayers' or 'getRemotePlayersVoiceChat'.
                **/
                virtual void                            onPlayerListChanged( bool /*localplayer*/, bool /*joining*/, yaf3d::BaseEntity* /*p_entity*/ ) {}

                //! Similar to onPlayerListChanged but only for players with voice chat enabled.
                virtual void                            onVoiceChatPlayerListChanged( bool /*joining*/, yaf3d::BaseEntity* /*p_entity*/ ) {}
        };

        //! Callback registration for changed player list.
        //! Use reg = true for registration and reg = false for deregistration
        void                                        registerCallbackPlayerListChanged( CallbackPlayerListChange* p_func, bool reg = true );

        //! Callback registration for changed player list supporting voice chat.
        //! Use reg = true for registration and reg = false for deregistration
        void                                        registerCallbackVoiceChatPlayerListChanged( CallbackPlayerListChange* p_func, bool reg = true );

    protected:

        yaf3d::BaseEntity*                          _p_localPlayer;

        UserInventory*                              _p_userInventory;

        std::vector< yaf3d::BaseEntity* >           _remotePlayers;

        std::vector< CallbackPlayerListChange* >    _cbsPlayerList;

        std::vector< yaf3d::BaseEntity* >           _remotePlayersVoiceChat;

        std::vector< CallbackPlayerListChange* >    _cbsPlayerListVoiceChat;

        unsigned int                                _playerControlModes;

    friend class yaf3d::Singleton< vrc::gameutils::PlayerUtils >;
};

inline void PlayerUtils::setPlayerControlModes( unsigned int modes )
{
    _playerControlModes = modes;
}

inline unsigned int PlayerUtils::getPlayerControlModes() const
{
    return _playerControlModes;
}

inline const std::vector< yaf3d::BaseEntity* >& PlayerUtils::getRemotePlayers()
{
    return _remotePlayers;
}

inline const std::vector< yaf3d::BaseEntity* >& PlayerUtils::getRemotePlayersVoiceChat()
{
    return _remotePlayersVoiceChat;
}

//! A generic input handler class with automatic adding and removing to / from viewer's event hanlder list
struct NullType {};
template< class T = NullType >
class GenericInputHandler : public osgGA::GUIEventHandler
{
    public:

        explicit                                    GenericInputHandler( T* p_obj = NULL ) :
                                                     _p_userObject( p_obj ),
                                                     _destroyed( false )
                                                    {
                                                        // register us in viewer to get event callbacks
                                                        yaf3d::Application::get()->getViewer()->addEventHandler( this );
                                                    }

        //! NOTE: delete such an object only if you have removed it from handler list by yourself!
        //        In normal case you should use the destroyHandler() method. The object is then deleted automatically at right place and time.
        virtual                                     ~GenericInputHandler()
                                                    {
                                                        if ( !_destroyed )
                                                            destroyHandler();
                                                    }

        //! Remove handler form viewer's handler list and destroy the object. Don't use the object after calling this method.
        void                                        destroyHandler()
                                                    {
                                                        // remove this handler from viewer's handler list
                                                        yaf3d::Application::get()->getViewer()->removeEventHandler( this );
                                                        _destroyed = true;
                                                    }

        //! Override this handler method in order to get events.
        virtual bool                                handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa ) = 0;

        //! Return the user object
        T*                                          getUserObject() { return _p_userObject; }

    protected:

        //! An optional object which can be accessed in 'handle' method.
        T*                                          _p_userObject;

        //! This flag is used for calling destroyHandler on destruction if the user has forgotten that.
        bool                                        _destroyed;
};

//! Single instance providing GUI-related utility services
class GuiUtils :
    public yaf3d::Singleton< vrc::gameutils::GuiUtils >,
    public yaf3d::GameState::CallbackStateChange,
    public yaf3d::GameState::CallbackAppWindowStateChange
{
    public:

                                                    GuiUtils();

        virtual                                     ~GuiUtils();

        //! Returns the main GUI window.
        //! Note: all other GUIs in a level must be attached to this one.
        CEGUI::Window*                              getMainGuiWindow();

        //! Show / hide main window
        void                                        showMainWindow( bool show );

        //! Get application specific GUI imageset.
        CEGUI::Imageset*                            getCustomImageSet();

        //! Get an image from custom imageset given its name, returns NULL if image does not exist.
        const CEGUI::Image*                         getCustomImage( const std::string& name );

        //! Hide mouse pointer
        void                                        hidePointer();

        //! Show mouse pointer. Pass 'true' in order to show, otherwise hide the pointer.
        void                                        showMousePointer( bool show );

        //! Create a non-looped sound given a sound file. Later the sound can be accessed via its 'name'.
        //! Returns 0 if something goes wrong, otherwise the sound ID created by SoundManger.
        unsigned int                                createSound( const std::string& name, const std::string& filename, float volume = 0.2f );

        //! Play a previousely created sound given its name.
        void                                        playSound( const std::string& name );

        //! Given a sound name return its sound ID. Returns 0 if sound name does not exist.
        unsigned int                                getSoundID( const std::string& name );

        //! Callback method for auto-shutdown when application is shutting down
        void                                        onStateChange( unsigned int state );

        //! Callback method for getting application window state changes
        void                                        onAppWindowStateChange( unsigned int state );

    protected:

        //! The main window instance
        CEGUI::Window*                              _p_mainWindow;

        //! yaf3d::Application's root window
        CEGUI::Window*                              _p_rootWindow;

        //! Application specific imageset
        CEGUI::Imageset*                            _p_vrcImageSet;

        typedef std::map< std::string, unsigned int > MapSound;

        //! A map of sound object and its associated name
        MapSound                                    _soundMap;

        //! Used for storing master sound volume on app window minimizing
        float                                       _masterVolume;

    friend class yaf3d::Singleton< vrc::gameutils::GuiUtils >;
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

        std::vector< std::string >                  _imageSets;
};

//! Read the six sides of a cube map and return a texture
osg::ref_ptr< osg::TextureCubeMap > readCubeMap( const std::vector< std::string >& texfiles );

//! This is a class for adjusting the transforming to eye coordinated.
/**  In osg examples you find it as class "MoveEarthySkyWithEyePointTransform"
*    It is used by entities like skybox and water.
*/
class EyeTransform : public osg::Transform
{
    public:

        /** Get the transformation matrix which moves from local coords to world coords.*/
        virtual bool                        computeLocalToWorldMatrix( osg::Matrix& matrix, osg::NodeVisitor* p_nv ) const
                                            {
                                                osgUtil::CullVisitor* p_cv = dynamic_cast< osgUtil::CullVisitor* >( p_nv );
                                                if ( p_cv )
                                                {
                                                    const osg::Vec3& eyePointLocal = p_cv->getEyeLocal();
                                                    matrix.preMult( osg::Matrix::translate( eyePointLocal ) );
                                                }
                                                return true;
                                            }

        /** Get the transformation matrix which moves from world coords to local coords.*/
        virtual bool                        computeWorldToLocalMatrix( osg::Matrix& matrix, osg::NodeVisitor* p_nv ) const
                                            {
                                                osgUtil::CullVisitor* p_cv = dynamic_cast< osgUtil::CullVisitor* >( p_nv );
                                                if ( p_cv )
                                                {
                                                    const osg::Vec3& eyePointLocal = p_cv->getEyeLocal();
                                                    matrix.postMult( osg::Matrix::translate( -eyePointLocal ) );
                                                }
                                                return true;
                                            }
};

//! Visitor for getting the world transformation considering all PositionAttitudeTransform nodes in path
class TransformationVisitor : public osg::NodeVisitor
{
    public:
                                            TransformationVisitor( osg::NodeVisitor::TraversalMode tmode = osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ) :
                                                osg::NodeVisitor( tmode )
                                            {
                                                // we take all nodes
                                                setTraversalMask( 0xffffffff );
                                            }

        virtual                             ~TransformationVisitor() {}

        void                                apply( osg::PositionAttitudeTransform& /*node*/ )
                                            {
                                                _matrix *= osg::computeLocalToWorld( getNodePath() );
                                            }

        const osg::Matrixf&                 getMatrix() const
                                            {
                                                return _matrix;
                                            }

    protected:

        osg::Matrixf                        _matrix;
};

//! Update texture matrix for cubemaps ( see osg's VertexProgram example )
class TexMatCallback : public osg::NodeCallback
{
    public:

        explicit                            TexMatCallback( osg::TexMat& tex ) :
                                             _texMat( tex ),
                                             _R(
                                                osg::Matrix::rotate( osg::DegreesToRadians( 90.0f ), 0.0f, 0.0f, 1.0f ) *
                                                osg::Matrix::rotate( osg::DegreesToRadians( 90.0f ), 1.0f, 0.0f, 0.0f )
                                               )
                                            {
                                            }

        virtual void                        operator()( osg::Node* p_node, osg::NodeVisitor* p_nv )
                                            {
                                                osgUtil::CullVisitor* p_cv = dynamic_cast< osgUtil::CullVisitor* >( p_nv );
                                                if ( p_cv )
                                                {
                                                    osg::RefMatrix* MV = p_cv->getModelViewMatrix();
                                                    osg::Quat q;
                                                    MV->get( q );
                                                    const osg::Matrix C = osg::Matrixf::rotate( q.inverse() );

                                                    _texMat.setMatrix( C * _R );
                                                }
                                                traverse( p_node, p_nv );
                                            }

        osg::TexMat&                        _texMat;

        osg::Matrixf                        _R;
};

} // namespace gameutils
} // namespace vrc

#endif //_VRC_GAMEUTILS_H_
