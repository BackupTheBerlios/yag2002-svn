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
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/fstream>
#include <osgDB/Registry>

namespace yaf3d
{

YAF3D_SINGLETON_IMPL( FileSystem )


//! Implementation of streambuf class
Streambuf::Streambuf( const std::string& filename ) :
 _p_begin( NULL ),
 _p_end( NULL ),
 _p_current( NULL ),
 _ok( false ),
 _buffSize( 0 )
{
    _ok = readVFSFile( filename );
}

Streambuf::~Streambuf()
{
    if ( _p_begin )
        delete[] _p_begin;
}

bool Streambuf::ok() const
{
    return _ok;
}

Streambuf::int_type Streambuf::underflow()
{
    if ( _p_current == _p_end )
        return traits_type::eof();

    return static_cast< Streambuf::int_type >( *_p_current ) & 0x000000ff;
}

Streambuf::int_type Streambuf::uflow()
{
    if ( _p_current == _p_end )
        return traits_type::eof();

    return static_cast< Streambuf::int_type >( *_p_current++ ) & 0x000000ff;
}

Streambuf::int_type Streambuf::pbackfail( int_type ch )
{
    if ( _p_current == _p_begin || (ch != traits_type::eof() && ch != _p_current[ -1 ] ) )
        return traits_type::eof();

    return static_cast< Streambuf::int_type >( *--_p_current ) & 0x000000ff;
}

std::streamsize Streambuf::showmanyc()
{
    return _p_end - _p_current;
}

Streambuf::pos_type Streambuf::seekoff( Streambuf::off_type off, Streambuf::seekdir way, Streambuf::openmode /*mode*/ )
{
    char* p_pos = NULL;
    if ( way == std::ios_base::beg )
        p_pos = _p_begin;
    else if ( way == std::ios_base::end )
        p_pos = _p_end;
    else if ( way == std::ios_base::cur )
        p_pos = _p_current;

    if ( !p_pos )
        return traits_type::eof();

    if ( ( ( p_pos + off ) > _p_end ) || ( ( p_pos + off ) < _p_begin ) )
        return traits_type::eof();

    _p_current = &p_pos[ off ];

    return ( &p_pos[ off ] - _p_begin );
}

Streambuf::pos_type Streambuf::seekpos( Streambuf::pos_type pos, Streambuf::openmode /*mode*/ )
{
    return seekoff( pos, std::ios_base::beg );
}

bool Streambuf::readVFSFile( const std::string& filename )
{
    PHYSFS_File* p_handle = PHYSFS_openRead( filename.c_str() );
    if ( !p_handle )
    {
        log_warning << "VFS: cannot open requested file: " << filename << std::endl;
        return false;
    }

    PHYSFS_sint64 filesize = PHYSFS_fileLength( p_handle );
    if ( filesize < 0 )
    {
        log_warning << "VFS: cannot determine file size: " << filename << std::endl;
        return false;
    }

    char* buffer = new char[ filesize + 1 ];
    PHYSFS_sint64 cnt = PHYSFS_read( p_handle, buffer, filesize, 1 );
    PHYSFS_close( p_handle );

    if ( cnt < 0 )
    {
        log_warning << "VFS: cannot read file: " << filename << std::endl;
        return false;
    }

    _p_begin   = buffer;
    _p_end     = &buffer[ filesize ];
    _p_current = _p_begin;
    _buffSize  = filesize;

    return true;
}

//! OSG Reader for physfs
class ReaderPhysFS : public osgDB::ReaderWriter
{
    public:

                                                    ReaderPhysFS();

        virtual                                     ~ReaderPhysFS() {}

        virtual const char*                         className() const;

        virtual bool                                acceptsExtension( const std::string& extension ) const;

        virtual osgDB::ReaderWriter::ReadResult     readObject( const std::string& file, const osgDB::ReaderWriter::Options* options ) const;

        virtual osgDB::ReaderWriter::ReadResult     readImage( const std::string& file, const osgDB::ReaderWriter::Options* options ) const;

        virtual osgDB::ReaderWriter::ReadResult     readNode( const std::string& file, const osgDB::ReaderWriter::Options* options ) const;

        virtual osgDB::ReaderWriter::ReadResult     readObject( std::istream& fin, const osgDB::ReaderWriter::Options* options ) const;

        virtual osgDB::ReaderWriter::ReadResult     readImage( std::istream& fin, const osgDB::ReaderWriter::Options* options ) const;

        virtual osgDB::ReaderWriter::ReadResult     readNode( std::istream& fin, const osgDB::ReaderWriter::Options* options ) const;

    protected:

