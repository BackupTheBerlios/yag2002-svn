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
 # story builder checks incoming events and conditions and creates
 #  new stories
 #
 #   date of creation:  05/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _VRC_STORYBUILDER_H_
#define _VRC_STORYBUILDER_H_

#include <vrc_main.h>
#include "vrc_storyevent.h"


namespace vrc
{

class Story;
class StorySystem;

//! Story builder class responsible for checking incoming events and creating new stories
class StoryBuilder
{
    public:

        /**
            Process the event and check if new stoies can be created caused by the event. Stores created stories in 'newstories'.
            Returns the number of created stories.
        */
        unsigned int                                processEvent( const StoryEvent& event, std::vector< Story* >& newstories );

    protected:

                                                    StoryBuilder();

        virtual                                     ~StoryBuilder();

    friend class StorySystem;
};

}

#endif // _VRC_STORYBUILDER_H_
