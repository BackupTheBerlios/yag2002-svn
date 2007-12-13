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

#include <main.h>
#include "stories.h"

//! Implement the stories singleton
BEDITOR_SINGLETON_IMPL( beditor::Stories )

namespace beditor
{

Stories::Stories()
{
}

Stories::~Stories()
{
}

void Stories::initialize()
{
}

void Stories::shutdown()
{
    clear();
    destroy();
}

void Stories::load( const std::string& filename ) throw( ... )
{
    std::vector< BaseNodePtr > stories;
    Storage::get()->read( filename, stories ); // note: this method can throw an exception
    // is the file has successfully loaded then update our story container
    clear();
    _stories = stories;
}

void Stories::store( const std::string& filename ) throw( ... )
{
    Storage::get()->write( filename, _stories );
}

std::vector< BaseNodePtr >& Stories::getStories()
{
    return _stories;
}

void Stories::clear()
{
    _stories.clear();
}

} // namespace beditor
