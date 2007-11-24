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
 _p_mainWnd( NULL ),
 _p_frame( NULL ),
 _p_btnOpen( NULL ),
 _p_listboxItems( NULL ),
 _p_imageItem( NULL ),
 _p_editboxItem( NULL )
{
    setupGui();
}

PropertyGui::~PropertyGui()
{
    try
    {
        if ( _p_mainWnd )
            CEGUI::WindowManager::getSingleton().destroyWindow( _p_mainWnd );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "PropertyGui: problem destroying gui." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

void PropertyGui::enable( bool en )
{
    if ( _p_mainWnd )
    {
        if ( en )
        {
            _p_mainWnd->activate();
            _p_mainWnd->show();
        }
        else
        {
            _p_mainWnd->deactivate();
            _p_mainWnd->hide();
        }
    }
}

void PropertyGui::setupGui()
{
    assert( _p_userInventory && "invalid user inventory!" );

    _p_mainWnd = yaf3d::GuiManager::get()->loadLayout( PROPERTIES_GUI_LAYOUT, NULL, PDLG_PREFIX );
    if ( !_p_mainWnd )
    {
        log_error << "PropertyGui: cannot find layout: " << PROPERTIES_GUI_LAYOUT << std::endl;
        return;
    }

    try
    {
        _p_frame = static_cast< CEGUI::FrameWindow* >( _p_mainWnd->getChild( PDLG_PREFIX "frame" ) );

        // subscribe for getting on-close callbacks
        _p_frame->subscribeEvent( CEGUI::FrameWindow::EventCloseClicked, CEGUI::Event::Subscriber( &vrc::PropertyGui::onClickedClose, this ) );
        CEGUI::PushButton* p_btnclose = static_cast< CEGUI::PushButton* >( _p_frame->getChild( PDLG_PREFIX "btn_close" ) );
        p_btnclose->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::PropertyGui::onClickedClose, this ) );

        // setup property frame open button
        _p_btnOpen = static_cast< CEGUI::PushButton* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Button", PDLG_PREFIX "_btn_openbox_" ) );
        _p_btnOpen->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::PropertyGui::onClickedOpen, this ) );
        _p_btnOpen->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( &vrc::PropertyGui::onHoverOpen, this ) );
        _p_btnOpen->setStandardImageryEnabled( false );
        _p_btnOpen->setPosition( CEGUI::Point( 0.0f, 0.6f ) );
        _p_btnOpen->setSize( CEGUI::Size( 0.08f, 0.1f ) );
        _p_mainWnd->addChildWindow( _p_btnOpen );

        // set editbox open button images
        const CEGUI::Image* p_image = vrc::gameutils::GuiUtils::get()->getCustomImage( IMAGE_NAME_HAND_NORMAL ); //! TODO: take a proper image
        CEGUI::RenderableImage* p_rendImage = new CEGUI::RenderableImage;
        p_rendImage->setImage( p_image );
        _p_btnOpen->setPushedImage( p_rendImage );
        _p_btnOpen->setNormalImage( p_rendImage );
        delete p_rendImage;

        p_image = vrc::gameutils::GuiUtils::get()->getCustomImage( IMAGE_NAME_HAND_HOOVER ); //! TODO: take a proper image
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
            CEGUI::TabPane*    p_paneInventory = static_cast< CEGUI::TabPane* >( p_tabctrl->getTabContents( PDLG_PREFIX "pane_inventory" ) );

            _p_listboxItems = static_cast< CEGUI::Listbox* >( p_paneInventory->getChild( PDLG_PREFIX "listbox_items" ) );
            _p_listboxItems->subscribeEvent( CEGUI::Listbox::EventSelectionChanged, CEGUI::Event::Subscriber( &vrc::PropertyGui::onItemSelChanged, this ) );
            
            _p_imageItem    = static_cast< CEGUI::StaticImage* >( p_paneInventory->getChild( PDLG_PREFIX "image_item" ) );
            _p_editboxItem   = static_cast< CEGUI::MultiLineEditbox* >( p_paneInventory->getChild( PDLG_PREFIX "item_description" ) );

            CEGUI::PushButton* p_btnitemuse = static_cast< CEGUI::PushButton* >( p_paneInventory->getChild( PDLG_PREFIX "btn_item_use" ) );
            p_btnitemuse->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::PropertyGui::onClickedItemUse, this ) );

            CEGUI::PushButton* p_btnitemdrop = static_cast< CEGUI::PushButton* >( p_paneInventory->getChild( PDLG_PREFIX "btn_item_drop" ) );
            p_btnitemdrop->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::PropertyGui::onClickedItemDrop, this ) );
        }
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "PropertyGui: cannot setup gui layout." << std::endl;
        log_out << "      reason: " << e.getMessage().c_str() << std::endl;

        _p_mainWnd->hide();
        _p_mainWnd->deactivate();

        return;
    }

    _p_mainWnd->setAlwaysOnTop( false );
    _p_mainWnd->activate();
    _p_mainWnd->show();
    _p_frame->hide();

    // update the inventory pane
    updateInventory();
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
    std::vector< InventoryItem* >& invlist = _p_userInventory->getItems();
    std::vector< InventoryItem* >::iterator p_invitem = invlist.begin(), p_end = invlist.end();
    CEGUI::ListboxTextItem * p_selitem = NULL;
    for ( ; p_invitem != p_end; ++p_invitem )
    {
        CEGUI::ListboxTextItem * p_item = new CEGUI::ListboxTextItem( ( *p_invitem )->getName() );
        p_item->setSelectionColours( col );
        p_item->setSelectionBrushImage( "TaharezLook", "ListboxSelectionBrush" );
        // set the inventory item object as list item object
        p_item->setUserData( *p_invitem );
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

bool PropertyGui::onHoverOpen( const CEGUI::EventArgs& /*arg*/ )
{
    // play mouse over sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_HOVER );
    return true;
}

bool PropertyGui::onClickedOpen( const CEGUI::EventArgs& /*arg*/ )
{
    // update inventory
    updateInventory();
    // center to screen
    _p_frame->setPosition( CEGUI::Point( 0.25f, 0.2f ) );
    _p_frame->show();
    _p_btnOpen->hide();

    // set the player interaction mode, thus avoid input processing of player keys
    gameutils::PlayerUtils::get()->setLockInteraction( true );

    return true;
}

bool PropertyGui::onClickedClose( const CEGUI::EventArgs& /*arg*/ )
{
    _p_frame->hide();
    _p_btnOpen->show();

    // remove the player interaction mode, thus allow input processing of player keys again
    gameutils::PlayerUtils::get()->setLockInteraction( false );

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
} // namespace vrc
