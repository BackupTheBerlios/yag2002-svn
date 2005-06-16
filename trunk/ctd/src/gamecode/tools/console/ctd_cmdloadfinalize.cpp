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
 # console command for finalizing level loading
 #
 #   date of creation:  14/06/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_basecmd.h"
#include "ctd_cmdloadfinalize.h"

using namespace std;

namespace CTD
{

//! Implement and register the command
CTD_IMPL_CONSOLE_CMD( CmdLoadFinalize );


CmdLoadFinalize::CmdLoadFinalize() :
 BaseConsoleCommand( CMD_NAME_LOADFINALIZE )
{
    setUsage( CMD_USAGE_LOADFINALIZE );
}

CmdLoadFinalize::~CmdLoadFinalize()
{
}

const std::string& CmdLoadFinalize::execute( const std::vector< std::string >& argmuments )
{
    LevelManager::get()->finalizeLoading();
    _cmdResult = "level finalized";

    return _cmdResult;
}

} // namespace CTD