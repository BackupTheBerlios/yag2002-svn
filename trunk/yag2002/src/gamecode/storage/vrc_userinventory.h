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

class UserInventory;
class StorageServer;
class StoragePostgreSQL;

//! Inventory item class
class InventoryItem
{
    public:

                                                    InventoryItem( const std::string& name );

                                                    ~InventoryItem();

        //! Get the item name
        inline const std::string&                   getName() const;

        //! Get the item count
        inline unsigned int                         getCount() const;

        //! Get the value of given parameter
        template< class Type >
        bool                                        getParamValue( const std::string& paramName, Type& value );

        //! Set the value of given parameter
        template< class Type >
        bool                                        setParamValue( const std::string& paramName, Type& value );

        //! Get the item parameters.
        inline std::map< std::string, std::string >& getParams();

    protected:

        //! Item name
        std::string                                 _name;

        //! Item count
        unsigned int                                _itemCount;

        //! Item's parameters as < param name, param value > map
        std::map< std::string, std::string >        _params;

   friend class UserInventory;
};


//! User inventory class
class UserInventory
{
    public:

        //! Type for item container
        typedef std::map< std::string /*item name*/, InventoryItem* >    Items;

    public:

        //! Create user inventory for user with given ID. The ID is used on server for finding users' inventory.
        explicit                                    UserInventory( unsigned int userID );

                                                    ~UserInventory();

        //! Get the user items
        Items&                                      getItems();

        //! Get item given its name. Returns NULL if no item with given name exists.
        InventoryItem*                              getItem( const std::string& itemName );

        //! Import the items from given the items in a packed string form.
        void                                        importItems( const std::string& data );

        //! Export the items to a packed string form and store the data in 'data'.
        void                                        exportItems( std::string& data );

        //! Add given item to inventory. The item parameters are packed in itemString.
        bool                                        addItem( const std::string& itemName, const std::string& itemString );

        //! Increase the count of item with given name by given count. Returns false if the item with given name does not exist in inventory.
        bool                                        increaseItem( const std::string& itemName, unsigned int count = 1 );

        //! decrease the count of item with given name by given count. Returns false if the item with given name does not exist in inventory.
        bool                                        decreaseItem( const std::string& itemName, unsigned int count = -1 );

        //! Remove the item with given name and ID
        bool                                        removeItem( const std::string& itemName );

    protected:

        //! Returns true if the inventory is cached in server
        inline bool                                 isCached() const;

        //! Set the cache flag
        inline void                                 setCached( bool cached );

        //! Invalidate the cache. If the inventory is not cached then the next request for inventory will fetch the data from database.
        inline void                                 invalidateCache();

        //! Remove all items
        void                                        removeAllItems();

        //! Unique user ID
        unsigned int                                _userID;

        //! Cache flag
        bool                                        _cached;

        //! User items
        Items                                       _items;

    friend class StorageServer;
    friend class StoragePostgreSQL;
};

//! Include the inline file
#include "vrc_userinventory.inl"

} // namespace vrc

#endif  // _VRC_USERINVENTORY_H_
