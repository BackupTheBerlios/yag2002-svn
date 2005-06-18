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
 # common utilities
 #
 #   date of creation:  02/25/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_base.h>
#include "ctd_utils.h"
#include <io.h>
#include <errno.h>

#ifdef WIN32
 #include <shellapi.h>
#endif

namespace CTD
{

std::string getTimeStamp()
{
    __time64_t ltime;
    _time64( &ltime );
    return std::string( _ctime64( &ltime ) );
}

std::string::size_type explode( const std::string& str, const std::string& separators, std::vector< std::string >* p_result )
{
	std::string::size_type len = str.length();
	if( !len )
		return 0;

	if( !separators.length() )
	{
		p_result->push_back( str );
		return 1;
	}

	std::string::size_type token = 0;
	std::string::size_type end   = 0;
	unsigned int      org   = p_result->size();

	while( end < len )
	{
		token = str.find_first_not_of( separators, end );
		end   = str.find_first_of( separators, token );

		if( token != std::string::npos )
			p_result->push_back( str.substr( token, ( end != std::string::npos ) ? ( end - token ) : std::string::npos ) );
	}

	return( p_result->size() - org );
}

std::string extractPath( const std::string& fullpath )
{
    std::string res = fullpath;
    // first clean the path
    for ( std::string::iterator i = res.begin(), e = res.end(); i != e; i++ ) if ( *i == '\\') *i = '/';
    res = res.substr( 0, res.rfind( "/" ) );
    if ( !res.empty() )
        return res;

    return "";
}

std::string extractFileName( const std::string& fullpath )
{
    std::string res = fullpath;
    // first clean the path
    for ( std::string::iterator i = res.begin(), e = res.end(); i != e; i++ ) if ( *i == '\\') *i = '/';
    res = res.substr( res.rfind( "/" ) );
    if ( res[ 0 ] == '/' ) res.erase( 0, 1 ); // cut leading slash
    if ( !res.empty() )
        return res;

    return "";
}

// helper class for enumerateDisplaySettings
class DispSettings
{
    public:
                                DispSettings() {};
                                ~DispSettings() {};

        unsigned int            width;
        unsigned int            height;
        unsigned int            colorbits;

        bool                    operator < ( DispSettings& ds )
                                {
                                    // we compare the areas of two screen resolutions here
                                    if ( ( ds.height * ds.width ) < ( height * width ) )
                                        return true;
                                    else 
                                        return false;
                                }

        bool                    operator == ( DispSettings& ds )
                                {
                                    if ( ds.height == height && ds.width == width && ds.colorbits == colorbits )
                                        return true;
                                    else 
                                        return false;
                                }
};

#ifdef WIN32

void enumerateDisplaySettings( std::vector< std::string >& settings, unsigned int colorbitsfilter )
{
    DWORD   modenum = 0;
    DEVMODE devmode;
    devmode.dmSize = sizeof( DEVMODE );

    std::list< DispSettings > sortedsettings;

    while ( EnumDisplaySettings( NULL, modenum, &devmode ) )
    {
        modenum++;
        DispSettings ds;
        ds.width     = devmode.dmPelsWidth;
        ds.height    = devmode.dmPelsHeight;
        ds.colorbits = devmode.dmBitsPerPel;
        if ( ds.colorbits >= colorbitsfilter )
        {
            sortedsettings.push_back( ds );
        }
    }
    // sort the list and fill the settings string list
    sortedsettings.sort();
    sortedsettings.unique();

    std::list< DispSettings >::iterator p_beg = sortedsettings.begin(), p_end = sortedsettings.end();
    for ( ; p_beg != p_end; p_beg++ ) 
    {
        std::stringstream resstring;
        resstring << p_beg->width << "x" << p_beg->height << "@" << p_beg->colorbits;
        settings.push_back( resstring.str() );
    }

}

std::string convertFileTime( FILETIME& ft )
{
    SYSTEMTIME systemTime;
    SYSTEMTIME localTime;
    // convert the time structure to system time
    if ( !FileTimeToSystemTime( &ft, &systemTime ) )
        return "";
    // now convert the system time to local time
    if ( !SystemTimeToTzSpecificLocalTime( NULL, &systemTime, &localTime ) )
        return "";

    // format the time string
    char buf[ 128 ];
    sprintf( buf, "%0.2d:%0.2d:%0.4d  %0.2d:%0.2d", localTime.wMonth, localTime.wDay, localTime.wYear, localTime.wHour, localTime.wMinute );
    return std::string( buf );
}

void getDirectoryListing( std::vector< std::string >& listing, const std::string& dir, const std::string& extension, bool appenddetails )
{
	WIN32_FIND_DATA findData;
	HANDLE          fileHandle;
	int             flag = 1;
    std::string     search ( ( extension == "" ) ? "*" : "*." );
	std::string     directory( dir );
	
    if ( directory == "" ) 
        directory = ".";

	directory += "/";
	search = directory + search + extension;
	fileHandle = FindFirstFile( search.c_str(), &findData );
	if ( fileHandle == INVALID_HANDLE_VALUE ) 
        return;

    std::list< std::pair< std::string, std::string > > files;
	while ( flag )
	{
        if ( strcmp( findData.cFileName, "." ) && strcmp( findData.cFileName, ".." ) )
        {
            std::string details;
            if ( appenddetails )
                details = convertFileTime( findData.ftLastWriteTime );

            // check for directories
            if( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY )
                files.push_back( make_pair( "[" + std::string( findData.cFileName ) + "]", details ) );
            else
                files.push_back( make_pair( findData.cFileName, details ) );
        }

		flag = FindNextFile( fileHandle, &findData );
	}
	FindClose( fileHandle );

    // sort and copy files into listing
    files.sort();
    std::list< std::pair< std::string, std::string > >::iterator p_beg = files.begin(), p_end = files.end();
    for ( ; p_beg != p_end; p_beg++ )
    {
        if ( appenddetails )
            listing.push_back( p_beg->second + "   " + p_beg->first );
        else
            listing.push_back( p_beg->first );
    }
}

bool checkDirectory( const std::string& dir )
{
	WIN32_FIND_DATA findData;
	HANDLE          fileHandle;
    std::string     cdir = dir;
    while ( cdir[ cdir.size() - 1 ] == '/' ) cdir.erase( cdir.size() - 1 ); // cut slashes at end
    fileHandle = FindFirstFile( cdir.c_str(), &findData );
	if ( ( fileHandle == INVALID_HANDLE_VALUE ) || !( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) )
    {
	    FindClose( fileHandle );
        return false;
    }
	FindClose( fileHandle );
    return true;
}

HANDLE spawnApplication( const std::string& cmd, const std::string& params )
{
    HANDLE hProc = NULL;
    SHELLEXECUTEINFO shellInfo;
    ::ZeroMemory( &shellInfo, sizeof( shellInfo ) );
    shellInfo.cbSize = sizeof( shellInfo );
    shellInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    shellInfo.lpFile = cmd.c_str();
    shellInfo.lpParameters = params.c_str();
    shellInfo.nShow = SW_SHOWMINIMIZED;
    if( ::ShellExecuteEx( &shellInfo ) )
    { 
        hProc = shellInfo.hProcess;
    }
    return hProc;
}

#endif

} // namespace CTD
