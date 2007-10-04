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
 # implementation of sql storage
 #
 #   date of creation:  09/30/2007
 #
 #   author:            mpretz
 #
 ################################################################*/

#ifndef _VRC_MYSQLSTORAGE_H_
#define _VRC_MYSQLSTORAGE_H_

#include <vrc_main.h>
#include "vrc_basestorage.h"
#include <mysql++.h>

namespace vrc
{

class MysqlStorage : public BaseStorage
{
    public:

                                                MysqlStorage();

        virtual                                 ~MysqlStorage();

        //TODO: überprüfen, ob diese methode wirklich benötigt wird
        bool                                    reconnect();

        virtual std::vector< col_pair >         getData( const std::string& data, const std::string& from, const std::vector< std::string >* p_cond );

        // if condition is NULL, an INSERT will be executed instead of UPDATE
        virtual bool                            setData( const std::string& data, const std::string& from, const std::vector< std::string >* p_cond );

    protected:

        virtual bool                            initialize( const ConnectionData& connData );

        virtual void                            release();

        bool                                    openConnection();

        void                                    closeConnection();

        std::string                             makeConditionExpr( const std::vector< std::string >* p_cond );

        mysqlpp::Connection*                    _p_databaseConnection;

        unsigned long                           _numOfQueryExecutes;

        unsigned long                           _numOfQueryStores;
};

} // namespace vrc

#endif  // _VRC_MYSQLSTORAGE_H_
