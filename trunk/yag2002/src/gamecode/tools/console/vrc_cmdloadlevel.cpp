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
 # console command for loading a level
 #
 #   date of creation:  06/14/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_basecmd.h"
#include "vrc_cmdloadlevel.h"

namespace vrc
{

//! Implement and register the command
VRC_IMPL_CONSOLE_CMD( CmdLoadLevel )


CmdLoadLevel::CmdLoadLevel() :
 BaseConsoleCommand( CMD_NAME_LOADLEVEL )
{
    setUsage( CMD_USAGE_LOADLEVEL );
}

CmdLoadLevel::~CmdLoadLevel()
{
}

const std::string& CmdLoadLevel::execute( const std::vector< std::string >& arguments )
{
    _cmdResult = "";

    if ( !arguments.size() )
    {
        _cmdResult = getUsage();
        return _cmdResult;
    }

    std::string levelfile;
    bool        finalize = true;
    if ( arguments[ 0 ] == "-nf" )
    {
        finalize = false;
        if ( arguments.size() < 2 )
        {
            _cmdResult = getUsage();
            return _cmdResult;
        }

        levelfile = arguments[ 1 ];
    }
    else
    {
        levelfile = arguments[ 0 ];
    }    
    
    _cmdResult = "loading level ...\n";
    yaf3d::LevelManager::get()->loadLevel( levelfile );

    if ( finalize )
    {
        _cmdResult += "finalizing loading ...\n";
        yaf3d::LevelManager::get()->finalizeLoading();
    }
    _cmdResult += "loading completed";

    return _cmdResult;
}

} // namespace vrc
