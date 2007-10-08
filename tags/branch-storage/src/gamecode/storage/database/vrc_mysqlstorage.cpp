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

#include <vrc_main.h>
#include "vrc_mysqlstorage.h"


namespace vrc
{

MysqlStorage::MysqlStorage() :
 _p_databaseConnection( NULL ),
 _numOfQueryExecutes( 0 ),
 _numOfQueryStores( 0 )
{
}

MysqlStorage::~MysqlStorage()
{
    if ( _p_databaseConnection )
       release();
}

bool MysqlStorage::initialize( const ConnectionData& connData )
{
    if ( !_p_databaseConnection )
    {
        try
        {
            // mem leaks!?
            _p_databaseConnection = new mysqlpp::Connection( mysqlpp::use_exceptions );

            // massive mem leaks!?
            _p_databaseConnection->enable_ssl( "client-key.pem", "client-cert.pem", "cacert.pem" );

            setConnectionData( connData );

            return openConnection();
        }
        catch ( const mysqlpp::BadQuery& err )
        {
            log_error << "mySQL: " << err.what() << std::endl;
            // throw exception or return false
            return false;
        }
        catch ( const mysqlpp::BadConversion& err )
        {
            log_error << "mySQL: " << err.what() << std::endl;
            // throw exception or return false
            return false;
        }
        catch ( const mysqlpp::Exception& err )
        {
            log_error << "mySQL: " << err.what() << std::endl;
            // throw exception or return false
            return false;
        }
    }
    else
    {
        log_error << "cannot initialize MysqlStorage: connection to database already exists" << std::endl;
        return false;
    }

    return true;
}

void MysqlStorage::release()
{
    closeConnection();

    if ( _p_databaseConnection )
        delete _p_databaseConnection;

    _p_databaseConnection = NULL;
}

bool MysqlStorage::reconnect()
{
    closeConnection();

    return openConnection();
}

std::vector< BaseStorage::col_pair > MysqlStorage::getData( const std::string& data, const std::string& from, const std::vector< std::string >* p_cond )
{
    //! NOTE: this method can be used only when a connection to db exists!

    std::vector< BaseStorage::col_pair > content;
    mysqlpp::Result res;

    try
    {

        mysqlpp::Query query = _p_databaseConnection->query();

        query << "SELECT " << data << " FROM `" << _p_connectionData->_schema << "`.`" << _p_connectionData->_content[ from ] << ( ( !p_cond ) ? "" : ( "` WHERE " + makeConditionExpr( p_cond ) ) );

        res = query.store();
        ++_numOfQueryStores;
    }
    catch ( const mysqlpp::Exception& e )
    {
        log_error << "mySQL: " << e.what() << std::endl;
        return content;
    }

    //TODO: untersuchen, ob "res" NULL sein kann

    BaseStorage::col_pair values;
    unsigned int s = res.size();
    if ( s != 0 )
    {
        // get through all available rows in database
        for ( unsigned int r = 0; r < s; ++r )
        {
            // for current row, get all values of all columns stored in result
            for ( unsigned int c = 0; c < res.columns(); ++c )
                values[ res.field_name( c ) ] = ( std::string )( ( mysqlpp::Row )res.at( r ) ).at( c );

            content.push_back( values );
            values.clear();
        }
    }

    return content;
}

bool MysqlStorage::setData( const std::string& data, const std::string& from, const std::vector< std::string >* p_cond )
{
    try
    {

        // check if exists, then update otherwise insert
        mysqlpp::Query query = _p_databaseConnection->query();

        if ( p_cond )
            query << "UPDATE `" << _p_connectionData->_schema << "`.`" << _p_connectionData->_content[ from ] << "` SET " << data << ( ( !p_cond ) ? "" : ( " WHERE " + makeConditionExpr( p_cond ) ) );
        else
        {
            std::string::size_type pos = data.find_first_of( "=" );
            query << "INSERT INTO `" << _p_connectionData->_schema << "`.`" << _p_connectionData->_content[ from ] << "` (" << data.substr( 0, pos ) << ") VALUES(" << data.substr( pos + 1, data.length() ) << ")";
        }

        query.execute();
        ++_numOfQueryExecutes;
    }
    catch ( const mysqlpp::Exception& e )
    {
        log_error << "mySQL: " << e.what() << std::endl;
        return false;
    }

    return true;
}

bool MysqlStorage::openConnection()
{
    // check for valid connection data, otherwise we cannot do anything
    if ( _p_connectionData )
    {
        if ( !_p_databaseConnection->connect( _p_connectionData->_schema.c_str(), _p_connectionData->_server.c_str(),
                                              _p_connectionData->_user.c_str(), _p_connectionData->_passwd.c_str(),
                                              _p_connectionData->_port ) )
        {
            log_error << "cannot establish connection to database" << std::endl;
            return false;
        }
    }
    else
    {
        log_error << "cannot establish connection to database: no connection data available" << std::endl;
        return false;
    }

    return true;
}

void MysqlStorage::closeConnection()
{
    if ( !_p_databaseConnection )
    {
        // no connection available
        log_warning << "cannot close connection to database: no active connection available" << std::endl;
    }

    _p_databaseConnection->close();
}

std::string MysqlStorage::makeConditionExpr( const std::vector< std::string >* p_cond )
{
    if ( !p_cond )
        return ( "" );

    std::string str;
    std::vector< std::string >::const_iterator p_beg = p_cond->begin(), p_end = p_cond->end();

    for ( ; p_beg != p_end; ++p_beg )
    {
        str.append( *p_beg );

        if ( ( p_beg + 1 ) != p_end )
            str.append( " AND " );
    }

    return str;
}

} // namespace vrc
