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

#ifndef _VRC_GAMEWORLD_H_
#define _VRC_GAMEWORLD_H_

#include <vrc_main.h>
#include "../vrc_gameitem.h"


#define NUMSLOTS 5

namespace vrc
{

class BaseStorage;

class GameWorld
{
    public:

        typedef std::vector< GameItem >         GameInventory;

        explicit                                GameWorld( BaseStorage* p_storage );

                                                ~GameWorld();

        // updating all game contents, like inventory, credits etc.
        // notice: it doesn't mean that storage is being updated
        // !!! needs storage access !!!
        bool                                    updateGameWorld();

        // !!! needs storage access !!!
        bool                                    addItem( const std::string& guid, unsigned int* slot = NULL );

        // !!! needs storage access !!!
        bool                                    removeItem( const std::string& guid, unsigned int* slot = NULL );

        GameInventory&                          getInventory();

        unsigned int                            getCredit();

        void                                    setUserId( unsigned int user_id );

        bool                                    isAdmin();

        bool                                    isInitialized();

    protected:

        BaseStorage*                            _p_storage;

        unsigned int                            _userID;

        unsigned int                            _inventoryID;

        bool                                    _admin;

        unsigned int                            _credits;

        // placeholder for all items of current user
        GameInventory                           _inventory;

        bool                                    _initialized;
};

} // namespace vrc

#endif  // _VRC_GAMEWORLD_H_
