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
 # player's contact gui implementation
 #
 #   date of creation:  18/10/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ****************************************************************/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_mailboxguisend.h"
#include "vrc_mailboxguicontacts.h"

//! Gui ressource related defines
#define MAILBOX_CONTACTS_DLG_PREFIX     "mailboxcontacts_"


namespace vrc
{

MailboxGuiContacts::MailboxGuiContacts( MailboxGuiSend* p_mailboxsend, const std::string& filename ) :
 _p_mailboxGuiSend( p_mailboxsend ),
 _p_frame( NULL ),
 _showDetails( false ),
 _p_listContacts( NULL ),
 _p_btnDetails( NULL ),
 _p_btnAddTO( NULL ),
 _p_btnAddCC( NULL ),
 _p_editboxTo( NULL ),
 _p_editboxCC( NULL ),
 _p_detailsGui( NULL )
{
    assert( _p_mailboxGuiSend && "invalid mailbox object!" );
    setupGui( filename );
}

MailboxGuiContacts::~MailboxGuiContacts()
{
    try
    {
        if ( _p_frame )
            CEGUI::WindowManager::getSingleton().destroyWindow( _p_frame );

        if ( _p_detailsGui )
            delete _p_detailsGui;
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "MailboxGuiContacts: problem destroying gui." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

CEGUI::FrameWindow* MailboxGuiContacts::getFrame()
{
    return _p_frame;
}

void MailboxGuiContacts::viewContacts()
{
    if ( !_p_frame )
        return;

    _showDetails = false;

    if ( _p_detailsGui )
        _p_detailsGui->hide();

    // request for user's contacts
    StorageClient::get()->requestUserContacts( this );

    _p_frame->show();
}

void MailboxGuiContacts::setRecipients( const std::string& to, const std::string& cc )
{
    _p_editboxTo->setText( to );
    _p_editboxCC->setText( cc );
}

void MailboxGuiContacts::getRecipients( std::string& to, std::string& cc )
{
    to = _p_editboxTo->getText().c_str();
    cc = _p_editboxCC->getText().c_str();
}

void MailboxGuiContacts::setupGui( const std::string& filename )
{
    try
    {
        CEGUI::Window* p_maingui = gameutils::GuiUtils::get()->getMainGuiWindow();
        // load the gui file
        CEGUI::Window*      p_layout = yaf3d::GuiManager::get()->loadLayout( filename, p_maingui, MAILBOX_CONTACTS_DLG_PREFIX );
        CEGUI::FrameWindow* p_frame  = static_cast< CEGUI::FrameWindow* >( p_layout );
        if ( !p_frame )
        {
            log_error << "MailboxGuiContacts: could not load ressource file for contacts window: " << std::endl;
            return;
        }

        // subscribe callbacks
        p_frame->subscribeEvent( CEGUI::FrameWindow::EventCloseClicked, CEGUI::Event::Subscriber( &vrc::MailboxGuiContacts::onClickedCancel, this ) );

        CEGUI::PushButton* p_btndetails = static_cast< CEGUI::PushButton* >( p_frame->getChild( MAILBOX_CONTACTS_DLG_PREFIX "btn_details" ) );
        p_btndetails->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::MailboxGuiContacts::onClickedDetails, this ) );

        CEGUI::PushButton* p_btnok = static_cast< CEGUI::PushButton* >( p_frame->getChild( MAILBOX_CONTACTS_DLG_PREFIX "btn_ok" ) );
        p_btnok->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::MailboxGuiContacts::onClickedOk, this ) );

