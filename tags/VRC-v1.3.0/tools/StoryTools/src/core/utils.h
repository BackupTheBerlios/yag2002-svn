/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2006, A. Botorabi
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
 # common utilities
 #
 #   date of creation:  02/25/2005
 #   adapted to editor: 12/11/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#ifndef _UTILS_H_
#define _UTILS_H_

#include <main.h>

namespace beditor
{

//! Returns a string with current date and time
std::string getTimeStamp();

//! Returns a string with current date
std::string getFormatedDate();

//! Returns a string with current time
std::string getFormatedTime();

//! Returns a string with current date and time ( the string is formated different than by getTimeStamp )
std::string getFormatedDateAndTime();

//! Emplode a given std string into vector elements, borrowed from evoluioN engine
std::string::size_type explode( const std::string& str, const std::string& separators, std::vector< std::string >* p_result );

//! Given a full path this function extracts the path cutting away the file name
std::string extractPath( const std::string& fullpath );

//! Given a full path this function extracts the file name
std::string extractFileName( const std::string& fullpath );

//! Given a path this functions replaces the backslashes by slashes
std::string cleanPath( const std::string& path );

//! Return the current working directory
std::string getCurrentWorkingDirectory();

//! Given a directory path check if it actually exists in OS file system
bool checkDirectory( const std::string& dir );

//! Given a directory this function retrieves all files inside for given extension in 'listing'. If appenddetails is true then the file info is also stored in list.
void getDirectoryListing( std::vector< std::string >& listing, const std::string& dir, const std::string& extension, bool appenddetails = false );

#ifdef WIN32
    #define YAF3D_SPAWN_PROC_ID   void*
#endif

#ifdef LINUX
    #define YAF3D_SPAWN_PROC_ID   int
#endif

//! Spawn an appication given its executable file name and its parameters in param ( white space separated )
YAF3D_SPAWN_PROC_ID spawnApplication( const std::string& cmd, const std::string& params );

} // namespace beditor

#endif //_UTILS_H_
