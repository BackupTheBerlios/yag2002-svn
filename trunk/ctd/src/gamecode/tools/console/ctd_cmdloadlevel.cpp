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
 # console command for loading a level
 #
 #   date of creation:  14/06/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_basecmd.h"
#include "ctd_cmdloadlevel.h"

using namespace std;

namespace CTD
{

//! Implement and register the command
CTD_IMPL_CONSOLE_CMD( CmdLoadLevel );


CmdLoadLevel::CmdLoadLevel() :
 BaseConsoleCommand( CMD_NAME_LOADLEVEL )
{
    setUsage( CMD_USAGE_LOADLEVEL );
}

CmdLoadLevel::~CmdLoadLevel()
{
}

const std::string& CmdLoadLevel::execute( const std::vector< std::string >& argmuments )
{
    _cmdResult = "";

    if ( !argmuments.size() )
    {
        _cmdResult = getUsage();
        return _cmdResult;
    }

    std::string levelfile;
    bool        finalize = true;
    if ( argmuments[ 0 ] == "-nf" )
    {
        finalize = false;
        if ( argmuments.size() < 2 )
        {
            _cmdResult = getUsage();
            return _cmdResult;
        }

        levelfile = argmuments[ 1 ];
    }
    else
    {
        levelfile = argmuments[ 0 ];
    }    
    
    _cmdResult = "loading level ...";
    LevelManager::get()->loadLevel( levelfile );

    if ( finalize )
    {
        _cmdResult += "finalizing loading ...";
        LevelManager::get()->finalizeLoading();
    }
    _cmdResult += "loading completed";

    return _cmdResult;
}

} // namespace CTD
