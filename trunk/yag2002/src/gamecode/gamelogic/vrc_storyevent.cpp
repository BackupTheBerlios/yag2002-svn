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

StoryEvent::StoryEvent(
                        unsigned int eventType,
                        unsigned int sourceType,
                        unsigned int sourceID,
                        unsigned int targetType,
                        unsigned int targetID,
                        unsigned int filter,
                        int          networkID,
                        unsigned int uiParam1,
                        unsigned int uiParam2,
                        float        fParam1,
                        float        fParam2,
                        std::string  sParam
                        ) :
 _sourceType( sourceType ),
 _sourceID( sourceID ),
 _targetType( targetType ),
 _targetID( targetID ),
 _networkID( networkID ),
 _eventType( eventType ),
 _filter( filter ),
 _uiParam1( uiParam1 ),
 _uiParam2( uiParam2 ),
 _fParam1( fParam1 ),
 _fParam2( fParam2 ),
 _sParam( sParam )
{
}

StoryEvent::~StoryEvent()
{
}

} // namespace vrc
