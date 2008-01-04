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
 # a story builds an interaction task with the player.
 #  it can be e.g. a quest.
 #
 #   date of creation:  05/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _VRC_STORY_H_
#define _VRC_STORY_H_

#include <vrc_main.h>
#include "vrc_storyevent.h"


namespace vrc
{

//! Story class responsible to process player events and progress the story state
class Story
{
    public:

        //! Construct a story for ower with given ID ( e.g. player or NPC ID ).
        explicit                                    Story( unsigned int owerID );

        virtual                                     ~Story();

        //! Process the event in story.
        void                                        processEvent( const StoryEvent& event );

        //! Get the owner ID.
        unsigned int                                getOwnerID() const { return _ownerID; }

    protected:

        //! Story owner ID
        unsigned int                                _ownerID;
};

}

#endif // _VRC_STORY_H_
