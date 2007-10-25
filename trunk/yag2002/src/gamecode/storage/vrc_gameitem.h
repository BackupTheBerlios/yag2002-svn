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

#ifndef _VRC_GAMEITEM_H_
#define _VRC_GAMEITEM_H_


#include <vrc_main.h>

namespace vrc
{

class GameItem
{
    public:

                                                GameItem();

                                                GameItem( const std::string& guid, const std::string& name, unsigned int count = 0 );

                                                ~GameItem();

        //! Get item GUID
        const std::string&                      getGuid() const;

        //! Get item name
        const std::string&                      getName() const;

        //! Get item count
        unsigned int                            getCount() const;

        //! Increase item count by given amount
        void                                    increaseCount( unsigned int amount = 1 );

        //! decrease item count by given amount
        void                                    decreaseCount( unsigned int amount = 1 );

    protected:

        //! Set item GUID
        void                                    setGuid( const std::string& guid );

        //! Set item name
        void                                    setName( const std::string& name );

        //! Set item count
        void                                    setCount( unsigned int count );

        //! Global unique item identifier
        std::string                             _guid;

        //! Item name
        std::string                             _name;

        //! Count of game items
        unsigned int                            _count;
};

} // namespace vrc

#endif  // _VRC_GAMEITEM_H_
