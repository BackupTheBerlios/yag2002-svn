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
 # gui-based io handler for an in-game console
 #
 #   date of creation:  09/09/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_console.h"
#include "ctd_consoleiocin.h"
#include <conio.h>


namespace vrc
{

#define CON_WND             "_console_gui_"
#define KEY_ISSUE_CMD       SDLK_RETURN
#define KEY_AUTOCOMPLETE    SDLK_TAB

ConsoleIOCin::ConsoleIOCin( EnConsole* p_console ) : 
 ConsoleIOBase( p_console ),
 _enable( true ),
 _terminate( false )
{
    // start the io thread
    start();
}
                                        
ConsoleIOCin::~ConsoleIOCin() 
{
}

void ConsoleIOCin::shutdown() 
{
    _terminate = true;
}

void ConsoleIOCin::enable( bool en )
{
    if ( _enable == en )
        return;

    _enable = en;
}

void ConsoleIOCin::setCmdLine( const std::string& text )
{
    std::cout << text;
}

void ConsoleIOCin::output( const std::string& text )
{
    std::cout << text << std::endl;
    std::cout << ">";
}

void ConsoleIOCin::run()
{
    std::string* p_input = new std::string;
    while( !_terminate )
    {
        int key = getchar();
        if ( key == 10 ) // end of line
        {
            _p_consoleEntity->issueCmd( *p_input );
            *p_input = "";
        }
        else
        {
            *p_input += key;
        }
    }
    delete p_input;
}


} // namespace vrc
