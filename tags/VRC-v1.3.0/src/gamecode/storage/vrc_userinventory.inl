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
#error "do not include this file directly, include vrc_userinventory.h instead"
#endif


inline const std::string& InventoryItem::getName() const
{
    return _name;
}

inline unsigned int InventoryItem::getID() const
{
    return _itemID;
}

inline unsigned int InventoryItem::getCount() const
{
    return _itemCount;
}

inline std::map< std::string, std::string >& InventoryItem::getParams()
{
    return _params;
}

template< class Type >
bool InventoryItem::getParamValue( const std::string& paramName, Type& value )
{
    std::map< std::string, std::string >::iterator p_item;
    p_item = _params.find( paramName );
    if ( p_item == _params.end() )
    {
        log_error << "InventoryItem " << _name << ": invalid parameter name " << paramName << std::endl;
        return false;
    }

    std::string val = p_item->second;
    if ( typeid( Type ) == typeid( int ) )
    {
        sscanf_s( val.c_str(), "%d", &value, val.length() );
    }
    else if ( typeid( Type ) == typeid( unsigned int ) )
    {
        sscanf_s( val.c_str(), "%u", &value, val.length() );
    }
    else if ( typeid( Type ) == typeid( float ) )
    {
        sscanf_s( val.c_str(), "%f", &value, val.length() );
    }

    return true;
}

bool UserInventory::isCached() const
{
    return _cached;
}

void UserInventory::setCached( bool cached )
{
    _cached = cached;
}


void UserInventory::invalidateCache()
{
    removeAllItems();
    _cached = false;
}
