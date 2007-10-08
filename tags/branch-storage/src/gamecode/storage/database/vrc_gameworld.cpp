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
 # class for handling gameworld requests using the storage
 #
 #   date of creation:  10/08/2007
 #
 #   author:            mpretz
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_gameworld.h"
#include "vrc_basestorage.h"


namespace vrc
{

GameWorld::GameWorld( BaseStorage* p_storage ) :
_p_storage( p_storage ),
_userID( 0 ),
_inventoryID( 0 ),
_admin( false ),
_credits( 0 ),
_initialized( false )
{
}

GameWorld::~GameWorld()
{
}

bool GameWorld::updateGameWorld()
{
    std::stringstream str;
    str << _userID;

    std::vector< std::string > cond;
    cond.push_back( "user_id=" +str.str() );
    std::vector< BaseStorage::col_pair > data = _p_storage->getData( "*", TBL_NAME_GAMEWORLD, &cond );

    if ( data.size() > 1 )    // !!! THIS CASE IS NOT ALLOWED !!!
    {
        // more than one user found with the same user id
        log_error << "GameWorld: semantic error in database (more than one user has been found)" << std::endl;

        return false;
    }
    else if ( data.size() == 0 )
        // the user has no game world information
        return true;

    // fill game world information of current user
    _admin       = ( data.at( 0 )[ "admin" ] == "1" );
    _credits     = ( unsigned int )atoi( data.at( 0 )[ "credits" ].c_str() );
    _inventoryID = ( unsigned int )atoi( data.at( 0 )[ "inventory_id" ].c_str() );

    // get all items in current user's inventory
    cond.clear();
    cond.push_back( "id=" + data.at( 0 )[ "inventory_id" ] );
    data = _p_storage->getData( "*", TBL_NAME_USERINVENTORY, &cond );

    for ( std::size_t i = 0; i < NUMSLOTS; ++i )
    {
        // before we update current user's inventory, we must check if item still exists
        cond.clear();
        std::stringstream strslot;
        strslot << "slot" << i + 1;
        cond.push_back( "guid='" + data.at( 0 )[ strslot.str() ] + "'" );
        std::vector< BaseStorage::col_pair > tmp = _p_storage->getData( "*", TBL_NAME_GAMEITEMS, &cond );

        if ( tmp.size() != 0 )
        {
            std::stringstream strcount;
            strcount << "count" << i + 1;
            // item has been found in database, add to user inventory
            _inventory[ i ] = GameItem( tmp.at( 0 )[ "guid" ], tmp.at( 0 )[ "name" ], ( unsigned int )atoi( data.at( 0 )[ strcount.str() ].c_str() ) );
        }
    }

    return true;
}

// we don't need to check if item-guid exists in the inventory
// because adding items are always done on existing inventory
bool GameWorld::addItem( const std::string& guid, unsigned int* slot )
{
    char buffer[ 65 ];
    std::vector< std::string > cond;

    _itoa_s( _inventoryID, buffer, 65, 10 );

    // find item in inventory, if it exists increase counter and
    // update database, otherwise add item to inventory

    GameInventory::iterator p_beg = _inventory.begin(), p_end = _inventory.end();

    for ( std::size_t i = 1; p_beg != p_end; ++p_beg, ++i )
    {
        if ( guid.compare( p_beg->getGuid() ) == 0 )
        {
            // item has been found: increase counter and update database
            p_beg->increaseCount();

            cond.push_back( "id=" + std::string( buffer ) );
            //TODO: sprintf auflösen
            sprintf( buffer, "count%d=%d", i, p_beg->getCount() );

            if ( !_p_storage->setData( buffer, TBL_NAME_USERINVENTORY, &cond ) )
            {
                log_error << "GameWorld: could not increase counter for current item (" << p_beg->getName() << " with " << cond.at( 0 ) << ")" << std::endl;
                return false;
            }

            *slot = i;
            return true;
        }
    }

    // in case of no item in inventory, add item to inventory and database
    // only when there's an empty slot in user's inventory

    // find an empty slot in inventory
    p_beg = _inventory.begin();

    for ( std::size_t i = 1; p_beg != p_end; ++p_beg, ++i )
    {
        if ( p_beg->getGuid().empty() )
        {
            // find name of item in database and create new item
            cond.push_back( "guid='" + guid + "'" );
            std::vector< BaseStorage::col_pair > data = _p_storage->getData( "name", TBL_NAME_GAMEITEMS, &cond );

            _inventory[ i - 1 ] = GameItem( guid, data.at( 0 )[ "name" ].c_str(), 1 );

            cond.clear();
            cond.push_back( "id=" + std::string( buffer ) );
            //TODO: sprintf auflösen
            sprintf( buffer, "slot%d='%s',count%d=1", i, guid.c_str(), i );

            if ( !_p_storage->setData( buffer, TBL_NAME_USERINVENTORY, &cond ) )
            {
                log_error << "GameWorld: could not add item (" << p_beg->getName() << " with " << cond.at( 0 ) << ") to storage" << std::endl;
                return false;
            }

            *slot = i;
            return true;
        }
    }

    log_error << "GameWorld: no empty slot in inventory available" << std::endl;
    return false;
}

// we don't need to check if item-guid exists in the inventory
// because removing items are always done on existing inventory
bool GameWorld::removeItem( const std::string& guid, unsigned int* slot )
{
    char buffer[ 65 ];
    std::vector< std::string > cond;

    _itoa_s( _inventoryID, buffer, 65, 10 );
    cond.push_back( "id=" + std::string( buffer ) );

    // find item in inventory, if there's more than one item
    // increase the counter and update database, otherwise remove
    // item completely from inventory

    GameInventory::iterator p_beg = _inventory.begin(), p_end = _inventory.end();

    for ( std::size_t i = 1; p_beg != p_end; ++p_beg, ++i )
    {
        if ( guid.compare( p_beg->getGuid() ) == 0 )
        {
            // item has been found: if only one item exists, then remove from inventory
            if ( p_beg->getCount() == 1 )
            {
                // updating database by resetting to default-value
                //TODO: sprintf auflösen
                sprintf( buffer, "slot%d=NULL,count%d=NULL", i, i );

                if ( !_p_storage->setData( buffer, TBL_NAME_USERINVENTORY, &cond ) )
                {
                    log_error << "GameWorld: could not remove current item (" << p_beg->getName() << " with " << cond.at( 0 ) << ") from storage" << std::endl;
                    return false;
                }

                // removing item from internal inventory
                *p_beg = GameItem();

                *slot = i;
                break;
            }
            else
            {
                // decreasing only the counter of current item (game world and database)
                p_beg->decreaseCount();

                //TODO: sprintf auflösen
                sprintf( buffer, "count%d=%d", i, p_beg->getCount() );

                if ( !_p_storage->setData( buffer, TBL_NAME_USERINVENTORY, &cond ) )
                {
                    log_error << "GameWorld: could not decrease counter for current item (" << p_beg->getName() << " with " << cond.at( 0 ) << ")" << std::endl;
                    return false;
                }

                *slot = i;
                break;
            }
        }
    }

    return true;
}

GameWorld::GameInventory& GameWorld::getInventory()
{
    return _inventory;
}

unsigned int GameWorld::getCredit()
{
    return _credits;
}

void GameWorld::setUserId( unsigned int userID )
{
    _userID = userID;
}

bool GameWorld::isAdmin()
{
    return _admin;
}

bool GameWorld::isInitialized()
{
    return _initialized;
}

} // namespace vrc
