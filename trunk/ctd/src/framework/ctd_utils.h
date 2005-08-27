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

#ifndef _CTD_UTILS_H_
#define _CTD_UTILS_H_

#include <ctd_base.h>
#include <ctd_application.h>

namespace CTD
{

//! Returns a string with current date and time
std::string getTimeStamp();

//! Emplode a given std string into vector elements, borrowed from evoluioN engine
std::string::size_type explode( const std::string& str, const std::string& separators, std::vector< std::string >* p_result );

//! Given a full path this function extracts the path cutting away the file name
std::string extractPath( const std::string& fullpath );

//! Given a full path this function extracts the file name
std::string extractFileName( const std::string& fullpath );

//! Given a path this functions replaces the backslashes by slashes
std::string cleanPath( const std::string& path );

//! Read the six sides of a cube map and return a texture
osg::ref_ptr< osg::TextureCubeMap > readCubeMap( const std::vector< std::string >& texfiles );

//! A generic input handler class with automatic adding and removing to / from viewer's event hanlder list
struct NullType {};
template< class T = NullType >
class GenericInputHandler : public osgGA::GUIEventHandler
{
    public:

        explicit                            GenericInputHandler( T* p_obj = NULL ) : 
                                             _p_userObject( p_obj ),
                                             _destroyed( false )
                                            {
                                                // register us in viewer to get event callbacks
                                                Application::get()->getViewer()->addEventHandler( this );
                                            }

        virtual                             ~GenericInputHandler() 
                                            {
                                                if ( !_destroyed )
                                                    destroyHandler();
                                            }

        //! Remove handler form viewer's handler list and destroy the object. Don't use the object after calling this method.
        void                                destroyHandler()
                                            {
                                                 // remove this handler from viewer's handler list
                                                 Application::get()->getViewer()->removeEventHandler( this );
                                                 _destroyed = true;
                                            }

        virtual bool                        handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa ) = 0;

        //! Return the user object
        T*                                  getUserObject() { return _p_userObject; }

    protected:

        //! An optional object which can be accessed in 'handle' method.
        T*                                  _p_userObject;

        //! This flag is used for calling destroyHandler on destruction if the user has forgotten that.
        bool                                _destroyed;
};

//! This is a class for adjusting the transforming to eye coordinated.
/**  In osg examples you find it as class "MoveEarthySkyWithEyePointTransform"
*    It is used by entities like skybox and water.
*/
class EyeTransform : public osg::Transform
{
    public:

        /** Get the transformation matrix which moves from local coords to world coords.*/
        virtual bool                        computeLocalToWorldMatrix( osg::Matrix& matrix, osg::NodeVisitor* p_nv ) const 
                                            {
                                                osgUtil::CullVisitor* p_cv = dynamic_cast< osgUtil::CullVisitor* >( p_nv );
                                                if ( p_cv )
                                                {
                                                    const osg::Vec3& eyePointLocal = p_cv->getEyeLocal();
                                                    matrix.preMult( osg::Matrix::translate( eyePointLocal ) );
                                                }
                                                return true;
                                            }

        /** Get the transformation matrix which moves from world coords to local coords.*/
        virtual bool                        computeWorldToLocalMatrix( osg::Matrix& matrix, osg::NodeVisitor* p_nv ) const
                                            {    
                                                osgUtil::CullVisitor* p_cv = dynamic_cast< osgUtil::CullVisitor* >( p_nv );
                                                if ( p_cv )
                                                {
                                                    const osg::Vec3& eyePointLocal = p_cv->getEyeLocal();
                                                    matrix.postMult( osg::Matrix::translate( -eyePointLocal ) );
                                                }
                                                return true;
                                            }
};

//! Visitor for getting the world transformation considering all PositionAttitudeTransform nodes in path
class TransformationVisitor : public osg::NodeVisitor
{
    public:
                                            TransformationVisitor( osg::NodeVisitor::TraversalMode tmode = osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ) :
                                                osg::NodeVisitor( tmode )
                                            {
                                                // we take all nodes
                                                setTraversalMask( 0xffffffff );
                                            }

                                            ~TransformationVisitor() {}

        void                                apply( osg::PositionAttitudeTransform& node )
                                            {                        
                                                _matrix *= osg::computeLocalToWorld( getNodePath() );
                                            }

        const osg::Matrixf&                 getMatrix() const
                                            {
                                                return _matrix;
                                            }

    protected:

        osg::Matrixf                        _matrix;
};

//! Update texture matrix for cubemaps ( see osg's VertexProgram example )
class TexMatCallback : public osg::NodeCallback
{
    public:

        explicit                            TexMatCallback( osg::TexMat& tex ) : 
                                             _texMat( tex ),
                                             _R( 
                                                osg::Matrix::rotate( osg::DegreesToRadians( 90.0f ), 0.0f, 0.0f, 1.0f ) *
                                                osg::Matrix::rotate( osg::DegreesToRadians( 90.0f ), 1.0f, 0.0f, 0.0f ) 
                                               )
                                            {
                                            }

        virtual void                        operator()( osg::Node* p_node, osg::NodeVisitor* p_nv )
                                            {
                                                osgUtil::CullVisitor* p_cv = dynamic_cast< osgUtil::CullVisitor* >( p_nv );
                                                if ( p_cv )
                                                {
                                                    const osg::Matrixf& MV = p_cv->getModelViewMatrix();
                                                    osg::Quat q;
                                                    MV.get( q );
                                                    const osg::Matrix C = osg::Matrixf::rotate( q.inverse() );

                                                    _texMat.setMatrix( C * _R );
                                                }
                                                traverse( p_node, p_nv );
                                            }

        osg::TexMat&                        _texMat;

        osg::Matrixf                        _R;
};

// functions with platform dependent implementations
//--------------------------------------------------

//! Return the current working directory
std::string getCurrentWorkingDirectory();

//! Given a directory path check if it actually exists in OS file system
bool checkDirectory( const std::string& dir );

//! Given a directory this function retrieves all files inside for given extension in 'listing'. If appenddetails is true then the file info is also stored in list.
void getDirectoryListing( std::vector< std::string >& listing, const std::string& dir, const std::string& extension, bool appenddetails = false );

#ifdef WIN32
    #define SPAWN_PROC_ID   HANDLE
#endif
#ifdef LINUX 
    #define SPAWN_PROC_ID   int
#endif	
//! Spawn an appication given its executable file name and its parameters in param ( white space separated )
SPAWN_PROC_ID spawnApplication( const std::string& cmd, const std::string& params );

//! Returns a sorted string list with possible display settings above given colorbits filter value (format: WidthxHeight@ColorBits)
void enumerateDisplaySettings( std::vector< std::string >& settings, unsigned int colorbitsfilter = 0 );


#ifdef WIN32

//! Utility function for validating application Heaps, set CTD_ENABLE_HEAPCHECK for enabling
  #if defined( CTD_ENABLE_HEAPCHECK )
    #define CTD_CHECK_HEAP()   checkHeap();
    // dynamically validate all heaps; trigger a user breakpoint if one of the heaps is corrupt
    void checkHeap();
  #else
    #define CTD_CHECK_HEAP()
  #endif
  
#endif
#ifdef LINUX 

    // heap check is not implemented on Linux
    #define CTD_CHECK_HEAP()   
	
#endif

} // namespace CTD

#endif //_CTD_UTILS_H_
