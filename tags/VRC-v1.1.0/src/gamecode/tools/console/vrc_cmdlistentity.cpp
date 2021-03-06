/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2006, A. Botorabi
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
 # console command for getting an entity list
 #
 #   date of creation:  14/06/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_basecmd.h"
#include "vrc_cmdlistentity.h"

namespace vrc
{

//! Implement and register the command
VRC_IMPL_CONSOLE_CMD( CmdListEntity );


CmdListEntity::CmdListEntity() :
 BaseConsoleCommand( CMD_NAME_LISTENTITY )
{
    setUsage( CMD_USAGE_LISTENTITY );
}

CmdListEntity::~CmdListEntity()
{
}

const std::string& CmdListEntity::execute( const std::vector< std::string >& arguments )
{
    _cmdResult = "list of currently existing entities:\n";

    std::string info;
    std::vector< yaf3d::BaseEntity* > entities;
    yaf3d::EntityManager::get()->getAllEntities( entities );
    std::vector< yaf3d::BaseEntity* >::iterator p_beg = entities.begin(), p_end = entities.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        info += ( *p_beg )->getInstanceName() + "(" + ( *p_beg )->getTypeName() + ")    ";
    }
    _cmdResult = info + "\n";
    return _cmdResult;
}

} // namespace vrc
