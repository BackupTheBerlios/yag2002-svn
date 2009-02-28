/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2009, A. Botorabi
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
 # this is the interface to the game thread
 #
 #   date of creation:  01/22/2009
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/


#include <vrc_main.h>
#include "gameinterface.h"
#include "appinterface.h"
#include "editor.h"
#include "navigation.h"
#include "editorutils.h"


//! Update period limits ( in seconds )
#define UPPER_UPDATE_PERIOD_LIMIT           1.0f / 2.0f
#define LOWER_UPDATE_PERIOD_LIMIT           1.0f / 65.0f

//! No-Default-Level option used by editor when starting game thread
#define GAME_THREAD_OPT_NO_DEFAULT_LEVEL    "-nodefaultlevel"


//! Game loop thread
GameInterface::GameInterface( const std::string& apppath ) :
 _p_game( NULL ),
 _p_appIface( NULL ),
 _p_navigation( NULL ),
 _terminate( false ),
 _appPath( apppath )
{
}

GameInterface::~GameInterface()
{
}

void GameInterface::initialize( AppInterface* p_appiface )
{
    assert( p_appiface && "invalid application interface!" );

    _p_appIface = p_appiface;

    // start the game thread
    start();
}

void GameInterface::run()
{
    assert( _appPath.length() && "invalid application path!" );

    for ( ; ; )
    {
        try
        {
            _p_game = yaf3d::Application::get();

            // assemble argv
            char*  p_arg0 = new char[ 512 ];
            char*  p_arg1 = new char[ 512 ];
            strncpy( p_arg0, _appPath.c_str(), 512 );
            strncpy( p_arg1, GAME_THREAD_OPT_NO_DEFAULT_LEVEL, 512 );
            char** p_argv = new char*[ 2 ];
            p_argv[ 0 ] = p_arg0;
            p_argv[ 1 ] = p_arg1;

            // initialize threading
            OpenThreads::Thread::Init();

            // initialize
            if ( !_p_game->initialize( 2, p_argv ) )
            {
                _p_appIface->sendCmd( AppInterface::CMD_ERROR, NULL );
                _p_game->shutdown();
                break;
            }

            delete[] p_arg0;
            delete[] p_arg1;
            delete[] p_argv;

            // create the game navigator
            _p_navigation = GameNavigator::get();
            _p_navigation->initialize();

            // set game state
            yaf3d::GameState::get()->setState( yaf3d::GameState::StartRunning );

            // now the network can start
            if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Client )
            {
                // try to start client networking
                try
                {
                    yaf3d::NetworkDevice::get()->startClient();
                    // send the notification on established network session
                    yaf3d::EntityNotification ennotify( YAF3D_NOTIFY_NETWORKING_ESTABLISHED );
                    yaf3d::EntityManager::get()->sendNotification( ennotify );
                }
                catch ( const yaf3d::NetworkException& e )
                {
                    log_error << "Application: error starting client networking, reason: " << e.what() << std::endl;
                    yaf3d::GameState::get()->setState( yaf3d::GameState::Quitting );
                }
            }

            osg::Timer       timer;
            osg::Timer_t     curTick   = timer.tick();
            osg::Timer_t     lastTick  = curTick;
            float            deltaTime = LOWER_UPDATE_PERIOD_LIMIT;

            // set game state
            yaf3d::GameState::get()->setState( yaf3d::GameState::MainLoop );

            _p_appIface->sendCmd( AppInterface::CMD_GAME_STARTED, NULL );

            // begin game loop
            while( yaf3d::GameState::get()->getState() != yaf3d::GameState::Quitting )
            {
                // limit the deltaTime as we have to be carefull with stability of physics calculation etc.
                if ( deltaTime > UPPER_UPDATE_PERIOD_LIMIT )
                {
                    deltaTime = UPPER_UPDATE_PERIOD_LIMIT;
                }
                else if ( deltaTime < LOWER_UPDATE_PERIOD_LIMIT )
                {
                    // limit the upper fps to about 60 frames / second ( there are crash problems in some gpu cards when vsync is disabled )
                    do
                    {
                        OpenThreads::Thread::microSleep( 1000 );
                        curTick   = timer.tick();
                        deltaTime = timer.delta_s( lastTick, curTick );
                    }
                    while ( deltaTime < LOWER_UPDATE_PERIOD_LIMIT );
                }

                // dispatch commands
                while ( dispatchCmd() ) {}

                // update the simulation
                if ( !yaf3d::Application::get()->getUpdateMutex().lock() )
                {
                    // update the navigator
                    if ( _p_navigation )
                        _p_navigation->update( deltaTime );

                    // update game
                    if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server ) // do we really need the server mode support here!?
                        yaf3d::Application::get()->updateServer( deltaTime );
                    else if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Client )
                        yaf3d::Application::get()->updateClient( deltaTime );
                    else yaf3d::Application::get()->updateStandalone( deltaTime );

                    // calculate new delta-time
                    lastTick  = curTick;
                    curTick   = timer.tick();
                    deltaTime = timer.delta_s( lastTick, curTick );

                    // check for stopping application
                    if ( _terminate )
                    {
                        _p_navigation->shutdown();
                        _p_navigation = NULL;

                        yaf3d::GameState::get()->setState( yaf3d::GameState::Quitting );
                    }

                    // unlock the update mutex
                    yaf3d::Application::get()->getUpdateMutex().unlock();
                }

                // check for termination
                if ( yaf3d::Application::get()->getViewer()->isTerminated() )
                    break;
            }

            // shutdown the game
            _p_game->shutdown();

            // leave the main loop
            break;
        }
        // catch exceptions and continue execution
        catch ( const std::exception& e )
        {
            log_error << "Editor: unhandled std exception occured: " << e.what() << std::endl;
        }
    }

    _p_game = NULL;
}

