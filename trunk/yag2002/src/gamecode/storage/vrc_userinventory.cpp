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


//! Seperator character for parameter name/value formatting
#define VRC_PARAM_SEPERATOR     '='


namespace vrc
{

InventoryItem::InventoryItem( unsigned int itemID, const std::string& name ) :
 _itemID( itemID ),
 _name( name ),
 _itemCount( 1 )
{
}

InventoryItem::~InventoryItem()
{
}

// partial specialization of templated method for std::string
template<>
bool InventoryItem::getParamValue( const std::string& paramName, std::string& value )
{
    std::map< std::string, std::string >::iterator p_item;
    p_item = _params.find( paramName );
    if ( p_item == _params.end() )
    {
        log_error << "InventoryItem " << _name << ": invalid parameter name " << paramName << std::endl;
        return false;
    }

    value = p_item->second;

    return true;
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

std::vector< InventoryItem* >& UserInventory::getItems()
{
    return _items;
}

void UserInventory::removeAllItems()
{
    // delete the items in list
    std::vector< InventoryItem* >::iterator p_item = _items.begin(), p_end= _items.end();
    for ( ; p_item != p_end; ++p_item )
        delete ( *p_item );

    _items.clear();
}

bool UserInventory::addItem( const std::string& itemName, unsigned int itemID, const std::string& itemString )
{
    // prepare the inventory string for value extraction
    std::vector< std::string > fields;
    yaf3d::explode( itemString, ",", &fields );

    InventoryItem* p_item = new InventoryItem( itemID, itemName );

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
                if ( c != VRC_PARAM_SEPERATOR )
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
    _items.push_back( p_item );

    return true;
}

bool UserInventory::increaseItem( const std::string& itemName, unsigned int count )
{
    std::vector< InventoryItem* >::iterator p_item = _items.begin(), p_end= _items.end();
    for ( ; p_item != p_end; ++p_item )
    {
        InventoryItem* p_invitem = *p_item;
        if ( p_invitem->getName() == itemName )
        {
            p_invitem->_itemCount += count;
            return true;
        }
    }

    return false;
}

bool UserInventory::decreaseItem( const std::string& itemName, unsigned int count )
{
    std::vector< InventoryItem* >::iterator p_item = _items.begin(), p_end= _items.end();
    for ( ; p_item != p_end; ++p_item )
    {
        InventoryItem* p_invitem = *p_item;
        if ( p_invitem->getName() == itemName )
        {
            if ( int( p_invitem->_itemCount ) - int( count ) > 0 )
            {
                p_invitem->_itemCount -= count;
            }
            else
            {
                p_invitem->_itemCount = 0;
            }
            return true;
        }
    }

    return false;
}

bool UserInventory::removeItem( const std::string& itemName )
{
    std::vector< InventoryItem* >::iterator p_item = _items.begin(), p_end= _items.end();
    for ( ; p_item != p_end; ++p_item )
    {
        if ( ( *p_item )->getName() == itemName )
        {
            _items.erase( p_item );
            delete ( *p_item );
            return true;
        }
    }

    return false;
}

} // namespace vrc
