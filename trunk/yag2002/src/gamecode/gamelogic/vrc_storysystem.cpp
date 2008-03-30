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
 # user interaction system including interaction stories
 #
 #   date of creation:  05/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_storyengine.h"
#include "vrc_storysystem.h"
#include "vrc_storynetworking.h"
#include <tools/vrc_consolegui.h>


#define SCRIPTING_LOG_FILE_NAME     "storysystem"


//! TODO: implement a console cmd for showing the console and remove this define
//#define ENABLE_CONSOLE

//! Implement the singleton
YAF3D_SINGLETON_IMPL( vrc::StorySystem )

namespace vrc
{

StorySystem::StorySystem() :
 _p_storyEngine( NULL ),
 _p_networking( NULL ),
 _p_log( NULL ),
 _p_console( NULL ),
 _sendEventsToActors( false )
{
    std::string filename( SCRIPTING_LOG_FILE_NAME "_server.log" );
    if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Standalone )
        filename = SCRIPTING_LOG_FILE_NAME "_standalone.log";
    else if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Client )
        filename = SCRIPTING_LOG_FILE_NAME "_client.log";

    _p_log = new yaf3d::Log;
    _p_log->addSink( "file", yaf3d::Application::get()->getMediaPath() + filename, yaf3d::Log::L_VERBOSE );

    //!TODO show up the console only in dev builds!
    if ( yaf3d::GameState::get()->getMode() != yaf3d::GameState::Server )
    {
//! TODO: implement a console cmd for showing the console
#ifdef ENABLE_CONSOLE
        _p_console = new ConsoleGUI;
        _p_console->initialize( "story system output", 0.1f, 0.68f, 0.84f, 0.29f, false, true );
        _p_log->addSink( "console", *_p_console, yaf3d::Log::L_VERBOSE );
#endif

    }

    storylog_info << "date " << yaf3d::getFormatedDateAndTime() << std::endl;
}

StorySystem::~StorySystem()
{
    if ( _p_log )
        delete _p_log;

    _p_log = NULL;

    if ( _p_console )
        delete _p_console;
}

void StorySystem::initialize( const std::string& storybookfile ) throw ( StorySystemException )
{
    assert( _p_storyEngine == NULL && "seems to be already initialized!" );

    std::string mode = ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server ) ? "(server):" : ":";
    storylog_info << "StorySystem " << mode << " initializing ..." << std::endl;
    log_info << "StorySystem " << mode << " initializing ..." << std::endl;

    // the story engine exists only on server
    if ( yaf3d::GameState::get()->getMode() & ( yaf3d::GameState::Server | yaf3d::GameState::Standalone ) )
    {
        storylog_info << "StorySystem: setting up story book" << std::endl;

        _p_storyEngine = new StoryEngine();
        _p_storyEngine->loadStoryBook( storybookfile );

        // register the story system in entity manager so it can get updated and notified
        yaf3d::EntityManager::get()->registerNotification( this );
        yaf3d::EntityManager::get()->registerUpdate( this );

        // start up the networking  on server
        if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server )
        {
            _p_networking = new StoryNetworking;
            _p_networking->Publish();
        }
    }
}

void StorySystem::shutdown()
{
    log_info << "StorySystem: shuttding down" << std::endl;
    storylog_info << "StorySystem: shuttding down" << std::endl;

    storylog_debug << "StorySystem: count of still registered actors " << _actors.size() << std::endl;
    _actors.clear();

    // destroy the story engine
    if ( _p_storyEngine )
        delete _p_storyEngine;

    _p_storyEngine = NULL;

    if ( yaf3d::GameState::get()->getMode() & ( yaf3d::GameState::Server | yaf3d::GameState::Standalone ) )
    {
        // deregister us from entity manager
        yaf3d::EntityManager::get()->registerNotification( this, false );
        yaf3d::EntityManager::get()->registerUpdate( this, false );
    }

    // on clients the networking is automatically deleted by networking device
    if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server )
        delete _p_networking;

    _p_networking = NULL;

    // destroy the singleton
    destroy();
}

