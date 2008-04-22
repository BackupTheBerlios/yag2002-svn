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

#include <main.h>
#include "utils.h"
#include "log.h"
#include <errno.h>
#include <list>

#ifdef LINUX
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <glob.h>
 #include <dirent.h>
 #include <spawn.h>
#endif

// disable the safety warnings on win32
#ifdef WIN32
 #pragma warning( disable: 4996 )
#endif

namespace beditor
{

// implement missing time functions on linux
#ifdef LINUX
char* _strtime( char* p_str )
{
    std::time_t t;
    std::time( &t );
    std::tm* tstruct = localtime( &t );
    std::stringstream timestr;
    timestr << tstruct->tm_hour << ":" << tstruct->tm_min << ":" << tstruct->tm_sec;
    strcpy( p_str, timestr.str().c_str() );
    return p_str;
}

char* _strdate( char* p_str )
{
    std::time_t t;
    char p_buf[ 128 ];
    std::time( &t );
    std::tm* p_tstruct = localtime( &t );
    strftime( p_buf, 128, "%m/%d/%y", p_tstruct );
    strcpy( p_str, p_buf );
    return p_str;
}
#endif // LINUX

std::string getTimeStamp()
{
    time_t t;
    time( &t );
    return std::string( asctime( localtime( &t ) ) );
}

std::string getFormatedDate()
{
    char p_buf[ 128 ];
    _strdate( p_buf );
    std::string datestamp( p_buf );
    return datestamp;
}

std::string getFormatedTime()
{
    char p_buf[ 128 ];
    _strtime( p_buf );
    std::string timestamp( p_buf );
    return timestamp;
}

std::string getFormatedDateAndTime()
{
    char p_buf[ 128 ];
    _strtime( p_buf );
    std::string timestamp( p_buf );
    _strdate( p_buf );
    std::string datestamp( p_buf );

    return datestamp + " " + timestamp;
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
    unsigned int      org   = ( unsigned int )p_result->size();

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
    for ( std::string::iterator i = res.begin(), e = res.end(); i != e; ++i ) if ( *i == '\\') *i = '/';
    res = res.substr( 0, res.rfind( "/" ) );
    if ( !res.empty() )
        return res;

    return "";
}

std::string extractFileName( const std::string& fullpath )
{
    std::string res = fullpath;
    // first clean the path
    for ( std::string::iterator i = res.begin(), e = res.end(); i != e; ++i ) if ( *i == '\\') *i = '/';
    res = res.substr( res.rfind( "/" ) );
    if ( res[ 0 ] == '/' ) res.erase( 0, 1 ); // cut leading slash
    if ( !res.empty() )
        return res;

    return "";
}

std::string cleanPath( const std::string& path )
{
    std::string cleanpath = path;
    for ( std::string::iterator i = cleanpath.begin(), e = cleanpath.end(); i != e; ++i )
        if ( *i == '\\') *i = '/';

    return cleanpath;
}

std::string getCurrentWorkingDirectory()
{
#ifdef WIN32

    char buf[ 512 ];
    GetCurrentDirectory( sizeof( buf ), buf );
    return std::string( buf );

#endif

#ifdef LINUX

    char buf[ 512 ];
    char* p_res = getcwd( buf, sizeof( buf ) );
    assert( p_res && "error getting current working directory" );

#endif

    // compilation error for unsupported platforms
    return std::string( buf );
}

#ifdef WIN32
std::string convertFileTime( const FILETIME& ft )
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
#endif


void getDirectoryListing( std::vector< std::string >& listing, const std::string& dir, const std::string& extension, bool appenddetails )
{
    std::list< std::pair< std::string, std::string > > files;

#ifdef WIN32
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
#endif

#ifdef LINUX
    struct stat     fileInfo;
    glob_t          ff_glob;
    std::string     directory( dir );

    if ( directory == "" )
        directory = ".";

    directory += "/";
    std::string search = directory + "*" + extension;

    // check if the directory exists
    if ( opendir( directory.c_str() ) == NULL )
        return;

    glob( search.c_str(), 0, NULL, &ff_glob );
    for ( size_t cnt = 0; cnt < ff_glob.gl_pathc; ++cnt )
    {
        std::string filename( ff_glob.gl_pathv[ cnt ] );
        filename.erase( 0, directory.size() );
        std::string details;
        // check for directories
        stat( std::string( directory + filename ).c_str(), &fileInfo );

        if ( appenddetails )
        {
            time_t modtime = fileInfo.st_mtime;
            details = ctime( &modtime );
            details.erase( details.size() - 1 ); // remove line break at end of string
        }

        if( S_ISDIR( fileInfo.st_mode ) )
            files.push_back( make_pair( "[" + std::string( filename ) + "]", details ) );
        else
            files.push_back( make_pair( filename, details ) );
    }
#endif

    // sort and copy files into listing
    files.sort();
    std::list< std::pair< std::string, std::string > >::iterator p_beg = files.begin(), p_end = files.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        if ( appenddetails )
            listing.push_back( p_beg->second + "   " + p_beg->first );
        else
            listing.push_back( p_beg->first );
    }
}

bool checkDirectory( const std::string& dir )
{
#ifdef WIN32

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

#endif

#ifdef LINUX

    struct stat     fileInfo;
    return ( stat( dir.c_str(), &fileInfo ) < 0 ) ? false : true;

#endif

    // platform not supported
    return false;
}

YAF3D_SPAWN_PROC_ID spawnApplication( const std::string& cmd, const std::string& params )
{
#ifdef WIN32

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

#endif

#ifdef LINUX

    pid_t pid = -1;
    posix_spawn_file_actions_t* p_fileactions = NULL;
    posix_spawnattr_t attr;
    char*             argv[ 10 ];
    char*             envp[ 2 ];

    // set default flags in spawn attribute
    posix_spawnattr_init( &attr );

    // assemble the argument list for spawing in a xterm
    // note: we assume that the system has xterm installed
    // spawn command: xterm -T "VRC server" -e ./vrc < params >
    argv[ 0 ] = strdup( "xterm" );
    argv[ 1 ] = strdup( "-display" );
    argv[ 2 ] = strdup( ":0.0" );
    argv[ 3 ] = strdup( "-T" );
    argv[ 4 ] = strdup( "\"VRC Server " YAF3D_VERSION "\"" );
    argv[ 5 ] = strdup( "-e" );
    std::string xtermexcec( cmd + " " + params );
    argv[ 6 ] = strdup( xtermexcec.c_str() );
    argv[ 7 ] = NULL;

    envp[ 0 ] = NULL;

    posix_spawnp( &pid,
                  argv[ 0 ],
                  p_fileactions,
                  &attr,
                  reinterpret_cast< char* const * >( &argv ),
                  reinterpret_cast< char* const * >( &envp )
                 );

    return static_cast< YAF3D_SPAWN_PROC_ID >( pid );

#endif

    // platform not supported
    return static_cast< YAF3D_SPAWN_PROC_ID >( 0 );
}

} // namespace yaf3d
