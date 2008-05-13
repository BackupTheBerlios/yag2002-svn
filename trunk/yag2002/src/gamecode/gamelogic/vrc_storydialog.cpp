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
 # story dialog implementation
 #
 #   date of creation:  08/05/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_storydialog.h"

//! GUI name prefix
#define STORY_DLG   "_storydlg"


namespace vrc
{

StoryDialog::StoryDialog( unsigned int dialogID, DialogCallback* p_cb ) :
 _dialogID( dialogID ),
 _p_cb( p_cb ),
 _p_wnd( NULL )
{
}

StoryDialog::~StoryDialog()
{
    closeDialog();
}

void StoryDialog::addAnswer( unsigned int type, std::string answertext )
{
    assert( ( _p_wnd == NULL ) && "the dialog is already created! do not call this method after dialog creation." );

    _answers.push_back( std::make_pair( type, answertext ) );
}

void StoryDialog::setDialogCallback( DialogCallback* p_cb )
{
    _p_cb = p_cb;
}

#if 0 //! TODO: remove this proof of concept
void StoryDialog::createDialog( const std::string& title, const std::string& text )
{
    if ( _p_wnd )
    {
        log_error << "StoryDialog: the gui is already created." << std::endl;
        return;
    }

    CEGUI::Window* p_mainwnd = gameutils::GuiUtils::get()->getMainGuiWindow();

    try
    {
        // we need unique gui element names
        static unsigned int num = 0;
        std::stringstream numstr;
        numstr << num;
        num++;

        // determine the text extends
        CEGUI::Font* p_font = yaf3d::GuiManager::get()->getFont( YAF3D_GUI_FONT8 ); // we take the font with pointsize 8
        assert( p_font && "font does not exist!" );

#if 0
        // consider line breaks in text
        std::vector< std::string > lines;
        yaf3d::explode( dialogtext, "\n", &lines );
        float textwidth  = 0.0f;
        // get maximal line width
        for ( std::size_t cnt = 0; cnt < lines.size(); ++cnt )
        {
            float lineextend = p_font->getTextExtent( lines[ cnt ] );
            if ( textwidth < lineextend )
                textwidth = lineextend;
        }
        std::vector< Answer >::const_iterator p_beg = _answers.begin(), p_end = _answers.end();
        for ( ; p_beg != p_end; ++p_beg )
        {
            if ( p_beg->first == eChoice )
            {
                float lineextend = p_font->getTextExtent( p_beg->second ) + 0.1f;
                if ( textwidth < lineextend )
                    textwidth = lineextend;
            }
        }

        float textheight = p_font->getLineSpacing() * ( float( lines.size() ) + float( _answers.size() ) * 1.5f );

        // consider the box border width and a small margin
        textheight += 20.0f;
        textwidth  += 30.0f;

        // now calculate the text width and height in projection screen coords
        float width, height;
        yaf3d::GuiManager::get()->getGuiArea( width, height );
        textwidth  /= width;
        textheight /= height;

        float dialogtextheight = float( lines.size() ) / ( float( lines.size() ) + float( _answers.size() ) * 1.5f );
#endif

        _p_wnd = static_cast< CEGUI::FrameWindow* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/FrameWindow", STORY_DLG "mainFrame" + numstr.str() ) );
//        _p_wnd->subscribeEvent( CEGUI::FrameWindow::EventCloseClicked, CEGUI::Event::Subscriber( &vrc::StoryDialog::onClickedClose, this ) );

        _p_wnd->setSize( CEGUI::Size( 0.25f, 0.30f ) );
        _p_wnd->setPosition( CEGUI::Point( 0.005f, 0.25f ) );
        _p_wnd->setAlpha( 0.8f );
        _p_wnd->setAlwaysOnTop( true );
        _p_wnd->setText( title );

        // create the text gui
        CEGUI::MultiLineEditbox* p_text = static_cast< CEGUI::MultiLineEditbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/MultiLineEditbox", STORY_DLG "text" + numstr.str() ) );
        p_text->setSize( CEGUI::Size( 1.0f, 1.0f ) );
        p_text->setPosition( CEGUI::Point( 0.0f, 0.0f ) );
        p_text->setFont( YAF3D_GUI_FONT8 );
        //p_text->setHorizontalFormatting( CEGUI::StaticText::LeftAligned );
        //p_text->setVerticalFormatting( CEGUI::StaticText::TopAligned );
        p_text->setReadOnly( true );
        _p_wnd->addChildWindow( p_text );
        // set the title and text
        p_text->setText( text );

#if 0
        // create the answer gui elements
        /*std::vector< Answer >::const_iterator*/ p_beg = _answers.begin(), p_end = _answers.end();
        for ( ; p_beg != p_end; ++p_beg )
        {
            std::stringstream answernumstr;
            answernumstr << num;
            num++;

            // we take a fix line height
            dialogtextheight += 0.15f;

            if ( p_beg->first == eChoice )
            {
                CEGUI::RadioButton* p_answer = static_cast< CEGUI::RadioButton* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/RadioButton", STORY_DLG "answer" + answernumstr.str() ) );
                p_answer->setSize( CEGUI::Size( 1.0f, 0.1f ) );
                p_answer->setPosition( CEGUI::Point( 0.1f, dialogtextheight ) );
                p_answer->setFont( YAF3D_GUI_FONT8 );
                // set the answer text
                p_answer->setText( p_beg->second );
                p_answer->setAlwaysOnTop( true );
                p_text->addChildWindow( p_answer );
            }
        }
#endif
        // create the answer gui
//        createAnswers( p_text, _answers );

        // append the dialog to main window
        p_mainwnd->addChildWindow( _p_wnd );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "StoryDialog: problem creating gui" << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

#endif

void StoryDialog::createDialog( const std::string& title, const std::string& text )
{
    if ( _p_wnd )
    {
        log_error << "StoryDialog: the gui is already created." << std::endl;
        return;
    }

    CEGUI::Window* p_mainwnd = gameutils::GuiUtils::get()->getMainGuiWindow();

    try
    {
        // assemble the text
        std::string dialogtext( text );

        // we need unique gui element names
        static unsigned int num = 0;
        std::stringstream numstr;
        numstr << num;
        num++;

        // determine the text extends
        CEGUI::Font* p_font = yaf3d::GuiManager::get()->getFont( YAF3D_GUI_FONT8 ); // we take the font with pointsize 8
        assert( p_font && "font does not exist!" );

        // consider line breaks in text
        std::vector< std::string > lines;
        yaf3d::explode( dialogtext, "\n", &lines );
        float textwidth  = 0.0f;
        // get maximal line width
        for ( std::size_t cnt = 0; cnt < lines.size(); ++cnt )
        {
            float lineextend = p_font->getTextExtent( lines[ cnt ] );
            if ( textwidth < lineextend )
                textwidth = lineextend;
        }
        std::vector< Answer >::const_iterator p_beg = _answers.begin(), p_end = _answers.end();
        for ( ; p_beg != p_end; ++p_beg )
        {
            if ( p_beg->first == eChoice )
            {
                float lineextend = p_font->getTextExtent( p_beg->second ) + 0.1f;
                if ( textwidth < lineextend )
                    textwidth = lineextend;
            }
            else if ( p_beg->first == eString )
            {
                float lineextend = p_font->getTextExtent( p_beg->second ) + 0.25f;
                if ( textwidth < lineextend )
                    textwidth = lineextend;
            }
        }

        // consider the box border width and a small margin
        float textheight = p_font->getLineSpacing() * ( float( lines.size() ) + float( _answers.size() ) * 1.5f );
        textheight += 30.0f;
        textwidth  += 30.0f;

        float answerareaheight = float( lines.size() ) / ( float( lines.size() ) + float( _answers.size() ) * 2.0f );

        _p_wnd = CEGUI::WindowManager::getSingleton().createWindow( "DefaultWindow", STORY_DLG "mainWnd" + numstr.str() );
        _p_wnd->setSize( CEGUI::Absolute, CEGUI::Size( textwidth, textheight + 40.0f ) );
        _p_wnd->setPosition( CEGUI::Point( 0.01f, 0.25f ) );
        _p_wnd->setAlpha( 0.9f );
        _p_wnd->setAlwaysOnTop( true );

        // the dialog colors
        CEGUI::colour color( 0.3f, 0.3f, 0.3f, 0.7f );
        CEGUI::colour colorframe( 0.2f, 0.2f, 0.2f, 0.5f );

        // create the title gui
        CEGUI::StaticText* p_title = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", STORY_DLG "title" + numstr.str() ) );
        float titlewidth = p_font->getTextExtent( title ) + 20.0f;
        p_title->setSize( CEGUI::Absolute, CEGUI::Size( titlewidth, 40.0f ) );
        p_title->setPosition( CEGUI::Point( 0.0f, 0.0f ) );
        p_title->setFont( YAF3D_GUI_FONT8 );
        p_title->setHorizontalFormatting( CEGUI::StaticText::LeftAligned );
        p_title->setVerticalFormatting( CEGUI::StaticText::TopAligned );
        p_title->setBackgroundColours( color );
        p_title->setFrameColours( colorframe );
        // set the title
        p_title->setText( title );
        _p_wnd->addChildWindow( p_title );

        // create the text gui
        CEGUI::StaticText* p_text = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", STORY_DLG "text" + numstr.str() ) );
        p_text->setSize( CEGUI::Absolute, CEGUI::Size( textwidth, textheight ) );
        p_text->setPosition( CEGUI::Absolute, CEGUI::Point( 0.0f, 40.0f ) );
        p_text->setFont( YAF3D_GUI_FONT8 );
        p_text->setHorizontalFormatting( CEGUI::StaticText::LeftAligned );
        p_text->setVerticalFormatting( CEGUI::StaticText::TopAligned );
        p_text->setBackgroundColours( color );
        p_text->setFrameColours( colorframe );
        // set the text
        p_text->setText( text );
        _p_wnd->addChildWindow( p_text );

        // begin of height position for answer gui elements
        answerareaheight += 0.1f;

        // create the answer gui elements
        p_beg = _answers.begin(), p_end = _answers.end();
        for ( ; p_beg != p_end; ++p_beg )
        {
            std::stringstream answernumstr;
            answernumstr << num;
            num++;

            CEGUI::Window* p_answer = NULL;

            if ( p_beg->first == eChoice )
            {
                p_answer = CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/RadioButton", STORY_DLG "answer" + answernumstr.str() );
                p_answer->setSize( CEGUI::Size( 1.0f, 0.15f ) );
                p_answer->setText( p_beg->second );
            }
            else if ( p_beg->first == eString )
            {
                p_answer = CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Editbox", STORY_DLG "answer" + answernumstr.str() );
                p_answer->setSize( CEGUI::Size( 0.15f, 0.13f ) );

                CEGUI::StaticText* p_desc = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", STORY_DLG "answerdesc" + answernumstr.str() ) );
                p_desc->setSize( CEGUI::Size( 0.75f, 0.13f ) );
                p_desc->setPosition( CEGUI::Point( 0.23f, answerareaheight ) );
                p_desc->setFont( YAF3D_GUI_FONT8 );
                p_desc->setAlwaysOnTop( true );
                p_desc->setBackgroundEnabled( false );
                p_desc->setFrameEnabled( false );
                p_desc->setText( p_beg->second );
                p_text->addChildWindow( p_desc );
            }
            else
            {
                log_error << "StoryDialog: unknown dialog element, skipping" << std::endl;
                continue;
            }

            p_answer->setPosition( CEGUI::Point( 0.05f, answerareaheight ) );
            p_answer->setFont( YAF3D_GUI_FONT8 );
            p_answer->setAlwaysOnTop( true );
            p_text->addChildWindow( p_answer );

            // we take a fix line height
            answerareaheight += 0.16f;
        }

        // append the dialog to main window
        p_mainwnd->addChildWindow( _p_wnd );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "StoryDialog: problem creating gui" << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

void StoryDialog::closeDialog()
{
    try
    {
        if ( _p_wnd )
            CEGUI::WindowManager::getSingleton().destroyWindow( _p_wnd );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "StoryDialog: problem destroying gui." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }

    _p_wnd = NULL;
}

void StoryDialog::show( bool en )
{
    if ( en )
        _p_wnd->show();
    else
        _p_wnd->hide();
}

unsigned int StoryDialog::getChoice()
{
    // TODO
    return 0;
}

std::string StoryDialog::getText( unsigned int num )
{
    // TODO
    return std::string( "" );
}

} // namespace vrc
