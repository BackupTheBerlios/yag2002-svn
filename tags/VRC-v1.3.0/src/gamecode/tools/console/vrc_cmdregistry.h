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
 # all console commands are registered using this registry instance
 #
 #   date of creation:  06/13/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _VRC_CMDREGISTRY_H_
#define _VRC_CMDREGISTRY_H_

#include <vrc_main.h>

namespace vrc
{

class EnConsole;
class BaseConsoleCommand;

class ConsoleCommandRegistry : public yaf3d::Singleton< ConsoleCommandRegistry >
{
    public:

        //! Register a new console command object. Returns false if the command is already registered.
        bool                                        registerCmd( BaseConsoleCommand* p_cmd );

        //! Get the command object given its name. Returns NULL if the command name could not be found.
        BaseConsoleCommand*                         getCmd( const std::string& cmdname );

        //! Given a text all possible matching commands are stored in 'candidates'. Returns the count of found matches.
        unsigned int                                getCmdCandidates( const std::string& text, std::vector< std::string >& candidates );

        //! Get all registered command. Returns the total count of commands ( is the same as the vector size of 'commands' ).
        unsigned int                                getAllCmds( std::vector< BaseConsoleCommand* >& commands );

    protected:

                                                    ConsoleCommandRegistry();

        virtual                                     ~ConsoleCommandRegistry();

        //! Lookup table for all registered commands < cmd name, cmd object >
        std::map< std::string, BaseConsoleCommand* > _cmdRegistry;

    friend class yaf3d::Singleton< ConsoleCommandRegistry >;
    friend class EnConsole;
};


} // namespace vrc

#endif // _VRC_CMDREGISTRY_H_
