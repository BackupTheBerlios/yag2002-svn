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
#include "ctd_application.h"


using namespace std;
using namespace CTD; 
using namespace osg; 

// media path relative to inst dir
#define CTD_MEDIA_PATH      "/media/"
// default level
#define CTD_DEFAULT_LEVEL   "gui/loader"

CTD_SINGLETON_IMPL( Application );

Application::Application():
_p_networkDevice( NULL ),
_p_entityManager( EntityManager::get() ),
_p_gameState( GameState::get() ),
_p_physics( Physics::get() ),
_p_viewer( NULL ),
_p_rootSceneNode( NULL ),
_screenWidth( 600 ),
_screenHeight( 400 )
{
}

Application::~Application()
{
}

void Application::shutdown()
{
    NetworkDevice::get()->shutdown();
    LevelManager::get()->shutdown();
    Configuration::get()->shutdown();
    GameState::get()->shutdown();
    delete _p_viewer;
    destroy();
}

bool Application::initialize( int argc, char **argv )
{
    // set game state
    _p_gameState->setState( GameState::Initializing );

    string arg_levelname;
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    ArgumentParser::Parameter levelparam( arg_levelname );
    if ( !arguments.read( "-level", arg_levelname ) )
        arg_levelname = CTD_DEFAULT_LEVEL;              // if no level defined then take the default one

    // fetch argument for using osgviewer instead of own camera and scene updating
    int   argpos;
    bool  useOsgViewer = false;
    if ( argpos = arguments.find( "-useosgviewer" ) )
    {
        useOsgViewer = true;
        arguments.remove( argpos );
    }
    // enable physics debug rendering?
    bool  enablePhysicsDebugRendering = false;
    if ( argpos = arguments.find( "-physicsdebug" ) )
    {
        enablePhysicsDebugRendering = true;
        arguments.remove( argpos );
    }

    if ( argpos = arguments.find( "-server" ) )
    {
        GameState::get()->setMode( GameState::Server );
        arguments.remove( argpos );
    }
    else if ( argpos = arguments.find( "-client" ) )
    {
        GameState::get()->setMode( GameState::Client );
        arguments.remove( argpos );
    }
    else
    {
        GameState::get()->setMode( GameState::Standalone );
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages( cout );
    }

    // set the media path as first step, other modules need the variable _mediaPath
    //-------------------
    _mediaPath = arguments.getApplicationName();
    //  clean path
    for ( string::iterator i = _mediaPath.begin(), e = _mediaPath.end(); i != e; i++ ) if ( *i == '\\') *i = '/';
    string tmp = _mediaPath.substr( 0, _mediaPath.rfind( "/" ) );
    tmp = tmp.substr( 0, tmp.rfind( "/" ) );
    tmp = tmp.substr( 0, tmp.rfind( "/" ) );
    _mediaPath = tmp;
    _mediaPath += CTD_MEDIA_PATH;
    //-------------------

    // load the settings
    //-------------------
    Configuration::get()->getSettingValue( CTD_GS_SCREENWIDTH,  _screenWidth    );
    Configuration::get()->getSettingValue( CTD_GS_SCREENHEIGHT, _screenHeight   );
    Configuration::get()->getSettingValue( CTD_GS_MOUSESENS,    _mouseSensivity );

    //-------------------

    // setup log system
    //-----------------
    //! TODO: set the loglevels basing on config file
    log.addSink( "file", getMediaPath() + "vrc.log", Log::L_ERROR );
    log.addSink( "stdout", cout, Log::L_ERROR );
 
    log << Log::LogLevel( Log::L_INFO ) << "initializing viewer" << endl;
    // construct the viewer
    //----------
    _p_viewer = new osgProducer::Viewer( arguments );

    unsigned int opt = 0;
    if ( useOsgViewer )
    {
        // set up the viewer services
        //opt |= osgProducer::Viewer::SKY_LIGHT_SOURCE;
        //opt |= ~osgProducer::Viewer::HEAD_LIGHT_SOURCE;
        opt |= ~osgProducer::Viewer::STATS_MANIPULATOR;
        opt |= ~osgProducer::Viewer::TRACKBALL_MANIPULATOR;
        opt |= ~osgProducer::Viewer::ESCAPE_SETS_DONE;
    }

    // now setup the viewer
    _p_viewer->setUpViewer( opt );
    //----------

    // set fullscreen / windowed mode
    Producer::Camera *p_cam = _p_viewer->getCamera( 0 );
    Producer::RenderSurface* p_rs = p_cam->getRenderSurface();
    p_rs->setWindowRectangle( 100, 100, _screenWidth, _screenHeight );
    unsigned int colorbits = 24;
    Configuration::get()->getSettingValue( CTD_GS_COLORBITS, colorbits );
    p_rs->addPixelAttribute( Producer::RenderSurface::DepthSize, colorbits );
    bool fullscreen;
    Configuration::get()->getSettingValue( CTD_GS_FULLSCREEN, fullscreen );
    p_rs->fullScreen( fullscreen );
    p_rs->useCursor( false ); //hide cursor

    // get details on keyboard and mouse bindings used by the viewer.
    _p_viewer->getUsage( *arguments.getApplicationUsage() );

    // setup keyboard map
    string keybType;
    Configuration::get()->getSettingValue( CTD_GS_KEYBOARD, keybType );
    log << Log::LogLevel( Log::L_INFO ) << "setup keyboard map to: " << keybType << endl;
    if ( keybType == CTD_GS_KEYBOARD_ENGLISH )
        KeyMap::get()->setup( KeyMap::English );
    else
        KeyMap::get()->setup( KeyMap::German );

    if ( GameState::get()->getMode() != GameState::Client )
    {
        log << Log::LogLevel( Log::L_INFO ) << "loading level '" << arg_levelname << "'" << endl;
        // load the level and setup things
        osg::ref_ptr< osg::Group > sceneroot = LevelManager::get()->loadLevel( arg_levelname );
        if ( !sceneroot.valid() )
            return false;
    }

    // get the instance of gui manager
    _p_guiManager = GuiManager::get();

    // setup networking
    _p_networkDevice = NetworkDevice::get();
    if ( GameState::get()->getMode() == GameState::Server )
    {
        //! TODO: try to get the servername from cmd option
        string servername( "vrc-server" );
        NodeInfo nodeinfo( arg_levelname, servername );
        unsigned int channel;
        Configuration::get()->getSettingValue( CTD_GS_SERVER_PORT, channel );
        _p_networkDevice->setupServer( channel, nodeinfo );
    }
    else if ( GameState::get()->getMode() == GameState::Client )
    {
        //! TODO: get the url from cmd option
        string url( "localhost" );
        string clientname( "vrc-client" );
        NodeInfo nodeinfo( "", clientname );
        unsigned int channel;
        Configuration::get()->getSettingValue( CTD_GS_SERVER_PORT, channel );

        if ( _p_networkDevice->setupClient( url, channel, nodeinfo ) )
            _p_networkDevice->startClient();

        // now load level
        string levelname = _p_networkDevice->getNodeInfo()->getLevelName();
        log << Log::LogLevel( Log::L_INFO ) << "loading level '" << levelname << "'" << endl;
        // load the level and setup things
        osg::ref_ptr< osg::Group > sceneroot = LevelManager::get()->loadLevel( levelname );
        if ( !sceneroot.valid() )
            return false;

        // if we directly start a client with cmd line option then we must send a leave-menu notification to entities
        //  as many entities do special steps when leaving the menu
        EntityNotification notify( CTD_NOTIFY_MENU_LEAVE, NULL );
        EntityManager::get()->sendNotification( notify );
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

    while( ( _p_gameState->getState() != GameState::Quitting ) && !_p_viewer->done() )
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
    }   

    // for the case that we quited via entity request ( such as menu's quit button pressing )
    _p_viewer->setDone( true );

    // wait for all cull and draw threads to complete before exit.
    _p_viewer->sync();
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

    // wait for all cullings and drawings to complete.
    _p_viewer->sync();

    // update the scene by traversing it with the the update visitor which will
    // call all node update callbacks and animations.
    _p_viewer->update();

    // fire off the cull and draw traversals of the scene.
    _p_viewer->frame();
}

void Application::updateServer( float deltaTime )
{
    // update server
    _p_networkDevice->updateServer( deltaTime );

    // update entities
    _p_entityManager->update( deltaTime  );

    // update physics
    _p_physics->update( deltaTime );

    //// update gui manager
    //_p_guiManager->update( deltaTime );

    //// wait for all cullings and drawings to complete.
    //_p_viewer->sync();

    //// update the scene by traversing it with the the update visitor which will
    //// call all node update callbacks and animations.
    //_p_viewer->update();

    //// fire off the cull and draw traversals of the scene.
    //_p_viewer->frame();
}

void Application::updateStandalone( float deltaTime )
{
    // update entities
    _p_entityManager->update( deltaTime  );

    // update physics
    _p_physics->update( deltaTime );

    // update gui manager
    _p_guiManager->update( deltaTime );

    // wait for all cullings and drawings to complete.
    _p_viewer->sync();

    // update the scene by traversing it with the the update visitor which will
    // call all node update callbacks and animations.
    _p_viewer->update();

    // fire off the cull and draw traversals of the scene.
    _p_viewer->frame();
}

void Application::stop()
{
    _p_gameState->setState( GameState::Quitting );
}
