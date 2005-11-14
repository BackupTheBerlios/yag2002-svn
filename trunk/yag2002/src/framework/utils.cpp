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

#include <base.h>
#include "application.h"
#include "log.h"
#include "utils.h"
#include <errno.h>

#ifdef WIN32
 #include <shellapi.h>
#endif
#ifdef LINUX
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <glob.h>
 #include <dirent.h>
#endif

namespace yaf3d
{

std::string getTimeStamp()
{
    time_t t;
    time( &t );
    return std::string( asctime( localtime( &t ) ) );
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

std::string cleanPath( const std::string& path )
{
    std::string cleanpath = path;
    for ( std::string::iterator i = cleanpath.begin(), e = cleanpath.end(); i != e; i++ )
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
    for ( size_t cnt = 0; cnt < ff_glob.gl_pathc; cnt++ )
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
    return ( stat( dir.c_str(), &fileInfo ) == NULL ) ? false : true;

#endif

    // platform not supported
    return false;
}

SPAWN_PROC_ID spawnApplication( const std::string& cmd, const std::string& params )
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
    
    // TODO: implementation
    return -1;
    
#endif

    // platform not supported
    return ( SPAWN_PROC_ID )0;
}

// helper class for enumerateDisplaySettings
class DispSettings
{
    public:
                                DispSettings() :
                                 width( 0 ),
                                 height( 0 ),
                                 colorbits( 0 )
                                {
                                }

                                ~DispSettings() {};

        unsigned int            width;
        unsigned int            height;
        unsigned int            colorbits;

        bool                    operator < ( const DispSettings& ds ) const
                                {
                                    // we compare the areas of two screen resolutions here
                                    if ( ( ds.height * ds.width ) < ( height * width ) )
                                        return true;
                                    else 
                                        return false;
                                }

        bool                    operator == ( const DispSettings& ds ) const
                                {
                                    if ( ds.height == height && ds.width == width && ds.colorbits == colorbits )
                                        return true;
                                    else 
                                        return false;
                                }
};

void enumerateDisplaySettings( std::vector< std::string >& settings, unsigned int colorbitsfilter )
{
    std::list< DispSettings > sortedsettings;

#ifdef WIN32

    DWORD   modenum = 0;
    DEVMODE devmode;
    devmode.dmSize = sizeof( DEVMODE );

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

#endif 
#ifdef LINUX

    SDL_Rect** pp_modes;
    // get available fullscreen/hardware modes
    pp_modes = SDL_ListModes( NULL, SDL_FULLSCREEN|SDL_HWSURFACE );

    // check is there are any modes available
    if( pp_modes == ( SDL_Rect ** )0 )
    {
       log << Log::LogLevel( Log::L_ERROR ) << "no video modes available!\n" << std::endl;
       return;
    }

    // check if our resolution is restricted
    if( pp_modes == ( SDL_Rect ** )-1 )
    {
        // all resolutions available, what to do here?
    }
    else
    {  
        for( int i = 0; pp_modes[ i ]; i++ )
        {

            DispSettings ds;
            ds.width     = pp_modes[ i ]->w;
            ds.height    = pp_modes[ i ]->h;
            ds.colorbits = SDL_GetVideoInfo()->vfmt->BitsPerPixel;
            if ( ds.colorbits >= colorbitsfilter )
            {
                sortedsettings.push_back( ds );
            }
        }
    }

#endif

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

#ifdef WIN32
#ifdef YAF3D_ENABLE_HEAPCHECK
void checkHeap()
{
    // go through all heaps and validate them, trigger a user breakpoint if one of the heaps is corrupt
    HANDLE heaps[ 20 ];  // a max size of 20 heaps should suffice
    DWORD numHeaps = GetProcessHeaps( 20, heaps );
    for ( DWORD i = 0; i < numHeaps; i ++ )
    {
        if ( HeapValidate( heaps[ i ], 0, NULL ) == FALSE )
        {
            __asm int 3;    // trigger user break point
        }
    }
}
#endif
#endif


osg::ref_ptr< osg::TextureCubeMap > readCubeMap( const std::vector< std::string >& texfiles )
{
    if ( texfiles.size() < 6 )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "readCubeMap: 6 tex files are needed, but only " << texfiles.size() << " given!" << std::endl;
        return NULL;
    }

    osg::TextureCubeMap* p_cubemap = new osg::TextureCubeMap;
    std::string mediapath = Application::get()->getMediaPath();
    osg::Image* imagePosX = osgDB::readImageFile( mediapath + texfiles[ 0 ] );
    osg::Image* imageNegX = osgDB::readImageFile( mediapath + texfiles[ 1 ] );
    osg::Image* imagePosY = osgDB::readImageFile( mediapath + texfiles[ 2 ] );
    osg::Image* imageNegY = osgDB::readImageFile( mediapath + texfiles[ 3 ] );
    osg::Image* imagePosZ = osgDB::readImageFile( mediapath + texfiles[ 4 ] );
    osg::Image* imageNegZ = osgDB::readImageFile( mediapath + texfiles[ 5 ] );

    if ( imagePosX && imageNegX && imagePosY && imageNegY && imagePosZ && imageNegZ )
    {
        p_cubemap->setImage( osg::TextureCubeMap::POSITIVE_X, imagePosX );
        p_cubemap->setImage( osg::TextureCubeMap::NEGATIVE_X, imageNegX );
        p_cubemap->setImage( osg::TextureCubeMap::POSITIVE_Y, imagePosY );
        p_cubemap->setImage( osg::TextureCubeMap::NEGATIVE_Y, imageNegY );
        p_cubemap->setImage( osg::TextureCubeMap::POSITIVE_Z, imagePosZ );
        p_cubemap->setImage( osg::TextureCubeMap::NEGATIVE_Z, imageNegZ );

        p_cubemap->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
        p_cubemap->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );
        p_cubemap->setWrap( osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE );

        p_cubemap->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_LINEAR );
        p_cubemap->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );
    }
    else
    {
        log << Log::LogLevel( Log::L_ERROR ) << "readCubeMap: could not setup all cubemap images" << std::endl;
    }

    return p_cubemap;
}

} // namespace yaf3d
