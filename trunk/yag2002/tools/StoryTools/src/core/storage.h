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
 # io manager
 #
 #   date of creation:  12/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _STORGE_H_
#define _STORGE_H_

#include <main.h>
#include <core/node.h>

namespace beditor
{

//! Class for storage related exceptions
class StorageException : public std::exception
{
    public:
                                                StorageException( const std::string& reason ) :
                                                 std::exception( reason.c_str() )
                                                {
                                                }

        virtual                                 ~StorageException() throw() {}

                                                StorageException( const StorageException& e ) :
                                                 std::exception( e )
                                                {
                                                }

    protected:

                                                StorageException();

        StorageException&                       operator = ( const StorageException& );
};


//! IO Storage
class Storage : public Singleton< Storage >
{
    public:

        //! Read from given file and construct the node hierarchy of stories; throws StorageException exceptions.
        void                                   read( const std::string& filename, std::vector< BaseNodePtr >& stories ) throw( ... );

        //! Write out the given stories; throws StorageException exceptions.
        void                                    write( const std::string& filename, std::vector< BaseNodePtr >& stories ) throw( ... );

    protected:

                                                Storage();

        virtual                                 ~Storage();

        //! Initialize the singleton
        void                                    initialize() throw( ... );

        // Shutdown the singleton
        void                                    shutdown();

    friend class Core;
    friend class Singleton< Storage >;
};

} // namespace beditor

#endif // _STORGE_H_
