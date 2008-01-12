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


#include <vrc_main.h>
#include "vrc_consolegui.h"

namespace vrc
{

ConsoleGUI::ConsoleGUI() :
 std::basic_ostream< char >( &_stream ),
 _p_wnd( NULL ),
 _p_inputWindow( NULL ),
 _p_outputWindow( NULL ),
 _enableTimeStamp( true ),
 _lineBufferSize( 100 ),
 _currLine( 0 )
{
    _stream.setConsole( this );
}

ConsoleGUI::~ConsoleGUI()
{
    try
    {
        if ( _p_wnd )
            CEGUI::WindowManager::getSingleton().destroyWindow( _p_wnd );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "ConsoleGUI: problem destroying gui." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

bool ConsoleGUI::initialize(
                             const std::string&  title,
                             float               x,
                             float               y,
                             float               width,
                             float               height,
                             bool                hasinput,
                             bool                enabletimestamp,
                             unsigned int        lineBufferSize,
                             CEGUI::Window*      p_parentwindow
                           )
{
    _lineBufferSize = lineBufferSize;

    CEGUI::Window* p_parent = p_parentwindow ? p_parentwindow : yaf3d::GuiManager::get()->getRootWindow();
    try
    {
        static unsigned int instance = 0;
        instance++;
        std::stringstream inst;
        inst << instance;

        float framewidth = width, frameheight = height;
        _p_wnd = static_cast< CEGUI::FrameWindow* >( CEGUI::WindowManager::getSingleton().createWindow( ( CEGUI::utf8* )"TaharezLook/FrameWindow", inst.str() + "_console_" ) );
        _p_wnd->subscribeEvent( CEGUI::FrameWindow::EventCloseClicked, CEGUI::Event::Subscriber( &vrc::ConsoleGUI::onClickedClose, this ) );
        _p_wnd->setText( title );
        _p_wnd->setPosition( CEGUI::Point( x, y ) );
        _p_wnd->setSize( CEGUI::Size( framewidth, frameheight ) );
        _p_wnd->setSizingEnabled( true );
        _p_wnd->setAlpha( 0.7f );
        _p_wnd->setMinimumSize( CEGUI::Size( 0.3f, 0.3f ) );
        _p_wnd->setFont( YAF3D_GUI_CONSOLE );
        _p_wnd->setAlwaysOnTop( true );
        _p_wnd->show();

        _p_outputWindow = static_cast< CEGUI::MultiLineEditbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/MultiLineEditbox", inst.str() + "_console_output_" ) );
        _p_outputWindow->setReadOnly( true );
        _p_outputWindow->setSize( CEGUI::Size( 0.96f, hasinput ? 0.7f : 0.93f ) );
        _p_outputWindow->setPosition( CEGUI::Relative, CEGUI::Point( 0.02f, 0.05f ) );
        _p_outputWindow->setFont( YAF3D_GUI_CONSOLE );
        _p_outputWindow->setAlpha( 0.7f );
        _p_wnd->addChildWindow( _p_outputWindow );

        if ( hasinput )
        {
            _p_inputWindow = static_cast< CEGUI::Editbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Editbox", inst.str() + "_console_input_" ) );
            _p_inputWindow->subscribeEvent( CEGUI::MultiLineEditbox::EventCharacterKey, CEGUI::Event::Subscriber( &vrc::ConsoleGUI::onInputTextChanged, this ) );
            _p_inputWindow->setSize( CEGUI::Size( 0.96f, 0.15f ) );
            _p_inputWindow->setPosition( CEGUI::Point( 0.02f, 0.8f ) );
            _p_inputWindow->setAlpha( 0.7f );
            _p_outputWindow->setFont( YAF3D_GUI_FONT8 );
            _p_inputWindow->setFont( YAF3D_GUI_CONSOLE );
            _p_wnd->addChildWindow( _p_inputWindow );
        }

        p_parent->addChildWindow( _p_wnd );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "ConsoleGUI: error creating gui" << std::endl;
        log_error << "  reason: " << e.getMessage().c_str() << std::endl;
        return false;
    }

    return true;
}

bool ConsoleGUI::onClickedClose( const CEGUI::EventArgs& arg )
{
    _p_wnd->deactivate();
    _p_wnd->hide();
    return true;
}

bool ConsoleGUI::onInputTextChanged( const CEGUI::EventArgs& arg )
{
    CEGUI::KeyEventArgs& ke = static_cast< CEGUI::KeyEventArgs& >( const_cast< CEGUI::EventArgs& >( arg ) );
    if ( ke.codepoint == SDLK_RETURN )
    {
        // skip empty lines
        if ( !_p_inputWindow->getText().length() )
            return true;

        addOutput( ">" + std::string( _p_inputWindow->getText().c_str() ) );
        processInput( _p_inputWindow->getText().c_str() );
        _p_inputWindow->setText( "" );
        _p_outputWindow->setCaratIndex( _p_outputWindow->getText().length() );
    }

    return true;
}

void ConsoleGUI::show( bool en )
{
    if ( !_p_wnd )
        return;

    if ( en )
        _p_wnd->show();
    else
        _p_wnd->hide();
}

void ConsoleGUI::enableTimeStamp( bool en )
{
    _enableTimeStamp = en;
}

void ConsoleGUI::addOutput( const std::string& msg )
{
    if ( _p_outputWindow )
    {
        std::string ts;
        
        if ( _enableTimeStamp )
            ts = "[" + yaf3d::getFormatedTime() + "] ";

        // eraze top of output window buffer for fitting in to specified max number of lines
        _currLine++;
        if ( _currLine > _lineBufferSize )
        {
            // dirty cast around the const return value, but it saves much performance ;-)
            CEGUI::String& buffer = ( CEGUI::String& )_p_outputWindow->getText();
            buffer.erase( 0, buffer.find( "\n", 0 ) + 1 );
            _currLine--;
        }

        _p_outputWindow->setText( _p_outputWindow->getText() + ts + msg );
        _p_outputWindow->setCaratIndex( _p_outputWindow->getText().length() );
    }
}

void ConsoleGUI::clearOutput()
{
    _p_outputWindow->setText( "" );
    _currLine = 0;
}

std::basic_ios< char >::int_type ConsoleGUI::ConStreamBuf::overflow( int_type c )
{
    if( !std::char_traits< char >::eq_int_type( c, std::char_traits< char >::eof() ) )
    {
        _msg += c;
        if( c == '\n' )
        {
            // add a carriage return to end of line
            _msg[ _msg.length() - 1 ] = '\r';
            _msg += "\n";
            _p_con->addOutput( _msg );
            _msg = "";
        }
    }

    return std::char_traits< char >::not_eof( c );
}

} // namespace vrc
