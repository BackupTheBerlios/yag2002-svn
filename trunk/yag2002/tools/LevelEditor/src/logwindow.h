/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2009, A. Botorabi
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
 # editor's log winow
 #
 #   date of creation:  01/25/2009
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _LOGWINDOW__H_
#define _LOGWINDOW__H_

#include <vrc_main.h>
#include "guibase.h"


#define SYMBOL_LOGWINDOW_STYLE      wxCAPTION|wxRESIZE_BORDER|wxDIALOG_NO_PARENT /*|wxSYSTEM_MENU|wxCLOSE_BOX*/
#define SYMBOL_LOGWINDOW_TITLE      _("Log")
#define SYMBOL_LOGWINDOW_IDNAME     ID_LOG
#define SYMBOL_LOGWINDOW_SIZE       wxSize(400, 300)
#define SYMBOL_LOGWINDOW_POSITION   wxDefaultPosition


class MainFrame;

//! Log window class
class LogWindow : public std::basic_ostream< char >, public wxDialog
{
    DECLARE_DYNAMIC_CLASS( LogWindow )
    DECLARE_EVENT_TABLE()

    public:

                                                    LogWindow();

        virtual                                     ~LogWindow();

        //! Enable/disable outputting timestamp for every output
        void                                        enableTimeStamp( bool en );

        //! Add message to output
        void                                        addOutput( const std::string& msg );

        //! Clear the output window
        void                                        clearOutput();

    protected:

        //! Override this method to get input from console
        virtual void                                processInput( const std::string& /*cmd*/ ) {}

        //! Stream buffer class used for output streaming
        class ConStreamBuf : public std::basic_streambuf< char >
        {
            public:

                                                         ConStreamBuf() {}

                virtual                                  ~ConStreamBuf() {}

                void                                     setWindow( LogWindow* p_win ) { _p_win = p_win; }

            protected:

                virtual std::basic_ios< char >::int_type overflow( int_type c );

                LogWindow*                               _p_win;

                std::string                              _msg;

        }                                           _stream;

        //! Enable outputting timestamp for every output
        bool                                        _enableTimeStamp;

        wxTextCtrl*                                 _p_textCtrl;

        //! Control identifiers
        enum
        {
            ID_LOG       = 10200,
            ID_PANEL_LOG = 10201,
            ID_TEXTCTRL  = 10202
        };
};

#endif // _LOGWINDOW__H_
