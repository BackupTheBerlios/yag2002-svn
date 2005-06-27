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
 # exception logger for writing a crash info file when an unhandled
 #  exception in application occurs.
 #
 # this code is basing on Matt Pietrek's article in 
 # Microsoft Systems Journal, April 1997 ( MSJEXHND.CPP / .H )
 #
 #   date of creation:  06/23/2003
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _EXCEPTION_LOGGER_H_
#define _EXCEPTION_LOGGER_H_

#include "ctd_main.h"

#ifdef WIN32

#include <tchar.h>

//! MS specific exception handler for generating crash report
class MSJExceptionHandler
{
    public:
     
                                                MSJExceptionHandler( );

                                                ~MSJExceptionHandler( );
     
        void                                    setLogFileName( PTSTR logFileName );
 
     private:
 
        // entry point where control comes on an unhandled exception
        static LONG WINAPI                      unhandledExceptionFilter( PEXCEPTION_POINTERS p_exceptionInfo );
     
        // where report info is extracted and generated 
        static void                             generateExceptionReport( PEXCEPTION_POINTERS p_exceptionInfo );
     
        // Helper functions
        static LPTSTR                           getExceptionString( DWORD code );

        static BOOL                             getLogicalAddress( PVOID p_addr, PTSTR p_module, DWORD len, DWORD& section, DWORD& offset );

        static void                             intelStackWalk( PCONTEXT p_context );

        static int __cdecl                      _tprintf( const TCHAR * format, ... );
     
        // Variables used by the class
        static TCHAR                            _p_logFileName[ MAX_PATH ];

        static LPTOP_LEVEL_EXCEPTION_FILTER     _p_previousFilter;
        
        static HANDLE                           _reportFile;
};

#endif // WIN32

#endif // _EXCEPTION_LOGGER_H_

