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

//! Story event class used for sending events from/to stories
class StoryEvent
{
    public:

        //! Event type
        enum EventType
        {
            //! Invalid event type
            eTypeUnknown    = 0x0000,
            //! Used by event senders
            eTypePickItem   = 0x1000,
            eTypeTalk       = 0x2000
        };

        //! Event filter type defining the propagation of the event to story or actor receivers
        enum EventReceiverFilter
        {
            //! Invalid event filter
            eFilterUnknown         = 0x0000,
            
            //! Event is sent to story book (stock stories), it can cause beginning of a new story. This flag can be combined with the flags below.
            eFilterStoryBook       = 0x0010,

            //! Event is sent only to receivers (stories) which have an ower ID = event source ID
            eFilterStoryPrivate    = 0x0100,
            //! Event is sent to all receivers
            eFilterStoryPublic     = 0x0200,
            //! Event is sent only to receivers (actors) which have an ower ID = event source ID
            eFilterActorPrivate    = 0x0400,
            //! Event is sent to all actor receivers
            eFilterActorPublic     = 0x0800
        };

        //! Create an event with given source, target, type and propagation filter. Several generic event parameters can also be defined.
                                                    StoryEvent(
                                                                unsigned int eventType,
                                                                unsigned int sourceType,
                                                                unsigned int sourceID,
                                                                unsigned int targetType,
                                                                unsigned int targetID,
                                                                unsigned int filter,

                                                                int          networkID,

                                                                unsigned int uiParam1 = 0,
                                                                unsigned int uiParam2 = 0,
                                                                float        fParam1 = 0.0f,
                                                                float        fParam2 = 0.0f,
                                                                std::string  sParam = ""
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

        //! Get the network ID of sender. This can be used to identify the event sending client.
        int                                         getNetworkID() const { return _networkID; }

        //! Get generic event parameters.
        unsigned int                                getUIParam1() const { return _uiParam1; }
        unsigned int                                getUIParam2() const { return _uiParam2; }
        float                                       getFParam1()  const { return _fParam1; }
        float                                       getFParam2()  const { return _fParam2; }
        const std::string&                          getSParam()  const { return _sParam; }

    protected:

        //! Source type triggering the event ( e.g. player )
        unsigned int                                _sourceType;

        //! Event's source instance, this specefies an instance of a source type ( ( e.g. player ID )
        unsigned int                                _sourceID;

        //! Event target, the event target specifies the target type.
        unsigned int                                _targetType;

        //! Event's target instance, this specefies an instance of a target type
        unsigned int                                _targetID;

        //! Event sender's network session ID
        int                                         _networkID;

        //! Event type
        unsigned int                                _eventType;

        //! Event filter
        unsigned int                                _filter;

        //! Event's generic parameters
        unsigned int                                _uiParam1;
        unsigned int                                _uiParam2;
        float                                       _fParam1;
        float                                       _fParam2;
        std::string                                 _sParam;
};


//! Base class used by actors for receiving events.
class StoryEventReceiver
{
    public:

                                                    StoryEventReceiver() {}

        virtual                                     ~StoryEventReceiver() {}

        //! Override this method in order to receive events.
        virtual void                                onReceiveEvent( const StoryEvent& event ) = 0;
};

}

#endif // _VRC_STORYEVENT_H_
