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
#include "vrc_story.h"

//! GUI name prefix
#define STORY_DLG   "_storydlg"


namespace vrc
{

StoryDialog::StoryDialog( DialogCallback* p_cb ) :
 _dialogID( 0 ),
 _networkID( 0 ),
 _sourceID( 0 ),
 _p_cb( p_cb ),
 _p_wnd( NULL ),
 _abortClicked( false )
{
}

StoryDialog::StoryDialog( const StoryDialogParams& params, DialogCallback* p_cb ) :
 _dialogID( params._id ),
 _networkID( params._destNetworkID ),
 _sourceID( params._sourceID ),
 _p_cb( p_cb ),
 _p_wnd( NULL ),
 _abortClicked( false )
{
    std::vector< StoryDialogParams::ChoiceInput >::const_iterator p_choice = params._choices.begin(), p_choiceEnd = params._choices.end();
    for ( ; p_choice != p_choiceEnd; ++p_choice )
        addChoice( p_choice->first, p_choice->second );

    std::vector< StoryDialogParams::TextInput >::const_iterator p_textinput = params._textFields.begin(), p_textinputEnd = params._textFields.end();
    for ( ; p_textinput != p_textinputEnd; ++p_textinput )
        addInputText( p_textinput->first, p_textinput->second );

    createDialog( params._id, params._title, params._text );
}

StoryDialog::~StoryDialog()
{
    closeDialog();
}

void StoryDialog::addChoice( std::string fieldname, bool defaultselection )
{
    assert( ( _p_wnd == NULL ) && "the dialog is already created! do not call this method after dialog creation." );

    InputField field;
    field._fieldType = eChoice;
    field._fieldName = fieldname;
    field._defaultSelection = defaultselection;
    _inputs.push_back( field );
}

void StoryDialog::addInputText( std::string fieldname, const std::string& defaultvalue )
{
    assert( ( _p_wnd == NULL ) && "the dialog is already created! do not call this method after dialog creation." );

    InputField field;
    field._fieldType = eString;
    field._fieldName = fieldname;
    field._defaultValue = defaultvalue;
    _inputs.push_back( field );
}

void StoryDialog::clearDialog()
{
    _inputs.clear();
    _choices.clear();
    _textFields.clear();
    _dialogID     = 0;
    _abortClicked = false;
    closeDialog();
}

void StoryDialog::setDialogCallback( DialogCallback* p_cb )
{
    _p_cb = p_cb;
}

bool StoryDialog::createDialog( unsigned int dialogID, const std::string& title, const std::string& text )
{
    if ( _p_wnd )
    {
        log_error << "StoryDialog: the gui is already created." << std::endl;
        return false;
    }

    // set the dialog ID.
    _dialogID = dialogID;

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
        std::vector< InputField >::const_iterator p_beg = _inputs.begin(), p_end = _inputs.end();
        for ( ; p_beg != p_end; ++p_beg )
        {
            if ( p_beg->_fieldType == eChoice )
            {
                float lineextend = p_font->getTextExtent( p_beg->_fieldName ) + 0.1f;
                if ( textwidth < lineextend )
                    textwidth = lineextend;
            }
            else if ( p_beg->_fieldType == eString )
            {
                float lineextend = p_font->getTextExtent( p_beg->_fieldName ) + 0.25f;
                if ( textwidth < lineextend )
                    textwidth = lineextend;
            }
        }

        // consider the box border width and a small margin
        float textheight = p_font->getLineSpacing() * ( float( lines.size() ) + float( _inputs.size() ) * 1.5f );
        textheight += 30.0f;
        textwidth  += 30.0f;
        textwidth  = std::max( 250.0f, textwidth );

        float inputareaheight = float( lines.size() ) / ( float( lines.size() ) + float( _inputs.size() ) * 2.0f );

        _p_wnd = CEGUI::WindowManager::getSingleton().createWindow( "DefaultWindow", STORY_DLG "mainWnd" + numstr.str() );
        _p_wnd->setSize( CEGUI::Absolute, CEGUI::Size( textwidth, textheight + 72.50f ) ); // consider the buttons
        _p_wnd->setPosition( CEGUI::Point( 0.01f, 0.25f ) );
        _p_wnd->setAlpha( 0.9f );
        _p_wnd->setAlwaysOnTop( true );

        // the dialog colors
        CEGUI::colour color( 0.2f, 0.2f, 0.2f, 0.5f ); // ( 0.3f, 0.3f, 0.3f, 0.7f );
        CEGUI::colour colorframe( 0.2f, 0.2f, 0.2f, 0.45f );

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

        // begin of height position for input gui elements
        inputareaheight += 0.1f;

        // create the input gui elements
        VOID_NUM selectionindex = 1; // selection index
        VOID_NUM textindex      = 1; // text field index
        p_beg = _inputs.begin(), p_end = _inputs.end();
        for ( ; p_beg != p_end; ++p_beg )
        {
            std::stringstream answernumstr;
            answernumstr << num;
            num++;

            CEGUI::Window* p_input = NULL;

            if ( p_beg->_fieldType == eChoice )
            {
                p_input = CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/RadioButton", STORY_DLG "input" + answernumstr.str() );
                p_input->setSize( CEGUI::Size( 1.0f, 0.15f ) );
                p_input->setText( p_beg->_fieldName );
                p_input->setUserData( reinterpret_cast< void* >( selectionindex++ ) );

                static_cast< CEGUI::RadioButton* >( p_input )->setSelected( p_beg->_defaultSelection );

                // add the radio button to list for later value query
                _choices.push_back( reinterpret_cast< CEGUI::RadioButton* >( p_input ) );
            }
            else if ( p_beg->_fieldType == eString )
            {
                p_input = CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Editbox", STORY_DLG "input" + answernumstr.str() );
                p_input->setSize( CEGUI::Size( 0.15f, 0.13f ) );
                p_input->setUserData( reinterpret_cast< void* >( textindex++ ) );
                p_input->setText( p_beg->_defaultValue );

                CEGUI::StaticText* p_desc = static_cast< CEGUI::StaticText* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticText", STORY_DLG "answerdesc" + answernumstr.str() ) );
                p_desc->setSize( CEGUI::Size( 0.75f, 0.13f ) );
                p_desc->setPosition( CEGUI::Point( 0.23f, inputareaheight ) );
                p_desc->setFont( YAF3D_GUI_FONT8 );
                p_desc->setAlwaysOnTop( true );
                p_desc->setBackgroundEnabled( false );
                p_desc->setFrameEnabled( false );
                p_desc->setText( p_beg->_fieldName );
                p_text->addChildWindow( p_desc );

                // add the text field to list for later value query
                _textFields.push_back( reinterpret_cast< CEGUI::Editbox* >( p_input ) );
            }
            else
            {
                log_error << "StoryDialog: unknown dialog element, skipping" << std::endl;
                continue;
            }

            p_input->setPosition( CEGUI::Point( 0.05f, inputareaheight ) );
            p_input->setFont( YAF3D_GUI_FONT8 );
            p_input->setAlwaysOnTop( true );
            p_text->addChildWindow( p_input );

            // we take a fix line height
            inputareaheight += 0.16f;
        }

        // append the ok and cancel buttons to box
        CEGUI::PushButton* p_btnok    = static_cast< CEGUI::PushButton* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Button", STORY_DLG "btnok" + numstr.str() ) );
        p_btnok->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::StoryDialog::onClickedOk, this ) );
        p_btnok->setSize( CEGUI::Absolute, CEGUI::Size( 100.0f, 30.0f ) );
        p_btnok->setPosition( CEGUI::Absolute, CEGUI::Point( textwidth - 100.0f, textheight + 42.50f ) );
        p_btnok->setFont( YAF3D_GUI_FONT8 );
        p_btnok->setAlwaysOnTop( true );
        p_btnok->setText( "Ok" );
        _p_wnd->addChildWindow( p_btnok );

        CEGUI::PushButton* p_btnabort = static_cast< CEGUI::PushButton* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Button", STORY_DLG "btncancel" + numstr.str() ) );
        p_btnabort->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::StoryDialog::onClickedAbort, this ) );
        p_btnabort->setSize( CEGUI::Absolute, CEGUI::Size( 100.0f, 30.0f ) );
        p_btnabort->setPosition( CEGUI::Absolute, CEGUI::Point( textwidth - 202.50f, textheight + 42.50f ) );
        p_btnabort->setFont( YAF3D_GUI_FONT8 );
        p_btnabort->setAlwaysOnTop( true );
        p_btnabort->setText( "Abort" );
        _p_wnd->addChildWindow( p_btnabort );

        // append the dialog to main window
        p_mainwnd->addChildWindow( _p_wnd );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "StoryDialog: problem creating gui" << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
        return false;
    }

    return true;
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
    unsigned int choice = 0;
    // search for selected radio button
    std::vector< CEGUI::RadioButton* >::iterator p_radio = _choices.begin(), p_end = _choices.end();
    for ( ; p_radio != p_end; ++p_radio )
    {
        if ( ( *p_radio )->isSelected() )
        {
            choice = static_cast< unsigned int >( reinterpret_cast< VOID_NUM >( ( *p_radio )->getUserData() ) );
            break;
        }
    }

    return choice;
}

