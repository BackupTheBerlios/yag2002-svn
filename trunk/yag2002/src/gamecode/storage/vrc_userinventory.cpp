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

#include <vrc_main.h>
#include "vrc_userinventory.h"


//! Packed item format specific defines
#define VRC_INV_ITEM_SEPARATOR              "@"
#define VRC_INV_ITEM_PARAM_SEPARATOR        ","
#define VRC_INV_ITEM_PARAM_VAL_SEPERATOR    '='


namespace vrc
{

InventoryItem::InventoryItem( const std::string& name ) :
 _name( name ),
 _itemCount( 1 )
{
}

InventoryItem::~InventoryItem()
{
}

UserInventory::UserInventory( unsigned int userID ) :
 _userID( userID ),
 _cached( false )
{
}

UserInventory::~UserInventory()
{
    // clear the item list
    removeAllItems();
}

UserInventory::Items& UserInventory::getItems()
{
    return _items;
}

void UserInventory::removeAllItems()
{
    // delete the items in list
    Items::iterator p_item = _items.begin(), p_end= _items.end();
    for ( ; p_item != p_end; ++p_item )
        delete p_item->second;

    _items.clear();
}

InventoryItem* UserInventory::getItem( const std::string& itemName )
{
    if ( _items.find( itemName ) == _items.end() )
        return NULL;

    return _items.find( itemName )->second;
}

void UserInventory::importItems( const std::string& data )
{
    std::vector< std::string > packeditems;
    yaf3d::explode( data, VRC_INV_ITEM_SEPARATOR, &packeditems );
    std::vector< std::string >::iterator p_packeditem = packeditems.begin(), p_packeditemEnd = packeditems.end();

    // go through all items and add them to inventory
    for ( ; p_packeditem != p_packeditemEnd; ++p_packeditem )
    {
        std::string& itemname = *p_packeditem;
        std::string::size_type pos = p_packeditem->find( VRC_INV_ITEM_PARAM_SEPARATOR );
        if ( ( pos == std::string::npos ) || !pos )
            continue;

        std::string itemprops = itemname.substr( pos );
        addItem( itemname.substr( 0, pos ), itemprops );
    }
}

void UserInventory::exportItems( std::string& data )
{
    data = "";

    Items::iterator p_item = _items.begin(), p_end= _items.end();
    for ( ; p_item != p_end; ++p_item )
    {
        InventoryItem* p_invitem = p_item->second;

        // item name
        data += p_invitem->getName() + VRC_INV_ITEM_PARAM_VAL_SEPERATOR;

        // item properties
        std::map< std::string, std::string >::const_iterator p_param = p_invitem->getParams().begin(), p_end = p_invitem->getParams().end();
        for ( ; p_param != p_end; ++ p_param )
            data += p_param->first + VRC_INV_ITEM_PARAM_VAL_SEPERATOR + p_param->second + VRC_INV_ITEM_PARAM_VAL_SEPERATOR;

        data += VRC_INV_ITEM_SEPARATOR;
    }
}

bool UserInventory::addItem( const std::string& itemName, const std::string& itemString )
{
    if ( _items.find( itemName ) != _items.end() )
    {
        log_error << "UserInventory: error in user inventory data, multiple items with the same name detected" << std::endl;
        return false;
    }

    // prepare the inventory string for value extraction
    std::vector< std::string > fields;
    yaf3d::explode( itemString, VRC_INV_ITEM_PARAM_SEPARATOR, &fields );

    InventoryItem* p_item = new InventoryItem( itemName );

    // fill in the parameter name and values
    std::vector< std::string >::iterator p_param = fields.begin(), p_end = fields.end();
    for ( ; p_param != p_end; ++p_param )
    {
        // parse in the param name / param value
        std::string token;
        std::string value;
        bool parsetoken = true;
        unsigned int len = static_cast< unsigned int >( p_param->length() );
        for ( unsigned int cnt = 0; cnt < len; cnt++ )
        {
            char c = ( *p_param )[ cnt ];
            if ( parsetoken )
            {
                if ( c != VRC_INV_ITEM_PARAM_VAL_SEPERATOR )
                    token += c;
                else
                    parsetoken = false;
            }
            else
            {
                value += c;
            }
        }
        // param name/value is stored as string pairs
        p_item->getParams()[ token ] = value;
    }

    // add the item to inventory
    _items[ itemName ] = p_item;

    return true;
}

bool UserInventory::increaseItem( const std::string& itemName, unsigned int count )
{
    if ( _items.find( itemName ) == _items.end() )
    {
        log_warning << "UserInventory: could not find item for increasing" << std::endl;
        return false;
    }

    InventoryItem* p_invitem = _items[ itemName ];
    p_invitem->_itemCount += count;

    return true;
}

bool UserInventory::decreaseItem( const std::string& itemName, unsigned int count )
{
    if ( _items.find( itemName ) == _items.end() )
    {
        log_warning << "UserInventory: could not find item for decreasing" << std::endl;
        return false;
    }

    InventoryItem* p_invitem = _items[ itemName ];
    p_invitem->_itemCount -= count;

    return true;
}

bool UserInventory::removeItem( const std::string& itemName )
{
    if ( _items.find( itemName ) == _items.end() )
    {
        log_warning << "UserInventory: could not find item for removing" << std::endl;
        return false;
    }

    InventoryItem* p_invitem = _items[ itemName ];
    delete p_invitem;

    _items.erase( _items.find( itemName ) );

    return true;
}

} // namespace vrc
