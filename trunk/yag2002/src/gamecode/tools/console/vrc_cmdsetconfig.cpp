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
 # console command for setting a value in current configuration
 #
 #   date of creation:  14/06/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_basecmd.h"
#include "vrc_cmdsetconfig.h"

namespace vrc
{

//! Implement and register the command
CTD_IMPL_CONSOLE_CMD( CmdSetConfig );


CmdSetConfig::CmdSetConfig() :
 BaseConsoleCommand( CMD_NAME_SETCONFIG )
{
    setUsage( CMD_USAGE_SETCONFIG );
}

CmdSetConfig::~CmdSetConfig()
{
}

const std::string& CmdSetConfig::execute( const std::vector< std::string >& arguments )
{
    _cmdResult = "";
    if ( arguments.size() < 2 )
    {
        _cmdResult = CMD_USAGE_SETCONFIG;
        _cmdResult += "\n";
        return _cmdResult;
    }
    if ( !yaf3d::Configuration::get()->setSettingValueAsString( arguments[ 0 ], arguments[ 1 ] ) )
    {
        _cmdResult += "could not find requested setting name '" + arguments[ 0 ] + "'. check for right spelling!\n";
    }

    return _cmdResult;
}

} // namespace vrc
