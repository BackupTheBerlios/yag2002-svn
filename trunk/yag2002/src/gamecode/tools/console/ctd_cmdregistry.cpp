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
 #   date of creation:  06/13/2005
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
    if ( _cmdRegistry.find( p_cmd->getCmdName() ) != _cmdRegistry.end() )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "ConsoleCommandRegistry: the command '" << p_cmd->getCmdName() << "' is already registered, ignoring it!" << endl;
        return false;
    }

    _cmdRegistry[ p_cmd->getCmdName() ] = p_cmd;

    return true;
}

BaseConsoleCommand* ConsoleCommandRegistry::getCmd( const std::string& cmdname )
{
    std::map< std::string, BaseConsoleCommand* >::iterator p_find = _cmdRegistry.find( cmdname );
    BaseConsoleCommand* p_cmd = ( p_find != _cmdRegistry.end() ) ? p_find->second : NULL;
    return p_cmd;
}

unsigned int ConsoleCommandRegistry::getAllCmds( std::vector< BaseConsoleCommand* >& commands )
{
    unsigned int cnt = 0;
    std::map< std::string, BaseConsoleCommand* >::iterator p_beg = _cmdRegistry.begin(), p_end = _cmdRegistry.end();
    for ( ; p_beg != p_end; p_beg++ )
    {
        commands.push_back( p_beg->second );
        cnt++;
    }

    return cnt;
}

unsigned int ConsoleCommandRegistry::getCmdCandidates( const std::string& text, std::vector< std::string >& candidates )
{
    unsigned int matchfound = 0;
    std::map< std::string, BaseConsoleCommand* >::iterator p_beg = _cmdRegistry.begin(), p_end = _cmdRegistry.end();
    for ( ; p_beg != p_end; p_beg++ )
    {        
        if ( p_beg->first.find( text ) == 0 )
        {
            candidates.push_back( p_beg->first );
            matchfound++;
        }
    }

    return matchfound;
}

} // namespace CTD
