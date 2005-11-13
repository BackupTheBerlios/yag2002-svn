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
 # console command for showing the current configuration
 #
 #   date of creation:  14/06/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_basecmd.h"
#include "ctd_cmdgetconfig.h"

namespace vrc
{

//! Implement and register the command
CTD_IMPL_CONSOLE_CMD( CmdGetConfig );


CmdGetConfig::CmdGetConfig() :
 BaseConsoleCommand( CMD_NAME_GETCONFIG )
{
    setUsage( CMD_USAGE_GETCONFIG );
}

CmdGetConfig::~CmdGetConfig()
{
}

const std::string& CmdGetConfig::execute( const std::vector< std::string >& arguments )
{
    _cmdResult =  "setting file: " + std::string( CTD_GAMESETTING_FILENAME ) + "\n";
    _cmdResult += "-----------------\n";

    std::vector< std::pair< std::string, std::string > > settings;
    yaf3d::Configuration::get()->getConfigurationAsString( settings );
    std::vector< std::pair< std::string, std::string > >::iterator p_beg = settings.begin(), p_end = settings.end();
    for ( ; p_beg != p_end; p_beg++ )
    {

        _cmdResult += p_beg->first + " [" + p_beg->second + "]\n";
    }

    return _cmdResult;
}

} // namespace vrc
