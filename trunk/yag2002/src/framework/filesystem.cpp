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
 # file system providing access to application media ressources
 #
 #   date of creation:  01/02/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <base.h>
#include "filesystem.h"
#include "log.h"
#include "utils.h"
#include <physfs.h>


namespace yaf3d
{

YAF3D_SINGLETON_IMPL( FileSystem )

//! Implementation of File
File::File( char* p_buf, unsigned int size, const std::string& path ) :
 _p_buffer( p_buf ),
 _size( size ),
 _pos( 0 ),
 _path( path )
{
}

File::~File()
{
    if ( _p_buffer )
        delete[] _p_buffer;
}

const std::string& File::getPath() const
{
    return _path;
}

unsigned int File::getSize() const
{
    return _size;
}

char* File::getBuffer()
{
    return _p_buffer;
}

char* File::readBuffer( unsigned int count, unsigned int& bytesread )
{
    // check for end of buffer reach
    if ( ( _pos + count ) > _size )
        count = _size - _pos;

    bytesread = count;
    _pos += bytesread;
    return &_p_buffer[ _pos - bytesread ];
}

char* File::readBuffer( unsigned int pos, unsigned int count, unsigned int& bytesread )
{
    // check for end of buffer reach
    if ( ( pos + count ) > _size )
        count = _size - pos;

    bytesread = count;
    _pos = pos + bytesread;
    return &_p_buffer[ pos ];
}

bool File::setBufferPos( unsigned int pos )
{
    if ( pos > _size )
        return false;

    _pos = pos;
    return true;
}

unsigned int File::getBufferPos() const
{
    return _pos;
}

char* File::releaseBuffer()
{
    char* p_tmp = _p_buffer;
    _p_buffer = NULL;
    return p_tmp;
}


// !Implementatino of FileSystem
FileSystem::FileSystem()
{
}

FileSystem::~FileSystem()
{
}

void FileSystem::initialize( char** p_argv ) throw ( FileSystemException )
{
    if ( !PHYSFS_init( p_argv[ 0 ] ) )
    {
        const char* p_reason = PHYSFS_getLastError();
        log_error << "FileSystem could not be initialized!" << std::endl;
        log_error << " reason: " << p_reason << std::endl;
        throw FileSystemException( std::string( "FileSystem could not be initialized. Reason: " ) + p_reason );
    }
}

void FileSystem::shutdown()
{
    log_info << "FileSystem: shutting down" << std::endl;

    if ( !PHYSFS_deinit() )
        log_error << "File system could not be shut down properly!" << std::endl;

    // destroy the singleton
    destroy();
}

void FileSystem::mountResource( const std::string& path, const std::string& mountpoint ) throw( FileSystemException )
{
    log_info << "FileSystem: mounting '" << path << "' to '" << mountpoint << "'" << std::endl;

    if ( !PHYSFS_mount( path.c_str(), mountpoint.c_str(), 1 ) )
    {
        const char* p_reason = PHYSFS_getLastError();
        log_error << "FileSystem could not mount '" << path << "' to '" << mountpoint << "'" << std::endl;
        log_error << " reason: " << p_reason << std::endl;
        throw FileSystemException( std::string( "FileSystem could not mount resource. Reason: " ) + p_reason );
    }
}

FilePtr FileSystem::getFile( const std::string& filename )
{
    FilePtr file;
    PHYSFS_File* p_handle = PHYSFS_openRead( cleanPath( filename ).c_str() );
    if ( !p_handle )
    {
        log_error << "FileSystem: cannot open requested file: " << filename << std::endl;
        return file;
    }

    PHYSFS_sint64 filesize = PHYSFS_fileLength( p_handle );
    if ( filesize < 0 )
    {
        log_error << "FileSystem: cannot determine file size: " << filename << std::endl;
        return file;
    }

    char*    p_buffer = new char[ filesize ];
    PHYSFS_sint64 cnt = PHYSFS_read( p_handle, p_buffer, filesize, 1 );
    PHYSFS_close( p_handle );

    if ( cnt < 0 )
    {
        log_error << "FileSystem: cannot read file: " << filename << std::endl;
        return file;
    }

    // setup a file object and return a smart pointer on it!
    file = new File( p_buffer, filesize, filename );

    return file;
}


unsigned int FileSystem::listFiles( std::vector< std::string >& files, const std::string& dir, const std::string& ext ) const
{
    if ( !dir.c_str() )
        return 0;

    char** p_filenames = PHYSFS_enumerateFiles( dir.c_str() );
    char** p_filename = p_filenames;
    std::string extension = "." + ext;
    for( ; *p_filename != NULL; ++p_filename )
    {
        std::string filename( *p_filename );
        if ( filename.length() > extension.length() )
        {
            if ( filename.find( extension, filename.length() - extension.length() - 1 ) != std::string::npos )
                files.push_back( *p_filename );
        }
    }

    PHYSFS_freeList( p_filenames );

    return static_cast< unsigned int >( files.size() );
}

} // namespace yaf3d
