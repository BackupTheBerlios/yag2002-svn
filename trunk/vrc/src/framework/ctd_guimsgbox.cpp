/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2004-2006, Ali Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU General Public License 
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public 
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
#include "ctd_utils.h"
#include "ctd_guimsgbox.h"

using namespace std;

namespace CTD
{

MessageBoxDialog::MessageBoxDialog( const string& title, const string& text, unsigned int type, bool autodelete, CEGUI::Window* p_parent ) :
_p_wnd( NULL ),
_p_clb( NULL ),
_p_parent( NULL ),
_autodel( autodelete )
{
    CEGUI::Window* p_root = NULL;
    if ( !p_parent )
    {
        _p_parent = CEGUI::WindowManager::getSingleton().getWindow( "Root" );
    }
    else
        _p_parent = p_parent;


    // determine the size of messagebox needed to show the required message text
    //--------
    CEGUI::Font* p_font = CEGUI::FontManager::getSingleton().getFont( CTD_GUI_FONT );

    // consider line breaks in text
    vector< string > lines;
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

            boxwidth  = max( 2.0 * minbuttonwidth + 0.1f, textwidth * 0.5f + 0.05 );
            boxheight = minbuttonheight + textheight + 0.1f; // + 0.1f for the upper frame
            break;

        case MessageBoxDialog::OK:

            boxwidth  = max( minbuttonwidth + 0.1f, textwidth + 0.05 );
            boxheight = minbuttonheight + textheight + 0.1f; // + 0.1f for the upper frame
            break;

        default:
            assert( NULL && "invalid message dialog type!" );
    }
    //--------

    // create dialog frame
    _p_wnd = static_cast< CEGUI::FrameWindow* >( CEGUI::WindowManager::getSingleton().createWindow( ( CEGUI::utf8* )"TaharezLook/FrameWindow", "_msg_box_" ) );
    _p_wnd->setText( title );
    _p_wnd->setAlwaysOnTop( true );
    _p_wnd->setCloseButtonEnabled( false );
    _p_wnd->setPosition( CEGUI::Point( 0.5f - boxwidth * 0.5f, 0.5f - boxheight * 0.5f ) ); // position into middle of screen
    _p_wnd->setSize( CEGUI::Size( boxwidth, boxheight ) );
    _p_wnd->setSizingEnabled( false );

    // create text area
    CEGUI::StaticText* p_msgtext = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( ( CEGUI::utf8* )"TaharezLook/StaticText", "_msg_box_text_" ) );
    p_msgtext->setText( text );
    p_msgtext->setPosition( CEGUI::Point( 0.05f, 0.05f ) );
    p_msgtext->setSize( CEGUI::Size( 0.9f, 0.9f ) );
    p_msgtext->setBackgroundEnabled( false );
    p_msgtext->setFormatting( CEGUI::StaticText::HorzCentred, CEGUI::StaticText::VertCentred );
    p_msgtext->setFrameEnabled( false );
    _p_wnd->addChildWindow( p_msgtext );

    // create buttons
    switch ( type )
    {
        case MessageBoxDialog::YES_NO:
        {
            // create 'Yes' button
            CEGUI::PushButton* p_btnyes = 
                static_cast< CEGUI::PushButton* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Button", "_msg_box_btn_yes_" ) );
            p_btnyes->setSize( CEGUI::Size( 0.45f, 0.1f ) );
            p_btnyes->setPosition( CEGUI::Point( 0.025f, 0.875f ) );
            p_btnyes->setText( "Yes" );
            _p_wnd->addChildWindow( p_btnyes );
            // set callback
            p_btnyes->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( MessageBoxDialog::onClickedYes, this ) );

            // create 'No' button
            CEGUI::PushButton* p_btnno = 
                static_cast< CEGUI::PushButton* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Button", "_msg_box_btn_no_" ) );
            p_btnno->setSize( CEGUI::Size( 0.45f, 0.1f ) );
            p_btnno->setPosition( CEGUI::Point( 0.525f, 0.875f ) );
            p_btnno->setText( "No" );
            _p_wnd->addChildWindow( p_btnno );
            // set callback
            p_btnno->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( MessageBoxDialog::onClickedNo, this ) );
        }
        break;

        case MessageBoxDialog::OK_CANCEL:
        {
            // create 'Ok' button
            CEGUI::PushButton* p_btnok = 
                static_cast< CEGUI::PushButton* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Button", "_msg_box_btn_ok_" ) );
            p_btnok->setSize( CEGUI::Size( 0.45f, 0.1f ) );
            p_btnok->setPosition( CEGUI::Point( 0.025f, 0.875f ) );
            p_btnok->setText( "Ok" );
            _p_wnd->addChildWindow( p_btnok );
            // set callback
            p_btnok->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( MessageBoxDialog::onClickedOk, this ) );

            // create 'Cancel' button
            CEGUI::PushButton* p_btncancel = 
                static_cast< CEGUI::PushButton* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Button", "_msg_box_btn_cancel_" ) );
            p_btncancel->setSize( CEGUI::Size( 0.45f, 0.1f ) );
            p_btncancel->setPosition( CEGUI::Point( 0.525f, 0.875f ) );
            p_btncancel->setText( "Cancel" );
            _p_wnd->addChildWindow( p_btncancel );
            // set callback
            p_btncancel->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( MessageBoxDialog::onClickedCancel, this ) );
        }
        break;

        case MessageBoxDialog::OK:
        {
            // create 'Ok' button
            CEGUI::PushButton* p_btnok = 
                static_cast< CEGUI::PushButton* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Button", "_msg_box_btn_ok_" ) );
            p_btnok->setSize( CEGUI::Size( 0.95f, 0.1f ) );
            p_btnok->setPosition( CEGUI::Point( 0.025f, 0.875f ) );
            p_btnok->setText( "Ok" );
            _p_wnd->addChildWindow( p_btnok );
            // set callback
            p_btnok->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( MessageBoxDialog::onClickedOk, this ) );
        }
        break;

        default:
            assert( NULL && "invalid message dialog type!" );
    }
}

MessageBoxDialog::~MessageBoxDialog()
{
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

} // namespace CTD