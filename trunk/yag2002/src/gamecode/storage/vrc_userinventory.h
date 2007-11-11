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
 # user inventory
 #
 #   date of creation:  10/11/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _VRC_USERINVENTORY_H_
#define _VRC_USERINVENTORY_H_


#include <vrc_main.h>

namespace vrc
{

class StorageServer;
class StoragePostgreSQL;

//! Inventory item class
class InventoryItem
{
    public:

                                                    InventoryItem( unsigned int itemID, const std::string& name );

                                                    ~InventoryItem();

        //! Get the item name
        const std::string&                          getItemName() const;

        //! Get the value of given parameter
        template< class Type >
        bool                                        getParamValue( const std::string& paramName, Type& value );

        //! Get the item parameters.
        inline std::map< std::string, std::string >& getParams();

    protected:

        //! Global unique item identifier
        unsigned int                                _itemID;

        //! Item name
        std::string                                 _name;

        //! Item's parameters as < param name, param value > map
        std::map< std::string, std::string >        _params;
};


//! User inventory class
class UserInventory
{
    public:

        explicit                                    UserInventory( unsigned int userID );

                                                    ~UserInventory();

        //! Get the user items
        std::vector< InventoryItem* >&              getItems();

        //! Get item with given name
        InventoryItem*                              getItem( const std::string& itemName );

    protected:

        //! Returns true if the inventory is cached in server
        inline bool                                 isCached() const;

        //! Set the cache flag
        inline void                                 setCached( bool cached );

        //! Invalidate the cache. If the inventory is not cached then the next request for inventory will fetch the data from database.
        inline void                                 invalidateCache();

        //! Remove all items
        void                                        removeAllItems();

        //! Add given item to inventory. The item parameters are encrypted in itemString.
        bool                                        addItem( const std::string& itemName, unsigned int itemID, const std::string& itemString );

        //! Unique user ID
        unsigned int                                _userID;

        //! Cache flag
        bool                                        _cached;

        //! User items
        std::vector< InventoryItem* >               _items;

    friend class StorageServer;
    friend class StoragePostgreSQL;
};

//! Include the inline file
#include "vrc_userinventory.inl"

} // namespace vrc

#endif  // _VRC_USERINVENTORY_H_
