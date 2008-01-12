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
#include "vrc_gamelogic.h"
#include "vrc_storyengine.h"
#include "vrc_storysystem.h"
#include <tools/vrc_consolegui.h>


#define SCRIPTING_LOG_FILE_NAME     "storysystem.log"


//! Implement the singleton
YAF3D_SINGLETON_IMPL( vrc::StorySystem )

namespace vrc
{

StorySystem::StorySystem() :
 _p_storyEngine( NULL )
{
    _p_log = new yaf3d::Log;
    _p_log->addSink( "file", yaf3d::Application::get()->getMediaPath() + std::string( SCRIPTING_LOG_FILE_NAME ), yaf3d::Log::L_VERBOSE );

    //!TODO show up the console only in dev builds!
    if ( yaf3d::GameState::get()->getMode() != yaf3d::GameState::Server )
    {
        _p_console = new ConsoleGUI;
        _p_console->initialize( "story system output", 0.1f, 0.68f, 0.84f, 0.29f, false, true );
        _p_log->addSink( "console", *_p_console, yaf3d::Log::L_VERBOSE );
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

    storylog_info << "StorySystem: setting up story book" << std::endl;

    _p_storyEngine = new StoryEngine();
    _p_storyEngine->loadStoryBook( storybookfile );

    // register the story system in entity manager so it can get updated and notified
    yaf3d::EntityManager::get()->registerNotification( this );
    yaf3d::EntityManager::get()->registerUpdate( this );

    //! TODO remove this test
#if 1
    StoryEvent event1( 1, 2, 3, 4, 5, StoryEvent::eFilterPublic, 42.0f, 43.f, "hello", "world" );
    processEvent( event1 );

    StoryEvent event2( 1, 2, 3, 4, 100, StoryEvent::eFilterPrivate, 42.0f, 43.f, "event should", "arrive" );
    processEvent( event2 );

#endif
}

void StorySystem::shutdown()
{
    // destroy the story engine
    if ( _p_storyEngine )
        delete _p_storyEngine;

    yaf3d::EntityManager::get()->registerNotification( this, false );
    yaf3d::EntityManager::get()->registerUpdate( this, false );

    // destroy the singleton
    destroy();
}

// Handle entity system notifications
void StorySystem::handleNotification( const yaf3d::EntityNotification& notification )
{
}

void StorySystem::updateEntity( float deltaTime )
{
    assert( _p_storyEngine && "system not ready!" );

    _p_storyEngine->update( deltaTime );
}


void StorySystem::processEvent( const StoryEvent& event )
{
    assert( _p_storyEngine && "system not ready!" );

    _p_storyEngine->processEvent( event );
}

} // namespace vrc
