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
 # player's chat gui control
 #
 #   date of creation:  04/19/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_chatgui.h"
#include "ctd_playerimpl.h"

using namespace osg;
using namespace std;

namespace CTD
{
// layout prefix
#define CHATLAYOUT_PREFIX       "chatbox_"
#define FADE_TIME               1.0f

#define CTD_IMAGE_SET           "CTDImageSet"
#define CTD_IMAGE_SET_FILE      "gui/imagesets/CTDImageSet.imageset"

PlayerChatGui* PlayerChatGui::s_chatGui = NULL;

PlayerChatGui::PlayerChatGui() :
_state( Idle ),
_p_playerImpl( NULL ),
_p_wnd( NULL ),
_p_frame( NULL ),
_p_btnHide( NULL ),
_p_btnMode( NULL ),
_p_editbox( NULL ),
_p_messagebox( NULL ),
_p_btnMsgArrived( NULL ),
_p_mouseImageWalkMode( NULL ),
_p_mouseImageDefault( NULL ),
_hidden( true ),
_modeEdit( false ),
_fadeTimer( 0 ),
_frameAlphaValue( 1.0f )
{
    assert( !s_chatGui && "chat gui is a singleton!" );
    s_chatGui = this;
}

PlayerChatGui::~PlayerChatGui()
{
    // restore pointer image and release it
    CEGUI::MouseCursor::getSingleton().setImage( _p_mouseImageDefault );
    GuiManager::get()->releasePointer();

    s_chatGui = NULL;

    try
    {
        if ( _p_wnd )
        {
            CEGUI::WindowManager::getSingleton().destroyWindow( _p_wnd );
            CEGUI::ImagesetManager::getSingleton().destroyImageset( CTD_IMAGE_SET );
        }
    }
    catch ( const CEGUI::Exception& e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "PlayerChatGui: problem cleaning up gui resources" << endl;
        log << "      reason: " << e.getMessage().c_str() << endl;
    }
}

void PlayerChatGui::initialize( BasePlayerImplementation* p_playerImpl, const string& layoutFile )
{   
    _p_playerImpl = p_playerImpl;

    // get the player name
    Configuration::get()->getSettingValue( CTD_GS_PLAYER_NAME, _playername );

    try
    {
        _p_wnd = static_cast< CEGUI::Window* >( GuiManager::get()->loadLayout( layoutFile, NULL, CHATLAYOUT_PREFIX ) );
        _p_frame = static_cast< CEGUI::Window* >( _p_wnd->getChild( CHATLAYOUT_PREFIX "fr_chatbox" ) );
		_p_frame->subscribeEvent( CEGUI::FrameWindow::EventCloseClicked, CEGUI::Event::Subscriber( &CTD::PlayerChatGui::onCloseFrame, this ) );
        _p_frame->setMinimumSize( CEGUI::Size( 0.1f, 0.08f ) );
        _p_frame->hide();

        _boxFrameSize = osg::Vec2f( _p_frame->getSize().d_width, _p_frame->getSize().d_height );
        _frameAlphaValue = _p_frame->getAlpha();
    }
    catch ( const CEGUI::Exception& e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** error loading layout '" << layoutFile << "'" << endl;
        log << "   reason: " << e.getMessage().c_str() << endl;
        return;
    }

    try
    {
        _p_editbox = static_cast< CEGUI::Editbox* >( _p_frame->getChild( CHATLAYOUT_PREFIX "eb_editbox" ) );
        _p_editbox->subscribeEvent( CEGUI::MultiLineEditbox::EventCharacterKey, CEGUI::Event::Subscriber( &CTD::PlayerChatGui::onEditboxTextChanged, this ) );

        _p_messagebox = static_cast< CEGUI::MultiLineEditbox* >( _p_frame->getChild( CHATLAYOUT_PREFIX "eb_messagebox" ) );
        _p_messagebox->setReadOnly( true );

        // setup chat box hide button with ctd specific image set
        _p_btnHide = static_cast< CEGUI::PushButton* >( _p_wnd->getChild( CHATLAYOUT_PREFIX "btn_hidebox" ) );
        _p_btnHide->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &CTD::PlayerChatGui::onClickedHide, this ) );
        _p_btnHide->setStandardImageryEnabled( false );

