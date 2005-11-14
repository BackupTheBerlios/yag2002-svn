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
 # console command for getting an entity list
 #
 #   date of creation:  06/14/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_basecmd.h"
#include "ctd_cmdentitylist.h"

using namespace std;

namespace CTD
{

//! Implement and register the command
CTD_IMPL_CONSOLE_CMD( CmdEntityList );


CmdEntityList::CmdEntityList() :
 BaseConsoleCommand( CMD_NAME_ENTITYLIST )
{
    setUsage( CMD_USAGE_ENTITYLIST );
}

CmdEntityList::~CmdEntityList()
{
}

const std::string& CmdEntityList::execute( const std::vector< std::string >& arguments )
{
    _cmdResult  = "";

    string info;
    std::string flags;
    std::vector< BaseEntity* > entities;
    EntityManager::get()->getAllEntities( entities );
    std::vector< BaseEntity* >::iterator p_beg = entities.begin(), p_end = entities.end();
    for ( ; p_beg != p_end; p_beg++ )
    {
        flags = "----";
        if ( ( *p_beg )->isPersistent() ) flags[ 0 ] = 'P';
        if ( ( *p_beg )->isTransformable() ) flags[ 1 ] = 'T';
        if ( EntityManager::get()->isRegisteredUpdate( *p_beg ) ) flags[ 2 ] = 'U';
        if ( EntityManager::get()->isRegisteredNotification( *p_beg ) ) flags[ 3 ] = 'N';

        info += ( *p_beg )->getInstanceName() + "(" + ( *p_beg )->getTypeName() + ", " + flags + ")\n";
    }
    _cmdResult += info + "\n";
    return _cmdResult;
}

} // namespace CTD
