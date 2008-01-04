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

#ifndef _VRC_STORYEVENT_H_
#define _VRC_STORYEVENT_H_

#include <vrc_main.h>

namespace vrc
{

//! Story event class
class StoryEvent
{
    public:

        //! Event filter type defining the propagation of the event to receivers
        enum EventFilter
        {
            eFilterUnknown    = 0x00,
            //! Event is sent only to receivers (stories) which have an ower ID = event source ID
            eFilterPrivate    = 0x100,
            //! Event is sent to all receivers
            eFilterPublic     = 0x200
        };

        //! Event type
        enum EventType
        {
            eTypeUnknown    = 0x00,
            eTypePickItem   = 0x10,
            eTypeTalk       = 0x20
        };

        //! Create an event with given source, target, type and propagation filter
                                                    StoryEvent( unsigned int source, unsigned int target, unsigned int type, unsigned int targetInstanceID = 0, unsigned int filter = eFilterPrivate );

        virtual                                     ~StoryEvent();

        //! Get event type, one of EventType enums.
        unsigned int                                getType() const { return _type; }

        //! Get event filter, one of EventFilter enums.
        unsigned int                                getFilter() const { return _filter; }

        //! Get source ID.
        unsigned int                                getSourceID() const { return _sourceID; }

        //! Get target ID. This specifies a target type.
        unsigned int                                getTargetID() const { return _targetID; }

        //! Get target instance ID. This specefies an instance of a target type.
        unsigned int                                getTargetInstanceID() const { return _targetInstanceID; }

    protected:

        //! Source triggering the event ( e.g. player ID )
        unsigned int                                _sourceID;

        //! Event target, the event target specifies the target type.
        unsigned int                                _targetID;

        //! Event's instance target, this specefies an instance of a target type
        unsigned int                                _targetInstanceID;

        //! Event type
        unsigned int                                _type;

        //! Event filter
        unsigned int                                _filter;
};

}

#endif // _VRC_STORYEVENT_H_
