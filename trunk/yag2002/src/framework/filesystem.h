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

#ifndef _FILESYSTEM_H_
#define _FILESYSTEM_H_

#include <base.h>
#include <singleton.h>
#include <smartptr.h>

namespace yaf3d
{

class FileSystem;
class Application;


//! Class for file system related exceptions
class FileSystemException : public std::runtime_error
{
    public:
                                                    FileSystemException( const std::string& reason ) :
                                                     std::runtime_error( reason )
                                                    {
                                                    }

        virtual                                     ~FileSystemException() throw() {}

                                                    FileSystemException( const FileSystemException& e ) :
                                                     std::runtime_error( e )
                                                    {
                                                    }

    protected:

                                                    FileSystemException();

        FileSystemException&                        operator = ( const FileSystemException& );
};


//! File class containing the content of a file as buffer
class File : RefCount< File >
{
    //! Set the smart pointer class as friend
    DECLARE_SMARTPTR_ACCESS( File )

    public:

        //! Get the full path of file.
        const std::string&                          getPath() const { return _path; }

        //! Get the buffer ( file size ).
        unsigned int                                getSize() const { return _size; }

        //! Get a buffer with the content of the file.
        char*                                       getBuffer() { return _p_buffer; }

        //! Release the control on buffer. On destruction, the buffer will not be destroyed then.
        char*                                       releaseBuffer()
                                                    {
                                                        char* p_tmp = _p_buffer;
                                                        _p_buffer = NULL;
                                                        return p_tmp;
                                                    }

    protected:

                                                    File( char* p_buf, unsigned int size, const std::string& path ) :
                                                     _p_buffer( p_buf ),
                                                     _size( size ),
                                                     _path( path )
                                                     {}

        virtual                                     ~File()
                                                    {
                                                        if ( _p_buffer )
                                                            delete[] _p_buffer;
                                                    }

        //! File buffer
        char*                                       _p_buffer;

        //! File size
        unsigned int                                _size;

        //! File's full path
        std::string                                 _path;

    friend class FileSystem;
};

//! Typedef for the file smart pointer
typedef SmartPtr< File >   FilePtr;


//! File system providing access to application media resources
class FileSystem : public Singleton< FileSystem >
{
    public:

        //! Get the file with given name. If the file could not be loaded then the file pointer object will be invalid ( use the function valid() of smart pointer to chek ).
        FilePtr                                     getFile( const std::string& filename );

        //! Get a list of files of given directory.
        std::vector< std::string >                  listFiles( const std::string& dir ) const;

    protected:

                                                    FileSystem();

        virtual                                     ~FileSystem();

        //! Initialize the file system. p_argv ist argv passed to application by os.
        void                                        initialize( char** p_argv ) throw( FileSystemException );

        //! Shutdown the file system.
        void                                        shutdown();

        //! Mount a new resource to given mount point, it can be a package ( e.g. a zip file ) or a folder.
        void                                        mountResource( const std::string& path, const std::string& mountpoint ) throw( FileSystemException );

    friend class Singleton< FileSystem >;
    friend class Application;
};

} // namespace yaf3d

#endif // _FILESYSTEM_H_
