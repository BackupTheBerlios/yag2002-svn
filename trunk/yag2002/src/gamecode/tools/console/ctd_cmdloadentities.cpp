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
 #   date of creation:  06/14/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_basecmd.h"
#include "ctd_cmdloadentities.h"

using namespace std;

namespace CTD
{

//! Implement and register the command
CTD_IMPL_CONSOLE_CMD( CmdLoadEntities );


CmdLoadEntities::CmdLoadEntities() :
 BaseConsoleCommand( CMD_NAME_LOADENTITIES )
{
    setUsage( CMD_USAGE_LOADENTITIES );
}

CmdLoadEntities::~CmdLoadEntities()
{
}

const std::string& CmdLoadEntities::execute( const std::vector< std::string >& arguments )
{
    _cmdResult = "";

    if ( !arguments.size() )
    {
        _cmdResult = getUsage();
        return _cmdResult;
    }
    std::string postfix;
    std::string file;
    bool        initentities = false;
    size_t      argsize = arguments.size();
    if ( argsize > 1 )
    {
        if ( arguments[ 0 ] == "-init" )
        {
            initentities = true;
            file = arguments[ 1 ];

            if ( argsize > 2 )
                postfix = arguments[ 2 ];
        }
        else
        {
            file = arguments[ 0 ];
            postfix = arguments[ 1 ];
        }
    }
    else
    {
        file = arguments[ 0 ];
    }

    _cmdResult = "loading entities ...\n";

    // load entities
    std::vector< BaseEntity* > entities;
    if ( !LevelManager::get()->loadEntities( file, &entities, postfix ) )
    {
        _cmdResult = "* error loading entity file";
        return _cmdResult;
    }
    _cmdResult = "loading completed\n";

    stringstream statistics;
    statistics << "number of created entities: " << entities.size();

    if ( !initentities )
    {
        _cmdResult += statistics.str();
        return _cmdResult;
    }

    _cmdResult = "setting up entities ...\n";

    // setup entities
    std::vector< BaseEntity* >::iterator pp_beg = entities.begin(), pp_end = entities.end();
    for ( ; pp_beg != pp_end; pp_beg++ )
    {
        _cmdResult += ( *pp_beg )->getInstanceName() + "(" + ( *pp_beg )->getTypeName() + ")   ";
        ( *pp_beg )->initialize();
    }

    pp_beg = entities.begin();
    for ( ; pp_beg != pp_end; pp_beg++ )
        ( *pp_beg )->postInitialize();

    _cmdResult += "setup completed";
    _cmdResult += statistics.str();

    return _cmdResult;
}

} // namespace CTD
