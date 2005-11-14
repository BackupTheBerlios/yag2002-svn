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
 # base class of all console command classes
 #
 #   date of creation:  06/13/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_basecmd.h"
#include "ctd_cmdregistry.h"

using namespace std;

namespace CTD
{

BaseConsoleCommand::BaseConsoleCommand( const std::string& cmdname ) :
 _cmdName( cmdname )
{
    assert( _cmdName.length() && "console command must have a valid name!" );
    bool res = ConsoleCommandRegistry::get()->registerCmd( this );
    assert( res && "console command must have an unique name!" );
}

BaseConsoleCommand::~BaseConsoleCommand()
{
}

} // namespace CTD
