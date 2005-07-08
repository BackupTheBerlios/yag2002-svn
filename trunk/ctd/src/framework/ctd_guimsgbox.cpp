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
 # graphical user interface: message box
 #
 #	 date of creation:	04/06/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_base.h>
#include "ctd_log.h"
#include "ctd_utils.h"
#include "ctd_guimsgbox.h"

namespace CTD
{

MessageBoxDialog::MessageBoxDialog( const std::string& title, const std::string& text, unsigned int type, bool autodelete, CEGUI::Window* p_parent ) :
_p_wnd( NULL ),
_p_clb( NULL ),
_p_parent( NULL ),
_autodel( autodelete ),
_p_msgtext( NULL )
{
    if ( !p_parent )
    {
        _p_parent = CEGUI::WindowManager::getSingleton().getWindow( "Root" );
    }
    else
        _p_parent = p_parent;


    // determine the size of messagebox needed to show the required message text
    //--------
    CEGUI::Font* p_font = GuiManager::get()->getFont( CTD_GUI_FONT10 ); // we take the font with pointsize 10
    assert( p_font && "font does not exist!" );

    // consider line breaks in text
    std::vector< std::string > lines;
    explode( text, "\n", &lines );
    float textwidth  = 0;
    // get maximal line width
    for ( size_t cnt = 0; cnt < lines.size(); cnt++ )
    {
        float lineextend = p_font->getTextExtent( lines[ cnt ] );
        if ( textwidth < lineextend )
            textwidth = lineextend;
    }    
    float textheight = p_font->getLineSpacing() * float( lines.size() );
    // now calculate the text width and height in projection screen coords
    float width, height;
    GuiManager::get()->getGuiArea( width, height );
    textwidth  /= width;
    textheight /= height;
    //--------

    // determine the total extends of messagebox depending on text and dialog type ( needed buttons )
    //--------
    float boxwidth  = 0;
    float boxheight = 0;
    float minbuttonwidth  = 0.1f;
    float minbuttonheight = 0.05f;
    switch ( type )
    {
        case MessageBoxDialog::YES_NO:
        case MessageBoxDialog::OK_CANCEL:

            boxwidth  = std::max( 2.0f * minbuttonwidth + 0.1f, textwidth + 0.05f );
            boxheight = minbuttonheight + textheight + 0.1f; // + 0.1f for the upper frame
            break;

        case MessageBoxDialog::OK:

            boxwidth  = std::max( minbuttonwidth + 0.1f, textwidth + 0.05f );
            boxheight = minbuttonheight + textheight + 0.1f; // + 0.1f for the upper frame
            break;

        default:
            assert( NULL && "invalid message dialog type!" );
    }
    //--------

    // we need a unique windows name for every messagebox, as 
    static int s_pf = 0;
    std::stringstream postfix;
    postfix << s_pf;
    s_pf++;
    
    try
    {
        // create dialog frame
        _p_wnd = static_cast< CEGUI::FrameWindow* >( CEGUI::WindowManager::getSingleton().createWindow( ( CEGUI::utf8* )"TaharezLook/FrameWindow", ( "_msg_box_" + postfix.str() ).c_str() ) );
        _p_wnd->setText( title );
        _p_wnd->setAlwaysOnTop( true );
        _p_wnd->setCloseButtonEnabled( false );
        _p_wnd->setPosition( CEGUI::Point( 0.5f - boxwidth * 0.5f, 0.5f - boxheight * 0.5f ) ); // position into middle of screen
        _p_wnd->setSize( CEGUI::Size( boxwidth, boxheight ) );
        _p_wnd->setSizingEnabled( false );

        // create text area
        _p_msgtext = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( ( CEGUI::utf8* )"TaharezLook/StaticText", ( "_msg_box_text_" + postfix.str() ).c_str() ) );
        _p_msgtext->setText( text );
        _p_msgtext->setPosition( CEGUI::Point( 0.05f, 0.05f ) );
        _p_msgtext->setSize( CEGUI::Size( 0.9f, 0.9f ) );
        _p_msgtext->setBackgroundEnabled( false );
        _p_msgtext->setFormatting( CEGUI::StaticText::HorzCentred, CEGUI::StaticText::VertCentred );
        _p_msgtext->setFrameEnabled( false );
        _p_wnd->addChildWindow( _p_msgtext );

        // create buttons
        switch ( type )
        {
            case MessageBoxDialog::YES_NO:
            {
                // create 'Yes' button
                CEGUI::PushButton* p_btnyes = 
                    static_cast< CEGUI::PushButton* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Button", ( "_msg_box_btn_yes_" + postfix.str() ).c_str() ) );
                p_btnyes->setSize( CEGUI::Size( 0.45f, 0.15f ) );
                p_btnyes->setPosition( CEGUI::Point( 0.025f, 0.825f ) );
                p_btnyes->setText( "Yes" );
                _p_wnd->addChildWindow( p_btnyes );
                // set callback
                p_btnyes->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &CTD::MessageBoxDialog::onClickedYes, this ) );

                // create 'No' button
                CEGUI::PushButton* p_btnno = 
                    static_cast< CEGUI::PushButton* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Button", ( "_msg_box_btn_no_" + postfix.str() ).c_str() ) );
                p_btnno->setSize( CEGUI::Size( 0.45f, 0.15f ) );
                p_btnno->setPosition( CEGUI::Point( 0.525f, 0.825f ) );
                p_btnno->setText( "No" );
                _p_wnd->addChildWindow( p_btnno );
                // set callback
                p_btnno->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &CTD::MessageBoxDialog::onClickedNo, this ) );
            }
            break;

            case MessageBoxDialog::OK_CANCEL:
            {
                // create 'Ok' button
                CEGUI::PushButton* p_btnok = 
                    static_cast< CEGUI::PushButton* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Button", ( "_msg_box_btn_ok_" + postfix.str() ).c_str() ) );
                p_btnok->setSize( CEGUI::Size( 0.45f, 0.15f ) );
                p_btnok->setPosition( CEGUI::Point( 0.025f, 0.825f ) );
                p_btnok->setText( "Ok" );
                _p_wnd->addChildWindow( p_btnok );
                // set callback
                p_btnok->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &CTD::MessageBoxDialog::onClickedOk, this ) );

                // create 'Cancel' button
                CEGUI::PushButton* p_btncancel = 
                    static_cast< CEGUI::PushButton* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Button", ( "_msg_box_btn_cancel_" + postfix.str() ).c_str() ) );
                p_btncancel->setSize( CEGUI::Size( 0.45f, 0.15f ) );
                p_btncancel->setPosition( CEGUI::Point( 0.525f, 0.825f ) );
                p_btncancel->setText( "Cancel" );
                _p_wnd->addChildWindow( p_btncancel );
                // set callback
			    p_btncancel->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &CTD::MessageBoxDialog::onClickedCancel, this ) );
            }
            break;

            case MessageBoxDialog::OK:
            {
                // create 'Ok' button
                CEGUI::PushButton* p_btnok = 
                    static_cast< CEGUI::PushButton* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Button", ( "_msg_box_btn_ok_" + postfix.str() ).c_str() ) );
                p_btnok->setSize( CEGUI::Size( 0.5f, 0.15f ) );
                p_btnok->setPosition( CEGUI::Point( 0.25f, 0.75f ) );
                p_btnok->setText( "Ok" );
                _p_wnd->addChildWindow( p_btnok );
                // set callback
			    p_btnok->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &CTD::MessageBoxDialog::onClickedOk, this ) );
            }
            break;

            default:
                assert( NULL && "invalid message dialog type!" );
        }
    }
    catch ( const CEGUI::Exception& e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "error creating messagebox" << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

MessageBoxDialog::~MessageBoxDialog()
{
}

void MessageBoxDialog::destroy()
{
    // if not auto deleting then just hide the message box
    if ( _autodel )
    {
        CEGUI::WindowManager::getSingleton().destroyWindow( _p_wnd );

        if ( _p_clb )
            delete _p_clb;

        delete this;
    }
}

void MessageBoxDialog::show()
{
    // if we were just hidden before then unhide, otherwise it is the first time for showing the dialog so add it into parent window
    if ( _p_wnd->isVisible() )
        _p_parent->addChildWindow( _p_wnd );
    else
        _p_wnd->setVisible( true );
}
 
void MessageBoxDialog::setClickCallback( MessageBoxDialog::ClickCallback* p_clb )
{
    _p_clb   = p_clb;
}

void MessageBoxDialog::processClick( unsigned int btn )
{
    if ( _p_clb )
        _p_clb->onClicked( btn );

    // if not auto deleting then just hide the message box
    if ( _autodel )
    {
        CEGUI::WindowManager::getSingleton().destroyWindow( _p_wnd );

        if ( _p_clb )
            delete _p_clb;

        delete this;
    }
    else
    {
        _p_wnd->setVisible( false );
    }
}

bool MessageBoxDialog::onClickedOk( const CEGUI::EventArgs& arg )
{
    processClick( MessageBoxDialog::BTN_OK ); 
    return true;
}

bool MessageBoxDialog::onClickedCancel( const CEGUI::EventArgs& arg )
{
    processClick( MessageBoxDialog::BTN_CANCEL ); 
    return true;
}

bool MessageBoxDialog::onClickedYes( const CEGUI::EventArgs& arg )
{
    processClick( MessageBoxDialog::BTN_YES ); 
    return true;
}

bool MessageBoxDialog::onClickedNo( const CEGUI::EventArgs& arg )
{
    processClick( MessageBoxDialog::BTN_NO ); 
    return true;
}

const std::string& MessageBoxDialog::getText() const
{
    // convert cegui's string to std string
    static std::string text;
    text = _p_msgtext->getText().c_str();
    return text;
}

void MessageBoxDialog::setText( const std::string& text )
{
    // convert std string to cegui's string
    CEGUI::String t( text.c_str() );
    _p_msgtext->setText( t );
}

} // namespace CTD
