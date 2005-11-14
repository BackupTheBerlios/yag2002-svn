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
 # console command for setting an entity attribute
 #
 #   date of creation:  06/16/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_basecmd.h"
#include "ctd_cmdentityattrset.h"

using namespace std;

namespace CTD
{

//! Implement and register the command
CTD_IMPL_CONSOLE_CMD( CmdEntityAttributeSet );


CmdEntityAttributeSet::CmdEntityAttributeSet() :
 BaseConsoleCommand( CMD_NAME_ENTITYATTRSET )
{
    setUsage( CMD_USAGE_ENTITYATTRSET );
}

CmdEntityAttributeSet::~CmdEntityAttributeSet()
{
}

const std::string& CmdEntityAttributeSet::execute( const std::vector< std::string >& arguments )
{
    _cmdResult = "";
    if ( arguments.size() < 3 )
    {
        _cmdResult = getUsage();
        return _cmdResult;
    }

    BaseEntity* p_entity = EntityManager::get()->findInstance( arguments[ 0 ] );
    if ( ! p_entity )
    {
        _cmdResult = "* entity instance '" + arguments[ 0 ] + "' does not exist";
        return _cmdResult;
    }

    // set attribute value using value string
    if ( !p_entity->getAttributeManager().setAttributeValueByString( arguments[ 1 ], arguments[ 2 ] ) )
    {
        _cmdResult = "* error setting attribute value, check the value format!";
    }
    else
    {
        // now send out a notification to the entity this way letting it know that we changed an attribute value
        //  the entity can decide itself if an appropriate action is necessary in this case
        EntityNotification notification( CTD_NOTIFY_ENTITY_ATTRIBUTE_CHANGED );
        EntityManager::get()->sendNotification( notification, p_entity );
    }

    return _cmdResult;
}

} // namespace CTD
