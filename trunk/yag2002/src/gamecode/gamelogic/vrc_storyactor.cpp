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

#include <vrc_main.h>
#include "vrc_storyactor.h"

namespace vrc
{

//! Actor ID set used for creating unique IDs
static std::set< int >  s_actorIDs;

//! Implementation of object type lookup
std::map< unsigned int, std::string >*  ActorRegistry::_p_actorTypes = NULL;
unsigned int                            ActorRegistry::_refCnt = 0;


BaseStoryActor::BaseStoryActor( unsigned int type ) :
 _actorType( type ),
 _actorID( 0 )
{
    // create pseudo-random number for actor ID
    int number = 0;
    do
    {
        number = rand();
    }
    while ( s_actorIDs.find(  number ) != s_actorIDs.end() );
    s_actorIDs.insert( number );
    _actorID = number;
}

BaseStoryActor::~BaseStoryActor()
{
}


//! Implementation of object registry
void ActorRegistry::registerEntityType( unsigned int ID, const std::string& entitytype )
{
    std::map< unsigned int, std::string >::iterator p_end = ActorRegistry::_p_actorTypes->end(), p_type;
    p_type = ActorRegistry::_p_actorTypes->find( ID );

    if ( p_type != p_end )
    {
        log_error << "ActorRegistry: type with ID " << ID << " is already registered!" << std::endl;
        return;
    }

    // register type
    ( *_p_actorTypes )[ ID ] = entitytype;
}

std::string ActorRegistry::getEntityType( unsigned int ID )
{
    std::map< unsigned int, std::string >::iterator p_end = ActorRegistry::_p_actorTypes->end(), p_type;
    p_type = ActorRegistry::_p_actorTypes->find( ID );
    if ( p_type == p_end )
    {
        log_error << "ActorRegistry: invalid object ID: " << ID << std::endl;
        return std::string( "" );
    }

    return p_type->second;
}

} // namespace vrc