        mutable bool                                _acceptAll;
};

ReaderPhysFS::ReaderPhysFS() :
 _acceptAll( false )
{
    supportsExtension( "physfs","PhysFS based virtual file system loader" );
}

const char* ReaderPhysFS::className() const
{
    return "PhysFS Reader";
}

bool ReaderPhysFS::acceptsExtension( const std::string& extension ) const
{
    // we need this mode in order to let this reader handle also images
    if ( _acceptAll )
        return true;

    if ( osgDB::equalCaseInsensitive( extension, "physfs" ) )
        return true;

    return false;
}

osgDB::ReaderWriter::ReadResult ReaderPhysFS::readObject( const std::string& file, const osgDB::ReaderWriter::Options* options ) const
{
#ifdef _DEBUG
    if ( !PHYSFS_isInit() )
    {
        osg::notify( osg::FATAL ) << "Error: PhysFS is not initialized" << std::endl;
        return osgDB::ReaderWriter::ReadResult::ERROR_IN_READING_FILE;
    }
#endif

    return readNode( file, options );
}

osgDB::ReaderWriter::ReadResult ReaderPhysFS::readImage( const std::string& file, const osgDB::ReaderWriter::Options* options ) const
{
#ifdef _DEBUG
    if ( !PHYSFS_isInit() )
    {
        osg::notify( osg::FATAL ) << "Error: PhysFS is not initialized" << std::endl;
        return osgDB::ReaderWriter::ReadResult::ERROR_IN_READING_FILE;
    }
#endif

    // try to find the proper path in vfs
    std::string path;
    if ( options )
    {
        osgDB::FilePathList pl = options->getDatabasePathList();
        while ( pl.size() )
        {
            std::string searchpath = pl.front();
            pl.pop_back();
            PHYSFS_File* p_handle = PHYSFS_openRead( cleanPath( searchpath + "/" + file ).c_str() );
            if ( p_handle )
            {
                path = searchpath;
                PHYSFS_close( p_handle );
                break;
            }
        }
    }

    // get the file from virtual file system and setup a stream
    Streambuf ibuf( path + "/" + yaf3d::cleanPath( file ) );
    if ( !ibuf.ok() )
        return osgDB::ReaderWriter::ReadResult::ERROR_IN_READING_FILE;

    std::istream input( &ibuf );

    // remove the reader extension restriction
    _acceptAll = false;

    std::string ext = osgDB::getFileExtension( file );
    osgDB::ReaderWriter* p_reader = osgDB::Registry::instance()->getReaderWriterForExtension( ext );
    if ( !p_reader )
    {
        osg::notify( osg::FATAL ) << "ReaderPhysFS::readImage cannot find image reader for " << file << std::endl;
        return osgDB::ReaderWriter::ReadResult::ERROR_IN_READING_FILE;
    }

    // set the filepath into list of seach paths
    osg::ref_ptr< osgDB::ReaderWriter::Options > lopt = options ? static_cast< osgDB::ReaderWriter::Options* >( options->clone( osg::CopyOp::SHALLOW_COPY ) ) : new osgDB::ReaderWriter::Options;
    lopt->getDatabasePathList().push_front( osgDB::getFilePath( file ) );

    _acceptAll = false;

    return p_reader->readImage( input, lopt );
}

osgDB::ReaderWriter::ReadResult ReaderPhysFS::readNode( const std::string& file, const osgDB::ReaderWriter::Options* options ) const
{
#ifdef _DEBUG
    if ( !PHYSFS_isInit() )
    {
        osg::notify( osg::FATAL ) << "Error: PhysFS is not initialized" << std::endl;
        return osgDB::ReaderWriter::ReadResult::ERROR_IN_READING_FILE;
    }
#endif

    std::string vfsext = osgDB::getLowerCaseFileExtension( file );
    if ( !acceptsExtension( vfsext ) )
        return osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;

    std::string actualfilename = osgDB::getNameLessExtension( file );

    // get the file from virtual file system and setup a stream
    Streambuf ibuf( actualfilename );
    if ( !ibuf.ok() )
        return osgDB::ReaderWriter::ReadResult::ERROR_IN_READING_FILE;

    std::istream input( &ibuf );

    // set the reader extension restriction
    _acceptAll = false;

    std::string ext = osgDB::getFileExtension( actualfilename );
    osgDB::ReaderWriter* p_reader = osgDB::Registry::instance()->getReaderWriterForExtension( ext );
    if ( !p_reader )
    {
        osg::notify( osg::WARN ) << "ReaderPhysFS::readNode cannot find reader for " << actualfilename << std::endl;
        return osgDB::ReaderWriter::ReadResult::ERROR_IN_READING_FILE;
    }

    // set the filepath into list of seach paths
    osg::ref_ptr< osgDB::ReaderWriter::Options > lopt = options ? static_cast< osgDB::ReaderWriter::Options* >( options->clone( osg::CopyOp::SHALLOW_COPY ) ) : new osgDB::ReaderWriter::Options;
    lopt->getDatabasePathList().push_front( osgDB::getFilePath( actualfilename ) );

    // remove the reader extension restriction in order to get the images too
    _acceptAll = true;
    return p_reader->readNode( input, lopt );
}

osgDB::ReaderWriter::ReadResult ReaderPhysFS::readObject( std::istream& /*fin*/, const osgDB::ReaderWriter::Options* /*options*/ ) const
{
    assert( NULL && "not supported!" );
    return osgDB::ReaderWriter::ReadResult::ERROR_IN_READING_FILE;
}

osgDB::ReaderWriter::ReadResult ReaderPhysFS::readImage( std::istream& /*fin*/, const osgDB::ReaderWriter::Options* /*options*/ ) const
{
    assert( NULL && "not supported!" );
    return osgDB::ReaderWriter::ReadResult::ERROR_IN_READING_FILE;
}

osgDB::ReaderWriter::ReadResult ReaderPhysFS::readNode( std::istream& /*fin*/, const osgDB::ReaderWriter::Options* /*options*/ ) const
{
    assert( NULL && "not supported!" );
    return osgDB::ReaderWriter::ReadResult::ERROR_IN_READING_FILE;
}

//! Register the physfs plugin in osg db
REGISTER_OSGPLUGIN( physfs, ReaderPhysFS )


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

StreambufPtr FileSystem::getStream( const std::string& filename )
{
    StreambufPtr stream = new Streambuf( filename );
    if ( !stream->ok() )
        return StreambufPtr();

    return stream;
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
