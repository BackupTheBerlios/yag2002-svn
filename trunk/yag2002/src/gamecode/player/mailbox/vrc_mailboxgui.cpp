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
 # player's mailbox gui implementation
 #
 #   date of creation:  31/07/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 ****************************************************************/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_mailboxgui.h"
#include "vrc_mailboxguimain.h"

//! Gui ressource related defines

#define MAILBOX_MAIN_DLG_PREFIX         "mailbox_"
#define MAILBOX_MAIN_GUI_LAYOUT         "gui/mailbox.xml"


namespace vrc
{

MailboxGui::MailboxGui() :
 _p_btnOpenMailbox( NULL ),
 _p_mailboxMain( NULL )
{
    setupGui();
}

MailboxGui::~MailboxGui()
{
    if ( _p_mailboxMain )
        delete _p_mailboxMain;

    try
    {
        if ( _p_btnOpenMailbox )
            CEGUI::WindowManager::getSingleton().destroyWindow( _p_btnOpenMailbox );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "MailboxGui: problem destroying gui." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

void MailboxGui::setupGui()
{
    // create the main mailbox gui
    _p_mailboxMain = new MailboxGuiMain( this, MAILBOX_MAIN_GUI_LAYOUT );
    if ( _p_mailboxMain->getFrame() )
    {
        _p_mailboxMain->getFrame()->hide();
        _p_mailboxMain->getFrame()->deactivate();
    }
    else
    {
        log_warning << "mailbox gui disabled" << std::endl;
        return;
    }

    // setup the mailbox button
    try
    {
        // setup mailbox frame open button
        {
            _p_btnOpenMailbox = static_cast< CEGUI::PushButton* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Button", MAILBOX_MAIN_DLG_PREFIX "_btn_openbox_" ) );
            _p_btnOpenMailbox->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::MailboxGui::onClickedOpenMailbox, this ) );
            _p_btnOpenMailbox->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( &vrc::MailboxGui::onHoverOpenMailbox, this ) );
            _p_btnOpenMailbox->setStandardImageryEnabled( false );
            _p_btnOpenMailbox->setPosition( CEGUI::Point( 0.005f, 0.6f ) );
            // get the imageset with the images and set the button size
            CEGUI::Imageset* p_iset = vrc::gameutils::GuiUtils::get()->getCustomImageSet();
            float pixwidth  = p_iset->getImageWidth( IMAGE_NAME_MAILBOX_NORMAL );
            float pixheight = p_iset->getImageHeight( IMAGE_NAME_MAILBOX_NORMAL );
            // set button size according to the image dimensions
            _p_btnOpenMailbox->setSize( CEGUI::Absolute, CEGUI::Size( pixwidth, pixheight ) );

            // set editbox open button images
            const CEGUI::Image* p_image = vrc::gameutils::GuiUtils::get()->getCustomImage( IMAGE_NAME_MAILBOX_NORMAL );
            CEGUI::RenderableImage* p_rendImage = new CEGUI::RenderableImage;
            p_rendImage->setImage( p_image );
            _p_btnOpenMailbox->setPushedImage( p_rendImage );
            _p_btnOpenMailbox->setNormalImage( p_rendImage );
            delete p_rendImage;

            p_image = vrc::gameutils::GuiUtils::get()->getCustomImage( IMAGE_NAME_MAILBOX_HOOVER );
            p_rendImage = new CEGUI::RenderableImage;
            p_rendImage->setImage( p_image );
            _p_btnOpenMailbox->setHoverImage( p_rendImage );
            delete p_rendImage;

            // append the button into main window gui
            CEGUI::Window* p_maingui = gameutils::GuiUtils::get()->getMainGuiWindow();
            p_maingui->addChildWindow( _p_btnOpenMailbox );
        }
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "MailboxGui: cannot setup gui layout." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
        return;
    }
}

void MailboxGui::mainOnClose()
{
    // play click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

    _p_mailboxMain->getFrame()->hide();
    _p_btnOpenMailbox->enable();

    // unlock the player control
    unsigned int ctrlmodes = gameutils::PlayerUtils::get()->getPlayerControlModes();
    ctrlmodes &= ~( gameutils::PlayerUtils::eLockPicking | gameutils::PlayerUtils::eLockCameraSwitch | gameutils::PlayerUtils::eLockLooking | gameutils::PlayerUtils::eLockMovement );
    gameutils::PlayerUtils::get()->setPlayerControlModes( ctrlmodes );
}

bool MailboxGui::onClickedOpenMailbox( const CEGUI::EventArgs& /*arg*/ )
{
    if ( !_p_mailboxMain->getFrame() )
        return true;

    // play click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

    _p_mailboxMain->updateMailbox();
    _p_mailboxMain->getFrame()->show();

    _p_btnOpenMailbox->disable();

    return true;
}

bool MailboxGui::onHoverOpenMailbox( const CEGUI::EventArgs& /*arg*/ )
{
    // play hover sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_HOVER );
    return true;
}

} // namespace vrc
