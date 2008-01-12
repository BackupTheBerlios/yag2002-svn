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

#ifndef _VRC_STORYENGINE_H_
#define _VRC_STORYENGINE_H_

#include <vrc_main.h>
#include "vrc_storyevent.h"
#include "vrc_storysystem.h"

namespace vrc
{

//! Story builder class responsible for checking incoming events and creating new stories
class StoryEngine
{
    protected:

                                                    StoryEngine();

        virtual                                     ~StoryEngine();

        //! Load the story book, this is a script file containing all stories.
        void                                        loadStoryBook( const std::string& filename ) throw ( StorySystemException );

        //! Process the event and check if new stoies can be created caused by the event. This is called by story system.
        void                                        processEvent( const StoryEvent& event );

        //! Update the active stories.
        void                                        update( float deltaTime );

        //! Create a new story from stock, return false if the story does not exist in stock.
        bool                                        beginStory( const std::string storytype, const std::string storyname, unsigned int ownerID );

        //! End the given story of given owner
        bool                                        closeStory( unsigned int ownerID, const std::string storyname );

        //! Internally used method for removing a list of stories from another list of stories
        void                                        removeStories( std::vector< StoryPtr >& stories, std::vector< StoryPtr >& toberemoved );

        //! Type for story stock
        typedef std::vector< StoryPtr >             StoryStock;

        StoryStock                                  _storyStock;

        //! Typedef for stories < owner ID, stories >. The stories are gathered into a lookup table with story owner as key.
        typedef std::map< unsigned int, std::vector< StoryPtr > > StoryInstances;

        //! Concrete instance of stories
        StoryInstances                              _stories;

        //! Stories which are created during one time stamp, on next update they get moved to the story lookup _stories.
        StoryInstances                              _newStories;

        //! Stories which are ended during one time stamp, on next update they get removed from the story lookup _stories.
        StoryInstances                              _endedStories;

        //! Current story time
        unsigned int                                _time;

        //! Log object for story system output
        yaf3d::Log*                                 _p_log;

    friend class Story;
    friend class StorySystem;
    friend class StoryBookLoader;
};

}

#endif // _VRC_STORYENGINE_H_