        CEGUI::PushButton* p_btncancel = static_cast< CEGUI::PushButton* >( p_frame->getChild( MAILBOX_CONTACTS_DLG_PREFIX "btn_cancel" ) );
        p_btncancel->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::MailboxGuiContacts::onClickedCancel, this ) );

        _p_listContacts = static_cast< CEGUI::Listbox* >( p_frame->getChild( MAILBOX_CONTACTS_DLG_PREFIX "list_contacts" ) );
        _p_listContacts->subscribeEvent( CEGUI::Listbox::EventSelectionChanged, CEGUI::Event::Subscriber( &vrc::MailboxGuiContacts::onListContactSelChanged, this ) );

        _p_btnAddTO = static_cast< CEGUI::PushButton* >( p_frame->getChild( MAILBOX_CONTACTS_DLG_PREFIX "btn_addto" ) );
        _p_btnAddTO->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::MailboxGuiContacts::onClickedAddTO, this ) );

        _p_btnAddCC              = static_cast< CEGUI::PushButton* >( p_frame->getChild( MAILBOX_CONTACTS_DLG_PREFIX "btn_addcc" ) );
        _p_btnAddCC->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::MailboxGuiContacts::onClickedAddCC, this ) );

        _p_btnDetails = static_cast< CEGUI::PushButton* >( p_frame->getChild( MAILBOX_CONTACTS_DLG_PREFIX "btn_details" ) );
        _p_editboxTo  = static_cast< CEGUI::Editbox* >( p_frame->getChild( MAILBOX_CONTACTS_DLG_PREFIX "text_to" ) );
        _p_editboxCC  = static_cast< CEGUI::Editbox* >( p_frame->getChild( MAILBOX_CONTACTS_DLG_PREFIX "text_cc" ) );

        // setup the details gui part
        CEGUI::StaticText* p_detailsWnd = static_cast< CEGUI::StaticText* >( p_frame->getChild( MAILBOX_CONTACTS_DLG_PREFIX "st_detailswindow" ) );
        _p_detailsGui = new ContactDetails( p_detailsWnd );

        _p_detailsGui->_p_editMemberSince  = static_cast< CEGUI::Editbox* >( p_detailsWnd->getChild( MAILBOX_CONTACTS_DLG_PREFIX "eb_memsince" ) );
        _p_detailsGui->_p_editStatus       = static_cast< CEGUI::Editbox* >( p_detailsWnd->getChild( MAILBOX_CONTACTS_DLG_PREFIX "eb_status" ) );
        _p_detailsGui->_p_cbOnline         = static_cast< CEGUI::Checkbox* >( p_detailsWnd->getChild( MAILBOX_CONTACTS_DLG_PREFIX "cb_online" ) );
        _p_detailsGui->_p_editABout        = static_cast< CEGUI::MultiLineEditbox* >( p_detailsWnd->getChild( MAILBOX_CONTACTS_DLG_PREFIX "eb_about" ) );

        _p_detailsGui->hide();

        // store the frame for later usage
        _p_frame = p_frame;
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "MailboxGuiContacts: cannot setup gui layout." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

void MailboxGuiContacts::contactsResult( bool success, const std::vector< std::string >& contacts )
{
    if ( !success )
    {
        log_error << "MailboxGuiContacts: error occured while getting contact data" << std::endl;
        return;
    }

    // fill up the list

    // set selection background color
    CEGUI::ColourRect col(
                            CEGUI::colour( 211.0f / 255.0f, 97.0f  / 255.0f, 0.0f, 0.8f ),
                            CEGUI::colour( 211.0f / 255.0f, 97.0f  / 255.0f, 0.0f, 0.8f ),
                            CEGUI::colour( 211.0f / 255.0f, 97.0f  / 255.0f, 0.0f, 0.8f ),
                            CEGUI::colour( 211.0f / 255.0f, 97.0f  / 255.0f, 0.0f, 0.8f )
                          );

    _p_listContacts->resetList();
    std::vector< std::string >::const_iterator p_name = contacts.begin(), p_end = contacts.end();
    for ( ; p_name != p_end; ++p_name )
    {
        CEGUI::ListboxTextItem * p_item = new CEGUI::ListboxTextItem( *p_name );
        p_item->setSelectionColours( col );
        p_item->setSelectionBrushImage( "TaharezLook", "ListboxSelectionBrush" );
        _p_listContacts->insertItem( p_item, NULL );
    }

    // select the first item
    if ( contacts.size() )
        _p_listContacts->setItemSelectState( static_cast< size_t >( 0 ), true );
}

void MailboxGuiContacts::accountInfoResult( tAccountInfoData& info )
{
    if ( !_p_detailsGui )
        return;

    std::string status, memsince;
    std::vector< std::string > fields;

    //! TODO: extracting user status and registration time out of the account data should be moved to a common place,
    //        atm, the player picker implements the same stuff redundantly.
    //##########
    yaf3d::explode( info._p_registrationDate, " ", &fields );
    if ( fields.size() > 0 )
        memsince = fields[ 0 ];

    fields.clear();
    yaf3d::explode( info._p_onlineTime, ":", &fields );
    if ( fields.size() > 0 )
    {
        // set the status depending on online time
        std::stringstream str;
        str << fields[ 0 ];
        unsigned int hours = 0;
        str >> hours;
        if ( hours < 1 )
            status = "Fresh meat";
        else if ( hours < 10 )
            status = "Knows some";
        else
            status = "Stone-Washed";
    }
    //##########

    _p_detailsGui->_p_editStatus->setText( status );
    _p_detailsGui->_p_editMemberSince->setText( memsince );

    _p_detailsGui->_p_editABout->setText( info._p_userDescription );

    if ( info._status & tAccountInfoData::eLoggedIn )
        _p_detailsGui->_p_cbOnline->setSelected( true );
    else
        _p_detailsGui->_p_cbOnline->setSelected( false );

}

