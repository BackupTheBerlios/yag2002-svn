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
#include "vrc_storybuilder.h"
#include "vrc_storysystem.h"


//! Implement the singleton
YAF3D_SINGLETON_IMPL( vrc::StorySystem )

namespace vrc
{

StorySystem::StorySystem() :
 _p_storyBuilder( NULL )
{
}

StorySystem::~StorySystem()
{
}

void StorySystem::initialize() throw ( StorySystemException )
{
    assert( _p_storyBuilder == NULL && "seems to be already initialized!" );

    _p_storyBuilder = new StoryBuilder();
}

void StorySystem::shutdown()
{
    // destroy the story builder
    if ( _p_storyBuilder )
        delete _p_storyBuilder;

    // destroy user tasks
    Stories::iterator p_user = _stories.begin(), p_end = _stories.end();
    std::vector< Story* >::iterator p_story, p_storyEnd;
    for ( ; p_user != p_end; ++p_user )
    {
        p_story = p_user->second.begin(), p_storyEnd = p_user->second.end();
        for ( ; p_story != p_storyEnd; ++p_story )
        {
            delete *p_story;
        }
    }

    // destroy the singleton
    destroy();
}

void StorySystem::processEvent( const StoryEvent& event )
{
    assert( _p_storyBuilder && "system nor ready!" );

    // check if new stories can be created caused by the event
    std::vector< Story* > stories;
    unsigned int numnewstories = _p_storyBuilder->processEvent( event, stories );
    if ( numnewstories > 0 )
    {
        std::vector< Story* >::iterator p_story, p_storyEnd;
        for ( ; p_story != p_storyEnd; ++p_story )
        {
            _stories[ event.getSourceID() ].push_back( *p_story );
        }
    }

    // pass the event to existing stories with matching event owner
    if ( event.getFilter() == StoryEvent::eFilterPrivate )
    {
        propagateEventToStories( event.getSourceID(), event );
    }
    // pass the event to all existing stories
    else if ( event.getFilter() == StoryEvent::eFilterPublic )
    {
        Stories::iterator p_owner = _stories.begin(), p_end = _stories.end();
        for ( ; p_owner != p_end; ++p_owner )
        {
            propagateEventToStories( p_owner->first, event );
        }
    }
}

void StorySystem::propagateEventToStories( unsigned int ownerID, const StoryEvent& event )
{
    Stories::iterator p_owner = _stories.find( ownerID );
    // are ther stories for given owner?
    if ( p_owner == _stories.end() )
        return;

    std::vector< Story* >::iterator p_story, p_storyEnd;
    p_story = p_owner->second.begin(), p_storyEnd = p_owner->second.end();
    for ( ; p_story != p_storyEnd; ++p_story )
    {
        ( *p_story )->processEvent( event );
    }
}

} // namespace vrc
