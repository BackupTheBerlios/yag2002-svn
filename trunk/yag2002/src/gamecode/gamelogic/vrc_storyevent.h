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

        //! Create an event with given source, target, type and propagation filter. Several generic event parameters can also be defined.
                                                    StoryEvent(
                                                                unsigned int eventType,
                                                                unsigned int sourceType,
                                                                unsigned int sourceID,
                                                                unsigned int targetType,
                                                                unsigned int targetID,
                                                                unsigned int filter = eFilterPublic,
                                                                float        fParam1 = 0.0f,
                                                                float        fParam2 = 0.0f,
                                                                std::string  sParam1 = "",
                                                                std::string  sParam2 = ""
                                                               );

        virtual                                     ~StoryEvent();

        //! Get event type, one of EventType enums.
        unsigned int                                getType() const { return _eventType; }

        //! Get event filter, one of EventFilter enums.
        unsigned int                                getFilter() const { return _filter; }

        //! Get source type, e.g. a player.
        unsigned int                                getSourceType() const { return _sourceType; }

        //! Get source ID. This specifies an instance of a source type, e.g. player Foo.
        unsigned int                                getSourceID() const { return _sourceID; }

        //! Get target ID. This specifies a target type.
        unsigned int                                getTargetType() const { return _targetType; }

        //! Get target ID. This specifies an instance of a target type.
        unsigned int                                getTargetID() const { return _targetID; }

        //! Get generic event parameters.
        float                                       getFParam1() const { return _fParam1; }
        float                                       getFParam2() const { return _fParam2; }
        const std::string&                          getSParam1() const { return _sParam1; }
        const std::string&                          getSParam2() const { return _sParam2; }

    protected:

        //! Source type triggering the event ( e.g. player )
        unsigned int                                _sourceType;

        //! Event's source instance, this specefies an instance of a source type ( ( e.g. player ID )
        unsigned int                                _sourceID;

        //! Event target, the event target specifies the target type.
        unsigned int                                _targetType;

        //! Event's target instance, this specefies an instance of a target type
        unsigned int                                _targetID;

        //! Event type
        unsigned int                                _eventType;

        //! Event filter
        unsigned int                                _filter;

        //! Event's generic parameters
        float                                       _fParam1;
        float                                       _fParam2;
        std::string                                 _sParam1;
        std::string                                 _sParam2;
};

}

#endif // _VRC_STORYEVENT_H_
