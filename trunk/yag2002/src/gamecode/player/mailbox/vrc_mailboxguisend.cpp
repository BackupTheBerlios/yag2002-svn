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
 # player's mail send gui implementation
 #
 #   date of creation:  01/08/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 ****************************************************************/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_mailboxguimain.h"
#include "vrc_mailboxguisend.h"

//! Gui ressource related defines
#define MAILBOX_SEND_DLG_PREFIX     "mailboxsend_"


namespace vrc
{

MailboxGuiSend::MailboxGuiSend( MailboxGuiMain* p_mailbox, const std::string& filename ) : 
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

MailboxGuiSend::~MailboxGuiSend()
{
    try
    {
        if ( _p_frame )
            CEGUI::WindowManager::getSingleton().destroyWindow( _p_frame );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "MailboxGuiSend: problem destroying gui." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

CEGUI::FrameWindow* MailboxGuiSend::getFrame()
{
    return _p_frame;
}

void MailboxGuiSend::sendMail()
{
    if ( !_p_frame )
        return;

    _p_editboxTo->setText( "" );
    _p_editboxCC->setText( "" );
    _p_editboxSubject->setText( "" );
    _p_editboxBody->setText( "" );

    _p_frame->show();
}

void MailboxGuiSend::replyMail( const MailboxContent& content )
{
    if ( !_p_frame )
        return;

    _p_editboxTo->setText( content._header._from );
    _p_editboxCC->setText( content._header._cc );
    _p_editboxSubject->setText( "Re: " + content._header._subject );

    std::string replybody( "\n\n" );
    assembleMailbody( content, replybody, true );

    _p_editboxBody->setText( replybody );
    _p_editboxBody->ensureCaratIsVisible();
    _p_editboxBody->activate();
}

void MailboxGuiSend::forwardMail( const MailboxContent& content )
{
    if ( !_p_frame )
        return;

    _p_editboxTo->setText( "" );
    _p_editboxCC->setText( "" );
    _p_editboxSubject->setText( "Fw: " + content._header._subject );

    std::string replybody( "\n\n" );
    assembleMailbody( content, replybody, false );

    _p_editboxBody->setText( replybody );
    _p_editboxBody->ensureCaratIsVisible();
    _p_editboxBody->activate();
}

void MailboxGuiSend::assembleMailbody( const MailboxContent& content, std::string& body, bool reply )
{
    body += "# --- Original Mail ---#\n";
    body += "> Date: " + content._header._date + "\n";
    body += "> From: " + content._header._from + "\n";
    body += "> To: " + content._header._to + "\n";
    body += "> Copy: " + content._header._cc + "\n";
    body += "> Subject: " + content._header._subject + "\n";
    body += "> Body:\n";

    if ( reply )
    {
        body += "> ";
        std::string::size_type len = content._body.length();
        for ( std::string::size_type cnt = 0; cnt < len; cnt++ )
        {
            if ( content._body[ cnt ] == 10 )
                body += "\n> ";
            else
                body += content._body[ cnt ];
        }
    }
    else // forward
    {
        body += content._body;
    }
}

void MailboxGuiSend::setupGui( const std::string& filename )
{
    try
    {
        CEGUI::Window* p_maingui = gameutils::GuiUtils::get()->getMainGuiWindow();
        // load the gui file
        CEGUI::Window*      p_layout = yaf3d::GuiManager::get()->loadLayout( filename, p_maingui, MAILBOX_SEND_DLG_PREFIX );
        CEGUI::FrameWindow* p_frame  = static_cast< CEGUI::FrameWindow* >( p_layout );
        if ( !p_frame )
        {
            log_error << "MailboxGuiSend: could not load ressource file for mailbox window: " << std::endl;
            return;
        }

        // subscribe for getting on-close callbacks
        p_frame->subscribeEvent( CEGUI::FrameWindow::EventCloseClicked, CEGUI::Event::Subscriber( &vrc::MailboxGuiSend::onClickedCancelSend, this ) );
        CEGUI::PushButton* p_btnclose = static_cast< CEGUI::PushButton* >( p_frame->getChild( MAILBOX_SEND_DLG_PREFIX "btn_cancel" ) );
        p_btnclose->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::MailboxGuiSend::onClickedCancelSend, this ) );

        CEGUI::PushButton* p_btnsend = static_cast< CEGUI::PushButton* >( p_frame->getChild( MAILBOX_SEND_DLG_PREFIX "btn_send" ) );
        p_btnsend->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::MailboxGuiSend::onClickedSend, this ) );

