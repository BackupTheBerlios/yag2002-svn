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
 # events triggered by / for user interaction with game world and NPCs
 #
 #   date of creation:  05/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_storyevent.h"


namespace vrc
{

StoryEvent::StoryEvent( unsigned int source, unsigned int target, unsigned int type, unsigned int targetInstanceID, unsigned int filter ) :
 _sourceID( source ),
 _targetID( target ),
 _targetInstanceID( targetInstanceID ),
 _type( type ),
 _filter( filter )
{
    assert( type == eTypePickItem || type == eTypeTalk );
    assert( filter == eFilterPrivate || filter == eFilterPublic );
}

StoryEvent::~StoryEvent()
{
}

} // namespace vrc
