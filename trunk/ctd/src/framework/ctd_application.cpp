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


using namespace std;
using namespace CTD; 
using namespace osg; 

// app icon and tile
#define CTD_APP_TITLE           "VRC"
#define CTD_APP_ICON            "icon.bmp"

// log file names
#define LOG_FILE_NAME           "vrc.log"
#define LOG_FILE_NAME_SERVER    "vrc-server.log"

// media path relative to inst dir
#define CTD_MEDIA_PATH          "/media/"
// default level
#define CTD_DEFAULT_LEVEL       "gui/loader"


// #define CTD_CHECK_HEAP()	

CTD_SINGLETON_IMPL( Application );


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
    log << Log::LogLevel( Log::L_INFO ) << "---------------------------------------" << endl;
    log << Log::LogLevel( Log::L_INFO ) << "shutting down, time: " << CTD::getTimeStamp() << endl;

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

#ifdef WIN32
// console handler for catching Ctrl+C events on WIN32 platform
BOOL WINAPI consoleHandler( DWORD ctrlType )
{
    switch ( ctrlType )
    {
        case CTRL_CLOSE_EVENT:
        case CTRL_C_EVENT:
            Application::get()->stop();
            break;

        default:
            ;
    }
    return TRUE;
}
#endif

bool Application::initialize( int argc, char **argv )
{
#ifdef CTD_ENABLE_HEAPCHECK
    // trigger debugger
//    __asm int 3;
#endif

    // set console handler in order to catch Ctrl+C and close events
#ifdef WIN32
    SetConsoleCtrlHandler( consoleHandler, TRUE );
#endif

    // set game state
    _p_gameState->setState( GameState::Initializing );

    string arg_levelname;
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments( &argc,argv );
    ArgumentParser::Parameter levelparam( arg_levelname );
    arguments.read( "-level", arg_levelname ); // read the level file if one given

    // fetch argument for using osgviewer instead of own camera and scene updating
    int   argpos;
    bool  useOsgViewer = false;
    if ( ( argpos = arguments.find( "-useosgviewer" ) ) != 0 )
    {
        useOsgViewer = true;
        arguments.remove( argpos );
    }
    // enable physics debug rendering?
    bool  enablePhysicsDebugRendering = false;
    if ( ( argpos = arguments.find( "-physicsdebug" ) ) != 0 )
    {
        enablePhysicsDebugRendering = true;
        arguments.remove( argpos );
    }

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
        arguments.writeErrorMessages( cout );
    }

    // set the media path as first step, other modules need it for loading resources etc.
    //-------------------
    _mediaPath = cleanPath( arguments.getApplicationName() );
    std::vector< std::string > path;
    explode( _mediaPath, "/", &path );
    if ( path.size() > 1 )
    {
        std::string dir( "/" );
        for ( size_t cnt = 0; cnt < path.size() - 3; cnt++ )
            dir += path[ cnt ] + "/";

        dir.erase( dir.size() -1 );
        _mediaPath = dir;
    }
    else
    {
        std::string dir = getCurrentWorkingDirectory();
        dir = cleanPath( dir );
        dir += "/";
        path.clear();
        explode( dir, "/", &path );
        dir = "";
        for ( size_t cnt = 0; cnt < path.size() - 2; cnt++ )
            dir += path[ cnt ] + "/";

        dir.erase( dir.size() -1 );
        _mediaPath = dir;
    }
    _mediaPath += CTD_MEDIA_PATH;
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

    log.addSink( "stdout", cout, Log::L_ERROR );

    log.enableSeverityLevelPrinting( false );
    log << Log::LogLevel( Log::L_INFO ) << "---------------------------------------" << endl;
    log << Log::LogLevel( Log::L_INFO ) << "Virtual Reality Chat (VRC)"              << endl;
    log << Log::LogLevel( Log::L_INFO ) << "version: " << string( VRC_VERSION )      << endl;
    log << Log::LogLevel( Log::L_INFO ) << "project: Yag2002"                        << endl;
    log << Log::LogLevel( Log::L_INFO ) << "site:    http://yag2002.sourceforge.net" << endl;
    log << Log::LogLevel( Log::L_INFO ) << "contact: info@botorabi.de"               << endl;
    log << Log::LogLevel( Log::L_INFO ) << "---------------------------------------" << endl;
    log << Log::LogLevel( Log::L_INFO ) << endl;
    log.enableSeverityLevelPrinting( true );

    log << Log::LogLevel( Log::L_INFO ) << "time: " << CTD::getTimeStamp() << endl;
    log << Log::LogLevel( Log::L_INFO ) << "initializing viewer" << endl;

    // setup the viewer
    //----------
    
    // load the display settings
    Configuration::get()->getSettingValue( CTD_GS_SCREENWIDTH,  _screenWidth );
    Configuration::get()->getSettingValue( CTD_GS_SCREENHEIGHT, _screenHeight );
    Configuration::get()->getSettingValue( CTD_GS_FULLSCREEN, _fullScreen );
    unsigned int colorBits = 24;
    Configuration::get()->getSettingValue( CTD_GS_COLORBITS, colorBits );

    // init SDL
    SDL_Init( SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE );
    // set the icon and caption title
    SDL_WM_SetCaption( CTD_APP_TITLE, NULL );
    SDL_Surface* p_bmpsurface = SDL_LoadBMP( CTD_APP_ICON );
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
        SDL_WM_SetCaption( CTD_APP_TITLE "-server", NULL );
    }
  	_p_viewer->setDisplayMode( _screenWidth, _screenHeight, colorBits, flags );
    _p_viewer->setCursorEnabled( false );    
    //------------

    // setup keyboard map
    string keybType;
    Configuration::get()->getSettingValue( CTD_GS_KEYBOARD, keybType );
    log << Log::LogLevel( Log::L_INFO ) << "setup keyboard map to: " << keybType << endl;
    if ( keybType == CTD_GS_KEYBOARD_ENGLISH )
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
        log << Log::LogLevel( Log::L_INFO ) << "loading level '" << arg_levelname << "'" << endl;
        // load the level and setup things
        osg::ref_ptr< osg::Group > sceneroot = LevelManager::get()->loadLevel( CTD_LEVEL_SERVER_DIR + arg_levelname );
        if ( !sceneroot.valid() )
            return false;
        // complete level loading
        LevelManager::get()->finalizeLoading();

        string servername;
        Configuration::get()->getSettingValue( CTD_GS_SERVER_NAME, servername );
        NodeInfo nodeinfo( arg_levelname, servername );
        unsigned int channel;
        Configuration::get()->getSettingValue( CTD_GS_SERVER_PORT, channel );
        _p_networkDevice->setupServer( channel, nodeinfo );
    }
    else if ( GameState::get()->getMode() == GameState::Client )
    {
        string url;
        Configuration::get()->getSettingValue( CTD_GS_SERVER_IP, url );
        string clientname( "vrc-client" );
        NodeInfo nodeinfo( "", clientname );
        unsigned int channel;
        Configuration::get()->getSettingValue( CTD_GS_SERVER_PORT, channel );

        if ( !_p_networkDevice->setupClient( url, channel, nodeinfo ) )
        {
            log << Log::LogLevel( Log::L_ERROR ) << "cannot setup client networking, exiting ..." << endl;
            return false;
        }

        // now load level
        string levelname = CTD_LEVEL_CLIENT_DIR + _p_networkDevice->getNodeInfo()->getLevelName();
        log << Log::LogLevel( Log::L_INFO ) << "loading level '" << levelname << "'" << endl;
        // load the level and setup things
        osg::ref_ptr< osg::Group > sceneroot = LevelManager::get()->loadLevel( levelname );
        if ( !sceneroot.valid() )
            return false;
        // complete level loading
        LevelManager::get()->finalizeLoading();

        // if we directly start a client with cmd line option then we must send a leave-menu notification to entities
        //  as many entities do special steps when leaving the menu
        EntityNotification notification( CTD_NOTIFY_MENU_LEAVE );
        EntityManager::get()->sendNotification( notification );
    }
    else // check for any level file name, so we try to start in Standalone mode
    {
        log << Log::LogLevel( Log::L_INFO ) << "loading level '" << arg_levelname << "'" << endl;
        // set game mode
        GameState::get()->setMode( GameState::Standalone );
        // load the level and setup things
        string defaultlevel = arg_levelname.length() ? ( string( CTD_LEVEL_SALONE_DIR ) + arg_levelname ) : string( CTD_DEFAULT_LEVEL );
        osg::ref_ptr< osg::Group > sceneroot = LevelManager::get()->loadLevel( defaultlevel );
        if ( !sceneroot.valid() )
            return false;
        // complete level loading
        LevelManager::get()->finalizeLoading();

        // if we directly start a client with cmd line option then we must send a leave-menu notification to entities
        //  as many entities do special steps when leaving the menu
        EntityNotification notification( CTD_NOTIFY_MENU_LEAVE );
        EntityManager::get()->sendNotification( notification );
    }

    // enable physics debug rendering
    if ( enablePhysicsDebugRendering )
        _p_physics->enableDebugRender();

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
    CTD_CHECK_HEAP();

    // begin game loop
    while( ( _p_gameState->getState() != GameState::Quitting ) && !_p_viewer->isTerminated() )
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
        CTD_CHECK_HEAP();
    }   
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

    // fire off the cull and draw traversals of the scene.
    _p_viewer->runOnce();

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

    // update physics
    //_p_physics->update( deltaTime );

    // update gui manager
    _p_guiManager->update( deltaTime );

     // fire off the cull and draw traversals of the scene.
    _p_viewer->runOnce();

    // yield a little processor time for other tasks on system
    OpenThreads::Thread::microSleep( 1000 );
}

void Application::updateStandalone( float deltaTime )
{
    // update entities
    _p_entityManager->update( deltaTime  );

    // update physics
    _p_physics->update( deltaTime );

    // update gui manager
    _p_guiManager->update( deltaTime );

    // fire off the cull and draw traversals of the scene.
    _p_viewer->runOnce();

    // yield a little processor time for other tasks on system
    if ( !_fullScreen )
        OpenThreads::Thread::microSleep( 1000 );
}

void Application::stop()
{
    _p_gameState->setState( GameState::Quitting );
}
