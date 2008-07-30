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
 # player's property gui implementation
 #
 #   date of creation:  12/11/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ****************************************************************/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_propgui.h"
#include "../../storage/vrc_userinventory.h"

#define PDLG_PREFIX                 "prop_"
#define PROPERTIES_GUI_LAYOUT       "gui/properties.xml"

namespace vrc
{

PropertyGui::PropertyGui( UserInventory* p_inv ) :
 _p_userInventory( p_inv ),
 _p_frame( NULL ),
 _p_btnOpen( NULL ),
 _p_listboxItems( NULL ),
 _p_imageItem( NULL ),
 _p_editboxItem( NULL ),
 _p_editboxNickName( NULL ),
 _p_editboxMemberSince( NULL ),
 _p_editboxOnlineTime( NULL ),
 _p_editboxStatus( NULL ),
 _p_editboxAboutMe( NULL )
{
    setupGui();
}

PropertyGui::~PropertyGui()
{
    try
    {
        if ( _p_frame )
            CEGUI::WindowManager::getSingleton().destroyWindow( _p_frame );

        if ( _p_btnOpen )
            CEGUI::WindowManager::getSingleton().destroyWindow( _p_btnOpen );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "PropertyGui: problem destroying gui." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

void PropertyGui::enable( bool en )
{
    if ( _p_frame )
    {
        if ( en )
        {
            _p_frame->activate();
        }
        else
        {
            _p_frame->deactivate();
        }
    }
}

void PropertyGui::setupGui()
{
    assert( _p_userInventory && "invalid user inventory!" );

    try
    {
        CEGUI::Window* p_maingui = gameutils::GuiUtils::get()->getMainGuiWindow();
        CEGUI::Window* p_layout  = yaf3d::GuiManager::get()->loadLayout( PROPERTIES_GUI_LAYOUT, p_maingui, PDLG_PREFIX );

        _p_frame = static_cast< CEGUI::FrameWindow* >( p_layout );
        if ( !_p_frame )
        {
            log_error << "PropertyGui: missing main gui frame" << std::endl;
            return;
        }

        // subscribe for getting on-close callbacks
        _p_frame->subscribeEvent( CEGUI::FrameWindow::EventCloseClicked, CEGUI::Event::Subscriber( &vrc::PropertyGui::onClickedClose, this ) );
        CEGUI::PushButton* p_btnclose = static_cast< CEGUI::PushButton* >( _p_frame->getChild( PDLG_PREFIX "btn_close" ) );
        p_btnclose->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::PropertyGui::onClickedClose, this ) );

        // setup property frame open button
        _p_btnOpen = static_cast< CEGUI::PushButton* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Button", PDLG_PREFIX "_btn_openbox_" ) );
        _p_btnOpen->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::PropertyGui::onClickedOpen, this ) );
        _p_btnOpen->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( &vrc::PropertyGui::onHoverOpen, this ) );
        _p_btnOpen->setStandardImageryEnabled( false );
        _p_btnOpen->setPosition( CEGUI::Point( 0.005f, 0.6f ) );
        // get the imageset with the images and set the button size
        CEGUI::Imageset* p_iset = vrc::gameutils::GuiUtils::get()->getCustomImageSet();
        float pixwidth  = p_iset->getImageWidth( IMAGE_NAME_INV_NORMAL );
        float pixheight = p_iset->getImageHeight( IMAGE_NAME_INV_NORMAL );
        // set button size according to the image dimensions
        _p_btnOpen->setSize( CEGUI::Absolute, CEGUI::Size( pixwidth, pixheight ) );

        p_maingui->addChildWindow( _p_btnOpen );

        // set editbox open button images
        const CEGUI::Image* p_image = vrc::gameutils::GuiUtils::get()->getCustomImage( IMAGE_NAME_INV_NORMAL );
        CEGUI::RenderableImage* p_rendImage = new CEGUI::RenderableImage;
        p_rendImage->setImage( p_image );
        _p_btnOpen->setPushedImage( p_rendImage );
        _p_btnOpen->setNormalImage( p_rendImage );
        delete p_rendImage;

        p_image = vrc::gameutils::GuiUtils::get()->getCustomImage( IMAGE_NAME_INV_HOOVER );
        p_rendImage = new CEGUI::RenderableImage;
        p_rendImage->setImage( p_image );
        _p_btnOpen->setHoverImage( p_rendImage );
        delete p_rendImage;

        // get and setup the item list
        CEGUI::TabControl* p_tabctrl = static_cast< CEGUI::TabControl* >( _p_frame->getChild( PDLG_PREFIX "tab_ctrl" ) );
        p_tabctrl->subscribeEvent( CEGUI::TabControl::EventSelectionChanged, CEGUI::Event::Subscriber( &vrc::PropertyGui::onTabChanged, this ) );

        // get content of pane Inventory
        //#############################
        {
            CEGUI::TabPane* p_paneInventory = static_cast< CEGUI::TabPane* >( p_tabctrl->getTabContents( PDLG_PREFIX "pane_inventory" ) );

            _p_listboxItems = static_cast< CEGUI::Listbox* >( p_paneInventory->getChild( PDLG_PREFIX "listbox_items" ) );
            _p_listboxItems->subscribeEvent( CEGUI::Listbox::EventSelectionChanged, CEGUI::Event::Subscriber( &vrc::PropertyGui::onItemSelChanged, this ) );

            _p_imageItem   = static_cast< CEGUI::StaticImage* >( p_paneInventory->getChild( PDLG_PREFIX "image_item" ) );
            _p_editboxItem = static_cast< CEGUI::MultiLineEditbox* >( p_paneInventory->getChild( PDLG_PREFIX "item_description" ) );

            CEGUI::PushButton* p_btnitemuse = static_cast< CEGUI::PushButton* >( p_paneInventory->getChild( PDLG_PREFIX "btn_item_use" ) );
            p_btnitemuse->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::PropertyGui::onClickedItemUse, this ) );

            CEGUI::PushButton* p_btnitemdrop = static_cast< CEGUI::PushButton* >( p_paneInventory->getChild( PDLG_PREFIX "btn_item_drop" ) );
            p_btnitemdrop->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::PropertyGui::onClickedItemDrop, this ) );
        }

        //! TODO: get content of pane Skills
        //#############################
        {
        }

        // get content of pane Profile
        //#############################
        {
            CEGUI::TabPane* p_paneProfile = static_cast< CEGUI::TabPane* >( p_tabctrl->getTabContents( PDLG_PREFIX "pane_profile" ) );

            _p_editboxNickName    = static_cast< CEGUI::Editbox* >( p_paneProfile->getChild( PDLG_PREFIX "eb_nickname" ) );

            _p_editboxMemberSince = static_cast< CEGUI::Editbox* >( p_paneProfile->getChild( PDLG_PREFIX "eb_membersince" ) );

            _p_editboxOnlineTime  = static_cast< CEGUI::Editbox* >( p_paneProfile->getChild( PDLG_PREFIX "eb_onlinetime" ) );

            _p_editboxStatus      = static_cast< CEGUI::Editbox* >( p_paneProfile->getChild( PDLG_PREFIX "eb_status" ) );

            _p_editboxAboutMe     = static_cast< CEGUI::MultiLineEditbox* >( p_paneProfile->getChild( PDLG_PREFIX "eb_aboutme" ) );

            CEGUI::PushButton* p_btprofileok = static_cast< CEGUI::PushButton* >( p_paneProfile->getChild( PDLG_PREFIX "btn_profile_ok" ) );
            p_btprofileok->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::PropertyGui::onClickedProfileOk, this ) );
        }
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "PropertyGui: cannot setup gui layout." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
        return;
    }

    // hide the inventory frame
    _p_frame->hide();
}

