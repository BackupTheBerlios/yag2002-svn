/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2002-2005, Ali Botorabi
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
 # player's chat gui control
 #
 #   date of creation:  04/19/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_base.h>
#include <ctd_physics.h>
#include <ctd_log.h>
#include <ctd_application.h>
#include <ctd_configuration.h>
#include "ctd_chatgui.h"
#include "ctd_player.h"

using namespace osg;
using namespace std;

namespace CTD
{
// layout prefix
#define CHATLAYOUT_PREFIX   "chatbox_"

PlayerChatGui::PlayerChatGui() :
_p_player( NULL ),
_p_wnd( NULL ),
_p_editbox( NULL ),
_p_messagebox( NULL )
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
    }
    catch ( CEGUI::Exception e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** error loading layout '" << layoutFile << "'" << endl;
        log << "   reason: " << e.getMessage().c_str() << endl;
        return;
    }

    try
    {
        _p_editbox = static_cast< CEGUI::MultiLineEditbox* >( _p_wnd->getChild( CHATLAYOUT_PREFIX "eb_editbox" ) );
        _p_editbox->subscribeEvent( CEGUI::MultiLineEditbox::EventCharacterKey, CEGUI::Event::Subscriber( PlayerChatGui::onEditboxTextChanged, this ) );

        _p_messagebox = static_cast< CEGUI::MultiLineEditbox* >( _p_wnd->getChild( CHATLAYOUT_PREFIX "eb_messagebox" ) );
        _p_messagebox->setReadOnly( true );
    }
    catch ( CEGUI::Exception e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** error setting up layout '" << layoutFile << "'" << endl;
        log << "   reason: " << e.getMessage().c_str() << endl;
        return;
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

} // namespace CTD
