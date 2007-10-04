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
 # base class for storage types
 #
 #   date of creation:  09/30/2007
 #
 #   author:            mpretz 
 #
 ################################################################*/

#ifndef _VRC_BASESTORAGE_H_
#define _VRC_BASESTORAGE_H_

#include <vrc_main.h>
#include "vrc_connectiondata.h"

//! Database table names
#define TBL_NAME_USERACCOUNTS   "userAccounts" 
#define TBL_NAME_GAMEWORLD      "gameWorld" 
#define TBL_NAME_GAMEITEMS      "gameItems" 
#define TBL_NAME_USERINVENTORY  "userInventory" 


namespace vrc
{

class StorageServer;

class BaseStorage
{
    public:

        // mapping columns to his own values
        typedef std::map< std::string, std::string > col_pair;

                                                BaseStorage();

        virtual                                 ~BaseStorage();

        inline const std::string&               getType() const;

        // get data from storage, based on location and a set of constraints/conditions (AND conjunction)
        virtual std::vector< col_pair >         getData( const std::string&, const std::string&, const std::vector< std::string >* ) = 0;

        // set one data to storage, based on location and a set of constraints/conditions (AND conjunction)
        virtual bool                            setData( const std::string&, const std::string&, const std::vector< std::string >* ) = 0;

        inline ConnectionData*                  getConnectionData() const;

        inline void                             setConnectionData( const ConnectionData& connData );

    protected:

                                                BaseStorage( const BaseStorage& );

        BaseStorage&                            operator =( BaseStorage& );

        virtual bool                            initialize( const ConnectionData& connData ) = 0;

        virtual void                            release() = 0;

        virtual bool                            openConnection() = 0;

        virtual void                            closeConnection() = 0;

        ConnectionData*                         _p_connectionData;

        std::string                             _type;

    friend class StorageServer;
};

#include "vrc_basestorage.inl"

} // namespace vrc

#endif  // _VRC_BASESTORAGE_H_
