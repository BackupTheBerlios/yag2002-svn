/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2004-2006, Ali Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU General Public License 
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public 
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
#include "ctd_log.h"

#include <osg/VertexProgram>
#include <Producer/Camera>

using namespace std;
using namespace CTD; 
using namespace osg; 

CTD_SINGLETON_IMPL( Application );

Application::Application():
_entityManager( EntityManager::get() ),
_running( false ),
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
    delete _p_viewer;

    destroy();
}

bool Application::initialize( int argc, char **argv )
{
    string levelname;
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    ArgumentParser::Parameter levelparam( levelname );
    if ( !arguments.read( "-level", levelparam ) )
    {
        return false;
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
    _mediaPath += "/media/";
    //-------------------

    // load the settings
    //-------------------
    Configuration::get()->getSettingValue( CTD_GS_SCREENWIDTH,  _screenWidth );
    Configuration::get()->getSettingValue( CTD_GS_SCREENHEIGHT, _screenHeight );
    //-------------------

    // setup log system
    //-----------------
    //! TODO: set the loglevels basing on config file
    log.addSink( "file", getMediaPath() + "ctd.log", Log::L_DEBUG );
    log.addSink( "stdout", cout, Log::L_DEBUG );
 
    log << Log::LogLevel( Log::L_INFO ) << "initializing viewer" << endl;
    // construct the viewer
    _p_viewer = new osgProducer::Viewer( arguments );

    // set up the value with sensible default event handlers.
    unsigned int opt = osgProducer::Viewer::STANDARD_SETTINGS;
    opt &= ~osgProducer::Viewer::HEAD_LIGHT_SOURCE;
    _p_viewer->setUpViewer( osgProducer::Viewer::SKY_LIGHT_SOURCE | opt );

    // set fullscreen / windowed mode
    Producer::Camera *p_cam = _p_viewer->getCamera(0);
    Producer::RenderSurface* p_rs = p_cam->getRenderSurface();
    p_rs->setWindowRectangle( 100, 100, _screenWidth, _screenHeight );
    p_rs->fullScreen( false );

    // get details on keyboard and mouse bindings used by the viewer.
    _p_viewer->getUsage( *arguments.getApplicationUsage() );

    log << Log::LogLevel( Log::L_INFO ) << "initializing physics" << endl;
    // init physics
    _p_physics = Physics::get();
    assert( _p_physics->initialize() );
    _p_physics->setWorldGravity( -9.8f );

    // load the level
    osg::ref_ptr< osg::Group > sceneroot = LevelManager::get()->load( getMediaPath() + levelname );
    if ( !sceneroot.valid() )
        return false;

    // store the main group in application
    setSceneRootNode( sceneroot.get() );
    // level manager sets the scene data in Application
    _p_viewer->setSceneData( sceneroot.get() );
 
    // finalize physics initialization when all entities are created
    assert( _p_physics->finalize( _p_rootSceneNode ) );
    // enable physics debug rendering
//    _p_physics->enableDebugRender();

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

    log << Log::LogLevel( Log::L_INFO ) << "initializing entities..." << endl;
    // setup entities
    _entityManager->initializeEntities();
    _entityManager->postInitializeEntities();

    log << Log::LogLevel( Log::L_INFO ) << "initializing gui system..." << endl;
    // initialize the gui system
    _p_guiManager = GuiManager::get();
    _p_guiManager->initialize();

    log << Log::LogLevel( Log::L_INFO ) << "initialization  succeeded" << endl;
    return true;
}

void Application::run()
{
    _running = true;

    // create the windows and run the threads.
    _p_viewer->realize();

    osg::Timer       timer;
    float            deltaTime = 0;
    osg::Timer_t     curTick   = 0;
    osg::Timer_t     lastTick  = 0;

    while( !_p_viewer->done() && _running )
    {
        lastTick  = curTick;
        curTick   = timer.tick();
        deltaTime = timer.delta_s( lastTick, curTick );

        // limit the deltaTime as we have to be carefull with stability of physics calculation etc.
        if ( deltaTime > 0.06f ) 
            deltaTime = 0.06f;
        else if ( deltaTime < 0.01f )
            deltaTime = 0.01f;

        // wait for all cull and draw threads to complete.
        _p_viewer->sync();

        // update entities
        _entityManager->update( deltaTime  );

        // update physics
        _p_physics->update( deltaTime );

        // update gui manager
        _p_guiManager->update( deltaTime );

        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        _p_viewer->update();
         
        // fire off the cull and draw traversals of the scene.
        _p_viewer->frame();
    }

    // exit viewer
    //_p_viewer->setDone( true );
    
    // wait for all cull and draw threads to complete before exit.
    _p_viewer->sync();

}

void Application::stop()
{
    _running = false;
}
