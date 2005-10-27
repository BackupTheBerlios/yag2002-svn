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
 #   date of creation:  10/16/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_chatguictrl.h"
#include "ctd_chatmgr.h"

namespace CTD
{
// specific imageset
#define CTD_IMAGE_SET           "CTDImageSet"
#define CTD_IMAGE_SET_FILE      "gui/imagesets/CTDImageSet.imageset"


ChatGuiCtrl::ChatGuiCtrl() :
_p_wnd( NULL ),
_p_btnCreate( NULL ),
_p_btnMode( NULL )
{
}

ChatGuiCtrl::~ChatGuiCtrl()
{
    try
    {
        if ( _p_btnCreate )
            CEGUI::WindowManager::getSingleton().destroyWindow( _p_btnCreate );
    }
    catch ( const CEGUI::Exception& e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "ChatGuiCtrl: problem cleaning up gui resources" << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

void ChatGuiCtrl::initialize( ChatManager* p_chatMgr )
{
    _p_chatMgr = p_chatMgr;

    try
    {
        _p_wnd = GuiManager::get()->getRootWindow();

        // load our custom imageset
        CEGUI::Imageset* p_imageSet = NULL;
        if ( CEGUI::ImagesetManager::getSingleton().isImagesetPresent( CTD_IMAGE_SET ) )
        {
            p_imageSet = CEGUI::ImagesetManager::getSingleton().getImageset( CTD_IMAGE_SET );
        }
        else
        {
            p_imageSet = CEGUI::ImagesetManager::getSingleton().createImageset( CTD_IMAGE_SET_FILE );
        }

        // create walk/edit mode button
        _p_btnMode = static_cast< CEGUI::StaticImage* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticImage", "_chatctrl_mode_" ) );
        _p_btnMode->setPosition( CEGUI::Point( 0.025f, 0.725f ) );
        _p_btnMode->setAlpha( 0.9f );
        _p_btnMode->setAlwaysOnTop( true );
        _p_btnMode->setPosition( CEGUI::Relative, CEGUI::Point( 0.0f, 0.8f ) );
        _p_btnMode->setSize( CEGUI::Size( 0.06f, 0.07f ) );
        _p_btnMode->setBackgroundEnabled( false );
        _p_btnMode->setFrameEnabled( false );
        _p_wnd->addChildWindow( _p_btnMode );
        const CEGUI::Image* p_image = &p_imageSet->getImage( "FootNormal" );
        _p_btnMode->setImage( p_image );
        _p_btnMode->hide();
    }
    catch ( const CEGUI::Exception& e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** error setting up chat ctrl gui" << std::endl;
        log << "   reason: " << e.getMessage().c_str() << std::endl;
        return;
    }

    // we begin with walkmode enabled
    _p_btnMode->show();
    GuiManager::get()->showMousePointer( false );
}

void ChatGuiCtrl::setEditMode( bool en )
{
    if ( !en )
        _p_btnMode->show();
    else
        _p_btnMode->hide();
}
        
void ChatGuiCtrl::show( bool en )
{
    if ( en )
        _p_wnd->show();
    else
        _p_wnd->hide();
}

} // namespace CTD
