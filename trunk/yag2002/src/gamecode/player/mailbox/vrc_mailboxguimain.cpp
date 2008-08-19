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
 # player's main mailbox gui implementation
 #
 #   date of creation:  01/08/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 ****************************************************************/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_mailboxgui.h"
#include "vrc_mailboxguimain.h"
#include "vrc_mailboxguiview.h"
#include "vrc_mailboxguisend.h"
#include <storage/vrc_mailboxclient.h>

//! Gui ressource related defines
#define MAILBOX_MAIN_DLG_PREFIX         "mailboxmain_"
#define MAILBOX_VIEW_GUI_LAYOUT         "gui/mailview.xml"
#define MAILBOX_SEND_GUI_LAYOUT         "gui/mailsend.xml"

#ifdef WIN32
    #define VOID_NUM    long long
#else
    #define VOID_NUM    long int
#endif

namespace vrc
{

MailboxGuiMain::MailboxGuiMain( MailboxGui* p_mailbox, const std::string& filename ) : 
 _p_mailboxGui( p_mailbox ),
 _p_mailboxGuiView( NULL ),
 _p_mailboxGuiSend( NULL ),
 _p_frame( NULL ),
 _p_listFolders( NULL ),
 _p_listHeaders( NULL )
{
    assert( _p_mailboxGui && "invalid mailbox object!" );
    setupGui( filename );
}

MailboxGuiMain::~MailboxGuiMain()
{
    try
    {
        if ( _p_frame )
            CEGUI::WindowManager::getSingleton().destroyWindow( _p_frame );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "MailboxGuiMain: problem destroying gui." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }

    if ( _p_mailboxGuiView )
        delete _p_mailboxGuiView;

    if ( _p_mailboxGuiSend )
        delete _p_mailboxGuiSend;
}

CEGUI::FrameWindow* MailboxGuiMain::getFrame()
{
    return _p_frame;
}

void MailboxGuiMain::updateMailbox()
{
    // send out the request for getting mailbox foders
    MailboxClient::get()->getMailFolders( this );
}

void MailboxGuiMain::setupGui( const std::string& filename )
{
    _p_mailboxGuiView = new MailboxGuiView( this, MAILBOX_VIEW_GUI_LAYOUT );
    _p_mailboxGuiSend = new MailboxGuiSend( this, MAILBOX_SEND_GUI_LAYOUT );

    try
    {
        CEGUI::Window* p_maingui = gameutils::GuiUtils::get()->getMainGuiWindow();
        // load the gui file
        CEGUI::Window*      p_layout = yaf3d::GuiManager::get()->loadLayout( filename, p_maingui, MAILBOX_MAIN_DLG_PREFIX );
        _p_frame  = static_cast< CEGUI::FrameWindow* >( p_layout );
        if ( !_p_frame )
        {
            log_error << "MailboxGuiMain: could not load ressource file for mailbox window: " << std::endl;
            return;
        }

        // subscribe for getting on-close callbacks
        _p_frame->subscribeEvent( CEGUI::FrameWindow::EventCloseClicked, CEGUI::Event::Subscriber( &vrc::MailboxGuiMain::onClickedClose, this ) );

        // get the listbox for mail folders
        _p_listFolders = static_cast< CEGUI::Listbox* >( _p_frame->getChild( MAILBOX_MAIN_DLG_PREFIX "list_folders" ) );
        _p_listFolders->subscribeEvent( CEGUI::Listbox::EventSelectionChanged, CEGUI::Event::Subscriber( &vrc::MailboxGuiMain::onFolderSelChanged, this ) );


        // subscribe for getting button send callback
        CEGUI::PushButton* p_btnsend = static_cast< CEGUI::PushButton* >( _p_frame->getChild( MAILBOX_MAIN_DLG_PREFIX "btn_newmail" ) );
        p_btnsend->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::MailboxGuiMain::onClickedSend, this ) );

