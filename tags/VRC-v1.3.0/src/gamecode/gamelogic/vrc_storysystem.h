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
 # story system providing functionality for execuring the stories
 #  in game such as quests.
 #
 #   date of creation:  05/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/


#ifndef _VRC_STORYSYSTEM_H_
#define _VRC_STORYSYSTEM_H_

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include <gamelogic/vrc_story.h>
#include <gamelogic/vrc_storyevent.h>
#include <gamelogic/vrc_storyactor.h>

namespace vrc
{

//! Class for story system related exceptions
class StorySystemException : public std::runtime_error
{
    public:
                                                    StorySystemException( const std::string& reason ) :
                                                     std::runtime_error( reason )
                                                    {
                                                    }

        virtual                                     ~StorySystemException() throw() {}

                                                    StorySystemException( const StorySystemException& e ) :
                                                     std::runtime_error( e )
                                                    {
                                                    }

    protected:

                                                    StorySystemException();

        StorySystemException&                       operator = ( const StorySystemException& );
};


class Story;
class StoryEngine;
class StoryNetworking;
class ConsoleGUI;

//! Story system singleton providing all necessary functionality for running stories
class StorySystem : public yaf3d::Singleton< StorySystem >, public yaf3d::BaseEntity
{
    public:

        //! Add an actor for receiving actor events
        bool                                        addActor( unsigned int actorID, StoryEventReceiver* p_receiver );

        //! Remove an actor from receiving actor events
        bool                                        removeActor( unsigned int actorID, StoryEventReceiver* p_receiver );

        //! Send an event to stories or actors, the recipients depend on the defined event filter.
        void                                        sendEvent( const StoryEvent& event );

    protected:

                                                    StorySystem();

        virtual                                     ~StorySystem();

        //! Initialize the story system with given story book script
        void                                        initialize( const std::string& storybookfile ) throw ( StorySystemException );

        //! Shutdown the story system
        void                                        shutdown();

        //! Set the networking object. On clients, this is used by networking object itself on creation. On server the networking object is created by story system.
        void                                        setNetworking( StoryNetworking* p_networking );

        //! This method is called by networking in client mode or by story engine in server/standalone mode
        void                                        receiveEvent( const StoryEvent& event );

        //! Entity related method overides
        //  ###############################

        // Handle entity system notifications
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        //! Update entity
        void                                        updateEntity( float deltaTime );

        //! No need for transformation node
        const bool                                  isTransformable() const { return false; }

        //  ###############################

        //! Get the story system log object
        inline yaf3d::Log*                          getStoryLog();

        //! Get the script engine
        inline StoryEngine*                         getStoryEngine();

        //! The story engine
        StoryEngine*                                _p_storyEngine;

        //! Story system's networking
        StoryNetworking*                            _p_networking;

        //! Log object for story system output
        yaf3d::Log*                                 _p_log;

        //! Debug console
        ConsoleGUI*                                 _p_console;

        //! Typedef for actors < ID, callback object >.
        typedef std::map< unsigned int, StoryEventReceiver* > Actors;

        //! Registered actors for receiving events
        Actors                                      _actors;

        //! Internal flag for identifying the event passing phase for actors
        bool                                        _sendEventsToActors;

    friend class Story;
    friend class StoryEngine;
    friend class StoryNetworking;
    friend class gameutils::VRCStateHandler;
    friend class yaf3d::Singleton< StorySystem >;
};

//! Inline methods
inline yaf3d::Log* StorySystem::getStoryLog()
{
    return _p_log;
}

inline StoryEngine* StorySystem::getStoryEngine()
{
    return _p_storyEngine;
}


//! Convenient macros for story system components to access the story log.
#define storylog_info      *_p_log << yaf3d::Log::LogLevel( yaf3d::Log::L_INFO )
#define storylog_debug     *_p_log << yaf3d::Log::LogLevel( yaf3d::Log::L_DEBUG )
#define storylog_error     *_p_log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR )
#define storylog_warning   *_p_log << yaf3d::Log::LogLevel( yaf3d::Log::L_WARNING )
#define storylog_verbose   *_p_log << yaf3d::Log::LogLevel( yaf3d::Log::L_VERBOSE )

}

#endif // _VRC_STORYSYSTEM_H_
