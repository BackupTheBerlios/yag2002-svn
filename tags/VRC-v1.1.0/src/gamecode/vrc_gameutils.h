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
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_GAMEUTILS_H_
#define _VRC_GAMEUTILS_H_

#include <base.h>
#include <singleton.h>

namespace vrc
{
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
#define VRC_GS_MOUSESENS                    "mouseSensitivity"
#define VRC_GS_INVERTMOUSE                  "mouseInverted"
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

//! VRC specific entity notification IDs
#define VRC_NOTIFY_PLAYERLIST_CHANGED       0xA0000000


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

        //! Store a pointer to local player entity
        void                                        setLocalPlayer( yaf3d::BaseEntity* p_entity );

        //! Return the previousely set local player entity
        yaf3d::BaseEntity*                          getLocalPlayer();

        //! Add a new remote player ( ghost ) into internal list
        void                                        addRemotePlayer( yaf3d::BaseEntity* p_entity );

        //! Remove a remote player from internal list
        void                                        removeRemotePlayer( yaf3d::BaseEntity* p_entity );

        //! Return the list of remote players
        inline std::vector< yaf3d::BaseEntity* >&   getRemotePlayers();

        //! Notification mechanism for changed player list. The method 'handleNotification' is used with id 'VRC_NOTIFY_PLAYERLIST_CHANGED'
        //! Use reg = true for registration and reg = false for deregistration
        void                                        registerNotificationPlayerListChanged( yaf3d::BaseEntity* p_entity, bool reg = true );

    protected:

        yaf3d::BaseEntity*                          _p_localPlayer;

        std::vector< yaf3d::BaseEntity* >           _remotePlayers;

        std::vector< yaf3d::BaseEntity* >           _notificationList;

    friend class yaf3d::Singleton< vrc::gameutils::PlayerUtils >;
};

inline std::vector< yaf3d::BaseEntity* >& PlayerUtils::getRemotePlayers()
{
    return _remotePlayers;
}

//! A generic input handler class with automatic adding and removing to / from viewer's event hanlder list
struct NullType {};
template< class T = NullType >
class GenericInputHandler : public osgGA::GUIEventHandler
{
    public:

        explicit                                  GenericInputHandler( T* p_obj = NULL ) : 
                                                   _p_userObject( p_obj ),
                                                   _destroyed( false )
                                                  {
                                                      // register us in viewer to get event callbacks
                                                      yaf3d::Application::get()->getViewer()->addEventHandler( this );
                                                  }

        virtual                                   ~GenericInputHandler() 
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
class GuiUtils : public yaf3d::Singleton< vrc::gameutils::GuiUtils >, public yaf3d::GameState::CallbackStateChange
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

        void                                apply( osg::PositionAttitudeTransform& node )
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
                                                    const osg::Matrixf& MV = p_cv->getModelViewMatrix();
                                                    osg::Quat q;
                                                    MV.get( q );
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
