/****************************************************************
 *  Project YAG2002 (http://yag2002.sourceforge.net)
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
 # main application class
 #
 #   date of creation:  02/17/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_base.h>
#include "ctd_levelmanager.h"
#include "ctd_entitymanager.h"
#include "ctd_physics.h"
#include "ctd_guimanager.h"
#include "ctd_configuration.h"
#include "ctd_keymap.h"
#include "ctd_gamestate.h"
#include "ctd_network.h"
#include "ctd_log.h"
#include "ctd_utils.h"
#include "ctd_application.h"


// app icon and tile
#define YAF3D_APP_TITLE           "VRC"
#define YAF3D_APP_ICON            "icon.bmp"

// environment variable for media directory
//  if not existing then the relative path '../../media' of executable is assumed
#define YAF3D_ENV_MEDIA_DIR	"YAF3D_ENV_MEDIA_DIR"

//! Location of level file directory for Server, Client, and Standalone level
#define YAF3D_LEVEL_SERVER_DIR    "level/server/"
#define YAF3D_LEVEL_CLIENT_DIR    "level/client/"
#define YAF3D_LEVEL_SALONE_DIR    "level/standalone/"

// log file names
#define LOG_FILE_NAME           "vrc.log"
#define LOG_FILE_NAME_SERVER    "vrc-server.log"

// media path relative to inst dir
#define YAF3D_MEDIA_PATH          "/media/"
// default level
#define YAF3D_DEFAULT_LEVEL       "gui/loader"


namespace yaf3d
{

YAF3D_SINGLETON_IMPL( Application );

Application::Application():
_p_networkDevice( NULL ),
_p_entityManager( EntityManager::get() ),
_p_guiManager( NULL ),
_p_physics( Physics::get() ),
_p_gameState( GameState::get() ),
_p_viewer( NULL ),
_screenWidth( 600 ),
_screenHeight( 400 ),
_fullScreen( false )
{
}

Application::~Application()
{
}

void Application::shutdown()
{
    log << Log::LogLevel( Log::L_INFO ) << std::endl;
    log << "---------------------------------------" << std::endl;
    log << "shutting down, time: " << yaf3d::getTimeStamp() << std::endl;

    NetworkDevice::get()->shutdown();
    LevelManager::get()->shutdown();
    Configuration::get()->shutdown();
    GameState::get()->shutdown();
    SettingsManager::get()->shutdown();
    KeyMap::get()->shutdown();

    delete _p_viewer;

    SDL_Quit();

    destroy();
}

bool Application::initialize( int argc, char **argv )
{
#ifdef YAF3D_ENABLE_HEAPCHECK
    // trigger debugger
    //__asm int 3;
#endif

    // set game state
    _p_gameState->setState( GameState::Initializing );

    std::string arg_levelname;
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments( &argc,argv );
    osg::ArgumentParser::Parameter levelparam( arg_levelname );
    arguments.read( "-level", arg_levelname ); // read the level file if one given

    int   argpos;
    // set proper game mode
    GameState::get()->setMode( GameState::Standalone );
    if ( ( argpos = arguments.find( "-server" ) ) != 0 )
    {
        GameState::get()->setMode( GameState::Server );
        arguments.remove( argpos );
    }
    else if ( ( argpos = arguments.find( "-client" ) ) != 0 )
    {
        GameState::get()->setMode( GameState::Client );
        arguments.remove( argpos );
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages( std::cout );
    }

    // set the media path as first step, other modules need it for loading resources etc.
    //-------------------
    std::vector< std::string > path;
    std::string dir;
    {
        char* p_env = getenv( YAF3D_ENV_MEDIA_DIR );
        if ( p_env )
        {
            _mediaPath = p_env;
        }
        else
        {
            dir = getCurrentWorkingDirectory();
            dir = cleanPath( dir );
            dir += "/";
            path.clear();
            explode( dir, "/", &path );
#ifdef LINUX
            dir = "/";
#endif
#ifdef WIN32
            dir = "";
#endif
            for ( size_t cnt = 0; cnt < path.size() - 2; cnt++ )
                dir += path[ cnt ] + "/";

            dir.erase( dir.size() -1 );
            _mediaPath = dir;
            _mediaPath += YAF3D_MEDIA_PATH;
        }
    }

    //-------------------
    // set the ful binary path of application
    _fulBinaryPath = arguments.getApplicationName();

    //-------------------

    // setup log system
    //-----------------
    //! TODO: set the loglevel basing on config file
    if ( GameState::get()->getMode() != GameState::Server )
        log.addSink( "file", getMediaPath() + std::string( LOG_FILE_NAME ), Log::L_ERROR );
    else
        log.addSink( "file", getMediaPath() + std::string( LOG_FILE_NAME_SERVER ), Log::L_ERROR );

    log.addSink( "stdout", std::cout, Log::L_ERROR );

    log.enableSeverityLevelPrinting( false );
    log << Log::LogLevel( Log::L_INFO ) << "---------------------------------------" << std::endl;
    log << Log::LogLevel( Log::L_INFO ) << "Virtual Reality Chat (VRC)"              << std::endl;
    log << Log::LogLevel( Log::L_INFO ) << "version: " << std::string( VRC_VERSION )      << std::endl;
    log << Log::LogLevel( Log::L_INFO ) << "project: Yag2002"                        << std::endl;
    log << Log::LogLevel( Log::L_INFO ) << "site:    http://yag2002.sourceforge.net" << std::endl;
    log << Log::LogLevel( Log::L_INFO ) << "contact: botorabi@gmx.net"               << std::endl;
    log << Log::LogLevel( Log::L_INFO ) << "---------------------------------------" << std::endl;
    log << Log::LogLevel( Log::L_INFO ) << std::endl;
    log.enableSeverityLevelPrinting( true );

    log << Log::LogLevel( Log::L_INFO ) << "time: " << yaf3d::getTimeStamp() << std::endl;
    log << Log::LogLevel( Log::L_INFO ) << "initializing viewer" << std::endl;

    log << Log::LogLevel( Log::L_INFO ) << "using media path: " << _mediaPath << std::endl;

    // setup the viewer
    //----------
    
    // load the display settings
    Configuration::get()->getSettingValue( YAF3D_GS_SCREENWIDTH,  _screenWidth  );
    Configuration::get()->getSettingValue( YAF3D_GS_SCREENHEIGHT, _screenHeight );
    Configuration::get()->getSettingValue( YAF3D_GS_FULLSCREEN,   _fullScreen   );
    unsigned int colorBits = 24;
    Configuration::get()->getSettingValue( YAF3D_GS_COLORBITS, colorBits );

    // init SDL
    SDL_Init( SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE );
    // set the icon and caption title
    SDL_WM_SetCaption( YAF3D_APP_TITLE, NULL );
    SDL_Surface* p_bmpsurface = SDL_LoadBMP( YAF3D_APP_ICON );
    if ( p_bmpsurface )
    {
        Uint32 col = SDL_MapRGB( p_bmpsurface->format, 255, 255, 255 );
        SDL_SetColorKey( p_bmpsurface, SDL_SRCCOLORKEY, col );
        SDL_WM_SetIcon( p_bmpsurface, NULL );
    }
    // enable unicode translation
    SDL_EnableUNICODE( 1 ); 
    SDL_EnableKeyRepeat( SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL ); // enable key repeating

    _p_viewer = new osgSDL::Viewer;
    _rootSceneNode = new osg::Group;
    _rootSceneNode->setName( "_topSceneGroup_" );
	osgSDL::Viewport*   p_viewport = new osgSDL::Viewport( _rootSceneNode.get() );
	osgUtil::SceneView* p_sceneView = p_viewport->getSceneView();
    p_sceneView->setDefaults( osgUtil::SceneView::COMPILE_GLOBJECTS_AT_INIT );
	_p_viewer->addViewport( p_viewport );
    _p_viewer->requestContinuousUpdate( true ); // force event generation for FRAMEs, we need this for animations, etc.
    int flags = SDL_HWSURFACE;
    if ( _fullScreen )
        flags |= SDL_FULLSCREEN;
    if ( GameState::get()->getMode() == GameState::Server )
    {
        SDL_WM_SetCaption( YAF3D_APP_TITLE "-server", NULL );
    }
  	_p_viewer->setDisplayMode( _screenWidth, _screenHeight, colorBits, flags );
    _p_viewer->setCursorEnabled( false );    
    //------------

    // setup keyboard map
    std::string keybType;
    Configuration::get()->getSettingValue( YAF3D_GS_KEYBOARD, keybType );
    log << Log::LogLevel( Log::L_INFO ) << "setup keyboard map to: " << keybType << std::endl;
    if ( keybType == YAF3D_GS_KEYBOARD_ENGLISH )
        KeyMap::get()->setup( KeyMap::English );
    else
        KeyMap::get()->setup( KeyMap::German );

    // get the instance of gui manager
    _p_guiManager = GuiManager::get();
    // setup networking
    _p_networkDevice = NetworkDevice::get();
    // avoid creating of remote clients so long we are initializing the system
    _p_networkDevice->lockObjects();
    if ( GameState::get()->getMode() == GameState::Server )
    {
        log << Log::LogLevel( Log::L_INFO ) << "loading level '" << arg_levelname << "'" << std::endl;
        // load the level and setup things
        osg::ref_ptr< osg::Group > sceneroot = LevelManager::get()->loadLevel( YAF3D_LEVEL_SERVER_DIR + arg_levelname );
        if ( !sceneroot.valid() )
            return false;

        // start networking before setting up entities
        std::string servername;
        Configuration::get()->getSettingValue( YAF3D_GS_SERVER_NAME, servername );
        NodeInfo nodeinfo( arg_levelname, servername );
        unsigned int channel;
        Configuration::get()->getSettingValue( YAF3D_GS_SERVER_PORT, channel );
        _p_networkDevice->setupServer( channel, nodeinfo );

        // complete level loading
        LevelManager::get()->finalizeLoading();

        // the server needs no drawing
        _p_viewer->setUpdateAllViewports( false );
    }
    else if ( GameState::get()->getMode() == GameState::Client )
    {
        std::string url;
        Configuration::get()->getSettingValue( YAF3D_GS_SERVER_IP, url );
        std::string clientname( "vrc-client" );
        NodeInfo nodeinfo( "", clientname );
        unsigned int channel;
        Configuration::get()->getSettingValue( YAF3D_GS_SERVER_PORT, channel );

        if ( !_p_networkDevice->setupClient( url, channel, nodeinfo ) )
        {
            log << Log::LogLevel( Log::L_ERROR ) << "cannot setup client networking, exiting ..." << std::endl;
            return false;
        }

        // now load level
        std::string levelname = YAF3D_LEVEL_CLIENT_DIR + _p_networkDevice->getNodeInfo()->getLevelName();
        log << Log::LogLevel( Log::L_INFO ) << "loading level '" << levelname << "'" << std::endl;
        // load the level and setup things
        osg::ref_ptr< osg::Group > sceneroot = LevelManager::get()->loadLevel( levelname );
        if ( !sceneroot.valid() )
            return false;
        // complete level loading
        LevelManager::get()->finalizeLoading();

        // if we directly start a client with cmd line option then we must send a leave-menu notification to entities
        //  as many entities do special steps when leaving the menu
        EntityNotification notification( YAF3D_NOTIFY_MENU_LEAVE );
        EntityManager::get()->sendNotification( notification );
    }
    else // check for any level file name, so we try to start in Standalone mode
    {
        log << Log::LogLevel( Log::L_INFO ) << "loading level '" << arg_levelname << "'" << std::endl;
        // set game mode
        GameState::get()->setMode( GameState::Standalone );
        // load the level and setup things
        std::string defaultlevel = arg_levelname.length() ? ( std::string( YAF3D_LEVEL_SALONE_DIR ) + arg_levelname ) : std::string( YAF3D_DEFAULT_LEVEL );
        osg::ref_ptr< osg::Group > sceneroot = LevelManager::get()->loadLevel( defaultlevel );
        if ( !sceneroot.valid() )
            return false;
        // complete level loading
        LevelManager::get()->finalizeLoading();

        // if we directly start a client with cmd line option then we must send a leave-menu notification to entities
        //  as many entities do special steps when leaving the menu
        EntityNotification notification( YAF3D_NOTIFY_MENU_LEAVE );
        EntityManager::get()->sendNotification( notification );
    }

    return true;
}

void Application::run()
{
    // set game state
    _p_gameState->setState( GameState::Running );

    osg::Timer       timer;
    float            deltaTime = 0;
    osg::Timer_t     curTick   = 0;
    osg::Timer_t     lastTick  = 0;

    // now the network can start
    if ( GameState::get()->getMode() == GameState::Client )
    {
        _p_networkDevice->startClient();
        _p_networkDevice->unlockObjects();
    }
    else if ( GameState::get()->getMode() == GameState::Server )
    {
        _p_networkDevice->unlockObjects();
    }

    // check heap if enabled ( used for detecting heap corruptions )
    YAF3D_CHECK_HEAP();

    // begin game loop
    while( _p_gameState->getState() != GameState::Quitting )
    {
        lastTick  = curTick;
        curTick   = timer.tick();
        deltaTime = timer.delta_s( lastTick, curTick );

        // limit the deltaTime as we have to be carefull with stability of physics calculation etc.
        if ( deltaTime > 0.06f ) 
            deltaTime = 0.06f;
        else if ( deltaTime < 0.001f )
            deltaTime = 0.001f;

        // update game
        if ( GameState::get()->getMode() == GameState::Server )
            updateServer( deltaTime );
        else if ( GameState::get()->getMode() == GameState::Client )
            updateClient( deltaTime );
        else updateStandalone( deltaTime );

        // check heap if enabled ( used for detecting heap corruptions )
        YAF3D_CHECK_HEAP();
    }
}

void Application::updateStandalone( float deltaTime )
{
    // update entities
    _p_entityManager->update( deltaTime  );

    // update physics
    _p_physics->update( deltaTime );

    // update gui manager
    _p_guiManager->update( deltaTime );

    // update viewer and draw scene
    _p_viewer->update();
    _p_viewer->draw();

    // check for termination
    if ( _p_viewer->isTerminated() )
        stop();

    // yield a little processor time for other tasks on system
    if ( !_fullScreen )
        OpenThreads::Thread::microSleep( 1000 );
}

void Application::updateClient( float deltaTime )
{
    // update client
    _p_networkDevice->updateClient( deltaTime );

    // update entities
    _p_entityManager->update( deltaTime  );

    // update physics
    _p_physics->update( deltaTime );

    // update gui manager
    _p_guiManager->update( deltaTime );

    // update viewer and draw scene
    _p_viewer->update();
    _p_viewer->draw();

    // check for termination
    if ( _p_viewer->isTerminated() )
        stop();

    // yield a little processor time for other tasks on system
    if ( !_fullScreen )
        OpenThreads::Thread::microSleep( 1000 );
}

void Application::updateServer( float deltaTime )
{
    // update server
    _p_networkDevice->updateServer( deltaTime );

    // update entities
    _p_entityManager->update( deltaTime  );

    // update physics on server? once we may need it, but not now
    //_p_physics->update( deltaTime );

    // update viewer, no draw on server
    _p_viewer->update();

    // check for termination
    if ( _p_viewer->isTerminated() )
        stop();

    // yield a little processor time for other tasks on system
    OpenThreads::Thread::microSleep( 1000 );
}

void Application::stop()
{
    _p_gameState->setState( GameState::Quitting );
}

} // namespace yaf3d