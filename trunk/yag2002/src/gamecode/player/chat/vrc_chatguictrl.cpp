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
 # player's chat gui control
 #
 #   date of creation:  10/16/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_chatguictrl.h"
#include "vrc_chatmgr.h"

namespace vrc
{

#define CHATLAYOUT_PREFIX       "chatctrl_"

ChatGuiCtrl::ChatGuiCtrl() :
_p_wnd( NULL ),
_p_btnMode( NULL ),
_editMode( false )
{
}

ChatGuiCtrl::~ChatGuiCtrl()
{
    try
    {
        if ( _p_btnMode )
            CEGUI::WindowManager::getSingleton().destroyWindow( _p_btnMode );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "ChatGuiCtrl: problem cleaning up gui resources" << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

void ChatGuiCtrl::initialize( ChatManager* p_chatMgr )
{
    _p_chatMgr = p_chatMgr;

    try
    {
        // get the main gui window and add some elements to it

        _p_wnd = gameutils::GuiUtils::get()->getMainGuiWindow();

        // create walk/edit mode button
        _p_btnMode = static_cast< CEGUI::StaticImage* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticImage", "_chatctrl_mode_" ) );
        _p_btnMode->setPosition( CEGUI::Point( 0.025f, 0.725f ) );
        _p_btnMode->setAlpha( 0.9f );
        _p_btnMode->setPosition( CEGUI::Relative, CEGUI::Point( 0.0f, 0.8f ) );
        _p_btnMode->setSize( CEGUI::Size( 0.06f, 0.07f ) );
        _p_btnMode->setBackgroundEnabled( false );
        _p_btnMode->setFrameEnabled( false );
        _p_wnd->addChildWindow( _p_btnMode );
        const CEGUI::Image* p_image = vrc::gameutils::GuiUtils::get()->getCustomImage( IMAGE_NAME_FOOT_NORMAL );
        _p_btnMode->setImage( p_image );
        _p_btnMode->hide();
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "*** error setting up chat ctrl gui" << std::endl;
        log_out << "   reason: " << e.getMessage().c_str() << std::endl;
        return;
    }

    // we begin with walkmode enabled
    _p_btnMode->show();
    gameutils::GuiUtils::get()->showMousePointer( false );
}

void ChatGuiCtrl::setEditMode( bool en )
{
    if ( !en )
    {
        _p_btnMode->show();
        gameutils::GuiUtils::get()->showMousePointer( false );
    }
    else
    {
        _p_btnMode->hide();
        gameutils::GuiUtils::get()->showMousePointer( true );
    }

    _editMode = en;
}
        
void ChatGuiCtrl::show( bool en )
{
    if ( en )
    {
        _p_wnd->show();
        // restore the mode
        setEditMode( _editMode );
    }
    else
    {
        _p_wnd->hide();
    }
}

} // namespace vrc
