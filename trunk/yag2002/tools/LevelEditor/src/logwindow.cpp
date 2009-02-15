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

#include <vrc_main.h>
#include "logwindow.h"
#include "editorutils.h"


//! Log sink name
#define LOG_WND_SINK_NAME   "leveleditor"


IMPLEMENT_DYNAMIC_CLASS( LogWindow, wxDialog )

BEGIN_EVENT_TABLE( LogWindow, wxDialog )

END_EVENT_TABLE()


LogWindow::LogWindow() :
 std::basic_ostream< char >( &_stream ),
 _enableTimeStamp( false ),
 _p_textCtrl( NULL )
{
    _stream.setWindow( this );

    wxDialog::Create( NULL, SYMBOL_LOGWINDOW_IDNAME, SYMBOL_LOGWINDOW_TITLE, SYMBOL_LOGWINDOW_POSITION, SYMBOL_LOGWINDOW_SIZE, SYMBOL_LOGWINDOW_STYLE );

    SetIcon( BitmapResource::get()->getIcon( EDITOR_RESID_ICON_LOGWND ) );

    SetExtraStyle( wxWS_EX_BLOCK_EVENTS );
    wxBoxSizer* p_sizer = new wxBoxSizer( wxVERTICAL );
    SetSizer( p_sizer );

    _p_textCtrl = new wxTextCtrl( this, ID_TEXTCTRL, _T( "" ), wxDefaultPosition, wxSize(600, 150), wxTE_MULTILINE|wxTE_READONLY );
    p_sizer->Add( _p_textCtrl, 1, wxGROW|wxALL, 5 );

    p_sizer->SetSizeHints( this );

    // TODO: work out a better was to move the window top proper position
    Move( 0, 800 );

    // add it as sink to yaf3d log system
    yaf3d::yaf3dlog.addSink( LOG_WND_SINK_NAME, *this, yaf3d::Log::L_VERBOSE );
}

LogWindow::~LogWindow()
{
    // remove the sink
    yaf3d::yaf3dlog.removeSink( LOG_WND_SINK_NAME );
}

void LogWindow::update()
{
    if ( !_p_textCtrl )
        return;

    while ( _logMutex.trylock() )
        OpenThreads::Thread::microSleep( 10000 );

    while ( _msgBuffer.size() )
    {
        _p_textCtrl->AppendText( _msgBuffer.front() );
        _msgBuffer.pop();
    }

    _logMutex.unlock();
}

void LogWindow::addOutput( const std::string& msg )
{
    if ( !_p_textCtrl )
        return;

    while ( _logMutex.trylock() )
        OpenThreads::Thread::microSleep( 1000 );

    _msgBuffer.push( msg );

    _logMutex.unlock();
}

void LogWindow::clearOutput()
{
    _p_textCtrl->SetValue( "" );
}

std::basic_ios< char >::int_type LogWindow::ConStreamBuf::overflow( int_type c )
{
    _p_win->_logMutex.lock();

    if( !std::char_traits< char >::eq_int_type( c, std::char_traits< char >::eof() ) )
    {
        _msg += c;
        if( c == '\n' )
        {
            std::string ts;
            if ( _p_win->_enableTimeStamp )
                ts = "[" + yaf3d::getFormatedTime() + "] ";

            _p_win->_msgBuffer.push( ts + _msg );

            _msg = "";
        }
    }

    std::basic_ios< char >::int_type ret = std::char_traits< char >::not_eof( c );

    _p_win->_logMutex.unlock();

    return ret;
}