void StoryDialog::setChoice( unsigned int index )
{
    // search for radio button and select it
    std::vector< CEGUI::RadioButton* >::iterator p_radio = _choices.begin(), p_end = _choices.end();
    for ( ; p_radio != p_end; ++p_radio )
    {
        unsigned int idx = static_cast< unsigned int >( reinterpret_cast< VOID_NUM >( ( *p_radio )->getUserData() ) );
        if ( idx == index )
        {
            ( *p_radio )->setSelected( true );
        }
        else
        {
            ( *p_radio )->setSelected( false );
        }
    }
}

std::string StoryDialog::getText( unsigned int num )
{
    if ( num > _textFields.size() || !num )
    {
        log_error << "StoryDialog: invalid text field index " << num << std::endl;
        return std::string( "" );
    }

    return std::string( _textFields[ num - 1 ]->getText().c_str() );
}

void StoryDialog::setText( unsigned int num, const std::string& text )
{
    if ( num > _textFields.size() || !num )
    {
        log_error << "StoryDialog: invalid text field index " << num << std::endl;
        return;
    }

    _textFields[ num - 1 ]->setText( text );
}

unsigned int StoryDialog::getCountTextInput() const
{
    return _textFields.size();
}

bool StoryDialog::onClickedOk( const CEGUI::EventArgs& /*arg*/ )
{
    _abortClicked = false;

    if ( _p_cb )
    {
        // collect the dialog results and call the callback method
        StoryDialogResults results;
        results._dialogAbort   = _abortClicked;
        results._id            = _dialogID;
        results._choice        = getChoice();
        results._destNetworkID = getNetworkID();
        results._sourceID      = getSourceID();

        unsigned int  cntvalues = getCountTextInput();
        for ( unsigned int cnt = 1; cnt <= cntvalues; cnt++ )
            results._textFields.push_back( getText( cnt ) );

        _p_cb->onDialogResult( results );
    }

    return true;
}

bool StoryDialog::onClickedAbort( const CEGUI::EventArgs& /*arg*/ )
{
    _abortClicked = true;

    if ( _p_cb )
    {
        // notify about aborting the dialog via callback method
        StoryDialogResults results;
        results._dialogAbort   = _abortClicked;
        results._id            = _dialogID;
        results._destNetworkID = getNetworkID();
        results._sourceID      = getSourceID();

        _p_cb->onDialogResult( results );
    }

    return true;
}

bool StoryDialog::clickedAbort() const
{
    return _abortClicked;
}

} // namespace vrc