        // subscribe for getting button update mailbox callback
        CEGUI::PushButton* p_btnupdate = static_cast< CEGUI::PushButton* >( _p_frame->getChild( MAILBOX_MAIN_DLG_PREFIX "btn_update" ) );
        p_btnupdate->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::MailboxGuiMain::onClickedUpdate, this ) );

        // get the listbox for mail headers
        _p_listHeaders = static_cast< CEGUI::Listbox* >( _p_frame->getChild( MAILBOX_MAIN_DLG_PREFIX "list_mails" ) );

        // subscribe for getting button view callback
        CEGUI::PushButton* p_btnview = static_cast< CEGUI::PushButton* >( _p_frame->getChild( MAILBOX_MAIN_DLG_PREFIX "btn_view" ) );
        p_btnview->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::MailboxGuiMain::onClickedView, this ) );

        CEGUI::PushButton* p_btnreply = static_cast< CEGUI::PushButton* >( _p_frame->getChild( MAILBOX_MAIN_DLG_PREFIX "btn_reply" ) );
        p_btnreply->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::MailboxGuiMain::onClickedReply, this ) );

        // subscribe for getting button update mailbox callback
        CEGUI::PushButton* p_btndelete = static_cast< CEGUI::PushButton* >( _p_frame->getChild( MAILBOX_MAIN_DLG_PREFIX "btn_delete" ) );
        p_btndelete->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::MailboxGuiMain::onClickedDelete, this ) );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "MailboxGuiMain: cannot setup gui layout." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }

    _p_mailboxGuiView->getFrame()->hide();
    _p_mailboxGuiSend->getFrame()->hide();
}

void MailboxGuiMain::mailboxResponse( const MailboxContent& mailcontent, unsigned int status, const std::string& response )
{
    // check for errors
    if ( status & MailboxContent::eError )
    {
        yaf3d::MessageBoxDialog* p_msg = new yaf3d::MessageBoxDialog( "Attention", "Problem occurred during mail operation:\n" + response, yaf3d::MessageBoxDialog::OK, true );
        p_msg->show();
        return;
    }

    switch ( status )
    {
        case MailboxContent::eRecvFolders: 
        {
            _p_listFolders->resetList();
            _p_listFolders->setSortingEnabled( true );

            // set selection background color
            CEGUI::ColourRect col(
                                    CEGUI::colour( 128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, 0.8f ),
                                    CEGUI::colour( 128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, 0.8f ),
                                    CEGUI::colour( 128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, 0.8f ),
                                    CEGUI::colour( 128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, 0.8f )
                                  );

            std::vector< std::string >::const_iterator p_folder = mailcontent._folders.begin(), p_end = mailcontent._folders.end();
            for ( ; p_folder != p_end; ++p_folder )
            {
                CEGUI::ListboxTextItem * p_item = new CEGUI::ListboxTextItem( *p_folder );
                p_item->setSelectionColours( col );
                p_item->setSelectionBrushImage( "TaharezLook", "ListboxSelectionBrush" );
                _p_listFolders->insertItem( p_item, NULL );
            }
            // select the first mail in list
            if ( mailcontent._folders.size() )
            {
                _p_listFolders->setItemSelectState( static_cast< std::size_t >( 0 ), true );
                // now update the mail headers
                MailboxClient::get()->getMailHeaders( this, MailboxHeader::eAll, mailcontent._folders[ 0 ] );
            }
        }
        break;

        case MailboxContent::eRecvHeaders:
        {
            // reset the list
            _p_listHeaders->resetList();

//! TODO: sorting is in wrong order, fix it. we have to provide an own sorting function
            _p_listHeaders->setSortingEnabled( true );

            // set selection background color
            CEGUI::ColourRect col(
                                    CEGUI::colour( 128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, 0.8f ),
                                    CEGUI::colour( 128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, 0.8f ),
                                    CEGUI::colour( 128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, 0.8f ),
                                    CEGUI::colour( 128.0f / 255.0f, 128.0f / 255.0f, 128.0f / 255.0f, 0.8f )
                                  );

            std::vector< MailboxHeader >::const_iterator p_header = mailcontent._headers.begin(), p_end = mailcontent._headers.end();
            for ( ; p_header != p_end; ++p_header )
            {
                // set the mail attributes
                std::string attr;
                if ( p_header->_attributes & MailboxHeader::ePriorityHigh )
                    attr += "!";
                if ( p_header->_attributes & MailboxHeader::eNotRead )
                    attr += "new";
                attr = attr.length() ? "(" + attr + ")," : "";

                std::string timestamp;
                std::vector< std::string > tfields;
                yaf3d::explode( p_header->_date, ".", &tfields );
                if ( tfields.size() > 1 )
                    timestamp = tfields[ 0 ];
                else
                    timestamp = p_header->_date;

                // the subject can contain utf8 characters!
                CEGUI::String subject( reinterpret_cast< const CEGUI::utf8* >( p_header->_subject.c_str() ) );
                CEGUI::String str =  attr + "[" + timestamp + "] " + p_header->_from + ", " + subject;
                CEGUI::ListboxTextItem * p_item = new CEGUI::ListboxTextItem( str );
                p_item->setSelectionColours( col );
                p_item->setSelectionBrushImage( "TaharezLook", "ListboxSelectionBrush" );
                // set the inventory item object as list item object
                p_item->setUserData( reinterpret_cast< void* >( static_cast< VOID_NUM >( p_header->_id ) ) );
                _p_listHeaders->insertItem( p_item, NULL );
            }
            // select the first mail in list
            if ( mailcontent._headers.size() )
            {
                _p_listHeaders->setItemSelectState( static_cast< std::size_t >( 0 ), true );
            }
        }
        break;

        case MailboxContent::eRecvMail/*eRecvReply*/:
        {
            _p_mailboxGuiSend->replyMail( mailcontent );
            _p_mailboxGuiSend->getFrame()->show();
            _p_frame->hide();
        }
        break;

        default:
            ;
    }
}