void StorySystem::setNetworking( StoryNetworking* p_networking )
{
    _p_networking = p_networking;
}

void StorySystem::handleNotification( const yaf3d::EntityNotification& /*notification*/ )
{
    // check if we need any notification
}

void StorySystem::updateEntity( float deltaTime )
{
    if ( yaf3d::GameState::get()->getMode() & ( yaf3d::GameState::Server | yaf3d::GameState::Standalone ) )
    {
        assert( _p_storyEngine && "system not ready!" );
        _p_storyEngine->update( deltaTime );
    }
}

bool StorySystem::addActor( unsigned int actorID, StoryEventReceiver* p_receiver )
{
    if ( _sendEventsToActors )
    {
        storylog_error << "StoryEngine: trying to modify the actor list during event receiving phase!" << std::endl;
        return false;
    }

    if ( _actors.find( actorID ) != _actors.end() )
    {
        storylog_error << "StoryEngine: actor already exists in receiver list" << std::endl;
        return false;
    }

    _actors[ actorID ] = p_receiver;

    return true;
}

bool StorySystem::removeActor( unsigned int actorID, StoryEventReceiver* /*p_receiver*/ )
{
    if ( _sendEventsToActors )
    {
        storylog_error << "StoryEngine: trying to modify the actor list during event receiving phase!" << std::endl;
        return false;
    }

    if ( _actors.find( actorID ) == _actors.end() )
    {
        storylog_error << "StoryEngine: cannot remove actor from receiver list, it does not exist" << std::endl;
        return false;
    }

    _actors.erase( _actors.find( actorID ) );

    return true;
}

void StorySystem::sendEvent( const StoryEvent& event )
{
    if ( yaf3d::GameState::get()->getMode() & ( yaf3d::GameState::Server | yaf3d::GameState::Standalone ) )
    {
        assert( _p_storyEngine && "system not ready!" );
        _p_storyEngine->processEvent( event );
    }
    else
    {
        if ( _p_networking )
            _p_networking->sendEvent( event );
    }
}

void StorySystem::receiveEvent( const StoryEvent& event )
{ // this method is called by networking ( only on clients ) or story engine ( on server or standalone )

    // pass the event to actors now
    unsigned int eventfilter = event.getFilter();
    unsigned int targetID = event.getTargetID();

    if ( yaf3d::GameState::get()->getMode() & ( yaf3d::GameState::Client | yaf3d::GameState::Standalone ) )
    {
        // actor private event
        if ( eventfilter & StoryEvent::eFilterActorPrivate )
        {
            // lock actor list modification during this phase
            _sendEventsToActors = true;

            Actors::iterator p_actor = _actors.find( targetID );
            if ( p_actor == _actors.end() )
            {
                storylog_error << "Story->Actor: event cannot be passed to unknown target ID: " << targetID << std::endl;
            }
            else
            {
                p_actor->second->onReceiveEvent( event );
            }
        }
        // actor public event
        else if ( eventfilter & StoryEvent::eFilterActorPublic )
        {
            // lock actor list modification during this phase
            _sendEventsToActors = true;

            Actors::iterator p_actor = _actors.begin(), p_actorEnd = _actors.end();
            for ( ; p_actor != p_actorEnd; ++p_actor )
            {
                p_actor->second->onReceiveEvent( event );
            }
        }
        else
        {
            storylog_warning << "Story->Actor: trying to send a wrong event type, target ID: " << targetID << std::endl;
        }

        // reset the actor sending events flag
        _sendEventsToActors = false;
    }
    // the server must pass the event to client by using the networking component
    else if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server )
    {
        if ( eventfilter & ( StoryEvent::eFilterActorPrivate | StoryEvent::eFilterActorPublic ) )
        {
            if ( _p_networking )
            {
                _p_networking->sendEvent( event );
            }
            else
            {
                storylog_warning << "Story->Actor: event could not be sent as networking not ready, target ID: " << targetID << std::endl;
            }
        }
        else
        {
            storylog_warning << "Story->Actor (server): trying to send a wrong event type, target ID: " << targetID << std::endl;
        }
    }
}

} // namespace vrc
