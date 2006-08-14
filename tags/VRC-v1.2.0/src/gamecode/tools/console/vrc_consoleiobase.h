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
 # base class of all kinds of console io handlers
 #
 #   date of creation:  09/09/2005
 #
 #   author:            ali botorabi (boto)
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_CONSOLE_IO_BASE_H_
#define _VRC_CONSOLE_IO_BASE_H_

#include <vrc_main.h>

namespace vrc
{

class EnConsole;

//! Base class for console IO handlers
class ConsoleIOBase
{
    public:

        explicit                                    ConsoleIOBase( EnConsole* p_console ) :
                                                     _p_consoleEntity( p_console )
                                                    {
                                                    }

        virtual                                     ~ConsoleIOBase() {}

        //! Enable / disable io
        virtual void                                enable( bool /*en*/ ) {};

        //! Set cmd line text
        virtual void                                setCmdLine( const std::string& text ) = 0;

        //! Output a text
        virtual void                                output( const std::string& text ) = 0;

        //! Shutdown the input handler
        virtual void                                shutdown() = 0;

    protected:

        EnConsole*                                  _p_consoleEntity;
};

} // namespace vrc

#endif // _VRC_CONSOLE_IO_BASE_H_
