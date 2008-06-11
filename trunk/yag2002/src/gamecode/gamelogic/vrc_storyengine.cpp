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
 # story engine checks incoming events and conditions and creates
 #  new stories. it further handles the progress of all stories.
 #
 #   date of creation:  05/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_storyengine.h"
#include "vrc_story.h"

namespace vrc
{

#define BOOK_SCRIPT_INTERFACE_NAME           "book"
#define BOOK_EXPOSED_METHOD_REGISTER_STORY   "registerStory"
#define BOOK_EXPOSED_METHOD_BEGIN_STORY      "beginStory"
#define BOOK_FCN_INITIALIZE                  "initialize"

//! Helper class for loading stories
class StoryBookLoader : public BaseScript< StoryBookLoader >
{
    public:

                                                    StoryBookLoader( StoryEngine* p_engine, yaf3d::Log* p_log ) :
                                                     _p_log( p_log ),
                                                     _p_storyEngine( p_engine )
                                                    {
                                                    }

        virtual                                     ~StoryBookLoader() {}

        //! Load all stories in given book file and store them in stories
        void                                        loadStoryBook( const std::string& filename );

        //! Exposed method for registering a story
        void                                        lregisterStory( const Params& arguments, Params& /*returnvalues*/ );

        //! Exposed method for creating a story
        void                                        lbeginStory( const Params& arguments, Params& /*returnvalues*/ );

    protected:

        std::set< std::string >                     _uniqueStoryTypes;

        yaf3d::Log*                                 _p_log;