void GameInterface::terminate()
{
    if ( _p_game )
    {
        while( _p_game->getUpdateMutex().lock() )
            microSleep( 1000 );

        _terminate = true;

        _p_game->getUpdateMutex().unlock();

        while( isRunning() )
            microSleep( 100000 );
    }
}

void GameInterface::sendCmd( unsigned int cmd, void* p_data )
{
    _cmdMutex.lock();

    _cmds.push( std::make_pair( cmd, p_data ) );

    _cmdMutex.unlock();
}

unsigned int GameInterface::dispatchCmd()
{
    if ( _cmdMutex.trylock() )
        return 0;

    if ( !_cmds.size() )
    {
        _cmdMutex.unlock();
        return 0;
    }

    std::pair< unsigned int, void* > cmd = _cmds.front();
    _cmds.pop();

    switch ( cmd.first )
    {
        case CMD_LOAD_LEVEL:
        {
            log_info << "[Editor] loading level ..." << std::endl;

            yaf3d::Application::get()->getUpdateMutex().lock();

            // set the proper game state
            yaf3d::GameState::get()->setState( yaf3d::GameState::StartingLevel );

            // load the requested level
            char* p_filename = reinterpret_cast< char* >( cmd.second );
            yaf3d::LevelManager::get()->loadLevel( p_filename );
            yaf3d::LevelManager::get()->finalizeLoading();

            // we have to simulate leaving the menu, this is needed by some entities
            yaf3d::EntityManager::get()->sendNotification( yaf3d::EntityNotification( YAF3D_NOTIFY_MENU_LEAVE ) );

            // set the user data into transformable entities, this is used for picking functionality
            std::vector< yaf3d::BaseEntity* > levelentities;
            yaf3d::EntityManager::get()->getAllEntities( levelentities );
            std::vector< yaf3d::BaseEntity* >::iterator p_beg = levelentities.begin(), p_end = levelentities.end();
            for ( ; p_beg != p_end; ++p_beg )
            {
                if ( ( *p_beg )->isTransformable() )
                {
                    osg::ref_ptr< EditorSGData > data = new EditorSGData( *p_beg );
                    ( *p_beg )->getTransformationNode()->setUserData( data.get() );
                }
            }

            // set the proper game state
            yaf3d::GameState::get()->setState( yaf3d::GameState::MainLoop );

            yaf3d::Application::get()->getUpdateMutex().unlock();

            delete[] p_filename;

            log_info << "[Editor] level loading completed" << std::endl;

            // notify the application interface
            _p_appIface->sendCmd( AppInterface::CMD_LEVEL_LOADED, NULL );
        }
        break;

        case CMD_UNLOAD_LEVEL:
        {
            yaf3d::Application::get()->getUpdateMutex().lock();

            // set the proper game state
            yaf3d::GameState::get()->setState( yaf3d::GameState::LeavingLevel );

            // fake the menu system
            yaf3d::EntityManager::get()->sendNotification( yaf3d::EntityNotification( YAF3D_NOTIFY_MENU_ENTER ) );

            // really clean the entities
            yaf3d::EntityManager::get()->sendNotification( yaf3d::EntityNotification( YAF3D_NOTIFY_SHUTDOWN ) );

            // unload the level
            yaf3d::LevelManager::get()->unloadLevel( true, true );

            // set the proper game state
            yaf3d::GameState::get()->setState( yaf3d::GameState::MainLoop );

            yaf3d::Application::get()->getUpdateMutex().unlock();

            // notify the application interface
            _p_appIface->sendCmd( AppInterface::CMD_LEVEL_UNLOADED, NULL );
        }
        break;

        case CMD_INIT_ENTITY:
        {
            yaf3d::BaseEntity* p_entity = reinterpret_cast< yaf3d::BaseEntity* >( cmd.second );
            assert( dynamic_cast< yaf3d::BaseEntity* >( p_entity ) );

            // the initialization of an entity must happen in game thread context!
            p_entity->initialize();
            p_entity->postInitialize();
            // some entities get last initializations when the leave menu notification comes!
            yaf3d::EntityNotification notify( YAF3D_NOTIFY_MENU_LEAVE );
            yaf3d::EntityManager::get()->sendNotification( notify, p_entity );

            // append the entity as user data to transformation node, this is needed for picking functionality
            if ( p_entity->isTransformable() )
            {
                osg::ref_ptr< EditorSGData > data = new EditorSGData( p_entity );
                p_entity->getTransformationNode()->setUserData( data.get() );
            }
        }
        break;

        case CMD_UPDATE_ENTITY:
        {
            yaf3d::BaseEntity* p_entity = reinterpret_cast< yaf3d::BaseEntity* >( cmd.second );
            assert( dynamic_cast< yaf3d::BaseEntity* >( p_entity ) );
            yaf3d::EntityManager::get()->sendNotification( YAF3D_NOTIFY_ENTITY_ATTRIBUTE_CHANGED, p_entity );
        }
        break;

        default:
            ;
    }

    unsigned int remainingcmds = _cmds.size();

    _cmdMutex.unlock();

    return remainingcmds;
}
