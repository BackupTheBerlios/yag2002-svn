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
class File : public RefCount< File >
{
    //! Set the smart pointer class as friend
    DECLARE_SMARTPTR_ACCESS( File )

    public:

        //! Get the full path of file.
        const std::string&                          getPath() const;

        //! Get the buffer ( file size ).
        unsigned int                                getSize() const;

        //! Get a buffer with the content of the file.
        char*                                       getBuffer();

        //! Read 'count' bytes from buffer at current position. Returns the actual read bytes in 'byesread'.
        char*                                       readBuffer( unsigned int count, unsigned int& bytesread );

        //! Read 'count' bytes from buffer at given position.  Returns the actual read bytes in 'byesread'.
        char*                                       readBuffer( unsigned int pos, unsigned int count, unsigned int& bytesread );

        //! Set current buffer position. Return false if the position exceeds the buffer size.
        bool                                        setBufferPos( unsigned int pos );

        //! Return current buffer position.
        unsigned int                                getBufferPos() const;

        //! Release the control on buffer. On destruction, the buffer will not be destroyed then when File is destroyed.
        //! NOTE: The user has to care about the buffer deletion when it is no longer needed. Use array deletion 'delete[]'.
        char*                                       releaseBuffer();

    protected:

                                                    File( char* p_buf, unsigned int size, const std::string& path );

        virtual                                     ~File();

        //! File buffer
        char*                                       _p_buffer;

        //! File size
        unsigned int                                _size;

        //! Current buffer position
        unsigned int                                _pos;

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

        //! Get the file with given name. If the file could not be loaded then the file pointer object will be invalid ( use the function valid() of smart pointer to check ).
        FilePtr                                     getFile( const std::string& filename );

        //! Get a list of files in given directory. Filter the files with given extension, if ext is not empty. Return the count of files.
        unsigned int                                listFiles( std::vector< std::string >& files, const std::string& dir, const std::string& ext = "" ) const;

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