        CEGUI::Imageset* p_imageSet = NULL;
        if ( CEGUI::ImagesetManager::getSingleton().isImagesetPresent( CTD_IMAGE_SET ) )
        {
            p_imageSet = CEGUI::ImagesetManager::getSingleton().getImageset( CTD_IMAGE_SET );
        }
        else
        {
            p_imageSet = CEGUI::ImagesetManager::getSingleton().createImageset( CTD_IMAGE_SET_FILE );
        }

        // set editbox unhide button images
        const CEGUI::Image* p_image = &p_imageSet->getImage( "HandNormal" );
        CEGUI::RenderableImage* p_rendImage = new CEGUI::RenderableImage;
        p_rendImage->setImage( p_image );
        _p_btnHide->setPushedImage( p_rendImage );
        _p_btnHide->setNormalImage( p_rendImage );
        delete p_rendImage;

        p_image = &p_imageSet->getImage( "HandHoover" );
        p_rendImage = new CEGUI::RenderableImage;
        p_rendImage->setImage( p_image );
        _p_btnHide->setHoverImage( p_rendImage );
        delete p_rendImage;

        _p_btnMsgArrived = static_cast< CEGUI::StaticImage* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticImage", CHATLAYOUT_PREFIX "btn_post" ) );
        _p_btnMsgArrived->setPosition( _p_btnHide->getPosition() );
        _p_btnMsgArrived->setAlpha( 0.9f );
        _p_btnMsgArrived->setAlwaysOnTop( true );
        _p_btnMsgArrived->setPosition( CEGUI::Relative, CEGUI::Point( 0.01f, 0.7f ) );
        _p_btnMsgArrived->setSize( CEGUI::Size( 0.03f, 0.03f ) );
        _p_btnMsgArrived->setBackgroundEnabled( false );
        _p_btnMsgArrived->setFrameEnabled( false );
        _p_wnd->addChildWindow( _p_btnMsgArrived );
        p_image = &p_imageSet->getImage( "Post" );
        _p_btnMsgArrived->setImage( p_image );
        _p_btnMsgArrived->hide();

        // setup mode button
        _p_btnMode = static_cast< CEGUI::PushButton* >( _p_wnd->getChild( CHATLAYOUT_PREFIX "btn_mode" ) );
        // we begin walkmode enabled
        _p_btnMode->disable();
        _p_btnMode->setStandardImageryEnabled( false );
        p_image = &p_imageSet->getImage( "FootNormal" );
        p_rendImage = new CEGUI::RenderableImage;
        p_rendImage->setImage( p_image );
        _p_btnMode->setDisabledImage( p_rendImage );
        delete p_rendImage;

        // get the edit / walk pointer images
        _p_mouseImageWalkMode = &p_imageSet->getImage( "Crosshair" );
        assert( _p_mouseImageWalkMode && " missing image Crosshair in CTDExtras image set!" );
        _p_mouseImageDefault = CEGUI::System::getSingleton().getDefaultMouseCursor();
    }
    catch ( const CEGUI::Exception& e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** error setting up layout '" << layoutFile << "'" << endl;
        log << "   reason: " << e.getMessage().c_str() << endl;
        return;
    }

    // we begin with walk mode, set the walk mouse pointer image
    CEGUI::System::getSingleton().setDefaultMouseCursor( _p_mouseImageWalkMode );
    setEditMode( false );
}

void PlayerChatGui::update( float deltaTime )
{
    switch( _state )
    {
        case Idle:
            break;

        case BoxFadeIn:
        {
            _p_frame->show();
            if ( _fadeTimer > FADE_TIME )
            {
                _fadeTimer = 0;
                // restore the initial size
                CEGUI::Size size( _boxFrameSize.x(), _boxFrameSize.y() ); 
                _p_frame->setAlpha( _frameAlphaValue );
                _p_frame->setSize( size );
                _p_btnHide->hide();
                _p_editbox->activate();
                _state = Idle;
                break;
            }
            _fadeTimer += deltaTime;
            // fade in the box
            float fadefac = _fadeTimer / FADE_TIME;
            CEGUI::Size size( _boxFrameSize.x() * fadefac, _boxFrameSize.y() * fadefac ); 
            _p_frame->setSize( size );
            _p_frame->setAlpha( fadefac * _frameAlphaValue );
            _p_btnHide->setAlpha( ( 1.0f - ( _fadeTimer / FADE_TIME ) ) * _frameAlphaValue );
        }
        break;

        case BoxFadeOut:
        {
            _p_btnHide->show();
            if ( _fadeTimer > FADE_TIME )
            {
                _fadeTimer = 0;
                // set size to zero
                CEGUI::Size size( 0, 0 ); 
                _p_frame->setSize( size );
                _state = Idle;
                _p_frame->hide();
                break;
            }
            _fadeTimer += deltaTime;
            // fade in the box
            float fadefac = 1.0f - ( _fadeTimer / FADE_TIME );
            CEGUI::Size size( _boxFrameSize.x() * fadefac, _boxFrameSize.y() * fadefac ); 
            _p_frame->setSize( size );
            _p_frame->setAlpha( fadefac * _frameAlphaValue );
            _p_btnHide->setAlpha( ( _fadeTimer / FADE_TIME ) * _frameAlphaValue );
        }
        break;

        default:
            assert( NULL && "invalid chat gui state!" );

    }
}

