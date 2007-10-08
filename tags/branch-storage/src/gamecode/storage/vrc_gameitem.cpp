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
 # class defining a game item
 #
 #   date of creation:  10/08/2007
 #
 #   author:            mpretz
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_gameitem.h"


namespace vrc
{

GameItem::GameItem() :
_guid( "" ),
_name( "" ),
_count( 0 )
{
}

GameItem::GameItem( const std::string& guid, const std::string& name, unsigned int count ) :
_guid( guid ),
_name( name ),
_count( count )
{
}

GameItem::~GameItem()
{
}

const std::string& GameItem::getGuid() const
{
    return _guid;
}

const std::string& GameItem::getName() const
{
    return _name;
}

unsigned int GameItem::getCount() const
{
    return _count;
}

void GameItem::setGuid( const std::string& guid )
{
    _guid = guid;
}

void GameItem::setName( const std::string& name )
{
    _name = name;
}

void GameItem::setCount( unsigned int count )
{
    _count = count;
}

void GameItem::increaseCount( unsigned int amount )
{
    _count += amount;
}

void GameItem::decreaseCount( unsigned int amount )
{
    if ( static_cast< int >( _count ) - static_cast< int >( amount ) > 0 )
        _count -= amount;
    else
        _count = 0;
}

} // namespace vrc
