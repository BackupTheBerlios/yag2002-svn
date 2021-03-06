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
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _UTILS_H_
#define _UTILS_H_

#include <base.h>
#include <application.h>

namespace yaf3d
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

// functions with platform dependent implementations
//--------------------------------------------------

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

//! Returns a sorted string list with possible display settings above given colorbits filter value (format: WidthxHeight@ColorBits)
void enumerateDisplaySettings( std::vector< std::string >& settings, unsigned int colorbitsfilter = 0 );

#ifdef WIN32

//! Utility function for validating application Heaps, set YAF3DENABLE_HEAPCHECK for enabling
  #if defined( YAF3D_ENABLE_HEAPCHECK )
    #define YAF3D_CHECK_HEAP()   checkHeap();
    // dynamically validate all heaps; trigger a user breakpoint if one of the heaps is corrupt
    void checkHeap();
  #else
    #define YAF3D_CHECK_HEAP()
  #endif
  
#endif
#ifdef LINUX 

    // heap check is not implemented on Linux
    #define YAF3D_CHECK_HEAP()   
	
#endif

} // namespace yaf3d

#endif //_UTILS_H_