        _p_editboxTo      = static_cast< CEGUI::Editbox* >( p_frame->getChild( MAILBOX_SEND_DLG_PREFIX "text_to" ) );
        _p_editboxCC      = static_cast< CEGUI::Editbox* >( p_frame->getChild( MAILBOX_SEND_DLG_PREFIX "text_cc" ) );
        _p_editboxSubject = static_cast< CEGUI::Editbox* >( p_frame->getChild( MAILBOX_SEND_DLG_PREFIX "text_subject" ) );
        _p_editboxBody    = static_cast< CEGUI::MultiLineEditbox* >( p_frame->getChild( MAILBOX_SEND_DLG_PREFIX "text_body" ) );

        _p_frame = p_frame;
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "MailboxGuiSend: cannot setup gui layout." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

void MailboxGuiSend::mailboxResponse( const MailboxContent& mailcontent, unsigned int status, const std::string& response )
{
    if ( !_p_frame ) 
        return;

    if ( status & MailboxContent::eError )
    {
        yaf3d::MessageBoxDialog* p_msg = new yaf3d::MessageBoxDialog( "Attention", "Problem occurred during mail operation:\n" + response, yaf3d::MessageBoxDialog::OK, true );
        p_msg->show();
        return;
    }
}

unsigned int MailboxGuiSend::mailboxGetUserID() const
{
    return _userID;
}

bool MailboxGuiSend::onClickedSend( const CEGUI::EventArgs& /*arg*/ )
{
    if ( !_p_frame )
        return true;

    // do some input checks
    if ( !_p_editboxTo->getText().length() )
    {
        yaf3d::MessageBoxDialog* p_msg = new yaf3d::MessageBoxDialog( "Attention", "You must fill the To field.", yaf3d::MessageBoxDialog::YES_NO, true );
        p_msg->show();
        return true;
    }

    if ( !_p_editboxSubject->getText().length() )
    {
        yaf3d::MessageBoxDialog* p_msg = new yaf3d::MessageBoxDialog( "Attention", "You must fill the Subject field.", yaf3d::MessageBoxDialog::YES_NO, true );
        p_msg->show();
        return true;
    }

    // send out the mail
    MailboxContent content;

    content._header._attributes = MailboxHeader::ePriorityNormal;
    content._header._to         = _p_editboxTo->getText().c_str();

    if ( _p_editboxCC->getText().length() )
        content._header._cc = _p_editboxCC->getText().c_str();

    if ( _p_editboxSubject->getText().length() )
        content._header._subject = _p_editboxSubject->getText().c_str();

    if ( _p_editboxBody->getText().length() )
        content._body = _p_editboxBody->getText().c_str();

    MailboxClient::get()->sendMail( this, content );

    _p_mailboxGuiMain->sendOnClose();

    return true;
}

bool MailboxGuiSend::onClickedCancelSend( const CEGUI::EventArgs& /*arg*/ )
{
    // play attention sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_ATTENTION );

    // open a messagebox and let the user confirm the abort
    {
        yaf3d::MessageBoxDialog* p_msg = new yaf3d::MessageBoxDialog( "", "You really want to cancel this mail?", yaf3d::MessageBoxDialog::YES_NO, true );

        // create a call back for yes/no buttons of messagebox
        class MsgYesNoClick: public yaf3d::MessageBoxDialog::ClickCallback
        {
            public:

                explicit                MsgYesNoClick( MailboxGuiSend* p_boxgui ) : _p_boxgui( p_boxgui ) {}

                virtual                 ~MsgYesNoClick() {}

                void                    onClicked( unsigned int btnId )
                                        {
                                            // did the user clicked yes? if so then store settings
                                            if ( btnId == yaf3d::MessageBoxDialog::BTN_YES )
                                            {
                                                _p_boxgui->_p_mailboxGuiMain->sendOnClose();
                                            }

                                            _p_boxgui->_p_frame->enable();
                                        }

                MailboxGuiSend*         _p_boxgui;
        };
        p_msg->setClickCallback( new MsgYesNoClick( this ) );
        p_msg->show();
        _p_frame->disable();
    }

    return true;
}

} // namespace vrc
