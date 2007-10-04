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
 # flyweight for storage connection data
 #
 #   date of creation:  09/30/2007
 #
 #   author:            mpretz 
 #
 ################################################################*/

#ifndef _VRC_CONNECTIONDATA_H_
#define _VRC_CONNECTIONDATA_H_


#include <vrc_main.h>

namespace vrc
{

class ConnectionData
{
    public:

                                                    ConnectionData() :
                                                    _port( 0 )
                                                    {}

        virtual                                     ~ConnectionData() {}


        // server connection for database storages only
        std::string                                 _server;

        // server port for database storages only
        unsigned int                                _port;

        // name of database inside the database storage-system
        std::string                                 _schema;

        // authentication information for database storages only
        std::string                                 _user;

        // authentication information for database storages only
        std::string                                 _passwd;

        // e.g. mapping game content on database tables or xml-files
        std::map< std::string, std::string >        _content;
};

} // namespace vrc

#endif  // _VRC_CONNECTIONDATA_H_