void PropertyGui::updateInventory()
{
    assert( _p_userInventory && "invalid user inventory!" );

    // just to be on the safe side
    if ( !_p_listboxItems )
        return;

    // reset the list
    _p_listboxItems->resetList();
    _p_listboxItems->setSortingEnabled( true );

    // set selection background color
    CEGUI::ColourRect col(
                            CEGUI::colour( 255.0f / 255.0f, 214.0f / 255.0f, 9.0f / 255.0f, 0.8f ),
                            CEGUI::colour( 12.0f  / 255.0f, 59.0f  / 255.0f, 0.0f         , 0.8f ),
                            CEGUI::colour( 255.0f / 255.0f, 214.0f / 255.0f, 9.0f / 255.0f, 0.8f ),
                            CEGUI::colour( 12.0f  / 255.0f, 59.0f  / 255.0f, 0.0f         , 0.8f )
                          );

    // fill up the list with user inventory items
    UserInventory::Items& invlist = _p_userInventory->getItems();
    UserInventory::Items::iterator p_invitem = invlist.begin(), p_end = invlist.end();
    CEGUI::ListboxTextItem * p_selitem = NULL;
    for ( ; p_invitem != p_end; ++p_invitem )
    {
        CEGUI::ListboxTextItem * p_item = new CEGUI::ListboxTextItem( p_invitem->first );
        p_item->setSelectionColours( col );
        p_item->setSelectionBrushImage( "TaharezLook", "ListboxSelectionBrush" );
        // set the inventory item object as list item object
        p_item->setUserData( p_invitem->second );
        _p_listboxItems->insertItem( p_item, NULL );
        // set the selection to first item in list
        if ( !p_selitem )
            p_selitem = p_item;
    }

    if ( p_selitem )
        _p_listboxItems->setItemSelectState( p_selitem, true );

    // update the description
    updateItemDescription();
}

