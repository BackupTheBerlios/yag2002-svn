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
 #   date of creation:  13/06/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_BASECMD_H_
#define _CTD_BASECMD_H_

#include <ctd_main.h>

namespace CTD
{
//! Helper macro for implementing and registering a command
#define CTD_IMPL_CONSOLE_CMD( cmd )   std::auto_ptr< cmd > cmd##_impl( new cmd );

class ConsoleCommandRegistry;

//! Base class for all console command classes
class BaseConsoleCommand
{
    public:
                                                    BaseConsoleCommand( const std::string& cmdname );

        virtual                                     ~BaseConsoleCommand();

        //! Get the command name
        inline const std::string&                   getCmdName();

        //! Execute the command
        virtual const std::string&                  execute( const std::vector< std::string >& argmuments ) = 0;

    protected:

        inline void                                 setUsage( const std::string& usage );

        inline const std::string&                   getUsage();

        std::string                                 _cmdName;

        std::string                                 _usage;

        std::string                                 _cmdResult;

    friend class ConsoleCommandRegistry;
};

inline const std::string& BaseConsoleCommand::getCmdName()
{
    return _cmdName;
}

inline void BaseConsoleCommand::setUsage( const std::string& usage )
{
    _usage = usage;
}

inline const std::string& BaseConsoleCommand::getUsage()
{
    return _usage;
}

} // namespace CTD

#endif // _CTD_BASECMD_H_
