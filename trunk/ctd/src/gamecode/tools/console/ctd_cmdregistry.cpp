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
 # all console commands are registered using this registry instance
 #
 #   date of creation:  13/06/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_cmdregistry.h"
#include "ctd_basecmd.h"

using namespace std;

namespace CTD
{

CTD_SINGLETON_IMPL( ConsoleCommandRegistry );

ConsoleCommandRegistry::ConsoleCommandRegistry()
{
}

ConsoleCommandRegistry::~ConsoleCommandRegistry()
{
    // we don't need to delete all registered command objects as they are created using std::auto_ptr
    // just clean up the lookup table
    _cmdRegistry.clear();
}

bool ConsoleCommandRegistry::registerCmd( BaseConsoleCommand* p_cmd )
{
    if ( _cmdRegistry[ p_cmd->getCmdName() ] )
        return false;

    _cmdRegistry[ p_cmd->getCmdName() ] = p_cmd;

    return true;
}

BaseConsoleCommand* ConsoleCommandRegistry::getCmd( const std::string& cmdname )
{
    return _cmdRegistry[ cmdname ];
}

} // namespace CTD
