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
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <base.h>
#include "utils.h"
#include "application.h"
#include "log.h"
#include <errno.h>
#include <SDL_syswm.h>

#ifdef LINUX
 #include <sys/types.h>
 #include <sys/stat.h>
 #include <glob.h>
 #include <dirent.h>
 #include <spawn.h>
 #include <signal.h>
#endif


namespace yaf3d
{


//! Signal handler for catching Ctrl+C and other system signals

#ifdef WIN32
BOOL WINAPI handlerRoutine( DWORD dwCtrlType )  //  control signal type
{
    // shutdown or Ctrl-C signal
    switch ( dwCtrlType )
    {
        case CTRL_CLOSE_EVENT:
        case CTRL_C_EVENT:
        case CTRL_SHUTDOWN_EVENT:
        {
            Application::get()->handleCtrlC();
            break;
        }
    }

    return TRUE;
}
#endif

#ifdef LINUX
void handlerRoutine( int /*sig*/ )
{
}
#endif

void implementSignalHandler()
{
    // set the event handler in win32
#ifdef WIN32
    SetConsoleCtrlHandler( handlerRoutine,  TRUE );
#endif

    // set the event handler in linux
#ifdef LINUX
    // NOTE: do not set the SIGINT handler, it makes trouble!
    //signal( SIGINT,  &handlerRoutine );
#endif
}


#ifdef LINUX

// used for Copy & Paste
static Display* SDL_Display = NULL;
static Window   SDL_Window;
static void ( *Lock_Display   )( void );
static void ( *Unlock_Display )( void );

// init stuff for Copy & Paste
void initCopyPaste()
{
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if ( SDL_GetWMInfo( &info ) )
    {
        if ( info.subsystem == SDL_SYSWM_X11 )
        {
            SDL_Display    = info.info.x11.display;
            SDL_Window     = info.info.x11.window;
            Lock_Display   = info.info.x11.lock_func;
            Unlock_Display = info.info.x11.unlock_func;

            SDL_EventState( SDL_SYSWMEVENT, SDL_ENABLE );
            //SDL_SetEventFilter( clipboardFilter );
        }
        else
        {
            SDL_SetError("SDL is not running on X11");
        }
    }
}

#endif

bool copyToClipboard( const std::wstring& text )
{
#ifdef WIN32

    // get the HWND of our app
    SDL_SysWMinfo sysinfo;
    SDL_VERSION( &sysinfo.version );
    SDL_GetWMInfo( &sysinfo );

    // open clipboard
    if ( !OpenClipboard( sysinfo.window ) )
        return false;

    // copy to clipboard data
    EmptyClipboard();
    std::size_t stringlen = text.length();
    HGLOBAL  hmem         = GlobalAlloc( GMEM_MOVEABLE, sizeof( wchar_t ) * ( stringlen + 1 ) );
    wchar_t* p_text       = ( wchar_t* )GlobalLock( hmem );

    // copy a save null-terminated string to clipboard buffer
    {
        unsigned int cnt = 0;
        for ( ; cnt < stringlen; cnt++ )
            p_text[ cnt ] = text[ cnt ];

        p_text[ cnt ] = 0;
    }

    GlobalUnlock( hmem );

    if ( !SetClipboardData( CF_UNICODETEXT, hmem ) )
    {
        CloseClipboard();
        return false;
    }

    CloseClipboard();

#endif
#ifdef LINUX

    if ( !SDL_Display )
       initCopyPaste();


    wchar_t* p_dst = ( wchar_t* )malloc( sizeof( wchar_t ) * text.length() + 2 );
    wcscpy( p_dst, text.c_str() );

    Lock_Display();
    if ( XGetSelectionOwner( SDL_Display, XA_PRIMARY ) != SDL_Window )
        XSetSelectionOwner( SDL_Display, XA_PRIMARY, SDL_Window, CurrentTime );

    XChangeProperty( SDL_Display, DefaultRootWindow( SDL_Display ), XA_CUT_BUFFER0, XA_STRING, 8, PropModeReplace, ( unsigned char* )p_dst, wcslen( p_dst ) );
    Unlock_Display();

    free( p_dst );

#endif

    return true;
}

bool getFromClipboard( std::wstring& text, unsigned int maxcnt )
{
#ifdef WIN32
    if ( !OpenClipboard( NULL ) )
        return false;

    HANDLE data  = GetClipboardData( CF_UNICODETEXT );
    if ( !data )
       return false;

    wchar_t* p_text = NULL;
    p_text = ( wchar_t* )GlobalLock( data );
    if ( !p_text )
        return false;

    GlobalUnlock( data );
    CloseClipboard();
    text = p_text;

    // limit the text
    if ( text.length() > maxcnt )
        text[ 255 ] = 0;

#endif
#ifdef LINUX

    if ( !SDL_Display )
      initCopyPaste();

    Window         owner;
    Atom           selection;
    Atom           selntype;
    int            selnformat;
    unsigned long  nbytes;
    unsigned long  overflow;
    unsigned char* p_src;

    Lock_Display();
    owner = XGetSelectionOwner( SDL_Display, XA_PRIMARY );
    Unlock_Display();

    if ( ( owner == None ) || ( owner == SDL_Window ) )
    {
        owner = DefaultRootWindow( SDL_Display );
        selection = XA_CUT_BUFFER0;
    }
    else
    {
        owner = SDL_Window;
        selection = XInternAtom( SDL_Display, "SDL_SELECTION", False );
        Lock_Display();
        XConvertSelection( SDL_Display, XA_PRIMARY, XA_STRING, selection, owner, CurrentTime );
        Unlock_Display();
     }

    Lock_Display();
    if ( XGetWindowProperty( SDL_Display, owner, selection, 0, INT_MAX/4, False, XA_STRING, &selntype, &selnformat,
         &nbytes, &overflow, reinterpret_cast< unsigned char ** >( &p_src ) ) == Success )
    {
        if ( p_src )
        {
            // limit the string to the max count
            if ( nbytes > maxcnt )
                p_src[ maxcnt - 1 ] = 0;

            if ( selntype == XA_STRING )
                text = reinterpret_cast< wchar_t* >( p_src );

            XFree( p_src );
        }
    }
    Unlock_Display();

#endif

    return true;
}

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

ImageTGA::ImageTGA() :
_channels( 0 ),
_sizeX( 0 ),
_sizeY( 0 ),
_p_data( NULL )
{
}

ImageTGA::~ImageTGA()
{
    if ( _p_data )
        delete[] _p_data;
}

bool ImageTGA::load( const std::string& filename )
{
    // try to read in the file
    FILE* p_file = fopen( filename.c_str(), "rb" );
    if ( !p_file )
        return false;

    long filesize = 0;
    fseek( p_file, 0, SEEK_END );
    filesize = ftell( p_file );
    fseek( p_file, 0, SEEK_SET );

    if ( !filesize )
    {
        fclose( p_file );
        return false;
    }

    unsigned char* p_buffer = new unsigned char[ filesize ];
    unsigned char* p_orgbuffer = p_buffer;
    fread( p_buffer, 1, filesize, p_file );
    fclose( p_file );

    short width = 0, height = 0;        // The dimensions of the image
    char  length = 0;                  // The length in bytes to the pixels
    char  imageType = 0;               // The image type (RLE, RGB, Alpha...)

    char bits = 0;                    // The bits per pixel for the image (16, 24, 32)
    int  channels = 0;                   // The channels of the image (3 = RGA : 4 = RGBA)
    int  stride = 0;                     // The stride (channels * width)
    int  i = 0;                          // A counter

    // Read in the length in bytes from the header to the pixel data
    memcpy(&length, p_buffer, sizeof(char));
    p_buffer += sizeof(char);

    // Jump over one byte
    ++p_buffer;

    // Read in the imageType (RLE, RGB, etc...)
    memcpy(&imageType, p_buffer, sizeof(char));
    p_buffer += sizeof(char);

    // Skip past general information we don't care about
    p_buffer += 9;

    // Read the width, height and bits per pixel (16, 24 or 32)
    memcpy(&width, p_buffer, sizeof(short));
    p_buffer += sizeof(short);
    memcpy(&height, p_buffer, sizeof(short));
    p_buffer += sizeof(short);
    memcpy(&bits, p_buffer, sizeof(char));
    p_buffer += sizeof(char);

    // Now we move the file pointer to the pixel data
    p_buffer += length + 1;

    // These defines are used to tell us about the type of TARGA file it is
    #define TGA_RGB       2      // This tells us it's a normal RGB (really BGR) file
    #define TGA_A         3      // This tells us it's a ALPHA file
    #define TGA_RLE      10      // This tells us that the targa is Run-Length Encoded (RLE)

    // Check if the image is RLE compressed or not
    if( imageType != TGA_RLE )
    {
        // Check if the image is a 24 or 32-bit image
        if( bits == 24 || bits == 32 )
        {
            // Calculate the channels (3 or 4) - (use bits >> 3 for more speed).
            // Next, we calculate the stride and allocate enough memory for the pixels.
            channels = bits / 8;
            stride = channels * width;
            _p_data = new unsigned char[ stride * height ];

            // Load in all the pixel data line by line
            for( int y = 0; y < height; ++y )
            {
                // Store a pointer to the current line of pixels
                unsigned char *pLine = &( _p_data[ stride * y ] );

                // Read in the current line of pixels
                memcpy( pLine, p_buffer, stride );
                p_buffer += stride;

                // Go through all of the pixels and swap the B and R values since TGA
                // files are stored as BGR instead of RGB (or use GL_BGR_EXT verses GL_RGB)
                for( i = 0; i < stride; i += channels )
                {
                    unsigned char temp = pLine[ i ];
                    pLine[i]     = pLine[i + 2];
                    pLine[i + 2] = temp;
                }
            }
        }
        // Check if the image is a 16 bit image (RGB stored in 1 unsigned short)
        else if( bits == 16 )
        {
            unsigned short pixels = 0;
            unsigned char r=0, g=0, b=0;

            // Since we convert 16-bit images to 24 bit, we hardcode the channels to 3.
            // We then calculate the stride and allocate memory for the pixels.
            channels = 3;
            stride = channels * width;
            _p_data = new unsigned char[ stride * height ];

            // Load in all the pixel data pixel by pixel
            for( i = 0; i < width*height; ++i )
            {
                // Read in the current pixel
                memcpy(&pixels, p_buffer, sizeof(unsigned short));
                p_buffer += sizeof(unsigned short);

                // To convert a 16-bit pixel into an R, G, B, we need to
                // do some masking and such to isolate each color value.
                // 0x1f = 11111 in binary, so since 5 bits are reserved in
                // each unsigned short for the R, G and B, we bit shift and mask
                // to find each value.  We then bit shift up by 3 to get the full color.
                b = static_cast<unsigned char>((pixels & 0x1f) << 3);
                g = static_cast<unsigned char>(((pixels >> 5) & 0x1f) << 3);
                r = static_cast<unsigned char>(((pixels >> 10) & 0x1f) << 3);

                // This essentially assigns the color to our array and swaps the
                // B and R values at the same time.
                _p_data[i * 3 + 0] = r;
                _p_data[i * 3 + 1] = g;
                _p_data[i * 3 + 2] = b;
            }
        }
        // Else return a NULL for a bad or unsupported pixel format
        else
        {
            delete[] p_orgbuffer;
            return false;
        }
    }
    // Else, it must be Run-Length Encoded (RLE)
    else
    {
        // Create some variables to hold the rleID, current colors read, channels, & stride.
        unsigned char rleID = 0;
        int colorsRead = 0;
        channels = bits / 8;
        stride = channels * width;

        // Next we want to allocate the memory for the pixels and create an array,
        // depending on the channel count, to read in for each pixel.
        _p_data = new unsigned char[ stride * height ];
        char *pColors = new char [ channels ];

        // Load in all the pixel data
        while( i < width*height )
        {
            // Read in the current color count + 1
            memcpy( &rleID, p_buffer, sizeof( unsigned char ) );
            p_buffer += sizeof( unsigned char );

            // Check if we don't have an encoded string of colors
            if( rleID < 128 )
            {
                // Increase the count by 1
                ++rleID;

                // Go through and read all the unique colors found
                while( rleID )
                {
                    // Read in the current color
                    memcpy( pColors, p_buffer, sizeof(char) * channels );
                    p_buffer += sizeof(char) * channels;

                    // Store the current pixel in our image array
                    _p_data[colorsRead + 0] = pColors[2];
                    _p_data[colorsRead + 1] = pColors[1];
                    _p_data[colorsRead + 2] = pColors[0];

                    // If we have a 4 channel 32-bit image, assign one more for the alpha
                    if(bits == 32)
                        _p_data[colorsRead + 3] = pColors[3];

                    // Increase the current pixels read, decrease the amount
                    // of pixels left, and increase the starting index for the next pixel.
                    ++i;
                    rleID--;
                    colorsRead += channels;
                }
            }
            // Else, let's read in a string of the same character
            else
            {
                // Minus the 128 ID + 1 (127) to get the color count that needs to be read
                rleID -= 127;

                // Read in the current color, which is the same for a while
                memcpy(pColors, p_buffer, sizeof(char) * channels);
                p_buffer += sizeof(char) * channels;

                // Go and read as many pixels as are the same
                while(rleID)
                {
                    // Assign the current pixel to the current index in our pixel array
                    _p_data[colorsRead + 0] = pColors[2];
                    _p_data[colorsRead + 1] = pColors[1];
                    _p_data[colorsRead + 2] = pColors[0];

                    // If we have a 4 channel 32-bit image, assign one more for the alpha
                    if(bits == 32)
                        _p_data[colorsRead + 3] = pColors[3];

                    // Increase the current pixels read, decrease the amount
                    // of pixels left, and increase the starting index for the next pixel.
                    ++i;
                    rleID--;
                    colorsRead += channels;
                }

            }

        }

        // Free up pColors
        delete[] pColors;
    }

    // Fill in our tImageTGA structure to pass back
    _channels = channels;
    _sizeX    = width;
    _sizeY    = height;

    delete[] p_orgbuffer;

    return true;
}

const unsigned char* ImageTGA::getData( unsigned int row, unsigned int column ) const
{
    if ( ( column > _sizeY ) || ( row > _sizeX ) )
        return NULL;

    return &_p_data[ _channels * ( column * _sizeX + row ) ];
}

void ImageTGA::flipY()
{
    int pitch = _sizeX * _channels;

    // flip the image bits...
    for ( unsigned int line = 0; line < _sizeY / 2; ++line )
    {
        int srcOffset = ( line * pitch );
        int dstOffest = ( ( _sizeY - line - 1 ) * pitch );

        for ( int colBit = 0; colBit < pitch; ++colBit )
        {
            unsigned char tmp = _p_data[ dstOffest + colBit ];
            _p_data[ dstOffest + colBit ] = _p_data[ srcOffset + colBit ];
            _p_data[ srcOffset + colBit ] = tmp;
        }
    }
}

unsigned int ImageTGA::getNumChannels() const
{
    return _channels;
}

void ImageTGA::getSize( unsigned int& sizeX, unsigned int& sizeY ) const
{
    sizeX = _sizeX;
    sizeY = _sizeY;
}

bool isGlslAvailable()
{
    const osg::GL2Extensions* p_extensions = osg::GL2Extensions::Get( 0, true );
    bool  glslavailable = false;

    if ( p_extensions )
        glslavailable = p_extensions->isGlslSupported();

    return glslavailable;
}

bool fileExists( const std::string& filename )
{
    std::fstream file;
    file.open( filename.c_str(), std::ios::in );
    if( file.is_open() )
    {
        file.close();
        return true;
    }
    file.close();
    return false;
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
        ++modenum;
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
       log_error << "no video modes available!\n" << std::endl;
       return;
    }

    // check if our resolution is restricted
    if( pp_modes == ( SDL_Rect ** )-1 )
    {
        // all resolutions available, what to do here?
    }
    else
    {
        for( int i = 0; pp_modes[ i ]; ++i )
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
    for ( ; p_beg != p_end; ++p_beg )
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
    for ( DWORD i = 0; i < numHeaps; ++i )
    {
        if ( HeapValidate( heaps[ i ], 0, NULL ) == FALSE )
        {
            __asm int 3;    // trigger user break point
        }
    }
}
#endif
#endif

} // namespace yaf3d
