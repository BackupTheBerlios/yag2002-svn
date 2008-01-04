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
#include "vrc_storyevent.h"
#include "vrc_story.h"

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


class StoryBuilder;

//! Story system singleton providing all necessary functionality for running stories
class StorySystem : public yaf3d::Singleton< StorySystem >
{
    public:

        //! Process an event
        void                                        processEvent( const StoryEvent& event );

    protected:

                                                    StorySystem();

        virtual                                     ~StorySystem();

        //! Initialize the storage server
        void                                        initialize() throw ( StorySystemException );

        //! Shutdown the storage server
        void                                        shutdown();

        //! Propagate given event to all stories with given ower ID
        void                                        propagateEventToStories( unsigned int ownerID, const StoryEvent& event );

        //! Typedef for stories < owner ID, stories >
        typedef std::map< unsigned int, std::vector< Story* > > Stories;

        //! Stories
        Stories                                     _stories;

        //! The story builder
        StoryBuilder*                               _p_storyBuilder;

    friend class gameutils::VRCStateHandler;
    friend class yaf3d::Singleton< StorySystem >;
};

}

#endif // _VRC_STORYSYSTEM_H_