        StoryEngine*                                _p_storyEngine;
};

void StoryBookLoader::loadStoryBook( const std::string& filename )
{
    try
    {
        // load script file and scope all exposed methods with module name BOOK_SCRIPT_INTERFACE_NAME
        loadScript( BOOK_SCRIPT_INTERFACE_NAME, filename, BaseScript< StoryBookLoader >::STRING );

        Params arguments;
        Params returnsvalues;

        // expose method for registering stories having the pseudo-signatur: void registerStory( story type, story file )
        {
            std::string storytype;
            std::string storyfile;
            arguments.add( storytype );
            arguments.add( storyfile );
            exposeMethod( BOOK_EXPOSED_METHOD_REGISTER_STORY, &StoryBookLoader::lregisterStory, arguments, returnsvalues );
        }

        arguments.clear();
        returnsvalues.clear();

        // expose method for creating stories having the pseudo-signatur: void createStory( story type, story name, owner ID )
        {
            std::string  storytype;
            std::string  storyname;
            unsigned int ownerID = 0;
            arguments.add( storytype );
            arguments.add( storyname );
            arguments.add( ownerID );
            exposeMethod( BOOK_EXPOSED_METHOD_BEGIN_STORY, &StoryBookLoader::lbeginStory, arguments, returnsvalues );
        }

        // execute the script after exposing methods; after this, all script functions are ready to be called now
        execute();

        // call the initialize function
        arguments.clear();
        callScriptFunction( BOOK_FCN_INITIALIZE, &arguments );
    }
    catch( const ScriptingException& e )
    {
        log_error << "error occured during setting up the story book: " << e.what() << std::endl;
        storylog_error << "error occured during setting up the story book: " << e.what() << std::endl;
    }
}

void StoryBookLoader::lregisterStory( const Params& arguments, Params& /*returnvalues*/ )
{
    if ( arguments.size() < 2 )
    {
        log_error << "StoryBookLoader: wrong argument count on calling setupStory!" << std::endl;
        storylog_error << "StoryBookLoader: wrong argument count on calling setupStory!" << std::endl;
        return;
    }

    std::string storytype = GET_SCRIPT_PARAMVALUE( arguments, 0, std::string );
    std::string storyfile = GET_SCRIPT_PARAMVALUE( arguments, 1, std::string );

    storylog_debug << "StoryBookLoader: registering story '" << storytype << "', " << storyfile << std::endl;

    // check uniqueness of story names
    if ( _uniqueStoryTypes.find( storytype ) != _uniqueStoryTypes.end() )
    {
        storylog_error << "StoryEngine:: detected duplicated story type name '" << storytype << "'" << std::endl;
    }
    else
    {
        // register the story name for later uniqueness check
        _uniqueStoryTypes.insert( storytype );

        // load and initialize the story and push it into story stock
        StoryPtr story = new Story( storytype, storytype + "_STOCK_" );
        if ( story->setup( storyfile ) )
            _p_storyEngine->_storyStock.push_back( story );
    }
}

void StoryBookLoader::lbeginStory( const Params& arguments, Params& /*returnvalues*/ )
{
    if ( arguments.size() < 3 )
    {
        storylog_error << "StoryBookLoader: invalid call of function '" << BOOK_EXPOSED_METHOD_BEGIN_STORY << "' without story type, name, and owner ID arguments!" << std::endl;
        return;
    }

    std::string  storytype = GET_SCRIPT_PARAMVALUE( arguments, 0, std::string );
    std::string  storyname = GET_SCRIPT_PARAMVALUE( arguments, 1, std::string );
    unsigned int ownerID   = GET_SCRIPT_PARAMVALUE( arguments, 2, unsigned int );

    if ( !_p_storyEngine->beginStory( storytype, storyname, ownerID ) )
        storylog_error << "StoryBookLoader: could not create story type '" << storytype << "' with name: '" << storyname << "' for owner ID: " << ownerID << std::endl;
    else
        storylog_verbose << "StoryBookLoader: created story '" << storytype << "' with name: '" << storyname << "' for owner ID: " << ownerID << std::endl;
}


//! Implementation of story engine
StoryEngine::StoryEngine() :
 _dialogIDs( 1000 ),
 _time( 0 )
{
    _p_log = StorySystem::get()->getStoryLog();
}

StoryEngine::~StoryEngine()
{
    storylog_debug << "StoryEngine: shuttding down" << std::endl;

    storylog_debug << "StoryEngine: count of stock stories " << _storyStock.size() << std::endl;
    _storyStock.clear();

    storylog_debug << "StoryEngine: count of active stories " << _stories.size() << std::endl;
    _stories.clear();

    storylog_debug << "StoryEngine: count of queued new stories " << _newStories.size() << std::endl;
    _newStories.clear();

    storylog_debug << "StoryEngine: count of queued ended stories " << _endedStories.size() << std::endl;
    _endedStories.clear();

    storylog_debug << "StoryEngine: count of queued events " << _eventQueue.size() << std::endl;
    _eventQueue.clear();
}

void StoryEngine::loadStoryBook( const std::string& filename ) throw ( StorySystemException )
{
    storylog_debug << "StoryEngine: loading story book '" << filename << "'" << std::endl;

    // load the story prototypes
    StoryBookLoader loader( this, StorySystem::get()->getStoryLog() );
    loader.loadStoryBook( filename );

    storylog_debug << "StoryEngine: count of stock stories " << _storyStock.size() << std::endl;
}

void StoryEngine::processEvent( const StoryEvent& event )
{
    unsigned int eventfilter = event.getFilter();

    // check for a valid event filter
    if ( !( eventfilter &
            (
             StoryEvent::eFilterStoryBook |
             StoryEvent::eFilterStoryPrivate |
             StoryEvent::eFilterStoryPublic |
             StoryEvent::eFilterActorPrivate |
             StoryEvent::eFilterActorPublic
            )
          ) )
    {
        storylog_error << "unknown story event filter " << eventfilter << std::endl;
        return;
    }

    StoryInstances::iterator p_story = _stories.begin(), p_storyEnd = _stories.end();
    unsigned int targetID = event.getTargetID();

    // first check the stock if new stories can be created caused by the event
    if ( eventfilter & StoryEvent::eFilterStoryBook )
    {
        StoryStock::iterator p_stock = _storyStock.begin(), p_stockEnd = _storyStock.end();
        for ( ; p_stock != p_stockEnd; ++p_stock )
            ( *p_stock )->processEvent( _time, event );
    }

    // private event sent to a story?
    if ( eventfilter & StoryEvent::eFilterStoryPrivate )
    {
        // find the target of event
        if ( _stories.find( targetID ) == p_storyEnd )
        {
            storylog_error << "Actor->Story: event cannot be passed to unknown target ID: " << targetID << std::endl;
        }
        else
        {
            // iterate through all stories of target
            std::vector< StoryPtr >::iterator p_currstory = _stories[ targetID ].begin(), p_currstoryEnd = _stories[ targetID ].end();
            for ( ; p_currstory != p_currstoryEnd; ++p_currstory )
            {
                ( *p_currstory )->processEvent( _time, event );
            }
        }
    }
    // story public events go to all stories
    else if ( eventfilter & StoryEvent::eFilterStoryPublic )
    {
        for ( p_story = _stories.begin(), p_storyEnd = _stories.end(); p_story != p_storyEnd; ++p_story )
        {
            // iterate through all lookup table entries
            std::vector< StoryPtr >::iterator p_currstory = p_story->second.begin(), p_currstoryEnd = p_story->second.end();
            for ( ; p_currstory != p_currstoryEnd; ++p_currstory )
            {
                 ( *p_currstory )->processEvent( _time, event );
            }
        }
    }
    // seems to be an actor event, pass it to story system (the actors are registered in StorySystem)
    else
    {
        StorySystem::get()->receiveEvent( event );
    }
}

unsigned int StoryEngine::registerDialog( Story* p_story )
{
    // create a new dialog ID
    _dialogIDs ++;

    _dialogs[ _dialogIDs ] = p_story;

    return _dialogIDs;
}

void StoryEngine::openDialog( const StoryDialogParams& params )
{
    StorySystem::get()->openDialog( params );
}

void StoryEngine::deregisterDialog( unsigned int dialogID )
{
    Dialogs::iterator p_dialog = _dialogs.find( dialogID );

    if ( p_dialog == _dialogs.end() )
    {
        log_error << "StoryEngine: try to de-register a dialog ID which does not exists " << dialogID << std::endl;
        return;
    }

    // remove the dialog ID from lookup
    _dialogs.erase( p_dialog );
}

void StoryEngine::processDialogResults( const StoryDialogResults& results )
{
    // find the story identified by the dialog id in internal dialog lookup
    Dialogs::iterator p_dialog = _dialogs.find( results._id );

    if ( p_dialog == _dialogs.end() )
    {
        log_error << "StoryEngine: dialog results received with an invalid dialog ID " << results._id << std::endl;
        return;
    }

    // propagate the results to story
    //! NOTE: a call of this method can modify _dialogs when new dialogs are created by this call. so p_dialog may be invalid after this call!
    p_dialog->second->processDialogResutls( results );
}

void StoryEngine::update( float deltaTime )
{
    // increment the story time
    _time++;

    // move new created stories to story container
    if ( _newStories.size() )
    {
        StoryInstances::iterator p_story = _newStories.begin(), p_storyEnd = _newStories.end();
        for ( ; p_story != p_storyEnd; ++p_story )
        {
            _stories[ p_story->first ].insert( _stories[ p_story->first ].end(), p_story->second.begin(), p_story->second.end() );
        }
        _newStories.clear();
    }

    // remove ended stories from story container
    if ( _endedStories.size() )
    {
        StoryInstances::iterator p_story = _endedStories.begin(), p_storyEnd = _endedStories.end();
        for ( ; p_story != p_storyEnd; ++p_story )
        {
            // check if the story really exists
            StoryInstances::iterator p_storyowner = _stories.find( p_story->first );
            if ( p_storyowner != _stories.end() )
            {
                // remove stories from owner
                removeStories( p_storyowner->second, p_story->second );
            }
        }
        _endedStories.clear();
    }

    // process queued events
    if ( _eventQueue.size() )
    {
        EventQueue::iterator p_event = _eventQueue.begin(), p_eventEnd = _eventQueue.end();
        for ( ; p_event != p_eventEnd; ++p_event )
            processEvent( *p_event );

        _eventQueue.clear();
    }

    // call the update of all active stories
    {
        StoryInstances::iterator p_storyowner = _stories.begin(), p_storyownerEnd = _stories.end();
        for ( ; p_storyowner != p_storyownerEnd; ++p_storyowner )
        {
            std::vector< StoryPtr >::iterator p_story = p_storyowner->second.begin(), p_storyEnd = p_storyowner->second.end();
            for ( ; p_story != p_storyEnd; ++p_story )
                ( *p_story )->update( deltaTime );
        }
    }
}

bool StoryEngine::beginStory( const std::string storytype, const std::string storyname, unsigned int ownerID )
{
    StoryStock::iterator p_stock = _storyStock.begin(), p_stockEnd = _storyStock.end();
    for ( ; p_stock != p_stockEnd; ++p_stock )
    {
        if ( ( *p_stock )->getType() == storytype )
            break;
    }

    if ( p_stock == p_stockEnd )
    {
        storylog_error << "StoryEngine: request for creation of an unknown story type '" << storytype << "'" << std::endl;
        return false;
    }

    // create the new stroy from stock
    StoryPtr newstory = ( *p_stock )->clone( ownerID, storyname );
    // add it to the temporary container, they get moved to _stories on next update
    _newStories[ ownerID ].push_back( newstory );

    return true;
}

bool StoryEngine::closeStory( unsigned int ownerID, const std::string storyname )
{
    StoryInstances::iterator p_storyowner = _stories.find( ownerID );
    if ( p_storyowner == _stories.end() )
    {
        storylog_error << "StoryEngine: request for closing a strory with unknown owner ID, story name '" << storyname << "' story owner ID:" << ownerID << std::endl;
        return false;
    }

    std::vector< StoryPtr >::iterator p_currstory = p_storyowner->second.begin(), p_currstoryEnd = p_storyowner->second.end();
    for ( ; p_currstory != p_currstoryEnd; ++p_currstory )
    {
        if ( ( *p_currstory )->getName() == storyname )
            break;
    }
    if ( p_currstory == p_currstoryEnd )
    {
        storylog_error << "StoryEngine: request for ending of an non-existing strory, story name '" << storyname << "' story owner ID:" << ownerID << std::endl;
        return false;
    }

    // add to removal container, it will be removed on next update
    _endedStories[ ownerID ].push_back( *p_currstory );

    return true;
}

void StoryEngine::removeStories( std::vector< StoryPtr >& stories, std::vector< StoryPtr >& toberemoved )
{
    std::vector< StoryPtr >::iterator p_removestory = toberemoved.begin(), p_removestoryEnd = toberemoved.end();
    for ( ; p_removestory != p_removestoryEnd; ++p_removestory )
    {
        std::vector< StoryPtr >::iterator p_currstory = stories.begin(), p_currstoryEnd = stories.end();
        for ( ; p_currstory != p_currstoryEnd; ++p_currstory )
        {
            if ( ( *p_currstory ).getRef() == ( *p_removestory ).getRef() )
            {
                stories.erase( p_currstory );
                p_currstory = stories.begin(), p_currstoryEnd = stories.end();
            }
        }
    }
}

void StoryEngine::enqueueEvent( const StoryEvent& event )
{
    _eventQueue.push_back( event );
}

} // namespace vrc
