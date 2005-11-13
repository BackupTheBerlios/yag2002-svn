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
 # console command for getting an entity attribute list
 #
 #   date of creation:  06/16/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_basecmd.h"
#include "vrc_cmdentityattrlist.h"

namespace vrc
{

//! Implement and register the command
CTD_IMPL_CONSOLE_CMD( CmdEntityAttributeList );


CmdEntityAttributeList::CmdEntityAttributeList() :
 BaseConsoleCommand( CMD_NAME_ENTITYATTRLIST )
{
    setUsage( CMD_USAGE_ENTITYATTRLIST );
}

CmdEntityAttributeList::~CmdEntityAttributeList()
{
}

const std::string& CmdEntityAttributeList::execute( const std::vector< std::string >& arguments )
{
    if ( arguments.size() < 1 )
    {
        _cmdResult = getUsage();
        return _cmdResult;
    }

    _cmdResult = "list of entity attributes:\n";

    yaf3d::BaseEntity* p_entity = yaf3d::EntityManager::get()->findInstance( arguments[ 0 ] );
    if ( ! p_entity )
    {
        _cmdResult = "* entity instance '" + arguments[ 0 ] + "' does not exist";
        return _cmdResult;
    }

    std::vector< std::pair< std::string, std::string > > attributes;
    p_entity->getAttributeManager().getAttributesAsString( attributes );

    std::string info;
    std::vector< std::pair< std::string, std::string > >::iterator p_beg = attributes.begin(), p_end = attributes.end();
    for ( ; p_beg != p_end; p_beg++ )
    {
        info += p_beg->first + "  [  " + p_beg->second + "  ] \n";
    }
    _cmdResult = info;

    return _cmdResult;
}

} // namespace vrc
