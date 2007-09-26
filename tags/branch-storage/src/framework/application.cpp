/****************************************************************
 *  Project YAG2002 (http://yag2002.sourceforge.net)
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
 # main application class
 #
 #   date of creation:  02/17/2005
 #
 #   author:            ali botorabi (boto)
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>
#include "application.h"
#include "entitymanager.h"
#include "configuration.h"
#include "terrainmanager.h"
#include "shadowmanager.h"
#include "levelmanager.h"
#include "soundmanager.h"
#include "guimanager.h"
#include "gamestate.h"
#include "physics.h"
#include "network.h"
#include "keymap.h"
#include "utils.h"
#include "log.h"

#include <SDL_cpuinfo.h>

// uncomment if the application should have an console stdout ( is used for server )
// when using ms compiler you have also to set linker parameter /SUBSYSTEM:CONSOLE
//#define YAF3D_HAS_CONSOLE

// app icon and tile
#define YAF3D_APP_TITLE             "Yaf3D"
#define YAF3D_APP_ICON              "icon.bmp"

// environment variable for media directory
//  if not existing then the relative path '../../media' of executable is assumed
#define YAF3D_ENV_MEDIA_DIR         "YAF3D_ENV_MEDIA_DIR"

// log file names
#define LOG_FILE_NAME               "vrc.log"
#define LOG_FILE_NAME_SERVER        "vrc-server.log"

// media path relative to inst dir
#define YAF3D_MEDIA_PATH            "/media/"
// default level
#define YAF3D_DEFAULT_LEVEL         "gui/loader"

// update period limits ( in seconds )
#define UPPER_UPDATE_PERIOD_LIMIT   1.0f / 2.0f
#define LOWER_UPDATE_PERIOD_LIMIT   1.0f / 65.0f

namespace yaf3d
{

//! Hanlder for application window state changes
//! This handler is used for disabling periodic update of some subsystems when application window is minimized.
//! This way a noticable amount of cpu usage can be saved.
class AppWindowStateHandler : public GameState::CallbackAppWindowStateChange
{
    public:

        explicit                AppWindowStateHandler( Application* p_app ) :
                                 _p_app( p_app )
                                {
                                    // register for getting application window state changes
                                    GameState::get()->registerCallbackAppWindowStateChange( this );
                                }

        virtual                 ~AppWindowStateHandler()
                                {
                                    // deregister for getting application window state changes
                                    yaf3d::GameState::get()->registerCallbackAppWindowStateChange( this, false );
                                }

    protected:

        //! This is called whenever the app window state changes
        void                    onAppWindowStateChange( unsigned int state )
                                {
                                    // when application window is minimized the let it the Application know
                                    if ( state == GameState::Minimized )
                                        _p_app->setAppWindowMinimized( true );
                                    else if ( ( state == yaf3d::GameState::Restored ) || ( state == yaf3d::GameState::GainedFocus ) )
                                        _p_app->setAppWindowMinimized( false );
                                }

        Application*            _p_app;
};

// Implementation of Application
YAF3D_SINGLETON_IMPL( Application )

Application::Application():
_p_networkDevice( NULL ),
_p_entityManager( EntityManager::get() ),
_p_guiManager( NULL ),
_p_soundManager( NULL ),
_p_physics( Physics::get() ),
_p_gameState( GameState::get() ),
_p_viewer( NULL ),
_screenWidth( 600 ),
_screenHeight( 400 ),
_fullScreen( false ),
_appWindowMinimized( false ),
_p_appWindowStateHandler( NULL )
{
}

Application::~Application()
{
}

void Application::shutdown()
{
    log_info << "---------------------------------------" << std::endl;
    log_info << "Application: shutting down, time: " << yaf3d::getTimeStamp() << std::endl;

    LevelManager::get()->shutdown();

    if ( _p_networkDevice )
        _p_networkDevice->shutdown();

    Configuration::get()->shutdown();

    // set the game state to shutdown so game code specific singletons get the chance for destruction
    _p_gameState->setState( GameState::Quitting );
    delete _p_appWindowStateHandler;
    _p_gameState->shutdown();

    TerrainManager::get()->shutdown();
    SettingsManager::get()->shutdown();
    ShadowManager::get()->shutdown();
    KeyMap::get()->shutdown();

    // delete viewer
    delete _p_viewer;

    // delete the top scene node
    _rootSceneNode = NULL;

    SDL_Quit();

    destroy();
}

bool Application::initialize( int argc, char **argv )
{
    // this is used when hunting for memory leaks
#ifdef YAF3D_ENABLE_HEAPCHECK
    // trigger debugger
    //__asm int 3;
#endif

    //! NOTE: on multi-core systems running win32, sometimes a noticable performance drop has been observed
    //        when the application uses more than one cpu for its threads. here we assign only one cpu to the entire app.
#ifdef WIN32
    SYSTEM_INFO sysInfo;
    GetSystemInfo( &sysInfo );
    if ( sysInfo.dwNumberOfProcessors > 1 )
    {
        // take the first cpu for our application
        DWORD_PTR  processAffinityMask = 0x1;
        BOOL       res = SetProcessAffinityMask( GetCurrentProcess(), processAffinityMask );
    }
#endif

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

    // note: before beginning to initialize the framework modules the media path must be set,
    //  other modules need it for loading resources etc.
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
            for ( size_t cnt = 0; cnt < path.size() - 2; ++cnt )
                dir += path[ cnt ] + "/";

            dir.erase( dir.size() -1 );
            _mediaPath = dir;
            _mediaPath += YAF3D_MEDIA_PATH;
        }
    }

    //-------------------
    // set the ful binary path of application
    _fulBinaryPath = arguments.getApplicationName();
    _fulBinaryPath = cleanPath( _fulBinaryPath );
    _fulBinaryPath = _fulBinaryPath.substr( 0, _fulBinaryPath.rfind( '/' ) );
    //-------------------

    // load the standard configuration before changing to 'Initializing' state
    Configuration::get()->load();

    // set game state
    _p_gameState->setState( GameState::Initializing );

    // setup log system
    {
        std::string loglevel;
        Log::Level  level = Log::L_ERROR;

        // get the log level from configuration
        Configuration::get()->getSettingValue( YAF3D_GS_LOG_LEVEL, loglevel );

        // check if we have to report an invalid log level in configuration
        if ( loglevel == "error" )
            level = Log::L_ERROR;
        else if ( loglevel == "warning" )
            level = Log::L_WARNING;
        else if ( loglevel == "debug" )
            level = Log::L_DEBUG;
        else if ( loglevel == "info" )
            level = Log::L_INFO;
        else if ( loglevel == "verbose" )
            level = Log::L_VERBOSE;
        else
            log_warning << "Application: configuration contains an invalid log level, possible values are: error, warning, debug, info, verbose. set to error." << std::endl;

        // create log sinks with configured log level
        if ( GameState::get()->getMode() != GameState::Server )
            log_out.addSink( "file", getMediaPath() + std::string( LOG_FILE_NAME ), level );
        else
            log_out.addSink( "file", getMediaPath() + std::string( LOG_FILE_NAME_SERVER ), level );

        // only the server needs an console stdout
#ifdef YAF3D_HAS_CONSOLE
        log_out.addSink( "stdout", std::cout, level );
#endif

    }

    log_out.enableSeverityLevelPrinting( false );
    log_info << std::endl;
    log_out << " *******************************************"    << std::endl;
    log_out << " * yaf3d -- Yet another Framework 3D       *"    << std::endl;
    log_out << " * version: " << std::string( YAF3D_VERSION ) << "                          *"  << std::endl;
    log_out << " * project: Yag2002                        *"    << std::endl;
    log_out << " * site:    http://yag2002.sourceforge.net *"    << std::endl;
    log_out << " * contact: botorabi@gmx.net               *"    << std::endl;
    log_out << " *******************************************"    << std::endl;
    log_out << "" << std::endl;
    log_out.enableSeverityLevelPrinting( true );

    log_out << "Application: time " << yaf3d::getTimeStamp();

    // print cpu info
    {
        std::stringstream cpuinfo;
        cpuinfo << "Application: CPU supports ";
        if ( SDL_HasRDTSC() )
            cpuinfo << "RDTSC ";
        if ( SDL_HasMMX() )
            cpuinfo << "MMX ";
        if ( SDL_HasMMXExt() )
            cpuinfo << "MMXExt ";
        if ( SDL_Has3DNow() )
            cpuinfo << "3DNow ";
        if ( SDL_Has3DNowExt() )
            cpuinfo << "3DNowExt ";
        if ( SDL_HasSSE() )
            cpuinfo << "SSE ";
        if ( SDL_HasSSE2() )
            cpuinfo << "SSE2 ";
        if ( SDL_HasAltiVec() )
            cpuinfo << "AltiVec ";

        log_out << cpuinfo.str() << std::endl;
    }

    log_out << "Application: initializing viewer" << std::endl;
    log_out << "Application: using media path: " << _mediaPath << std::endl;

    // setup the viewer
    //----------

    // load the display settings
    Configuration::get()->getSettingValue( YAF3D_GS_SCREENWIDTH,  _screenWidth  );
    Configuration::get()->getSettingValue( YAF3D_GS_SCREENHEIGHT, _screenHeight );
    Configuration::get()->getSettingValue( YAF3D_GS_FULLSCREEN,   _fullScreen   );
    unsigned int colorBits = 24;
    Configuration::get()->getSettingValue( YAF3D_GS_COLORBITS, colorBits );

    // set the icon and caption title only for non-servers
    if ( GameState::get()->getMode() != GameState::Server )
    {
       // init SDL with video
        SDL_Init( SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE );

        // set application window's title
        _appWindowTitle = YAF3D_APP_TITLE " " YAF3D_VERSION;
        setWindowTitle( _appWindowTitle );

        SDL_Surface* p_bmpsurface = SDL_LoadBMP( YAF3D_APP_ICON );
        if ( p_bmpsurface )
        {
            Uint32 col = SDL_MapRGB( p_bmpsurface->format, 255, 255, 255 );
            SDL_SetColorKey( p_bmpsurface, SDL_SRCCOLORKEY, col );
            SDL_WM_SetIcon( p_bmpsurface, NULL );
        }
    }
    else
    {
        // init SDl witout video for server
        SDL_Init( SDL_INIT_NOPARACHUTE );
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

    _p_viewer->setDisplayMode( _screenWidth, _screenHeight, colorBits, flags );
    _p_viewer->setCursorEnabled( false );
    //------------

    // setup keyboard map
    std::string keybType;
    Configuration::get()->getSettingValue( YAF3D_GS_KEYBOARD, keybType );
    log_info << "Application: setup keyboard map to: " << keybType << std::endl;
    if ( keybType == YAF3D_GS_KEYBOARD_ENGLISH )
        KeyMap::get()->setup( KeyMap::English );
    else
        KeyMap::get()->setup( KeyMap::German );

    // get the instance of gui manager
    _p_guiManager = GuiManager::get();

    // setup networking
    _p_networkDevice = NetworkDevice::get();
    if ( GameState::get()->getMode() == GameState::Server )
    {
        log_info << "Application: loading level file '" << arg_levelname << "'" << std::endl;
        // load the level and setup things
        osg::ref_ptr< osg::Group > sceneroot = LevelManager::get()->loadLevel( YAF3D_LEVEL_SERVER_DIR + arg_levelname );
        if ( !sceneroot.valid() )
            return false;

        // append the level node to scene root node
        _rootSceneNode->addChild( sceneroot.get() );

        // start networking before setting up entities
        std::string servername;
        Configuration::get()->getSettingValue( YAF3D_GS_SERVER_NAME, servername );
        NodeInfo nodeinfo( arg_levelname, servername );
        unsigned int channel = 0;
        Configuration::get()->getSettingValue( YAF3D_GS_SERVER_PORT, channel );

        // try to setup server
        try
        {
            _p_networkDevice->setupServer( channel, nodeinfo );
        }
        catch ( const NetworkException& e )
        {
            log_error << "Application: error starting server, reason: " << e.what() << std::endl;
            return false;
        }

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
        unsigned int channel = 0;
        Configuration::get()->getSettingValue( YAF3D_GS_SERVER_PORT, channel );

        // try to setup client networking
        try
        {
            _p_networkDevice->setupClient( url, channel, nodeinfo );
        }
        catch ( const NetworkException& e )
        {
            log_error << "Application: error setting up client networking, reason: " << e.what() << std::endl;
            return false;
        }

        // now load level
        std::string levelname = YAF3D_LEVEL_CLIENT_DIR + _p_networkDevice->getNodeInfo()->getLevelName();
        log_info << "Application: loading level file '" << levelname << "'" << std::endl;
        // load the level and setup things
        osg::ref_ptr< osg::Group > sceneroot = LevelManager::get()->loadLevel( levelname );
        if ( !sceneroot.valid() )
            return false;

        // append the level node to scene root node
        _rootSceneNode->addChild( sceneroot.get() );

        // complete level loading
        LevelManager::get()->finalizeLoading();

        // if we directly start a client with cmd line option then we must send a leave-menu notification to entities
        //  as many entities do special steps when leaving the menu
        EntityNotification notification( YAF3D_NOTIFY_MENU_LEAVE );
        EntityManager::get()->sendNotification( notification );
    }
    else // check for any level file name, so we try to start in Standalone mode
    {
        std::string defaultlevel = arg_levelname.length() ? ( std::string( YAF3D_LEVEL_SALONE_DIR ) + arg_levelname ) : std::string( YAF3D_DEFAULT_LEVEL );
        log_info << "Application: loading level file '" << defaultlevel << "'" << std::endl;
        // set game mode
        GameState::get()->setMode( GameState::Standalone );
        // load the level and setup things
        osg::ref_ptr< osg::Group > sceneroot = LevelManager::get()->loadLevel( defaultlevel );
        if ( !sceneroot.valid() )
            return false;

        // append the level node to scene root node
        _rootSceneNode->addChild( sceneroot.get() );

        // complete level loading
        LevelManager::get()->finalizeLoading();

        // if we directly start a level with cmd line option then we must send a leave-menu notification to entities
        //  as many entities do special steps when leaving the menu
        if ( arg_levelname.length() )
        {
            EntityNotification notification( YAF3D_NOTIFY_MENU_LEAVE );
            EntityManager::get()->sendNotification( notification );
        }
    }

    // setup the shadow mananger now
    if ( GameState::get()->getMode() != GameState::Server )
    {
        bool shadow = true;
        // if glsl is not available then disable dynamic shadow flag in configuration
        if ( !yaf3d::isGlslAvailable() )
        {
            log_info << "Dynamic shadows disabled as GLSL is not available!" << std::endl;
            shadow = false;
            yaf3d::Configuration::get()->setSettingValue( YAF3D_GS_SHADOW_ENABLE, shadow );
            yaf3d::Configuration::get()->store();
        }

        if ( shadow )
        {
            unsigned int shadowTexSizeX = 0, shadowTexSizeY = 0, shadowTexChannel = 0;
            yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_SHADOW_TEXSIZEX, shadowTexSizeX );
            yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_SHADOW_TEXSIZEY, shadowTexSizeY );
            yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_SHADOW_TEXCHANNEL, shadowTexChannel );
            ShadowManager::get()->setup( shadowTexSizeX, shadowTexSizeY, shadowTexChannel );
        }
    }

    return true;
}

void Application::run()
{
    // set game state
    _p_gameState->setState( GameState::Running );

    // store sound manager reference for faster access in loop
    _p_soundManager = SoundManager::get();

    // from now on game state can handle application window state changes
    _p_gameState->initAppWindowStateHandler();

    // setup local app window state handler
    _p_appWindowStateHandler = new AppWindowStateHandler( this );

    // now the network can start
    if ( GameState::get()->getMode() == GameState::Client )
    {
        // try to start client networking
        try
        {
            _p_networkDevice->startClient();
        }
        catch ( const NetworkException& e )
        {
            log_error << "Application: error starting client networking, reason: " << e.what() << std::endl;
            _p_gameState->setState( GameState::Quitting );
        }
    }

    // check heap if enabled ( used for detecting heap corruptions )
    YAF3D_CHECK_HEAP();

    osg::Timer       timer;
    osg::Timer_t     curTick   = timer.tick();
    osg::Timer_t     lastTick  = curTick;
    float            deltaTime = LOWER_UPDATE_PERIOD_LIMIT;

    // begin game loop
    while( _p_gameState->getState() != GameState::Quitting )
    {
        // limit the deltaTime as we have to be carefull with stability of physics calculation etc.
        if ( deltaTime > UPPER_UPDATE_PERIOD_LIMIT )
        {
            deltaTime = UPPER_UPDATE_PERIOD_LIMIT;
        }
        else if ( deltaTime < LOWER_UPDATE_PERIOD_LIMIT )
        {

            // limit the upper fps to about 60 frames / second ( there are crash problems in some gpu cards when vsynch is disabled )
            do
            {
                OpenThreads::Thread::microSleep( 100 );
                curTick   = timer.tick();
                deltaTime = timer.delta_s( lastTick, curTick );
            }
            while ( deltaTime < LOWER_UPDATE_PERIOD_LIMIT );
        }

        // update game
        if ( GameState::get()->getMode() == GameState::Server )
            updateServer( deltaTime );
        else if ( GameState::get()->getMode() == GameState::Client )
            updateClient( deltaTime );
        else updateStandalone( deltaTime );

        // calculate new delta-time
        lastTick  = curTick;
        curTick   = timer.tick();
        deltaTime = timer.delta_s( lastTick, curTick );

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

    // update viewer
    _p_viewer->update();

    // same some cpu usage when app window is minimized
    if ( !_appWindowMinimized )
    {
        // update gui manager
        _p_guiManager->update( deltaTime );
        // update sound system
        _p_soundManager->update( deltaTime );
        // draw the scene
        _p_viewer->draw();
    }

    // check for termination
    if ( _p_viewer->isTerminated() )
        stop();

    // yield a little processor time for other tasks on system
    if ( !_fullScreen )
        OpenThreads::Thread::microSleep( 100 );
}

void Application::updateClient( float deltaTime )
{
    // update client
    _p_networkDevice->updateClient( deltaTime );

    // update entities
    _p_entityManager->update( deltaTime  );

    // update physics
    _p_physics->update( deltaTime );

    // update viewer
    _p_viewer->update();

    // same some cpu usage when app window is minimized
    if ( !_appWindowMinimized )
    {
        // update gui manager
        _p_guiManager->update( deltaTime );

        // update sound system
        _p_soundManager->update( deltaTime );

        // draw the scene
        _p_viewer->draw();
    }

    // check for termination
    if ( _p_viewer->isTerminated() )
        stop();

    // yield a little processor time for other tasks on system
    if ( !_fullScreen )
        OpenThreads::Thread::microSleep( 100 );
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
