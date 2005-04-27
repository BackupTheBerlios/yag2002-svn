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
#include "ctd_application.h"
#include "ctd_levelmanager.h"
#include "ctd_entitymanager.h"
#include "ctd_physics.h"
#include "ctd_guimanager.h"
#include "ctd_configuration.h"
#include "ctd_keymap.h"
#include "ctd_gamestate.h"
#include "ctd_log.h"

#include <osg/VertexProgram>
#include <Producer/Camera>
#include <osgProducer/ViewerEventHandler>

using namespace std;
using namespace CTD; 
using namespace osg; 

// media path relative to inst dir
#define CTD_MEDIA_PATH  "/media/"

CTD_SINGLETON_IMPL( Application );

Application::Application():
_entityManager( EntityManager::get() ),
_p_gameState( GameState::get() ),
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
    _entityManager->shutdown();
    LevelManager::get()->shutdown();
    _p_soundManager->shutdown();
    _p_physics->shutdown();
    Configuration::get()->shutdown();
    _p_guiManager->shutdown();
    _p_gameState->shutdown();
    delete _p_viewer;

    destroy();
}

bool Application::initialize( int argc, char **argv )
{
    // set game state
    _p_gameState->setState( GameState::Initializing );

    // set mode to none; server or client must be choosen later
    _p_gameState->setMode( GameState::None );

    string arg_levelname;
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    ArgumentParser::Parameter levelparam( arg_levelname );
    if ( !arguments.read( "-level", arg_levelname ) )
    {
        return false;
    }

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
        //opt |= ~osgProducer::Viewer::ESCAPE_SETS_DONE;
    }

    // now setup the viewer
    _p_viewer->setUpViewer( opt );
    //----------

    // set fullscreen / windowed mode
    Producer::Camera *p_cam = _p_viewer->getCamera(0);
    Producer::RenderSurface* p_rs = p_cam->getRenderSurface();
    p_rs->setWindowRectangle( 100, 100, _screenWidth, _screenHeight );
    p_rs->fullScreen( false );
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

    log << Log::LogLevel( Log::L_INFO ) << "initializing physics" << endl;
    // init physics
    _p_physics = Physics::get();
    assert( _p_physics->initialize() );
    _p_physics->setWorldGravity( -9.8f );

    // load the level
    osg::ref_ptr< osg::Group > sceneroot = LevelManager::get()->load( getMediaPath() + arg_levelname );
    if ( !sceneroot.valid() )
        return false;

    // store the main group in application
    setSceneRootNode( sceneroot.get() );
    // level manager sets the scene data in Application
    _p_viewer->setSceneData( sceneroot.get() );
 
    log << Log::LogLevel( Log::L_INFO ) << "finalizing physics setup" << endl;
    // finalize physics initialization when all entities are created
    assert( _p_physics->finalize( _p_rootSceneNode ) );
    // enable physics debug rendering
    if ( enablePhysicsDebugRendering )
        _p_physics->enableDebugRender();

    // initialize sound manager
    _p_soundManager = osgAL::SoundManager::instance();
    try 
    {
        _p_soundManager->init( 16 );
        _p_soundManager->getEnvironment()->setDistanceModel( openalpp::InverseDistance );
        _p_soundManager->getEnvironment()->setDopplerFactor( 1 );
        osgAL::SoundRoot* p_soundRoot = new osgAL::SoundRoot;
        sceneroot->addChild( p_soundRoot );
    }
    catch( openalpp::InitError e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** cannot initialize sound device openAL. reason: '" << e.what() << "'" << endl;
        log << Log::LogLevel( Log::L_ERROR ) << "***   have you already installed the openAL drivers?" << endl;
        return false;
    }

    log << Log::LogLevel( Log::L_INFO ) << "initializing gui system..." << endl;
    // initialize the gui system
    _p_guiManager = GuiManager::get();
    _p_guiManager->initialize();
    // note: after initialization of gui system the viewer must be realized (in order to get gui's renderer
    //  initialized) before the entities are initialized
    // use single-threading (entity specific gl commands need it)
    _p_viewer->realize( osgProducer::Viewer::ThreadingModel::SingleThreaded ); 
    _p_viewer->sync();
    //--------

    log << Log::LogLevel( Log::L_INFO ) << "starting entity setup ..." << endl;
    osg::Timer      timer;
    osg::Timer_t    curTick    = 0;
    osg::Timer_t    startTick  = 0;
    float           time4Init      = 0;
    float           time4PostInit  = 0;
    // setup entities
    log << Log::LogLevel( Log::L_INFO ) << " initializing entities..." << endl;

    {
        startTick  = timer.tick();

        _entityManager->initializeEntities();
        
        curTick    = timer.tick();
        time4Init  = timer.delta_s( startTick, curTick );
    }

    log << Log::LogLevel( Log::L_INFO ) << " post-initializing entities..." << endl;
    {
        startTick  = timer.tick();
    
        _entityManager->postInitializeEntities();

        curTick        = timer.tick();
        time4PostInit  = timer.delta_s( startTick, curTick );
    }
    log << Log::LogLevel( Log::L_INFO ) << "entity setup completed" << endl;

    log << Log::LogLevel( Log::L_INFO ) << "--------------------------------------------" << endl;
    log << Log::LogLevel( Log::L_INFO ) << "needed time for initialization: " << time4Init << " seconds" << endl;
    log << Log::LogLevel( Log::L_INFO ) << "needed time for post-initialization: " << time4PostInit << " seconds" <<  endl;
    log << Log::LogLevel( Log::L_INFO ) << "total time for setting up: " << time4Init + time4PostInit << " seconds" <<  endl;
    log << Log::LogLevel( Log::L_INFO ) << "--------------------------------------------" << endl;

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
        else if ( deltaTime < 0.01f )
            deltaTime = 0.01f;

        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        _p_viewer->update();

        // update entities
        _entityManager->update( deltaTime  );

        // update physics
        _p_physics->update( deltaTime );

        // update gui manager
        _p_guiManager->update( deltaTime );
         
        // wait for all cull and draw threads to complete.
        _p_viewer->sync();

        // fire off the cull and draw traversals of the scene.
        _p_viewer->frame();
    }   

    // for the case that we quited via entity request ( such as menu's quit button pressing )
    _p_viewer->setDone( true );

    // wait for all cull and draw threads to complete before exit.
    _p_viewer->sync();
}

void Application::stop()
{
    _p_gameState->setState( GameState::Quitting );
}
