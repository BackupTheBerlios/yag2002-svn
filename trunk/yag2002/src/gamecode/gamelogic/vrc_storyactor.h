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
 # base class for every kind of actor involved in stories
 #
 #   date of creation:  15/01/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _VRC_STORYACTOR_H_
#define _VRC_STORYACTOR_H_

#include <vrc_main.h>

namespace vrc
{

//! Base class for a story actor
class BaseStoryActor
{
    public:

        //! Create a story actor of given type. If the type will be known later then pass a zero and use 'setActorType' later.
        explicit                                    BaseStoryActor( unsigned int type );

        virtual                                     ~BaseStoryActor();

        //! Get the unique actor type.
        unsigned int                                getActorType() const { return _actorType; }

        //! Get the unique actor ID.
        unsigned int                                getActorID() const { return _actorID; }

    protected:

        //! Set the unique actor type, used when creating remove actors on clients.
        void                                        setActorType( unsigned int type ) { _actorType = type; }

        //! Set the unique actor ID, used when creating remove actors on clients.
        void                                        setActorID( unsigned int id ) { _actorID = id; }

        //! Unique actor type
        unsigned int                                _actorType;

        //! Unique actor ID
        unsigned int                                _actorID;
};

//! Class for registration of actor and entity type
/**
    Every actor type must register its 'type' and must be also derived from BaseEntity.
*/
class ActorRegistry
{
    public:

        //! 'type' is a unique actor type and entitytype is a unique entity type
                                                    ActorRegistry( unsigned int type, const std::string& entitytype )
                                                    {
                                                        ++_refCnt;

                                                        if ( !_p_actorTypes )
                                                            _p_actorTypes = new std::map< unsigned int, std::string >;

                                                        ( *_p_actorTypes )[ type ] = entitytype;
                                                    }

        //! Destroy the registry object.
        virtual                                     ~ActorRegistry()
                                                    {
                                                        --_refCnt;
                                                        if ( !_refCnt )
                                                        {
                                                            delete _p_actorTypes;
                                                            _p_actorTypes = NULL;
                                                        }
                                                    }

        //! Register an object with given actor type.
        static void                                 registerEntityType( unsigned int type, const std::string& entitytype );

        //! Given an actor type return its entity type.
        static std::string                          getEntityType( unsigned int type );

    protected:

        //! Ref count
        static unsigned int                             _refCnt;

        //! Actor type/entity type lookup
        static std::map< unsigned int, std::string >*   _p_actorTypes;
};

//! Convenient macro for registering an actor type with given entity type. This registry is used for creating actors on clients. On server, actors are created directly on start up.
#define VRC_REGISTER_ACTOR( type, entitytype )     static std::auto_ptr< ActorRegistry > ActorRegistry_impl_auto( new ActorRegistry( ( type ), ( entitytype ) ) );


}

#endif // _VRC_STORYACTOR_H_
