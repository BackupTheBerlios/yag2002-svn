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
 # main application entry
 #
 #   date of creation:  01/13/2003
 #
 #   author:            ali botorabi (boto)
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/


#include "vrc_main.h"
#include <application.h>

void errormsg();

int main( int argc, char *argv[] )
{
    yaf3d::Application* p_app = NULL;
    try
    {
        p_app = yaf3d::Application::get();

        if ( !p_app->initialize( argc, argv ) )
            return -1;

        p_app->run();
        p_app->shutdown();
    }
    catch ( const std::exception& e )
    {
        log_error << "unhandled std exception occured: " << e.what() << std::endl;
        errormsg();
        return -1;
    }

#ifndef WIN32
    catch ( ... )
    {
        log_error << "unhandled exception occured" << std::endl;
        errormsg();
        return -2;
    }
#endif

    return 0;
}

void errormsg()
{
    log_error << "please report this bug to project admin ( http://yag2002.sf.net )" << std::endl;
    log_error << "please send also the vrc.crashreport and all log files in media directory" << std::endl;
}

