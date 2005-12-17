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

#ifndef _VRC_BASECMD_H_
#define _VRC_BASECMD_H_

#include <vrc_main.h>

namespace vrc
{
//! Helper macro for implementing and registering a command
#define VRC_IMPL_CONSOLE_CMD( cmd )   std::auto_ptr< cmd > cmd##_impl( new cmd );

class ConsoleCommandRegistry;

//! Base class for all console command classes
class BaseConsoleCommand
{
    public:
                                                    BaseConsoleCommand( const std::string& cmdname );

        virtual                                     ~BaseConsoleCommand();

        //! Get the command name
        inline const std::string&                   getCmdName() const;

        //! Execute the command
        virtual const std::string&                  execute( const std::vector< std::string >& arguments ) = 0;

        //! Get command help and usage text.
        inline const std::string&                   getUsage() const;

    protected:

        inline void                                 setUsage( const std::string& usage );

        std::string                                 _cmdName;

        std::string                                 _usage;

        std::string                                 _cmdResult;

    friend class ConsoleCommandRegistry;
};

inline const std::string& BaseConsoleCommand::getCmdName() const
{
    return _cmdName;
}

inline void BaseConsoleCommand::setUsage( const std::string& usage ) 
{
    _usage = usage;
}

inline const std::string& BaseConsoleCommand::getUsage() const
{
    return _usage;
}

} // namespace vrc

#endif // _VRC_BASECMD_H_