bool MailboxGuiContacts::onClickedDetails( const CEGUI::EventArgs& /*arg*/ )
{
    // don't show up the details gui if no list elements exist
    if ( !_p_listContacts->getItemCount() && !_showDetails )
        return true;

    _showDetails = !_showDetails;
    _p_btnDetails->setText( _showDetails ? ">>" : "<<" );

    if ( _showDetails )
    {
        // first show the gui then select the first item for getting detailed info on selected contact
        _p_detailsGui->show();

        // select the first entry ( the gui must be visible in order the selection to work here! )
        CEGUI::EventArgs args;
        onListContactSelChanged( args );
    }
    else
    {
        _p_detailsGui->hide();
    }

    return true;
}

bool MailboxGuiContacts::onClickedAddTO( const CEGUI::EventArgs& /*arg*/ )
{
    CEGUI::ListboxItem* p_sel = _p_listContacts->getFirstSelectedItem();
    if ( !p_sel )
        return true;

    if ( !p_sel->getText().length() )
        return true;

    // avoid name duplicates in To and Cc fields
    if ( _p_editboxCC->getText().length() || _p_editboxTo->getText().length() )
    {
        std::vector< std::string > names;
        yaf3d::explode( _p_editboxTo->getText().c_str(), ",", &names );
        for ( std::size_t cnt = 0; cnt < names.size(); cnt++ )
            if ( p_sel->getText() == names[ cnt ] )
                return true;

        names.clear();
        yaf3d::explode( _p_editboxCC->getText().c_str(), ",", &names );
        for ( std::size_t cnt = 0; cnt < names.size(); cnt++ )
            if ( p_sel->getText() == names[ cnt ] )
                return true;
    }

    _p_editboxTo->setText( _p_editboxTo->getText() + ( _p_editboxTo->getText().length() ? "," : "" ) + p_sel->getText() );

    return true;
}

bool MailboxGuiContacts::onClickedAddCC( const CEGUI::EventArgs& /*arg*/ )
{
    CEGUI::ListboxItem* p_sel = _p_listContacts->getFirstSelectedItem();
    if ( !p_sel )
        return true;

    if ( !p_sel->getText().length() )
        return true;

    // avoid name duplicates
    if ( _p_editboxCC->getText().length() || _p_editboxTo->getText().length() )
    {
        std::vector< std::string > names;
        yaf3d::explode( _p_editboxCC->getText().c_str(), ",", &names );
        for ( std::size_t cnt = 0; cnt < names.size(); cnt++ )
            if ( p_sel->getText() == names[ cnt ] )
                return true;

        names.clear();
        yaf3d::explode( _p_editboxTo->getText().c_str(), ",", &names );
        for ( std::size_t cnt = 0; cnt < names.size(); cnt++ )
            if ( p_sel->getText() == names[ cnt ] )
                return true;
    }

    _p_editboxCC->setText( _p_editboxCC->getText() + ( _p_editboxCC->getText().length() ? "," : "" ) + p_sel->getText() );

    return true;
}

bool MailboxGuiContacts::onClickedOk( const CEGUI::EventArgs& /*arg*/ )
{
    _p_mailboxGuiSend->onCloseContacts( true );
    return true;
}

bool MailboxGuiContacts::onClickedCancel( const CEGUI::EventArgs& /*arg*/ )
{
    _p_mailboxGuiSend->onCloseContacts( false );
    return true;
}

bool MailboxGuiContacts::onListContactSelChanged( const CEGUI::EventArgs& /*arg*/ )
{
    if ( !_showDetails )
        return true;

    // clear the gui elements
    _p_detailsGui->_p_editStatus->setText( "" );
    _p_detailsGui->_p_editMemberSince->setText( "" );
    _p_detailsGui->_p_cbOnline->setSelected( false );
    _p_detailsGui->_p_editABout->setText( "" );

    // get selection
    CEGUI::ListboxItem* p_sel = _p_listContacts->getFirstSelectedItem();
    if ( !p_sel )
        return true;

    std::string sel = p_sel->getText().c_str();
    if ( !sel.length() )
        return true;

    // get the user contacts
    StorageClient::get()->requestPublicAccountInfo( sel, this );

    return true;
}

} // namespace vrc
