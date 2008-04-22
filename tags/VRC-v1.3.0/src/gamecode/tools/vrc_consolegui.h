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
 # Class used for a console IO, it can be also used as a log sink.
 #
 #   date of creation:  01/12/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/


#ifndef _VRC_CONSOLEGUI_H_
#define _VRC_CONSOLEGUI_H_

#include <vrc_main.h>

namespace vrc
{

//! Console GUI class
//! An object of this class or derived classes can be handles as an output stream and be set as sink in log system.
class ConsoleGUI : public std::basic_ostream< char >
{
    public:

                                                    ConsoleGUI();

        virtual                                     ~ConsoleGUI();

        //! Initialize the console window, position and dimensions are realitve to main window. Let p_parentwindow = NULL for taking the main app window as parent.
        bool                                        initialize( 
                                                                const std::string&  title,
                                                                float               x = 0.1f,
                                                                float               y = 0.1f,
                                                                float               width = 0.3f,
                                                                float               height = 0.2f,
                                                                bool                hasinput = true,
                                                                bool                enabletimestamp = true,
                                                                unsigned int        lineBufferSize = 100,
                                                                CEGUI::Window*      p_parentwindow = NULL
                                                               );

        //! Show/hide the editor
        void                                        show( bool en );


        //! Enable/disable outputting timestamp for every output
        void                                        enableTimeStamp( bool en );

        //! Add message to output
        void                                        addOutput( const std::string& msg );

        //! Clear the output window
        void                                        clearOutput();

    protected:

        //! Callback for closing the frame
        bool                                        onClickedClose( const CEGUI::EventArgs& arg );

        //! Callback for getting cmd line
        bool                                        onInputTextChanged( const CEGUI::EventArgs& arg );

        //! Override this method to get input from console
        virtual void                                processInput( const std::string& cmd ) {}

        //! GUI window object
        CEGUI::FrameWindow*                         _p_wnd;

        //! Input area
        CEGUI::Editbox*                             _p_inputWindow;

        //! Output area
        CEGUI::MultiLineEditbox*                    _p_outputWindow;

        //! Enable outputting timestamp for every output
        bool                                        _enableTimeStamp;

        //! Max lines hold in output buffer
        unsigned int                                _lineBufferSize;

        //! Current count of lines in buffer
        unsigned int                                _currLine;

        //! Stream buffer class used for output streaming
        class ConStreamBuf : public std::basic_streambuf< char >
        {
            public:

                                                         ConStreamBuf() {}

                virtual                                  ~ConStreamBuf() {}

                void                                     setConsole( ConsoleGUI* p_con ) { _p_con = p_con; }

            protected:

                virtual std::basic_ios< char >::int_type overflow( int_type c );

                ConsoleGUI*                              _p_con;

                std::string                              _msg;

        }                                           _stream;

};

}

#endif // _VRC_CONSOLEGUI_H_
