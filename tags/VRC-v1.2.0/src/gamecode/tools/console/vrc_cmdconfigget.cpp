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
 # console command for showing the current configuration
 #
 #   date of creation:  14/06/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_basecmd.h"
#include "vrc_cmdconfigget.h"

namespace vrc
{

//! Implement and register the command
VRC_IMPL_CONSOLE_CMD( CmdConfigGet )


CmdConfigGet::CmdConfigGet() :
 BaseConsoleCommand( CMD_NAME_CONFIGGET )
{
    setUsage( CMD_USAGE_CONFIGGET );
}

CmdConfigGet::~CmdConfigGet()
{
}

const std::string& CmdConfigGet::execute( const std::vector< std::string >& arguments )
{
    if ( arguments.size() != 1 )
    {
        _cmdResult += getUsage();
        return _cmdResult;
    }

    std::vector< std::pair< std::string, std::string > > settings;
    yaf3d::Configuration::get()->getConfigurationAsString( settings );
    std::vector< std::pair< std::string, std::string > >::iterator p_beg = settings.begin(), p_end = settings.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        if ( p_beg->first == arguments[ 0 ] )
        {
            _cmdResult += p_beg->first + " [" + p_beg->second + "]\n";
            break;
        }
    }
    if ( p_beg == p_end )
        _cmdResult = "configuration setting '" + arguments[ 0 ] +  "' does not exist";

    return _cmdResult;
}

} // namespace vrc
