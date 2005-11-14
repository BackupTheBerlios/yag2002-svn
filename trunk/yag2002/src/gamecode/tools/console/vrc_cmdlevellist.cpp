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
 # console command for listing all available levels
 #
 #   date of creation:  06/17/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_basecmd.h"
#include "vrc_cmdlevellist.h"

namespace vrc
{

//! Implement and register the command
CTD_IMPL_CONSOLE_CMD( CmdLevelList );


CmdLevelList::CmdLevelList() :
 BaseConsoleCommand( CMD_NAME_LEVELLIST )
{
    setUsage( CMD_USAGE_LEVELLIST );
}

CmdLevelList::~CmdLevelList()
{
}

const std::string& CmdLevelList::execute( const std::vector< std::string >& arguments )
{
    _cmdResult = "";

    // show the content of the folders for server, client and standalone modes
    std::vector< std::string > dirs;
    dirs.push_back( YAF3D_LEVEL_SALONE_DIR );
    dirs.push_back( YAF3D_LEVEL_SERVER_DIR );
    dirs.push_back( YAF3D_LEVEL_CLIENT_DIR );
    std::vector< std::string > filelisting;
    std::vector< std::string >::iterator p_dir = dirs.begin(), p_dirend = dirs.end();
    for ( ; p_dir != p_dirend; p_dir++ )
    {
        filelisting.clear();
        _cmdResult += "# levels availabe in '" + *p_dir + "':\n";
        // get a directory listing
        yaf3d::getDirectoryListing( filelisting, yaf3d::Application::get()->getMediaPath() + *p_dir, "lvl" );
        std::vector< std::string >::iterator p_beg = filelisting.begin(), p_end = filelisting.end();
        for ( ; p_beg != p_end; p_beg++ )
            _cmdResult += *p_beg + "\n";
    }

    return _cmdResult;
}

} // namespace vrc
