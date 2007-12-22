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
 # all stories are hold in this singleton
 #
 #   date of creation:  13/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _STORIES_H_
#define _STORIES_H_

#include <main.h>
#include <core/storage.h>
#include "node.h"

namespace beditor
{

//! Story manager loads/stores stories
class Stories : public Singleton< Stories >
{
    public:

        //! Load stories from file; throws StorageException if something goes wrong with loading.
        void                                    load( const std::string& filename ) throw( ... );

        //! Store stories to file; throws StorageException if something goes wrong with storing.
        //! Let filename blank in order to store to last opened file.
        void                                    store( const std::string& filename = "" ) throw( ... );

        //! Get the story container
        std::vector< BaseNodePtr >&             getStories();

        //! Clear the story container
        void                                    clear();

    protected:

                                                Stories();

        virtual                                 ~Stories();

        //! Initialize the stories singleton
        void                                    initialize();

        //! Shutdown the stories singleton
        void                                    shutdown();

        //! Currently loaded file
        std::string                             _file;

        //! Story container
        std::vector< BaseNodePtr >              _stories;

    friend class Core;
    friend class Singleton< Stories >;
};

} // namespace beditor

#endif // _STORIES_H_