void PlayerChatGui::show( bool visible )
{
    if ( visible )
    {
        _p_wnd->show();

        // reset the mode in order to let things get updated
        setEditMode( _modeEdit );

        // restore pointer depending on chat mode and camera view mode
        if ( _modeEdit )
                GuiManager::get()->showMousePointer( true );
        else
            if ( _p_playerImpl->getCameraMode() == BasePlayerImplementation::Ego )
                GuiManager::get()->showMousePointer( true );
            else
                GuiManager::get()->showMousePointer( false );

    }
    else
    {
        _p_wnd->hide();

        CEGUI::MouseCursor::getSingleton().setImage( _p_mouseImageDefault );
        GuiManager::get()->releasePointer();

        // restore default mouse cursor image
        CEGUI::System::getSingleton().setDefaultMouseCursor( _p_mouseImageDefault );
    }
}

void PlayerChatGui::addMessage( const CEGUI::String& msg, const CEGUI::String& author )
{
    CEGUI::String buffer = PlayerChatGui::_p_messagebox->getText();
    buffer += author + "> " + msg;
    PlayerChatGui::_p_messagebox->setText( buffer );
    // set carat position in order to trigger text scrolling after a new line has been added
    PlayerChatGui::_p_messagebox->setCaratIndex( buffer.length() - 1 );

    // if the chat box is hidden then signalize to player that a new message arrived
    if ( _hidden )
        _p_btnMsgArrived->show();
}

bool PlayerChatGui::onCloseFrame( const CEGUI::EventArgs& arg )
{
    onClickedHide( arg );
    return true;
}

bool PlayerChatGui::onEditboxTextChanged( const CEGUI::EventArgs& arg )
{
    // check for 'Return' key
    CEGUI::KeyEventArgs& ke = static_cast< CEGUI::KeyEventArgs& >( const_cast< CEGUI::EventArgs& >( arg ) );
    if ( ke.codepoint == SDLK_RETURN )
    {
        // in standalone mode we have no networking
        if ( GameState::get()->getMode() != GameState::Standalone )
            _p_playerImpl->distributeChatMessage( _p_editbox->getText() ); // send the msg over net

        // add the msg to local chat box
        addMessage( _p_editbox->getText(), _playername );
        _p_editbox->setText( "" );
    }
    return true;
}

bool PlayerChatGui::onClickedHide( const CEGUI::EventArgs& arg )
{
    // are we already in fading action?
    if ( _state != Idle )
        return true;

    _hidden = !_hidden;

    if ( _hidden )
    {
        // store the current size for later fade-out
        _boxFrameSize = osg::Vec2f( _p_frame->getSize().d_width, _p_frame->getSize().d_height );
        _state = BoxFadeOut;
    }
    else
    {
        _state = BoxFadeIn;
        _p_btnMsgArrived->hide();
    }

    return true;
}

void PlayerChatGui::setEditMode( bool edit )
{
    if ( edit )
    {
        _p_btnMode->hide();
        CEGUI::MouseCursor::getSingleton().setImage( _p_mouseImageDefault );
        CEGUI::System::getSingleton().setDefaultMouseCursor( _p_mouseImageDefault );
        GuiManager::get()->releasePointer();
        _p_editbox->activate();
    }
    else
    {
        _p_btnMode->show();
        CEGUI::MouseCursor::getSingleton().setImage( _p_mouseImageWalkMode );
        CEGUI::System::getSingleton().setDefaultMouseCursor( _p_mouseImageWalkMode );
        // in walk mode we fix the pointer (crosshair) in the middle of screen
        GuiManager::get()->lockPointer( 0.0f, 0.0f );
        _p_editbox->deactivate();
    }

    _modeEdit = edit;
}

} // namespace CTD
