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

#include <vrc_main.h>
#include "vrc_basecmd.h"
#include "vrc_cmdloadentities.h"

namespace vrc
{

//! Implement and register the command
VRC_IMPL_CONSOLE_CMD( CmdLoadEntities );


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
    std::vector< yaf3d::BaseEntity* > entities;
    if ( !yaf3d::LevelManager::get()->loadEntities( file, &entities, postfix ) )
    {
        _cmdResult = "* error loading entity file";
        return _cmdResult;
    }
    _cmdResult = "loading completed\n";

    std::stringstream statistics;
    statistics << "number of created entities: " << entities.size();

    if ( !initentities )
    {
        _cmdResult += statistics.str();
        return _cmdResult;
    }

    _cmdResult = "setting up entities ...\n";

    // setup entities
    std::vector< yaf3d::BaseEntity* >::iterator p_beg = entities.begin(), p_end = entities.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        _cmdResult += ( *p_beg )->getInstanceName() + "(" + ( *p_beg )->getTypeName() + ")   ";
        ( *p_beg )->initialize();
    }

    p_beg = entities.begin();
    for ( ; p_beg != p_end; ++p_beg )
        ( *p_beg )->postInitialize();

    _cmdResult += "setup completed";
    _cmdResult += statistics.str();

    return _cmdResult;
}

} // namespace vrc
