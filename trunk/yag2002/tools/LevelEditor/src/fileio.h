/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2009, A. Botorabi
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
 # serialization functionality
 #
 #   date of creation:  02/03/2009
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _FILEIO_H_
#define _FILEIO_H_

#include <vrc_main.h>


//! Class for writing a level to file.
class FileOutputLevel
{
    public:

                                                        FileOutputLevel();

        virtual                                         ~FileOutputLevel();

        //! Output the given entities to a yaf3d level file. if 'writehidden' is true then hidden entities are written, too.
        //! NOTE: hidded entities are marked as such with instance name beginning with and underline ('_').
        bool                                            write( const std::vector< yaf3d::BaseEntity* >& level,
                                                               const std::string& filename,
                                                               const std::string& levelname,
                                                               bool writehidden = false );

    protected:

        //! Write a single entity.
        std::string                                     writeEntity( yaf3d::BaseEntity* p_entity );
};

#endif // _FILEIO_H_