void PropertyGui::updateItemDescription()
{
    std::string description;

    if ( !_p_listboxItems->getSelectedCount() )
    {
        // erase the description field
        _p_editboxItem->setText( description );
        return;
    }

    // extract the parameter name / value and fill the description field
    CEGUI::ListboxItem* p_selitem = _p_listboxItems->getNextSelected( NULL );
    InventoryItem* p_invitem = static_cast< InventoryItem* >( p_selitem->getUserData() );

    std::stringstream itemcnt;
    itemcnt << p_invitem->getCount();
    description += "Count:   " + itemcnt.str() + "\n";

    std::map< std::string, std::string >& params = p_invitem->getParams();
    std::map< std::string, std::string >::iterator p_param = params.begin(), p_end = params.end();
    for ( ; p_param != p_end; ++p_param )
    {
        description += p_param->first + ":   " + p_param->second + "\n";
    }

    _p_editboxItem->setText( description );
}

void PropertyGui::updateProfile()
{
    if ( !StorageClient::get()->requestAccountInfo( StorageClient::get()->getUserID(), this ) )
    {
        log_warning << "PropertyGui: cannot get account info" << std::endl;
    }
}

bool PropertyGui::onHoverOpen( const CEGUI::EventArgs& /*arg*/ )
{
    // play mouse over sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_HOVER );
    return true;
}

bool PropertyGui::onClickedOpen( const CEGUI::EventArgs& /*arg*/ )
{
    // check the player control mode, skip the open request if another gui is open
    unsigned int ctrlmodes = gameutils::PlayerUtils::get()->getPlayerControlModes();
    if ( ctrlmodes & gameutils::PlayerUtils::eLockLooking )
        return true;

    // update inventory
    updateInventory();

    // update profile
    updateProfile();

    // center to screen
    _p_frame->setPosition( CEGUI::Point( 0.25f, 0.2f ) );
    _p_frame->show();
    _p_btnOpen->hide();

    // lock the player control
    ctrlmodes |= ( gameutils::PlayerUtils::eLockPicking | gameutils::PlayerUtils::eLockCameraSwitch | gameutils::PlayerUtils::eLockLooking | gameutils::PlayerUtils::eLockMovement );
    gameutils::PlayerUtils::get()->setPlayerControlModes( ctrlmodes );

    return true;
}

