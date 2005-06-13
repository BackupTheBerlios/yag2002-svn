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
 # all console commands are registered using this registry instance
 #
 #   date of creation:  13/06/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_CMDREGISTRY_H_
#define _CTD_CMDREGISTRY_H_

#include <ctd_main.h>

namespace CTD
{

class EnConsole;
class BaseConsoleCommand;

class ConsoleCommandRegistry : public Singleton< ConsoleCommandRegistry >
{
    public:

        //! Register a new console command object. Returns false if the command is already registered.
        bool                                        registerCmd( BaseConsoleCommand* p_cmd );

        //! Get the command object given its name. Returns NULL if the command name could not be found.
        BaseConsoleCommand*                         getCmd( const std::string& cmdname );

    protected:

                                                    ConsoleCommandRegistry();

        virtual                                     ~ConsoleCommandRegistry();

        //! Lookup table for all registered commands < cmd name, cmd object >
        std::map< std::string, BaseConsoleCommand* > _cmdRegistry;

    friend class Singleton< ConsoleCommandRegistry >;
    friend class EnConsole;
};


} // namespace CTD

#endif // _CTD_CMDREGISTRY_H_
