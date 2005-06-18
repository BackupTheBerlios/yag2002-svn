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

// Win32 specific functions
#ifdef WIN32

//! Spawn an appication given its executable file name and its parameters in param ( white space separated )
HANDLE spawnApplication( const std::string& cmd, const std::string& params );

//! Given a directory this function retrieves all files inside for given extension in 'listing'. If appenddetails is true then the file info is also stored in list.
void getDirectoryListing( std::vector< std::string >& listing, const std::string& dir, const std::string& extension, bool appenddetails = false );

//! Given a directory path check if it actually exists in OS file system
bool checkDirectory( const std::string& dir );

//! Returns a sorted string list with possible display settings above given colorbits filter value (format: WidthxHeight@ColorBits)
void enumerateDisplaySettings( std::vector< std::string >& settings, unsigned int colorbitsfilter = 0 );

#endif

//! A generic input handler class with automatic adding and removing to / from viewer's event hanlder list
struct NullType {};
template< class T = NullType >
class GenericInputHandler : public osgGA::GUIEventHandler
{
    public:

                                            GenericInputHandler( T* p_obj = NULL ) : _p_userObject( p_obj )
                                            {
                                                // register us in viewer to get event callbacks
                                                osg::ref_ptr< GenericInputHandler > ih( this );
                                                Application::get()->getViewer()->getEventHandlerList().push_back( ih.get() );
                                            }

        virtual                             ~GenericInputHandler() {}

        //! Remove handler form viewer's handler list and destroy the object. Don't use the object after calling this method.
        void                                destroyHandler()
                                            {
                                                 // remove this handler from viewer's handler list
                                                 osgProducer::Viewer::EventHandlerList& eh = Application::get()->getViewer()->getEventHandlerList();
                                                 osgProducer::Viewer::EventHandlerList::iterator beg = eh.begin(), end = eh.end();
                                                 for ( ; beg != end; beg++ )
                                                 {
                                                     if ( *beg == this )
                                                     {
                                                         eh.erase( beg );
                                                         break;
                                                     }
                                                 }
                                             }

        virtual bool                         handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa ) = 0;

    protected:

        //! An optional object which can be accessed in 'handle' method.
        T*                                  _p_userObject;
};

//! This is a class for adjusting the transforming to eye coordinated.
/**  In osg examples you find it as class "MoveEarthySkyWithEyePointTransform"
*    It is used by entities like skybox and water.
*/
class EyeTransform : public osg::Transform
{
    public:

        /** Get the transformation matrix which moves from local coords to world coords.*/
        virtual bool                        computeLocalToWorldMatrix( osg::Matrix& matrix, osg::NodeVisitor* nv ) const 
                                            {
                                                osgUtil::CullVisitor* cv = dynamic_cast< osgUtil::CullVisitor* >( nv );
                                                if ( cv )
                                                {
                                                    osg::Vec3 eyePointLocal = cv->getEyeLocal();
                                                    matrix.preMult( osg::Matrix::translate( eyePointLocal.x(), eyePointLocal.y(), eyePointLocal.z() ) );
                                                }
                                                return true;
                                            }

        /** Get the transformation matrix which moves from world coords to local coords.*/
        virtual bool                        computeWorldToLocalMatrix( osg::Matrix& matrix, osg::NodeVisitor* nv ) const
                                            {    
                                                osgUtil::CullVisitor* cv = dynamic_cast< osgUtil::CullVisitor* >( nv );
                                                if ( cv )
                                                {
                                                    osg::Vec3 eyePointLocal = cv->getEyeLocal();
                                                    matrix.postMult( osg::Matrix::translate( -eyePointLocal.x(), -eyePointLocal.y(), -eyePointLocal.z() ) );
                                                }
                                                return true;
                                            }
};

//! Visitor for getting the world transformation considering all PositionAttitudeTransform nodes in path
class TransformationVisitor : public osg::NodeVisitor
{
    public:
                                            TransformationVisitor( osg::NodeVisitor::TraversalMode tm = osg::NodeVisitor::TRAVERSE_ALL_CHILDREN ) :
                                                osg::NodeVisitor( tm )
                                            {
                                                // we take all nodes
                                                setTraversalMask( 0xffffffff );
                                            }

                                            ~TransformationVisitor() {}

        void                                apply( osg::PositionAttitudeTransform& node )
                                            {                        
                                                _matrix *= computeLocalToWorld( getNodePath() );
                                            }

        const osg::Matrixf&                 getMatrix()
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

                                            TexMatCallback( osg::TexMat& tm ) : 
                                             _texMat( tm ),
                                             _R( osg::Matrix::rotate( osg::DegreesToRadians( 90.0f ), 1.0f, 0.0f, 0.0f ) )
                                            {
                                            }

        virtual void                        operator()( osg::Node* node, osg::NodeVisitor* nv )
                                            {
                                                osgUtil::CullVisitor* cv = dynamic_cast< osgUtil::CullVisitor* >( nv );
                                                if ( cv )
                                                {
                                                    const osg::Matrix& MV = cv->getModelViewMatrix();
                                                    osg::Quat q;
                                                    MV.get( q );
                                                    const osg::Matrix C = osg::Matrix::rotate( q.inverse() );

                                                    _texMat.setMatrix( C * _R );
                                                }
                                                traverse( node, nv );
                                            }

        osg::TexMat&                        _texMat;

        osg::Matrix                         _R;
};

} // namespace CTD

#endif //_CTD_UTILS_H_
