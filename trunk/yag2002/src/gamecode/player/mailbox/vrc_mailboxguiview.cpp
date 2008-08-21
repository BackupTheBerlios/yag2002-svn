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
 # player's mail view gui implementation
 #
 #   date of creation:  01/08/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ****************************************************************/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_mailboxguimain.h"
#include "vrc_mailboxguiview.h"

//! Gui ressource related defines
#define MAILBOX_VIEW_DLG_PREFIX     "mailboxview_"


namespace vrc
{

MailboxGuiView::MailboxGuiView( MailboxGuiMain* p_mailbox, const std::string& filename ) :
 _p_mailboxGuiMain( p_mailbox ),
 _p_frame( NULL ),
 _p_editboxTo( NULL ),
 _p_editboxCC( NULL ),
 _p_editboxSubject( NULL ),
 _p_editboxBody( NULL )
{
    assert( _p_mailboxGuiMain && "invalid mailbox object!" );
    setupGui( filename );
}

MailboxGuiView::~MailboxGuiView()
{
    try
    {
        if ( _p_frame )
            CEGUI::WindowManager::getSingleton().destroyWindow( _p_frame );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "MailboxGuiView: problem destroying gui." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

CEGUI::FrameWindow* MailboxGuiView::getFrame()
{
    return _p_frame;
}

void MailboxGuiView::viewMail( unsigned int mailID )
{
    if ( !_p_frame )
        return;

    // request the mail client for the content, the response is delivered via the callback method 'mailboxResponse' below
    MailboxClient::get()->getMail( this, mailID );
}

void MailboxGuiView::setupGui( const std::string& filename )
{
    try
    {
        CEGUI::Window* p_maingui = gameutils::GuiUtils::get()->getMainGuiWindow();
        // load the gui file
        CEGUI::Window*      p_layout = yaf3d::GuiManager::get()->loadLayout( filename, p_maingui, MAILBOX_VIEW_DLG_PREFIX );
        CEGUI::FrameWindow* p_frame  = static_cast< CEGUI::FrameWindow* >( p_layout );
        if ( !p_frame )
        {
            log_error << "MailboxGuiView: could not load ressource file for mailbox window: " << std::endl;
            return;
        }

        // subscribe for getting on-close callbacks
        p_frame->subscribeEvent( CEGUI::FrameWindow::EventCloseClicked, CEGUI::Event::Subscriber( &vrc::MailboxGuiView::onClickedClose, this ) );
        CEGUI::PushButton* p_btnclose = static_cast< CEGUI::PushButton* >( p_frame->getChild( MAILBOX_VIEW_DLG_PREFIX "btn_close" ) );
        p_btnclose->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::MailboxGuiView::onClickedClose, this ) );

        _p_editboxTo      = static_cast< CEGUI::Editbox* >( p_frame->getChild( MAILBOX_VIEW_DLG_PREFIX "text_to" ) );
        _p_editboxCC      = static_cast< CEGUI::Editbox* >( p_frame->getChild( MAILBOX_VIEW_DLG_PREFIX "text_cc" ) );
        _p_editboxSubject = static_cast< CEGUI::Editbox* >( p_frame->getChild( MAILBOX_VIEW_DLG_PREFIX "text_subject" ) );
        _p_editboxBody    = static_cast< CEGUI::MultiLineEditbox* >( p_frame->getChild( MAILBOX_VIEW_DLG_PREFIX "text_body" ) );

        _p_frame = p_frame;
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "MailboxGuiView: cannot setup gui layout." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

void MailboxGuiView::mailboxResponse( const MailboxContent& mailcontent, unsigned int /*status*/, const std::string& /*response*/ )
{
    if ( !_p_frame )
        return;

    // clean the fields first
    _p_frame->setText( "" );
    _p_editboxTo->setText( "" );
    _p_editboxCC->setText( "" );
    _p_editboxSubject->setText( "" );
    _p_editboxBody->setText( "" );

    // after requesting for a mail body, this callback is used
    _p_frame->setText( "Mail from: " + mailcontent._header._from + ", received: " + mailcontent._header._date );
    // attention: currently, only subject and body can contain utf8 characters, but we handle 'to' and 'cc' also as they would contain utf8 (just for fun)
    _p_editboxTo->setText( reinterpret_cast< const CEGUI::utf8* >( mailcontent._header._to.c_str() ) );
    _p_editboxCC->setText( reinterpret_cast< const CEGUI::utf8* >( mailcontent._header._cc.c_str() ) );
    _p_editboxSubject->setText( reinterpret_cast< const CEGUI::utf8* >( mailcontent._header._subject.c_str() ) );
    _p_editboxBody->setText( reinterpret_cast< const CEGUI::utf8* >( mailcontent._body.c_str() ) );

    // now show up the view gui
    _p_frame->show();
}

unsigned int MailboxGuiView::mailboxGetUserID() const
{
    return _userID;
}

bool MailboxGuiView::onClickedClose( const CEGUI::EventArgs& /*arg*/ )
{
    // play click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

    _p_mailboxGuiMain->viewOnClose();

    return true;
}

} // namespace vrc
