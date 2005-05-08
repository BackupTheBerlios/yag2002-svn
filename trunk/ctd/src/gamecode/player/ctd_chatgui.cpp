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
#include "ctd_player.h"

using namespace osg;
using namespace std;

namespace CTD
{
// layout prefix
#define CHATLAYOUT_PREFIX   "chatbox_"
#define FADEIN_TIME         1.0f
 
PlayerChatGui::PlayerChatGui() :
_p_player( NULL ),
_p_wnd( NULL ),
_p_frame( NULL ),
_p_editbox( NULL ),
_p_messagebox( NULL ),
_modeEdit( true ),
_hidden( false ),
_state( BoxFadeIn ),
_fadeTimer( 0 ),
_frameAlphaValue( 1.0f )
{
}

PlayerChatGui::~PlayerChatGui()
{
    if ( _p_wnd )
        CEGUI::WindowManager::getSingleton().destroyWindow( _p_wnd );
}

void PlayerChatGui::initialize( EnPlayer* p_player, const string& layoutFile )
{   
    _p_player = p_player;

    // get the player name
    Configuration::get()->getSettingValue( CTD_GS_PLAYERNAME, _playername );

    try
    {
        _p_wnd = static_cast< CEGUI::Window* >( GuiManager::get()->loadLayout( layoutFile, NULL, CHATLAYOUT_PREFIX ) );
        _p_frame = static_cast< CEGUI::Window* >( _p_wnd->getChild( CHATLAYOUT_PREFIX "fr_chatbox" ) );
        _boxFrameSize = osg::Vec2f( _p_frame->getSize().d_width, _p_frame->getSize().d_height );
        _frameAlphaValue = _p_frame->getAlpha();
        _p_frame->setMinimumSize( CEGUI::Size( 0.1f, 0.08f ) );
    }
    catch ( CEGUI::Exception e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** error loading layout '" << layoutFile << "'" << endl;
        log << "   reason: " << e.getMessage().c_str() << endl;
        return;
    }

    try
    {
        CEGUI::FrameWindow* p_frameWnd = static_cast< CEGUI::FrameWindow* >( _p_frame );
        p_frameWnd->subscribeEvent( CEGUI::FrameWindow::EventCloseClicked, CEGUI::Event::Subscriber( PlayerChatGui::onCloseFrame, this ) );

        _p_editbox = static_cast< CEGUI::Editbox* >( _p_frame->getChild( CHATLAYOUT_PREFIX "eb_editbox" ) );
        _p_editbox->subscribeEvent( CEGUI::MultiLineEditbox::EventCharacterKey, CEGUI::Event::Subscriber( PlayerChatGui::onEditboxTextChanged, this ) );

        _p_messagebox = static_cast< CEGUI::MultiLineEditbox* >( _p_frame->getChild( CHATLAYOUT_PREFIX "eb_messagebox" ) );
        _p_messagebox->setReadOnly( true );

        _p_btnHide = static_cast< CEGUI::PushButton* >( _p_wnd->getChild( CHATLAYOUT_PREFIX "btn_hidebox" ) );
        _p_btnHide->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( PlayerChatGui::onClickedHide, this ) );

        _p_btnMode = static_cast< CEGUI::PushButton* >( _p_wnd->getChild( CHATLAYOUT_PREFIX "btn_mode" ) );
        _p_btnMode->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( PlayerChatGui::onClickedMode, this ) );

    }
    catch ( CEGUI::Exception e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** error setting up layout '" << layoutFile << "'" << endl;
        log << "   reason: " << e.getMessage().c_str() << endl;
        return;
    }
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
            if ( _fadeTimer > FADEIN_TIME )
            {
                _fadeTimer = 0;
                // restore the initial size
                CEGUI::Size size( _boxFrameSize.x(), _boxFrameSize.y() ); 
                _p_frame->setAlpha( _frameAlphaValue );
                _p_frame->setSize( size );
                _state = Idle;
                break;
            }
            _fadeTimer += deltaTime;
            // fade in the box
            float fadefac = _fadeTimer / FADEIN_TIME;
            CEGUI::Size size( _boxFrameSize.x() * fadefac, _boxFrameSize.y() * fadefac ); 
            _p_frame->setSize( size );
            _p_frame->setAlpha( fadefac * _frameAlphaValue );
        }
        break;

        case BoxFadeOut:
        {
            if ( _fadeTimer > FADEIN_TIME )
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
            float fadefac = 1.0f - ( _fadeTimer / FADEIN_TIME );
            CEGUI::Size size( _boxFrameSize.x() * fadefac, _boxFrameSize.y() * fadefac ); 
            _p_frame->setSize( size );
            _p_frame->setAlpha( fadefac * _frameAlphaValue );
        }
        break;

        default:
            assert( NULL && "invalid chat gui state!" );

    }
}

void PlayerChatGui::show( bool visible )
{
    if ( visible )
        _p_wnd->show();
    else
        _p_wnd->hide();
}

void PlayerChatGui::addMessage( const CEGUI::String& msg, const CEGUI::String& author )
{
    CEGUI::String buffer = _p_messagebox->getText();
    buffer += author + "> " + msg;
    _p_messagebox->setText( buffer );
    // set carat position in order to trigger text scrolling after a new line has been added
    _p_messagebox->setCaratIndex( buffer.length() - 1 );
}

bool PlayerChatGui::onCloseFrame( const CEGUI::EventArgs& arg )
{
    onClickedHide( arg );
    return true;
}

bool PlayerChatGui::onEditboxTextChanged( const CEGUI::EventArgs& arg )
{
    // check for 'Return' key
    CEGUI::KeyEventArgs* ke = static_cast< CEGUI::KeyEventArgs* >( &( CEGUI::EventArgs& )arg );
    if ( ke->codepoint == osgGA::GUIEventAdapter::KEY_Return )
    {
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
    }

    return true;
}

bool PlayerChatGui::onClickedMode( const CEGUI::EventArgs& arg )
{
    // enable / disable player control
    _p_player->enableControl( !_modeEdit );
    _modeEdit = !_modeEdit;

   return true;
}

} // namespace CTD