bool PropertyGui::onClickedClose( const CEGUI::EventArgs& /*arg*/ )
{
    _p_frame->hide();
    _p_btnOpen->show();

    // unlock the player control
    unsigned int ctrlmodes = gameutils::PlayerUtils::get()->getPlayerControlModes();
    ctrlmodes &= ~( gameutils::PlayerUtils::eLockPicking | gameutils::PlayerUtils::eLockCameraSwitch | gameutils::PlayerUtils::eLockLooking | gameutils::PlayerUtils::eLockMovement );
    gameutils::PlayerUtils::get()->setPlayerControlModes( ctrlmodes );

    return true;
}

bool PropertyGui::onTabChanged( const CEGUI::EventArgs& /*arg*/ )
{
    // play mouse click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );
    return true;
}

bool PropertyGui::onItemSelChanged( const CEGUI::EventArgs& /*arg*/ )
{
    // play mouse click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

    // update inventory
    updateItemDescription();

    return true;
}

bool PropertyGui::onClickedItemUse( const CEGUI::EventArgs& /*arg*/ )
{
    // play mouse click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );


    // TODO: ...


    return true;
}

bool PropertyGui::onClickedItemDrop( const CEGUI::EventArgs& /*arg*/ )
{
    // play mouse click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );


    // TODO: ...


    return true;
}

bool PropertyGui::onClickedProfileOk( const CEGUI::EventArgs& /*arg*/ )
{
    // play mouse click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

    assert( _p_editboxAboutMe );

    // update the user description on server
    tAccountInfoData info;
    memset( &info, 0, sizeof( info ) );
    info._userID = StorageClient::get()->getUserID();
    strcpy_s( info._p_userDescription, sizeof( info._p_userDescription ) - 1, _p_editboxAboutMe->getText().c_str() );
    info._p_userDescription[ sizeof( info._p_userDescription ) - 1 ] = 0;

    if ( !StorageClient::get()->updateAccountInfo( info ) )
    {
        log_warning << "PropertyGui: cannot update account info" << std::endl;
    }

    return true;
}

void PropertyGui::accountInfoResult( tAccountInfoData& info )
{
    assert( _p_editboxNickName );
    assert( _p_editboxMemberSince );
    assert( _p_editboxOnlineTime );
    assert( _p_editboxAboutMe );

    // format the online time and registration date
    std::vector< std::string > fields;
    std::string regdate, onlinetime, status;

    yaf3d::explode( info._p_onlineTime, ":", &fields );
    if ( fields.size() > 0 )
    {
        onlinetime += fields[ 0 ];

        // set the status depending on online time
        std::stringstream str;
        str << fields[ 0 ];
        unsigned int hours = 0;
        str >> hours;
        if ( hours < 1 )
        {
            status = "Fresh meat";
        }
        else if ( hours < 10 )
        {
            status = "Knows some";
        }
        else
        {
            status = "Stone-Washed";
        }
    }
    if ( fields.size() > 1 )
    {
        onlinetime += ":" + fields[ 1 ];
    }

    fields.clear();
    yaf3d::explode( info._p_registrationDate, " ", &fields );
    if ( fields.size() > 0 )
        regdate = fields[ 0 ];

    //! TODO: there are also other priviledge flags such as admin, moderator, banned etc.!
    //if ( info._priviledges )
    //    status = "Moderator";

    _p_editboxNickName->setText( info._p_nickName );
    _p_editboxMemberSince->setText( regdate );
    _p_editboxOnlineTime->setText( onlinetime );
    _p_editboxStatus->setText( status );
    _p_editboxAboutMe->setText( info._p_userDescription );
}

} // namespace vrc
