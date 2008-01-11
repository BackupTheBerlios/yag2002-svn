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

#include <vrc_main.h>
#include "vrc_storybuilder.h"
#include "vrc_story.h"

namespace vrc
{

StoryBuilder::StoryBuilder()
{
}

StoryBuilder::~StoryBuilder()
{
}

unsigned int StoryBuilder::processEvent( const StoryEvent& event, std::vector< Story* >& newstories )
{
    //! TODO: call checkStart on all available story templates. for each, if the function returns true then create new story
    //        idea: the story templates contain the information about by which events they are triggered, so we can do a smart filtering before calling the checkStart
    //        the stories must be personalized ( storing owner ID, setup of possible timeout, etc. )

    return 0;
}

} // namespace vrc