unsigned int MailboxGuiMain::mailboxGetUserID() const
{
    return _userID;
}

bool MailboxGuiMain::onClickedClose( const CEGUI::EventArgs& /*arg*/ )
{
    // play click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

    _p_mailboxGui->mainOnClose();

    return true;
}

bool MailboxGuiMain::onClickedSend( const CEGUI::EventArgs& /*arg*/ )
{
    // play click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

    _p_mailboxGuiSend->sendMail();
    _p_frame->hide();

    return true;
}

bool MailboxGuiMain::onFolderSelChanged( const CEGUI::EventArgs& arg )
{
    // play click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

    onClickedUpdate( arg );

    return true;
}
bool MailboxGuiMain::onClickedUpdate( const CEGUI::EventArgs& /*arg*/ )
{
    if ( !_p_listHeaders || !_p_listFolders || !_p_listFolders->getSelectedCount() )
        return true;

    // send out the request for getting mail headers if a valid folder name has been selected
    CEGUI::ListboxItem* p_selitem = _p_listFolders->getNextSelected( NULL );
    if ( p_selitem->getText().length() )
        MailboxClient::get()->getMailHeaders( this, MailboxHeader::eAll, p_selitem->getText().c_str() );

    return true;
}

bool MailboxGuiMain::onClickedView( const CEGUI::EventArgs& /*arg*/ )
{
    if ( !_p_listHeaders->getSelectedCount() )
    {
        yaf3d::MessageBoxDialog* p_msg = new yaf3d::MessageBoxDialog( "", "First select a mail in list for viewing.", yaf3d::MessageBoxDialog::OK, true );
        p_msg->show();
        return true;
    }

    // play click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

    // pick the selected mail in list and view the mail
    // get the selected mail id
    CEGUI::ListboxItem* p_selitem = _p_listHeaders->getNextSelected( NULL );
    unsigned int id = static_cast< unsigned int >( reinterpret_cast< VOID_NUM >( p_selitem->getUserData() ) );
    _p_mailboxGuiView->viewMail( id );
    _p_frame->hide();

    return true;
}

bool MailboxGuiMain::onClickedReply( const CEGUI::EventArgs& /*arg*/ )
{
    if ( !_p_listHeaders->getSelectedCount() )
    {
        yaf3d::MessageBoxDialog* p_msg = new yaf3d::MessageBoxDialog( "", "First select a mail in list for viewing.", yaf3d::MessageBoxDialog::OK, true );
        p_msg->show();
        return true;
    }

    // play click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

    // pick the selected mail in list and view the mail
    // get the selected mail id
    CEGUI::ListboxItem* p_selitem = _p_listHeaders->getNextSelected( NULL );
    unsigned int id = static_cast< unsigned int >( reinterpret_cast< VOID_NUM >( p_selitem->getUserData() ) );

    // first we retrieve the content, then send a reply
    MailboxClient::get()->getMail( this, id );

    return true;
}

bool MailboxGuiMain::onClickedDelete( const CEGUI::EventArgs& /*arg*/ )
{
    //! TODO ...

    yaf3d::MessageBoxDialog* p_msg = new yaf3d::MessageBoxDialog( "", "DELETE under construction", yaf3d::MessageBoxDialog::OK, true );
    p_msg->show();

    return true;
}

void MailboxGuiMain::viewOnClose()
{
    _p_mailboxGuiView->getFrame()->hide();

    _p_frame->enable();
    _p_frame->show();
}

void MailboxGuiMain::sendOnClose()
{
    _p_mailboxGuiSend->getFrame()->hide();

    _p_frame->enable();
    _p_frame->show();
}

} // namespace vrc
