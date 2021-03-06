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

#ifndef _VRC_PROPGUI_H_
#define _VRC_PROPGUI_H_

#include <vrc_main.h>


namespace vrc
{

class UserInventory;

//! Class for player's property gui
class PropertyGui
{
    public:

        explicit                                    PropertyGui( UserInventory* p_inv );

        virtual                                     ~PropertyGui();

        //! Enable/disable the GUI. Disable the GUI when entering the menu, and enable it when leaving the menu
        void                                        enable( bool en );

    protected:

        //! Setup the GUI
        void                                        setupGui();

        //! Update the inventory display
        void                                        updateInventory();

        //! Update the item description
        void                                        updateItemDescription();

        //! Callback for mouse over open button
        bool                                        onHoverOpen( const CEGUI::EventArgs& arg );

        //! Callback for click on open button
        bool                                        onClickedOpen( const CEGUI::EventArgs& arg );

        //! Callback for on-close event
        bool                                        onClickedClose( const CEGUI::EventArgs& arg );

        //! Callback for tab-changed event
        bool                                        onTabChanged( const CEGUI::EventArgs& arg );

        //! Callback for inventory item selection change
        bool                                        onItemSelChanged( const CEGUI::EventArgs& arg );

        //! Callback for inventory item use button
        bool                                        onClickedItemUse( const CEGUI::EventArgs& arg );

        //! Callback for inventory item drop button
        bool                                        onClickedItemDrop( const CEGUI::EventArgs& arg );

        //! User inventory
        UserInventory*                              _p_userInventory;

        //! Frame object
        CEGUI::FrameWindow*                         _p_frame;

        //! Open GUI button
        CEGUI::PushButton*                          _p_btnOpen;

        //! Inventory pane: item list
        CEGUI::Listbox*                             _p_listboxItems;

        //! Inventory pane: image of selected item
        CEGUI::StaticImage*                         _p_imageItem;

        //! Inventory pane: description field
        CEGUI::MultiLineEditbox*                    _p_editboxItem;
};

} // namespace vrc

#endif // _VRC_PROPGUI_H_
