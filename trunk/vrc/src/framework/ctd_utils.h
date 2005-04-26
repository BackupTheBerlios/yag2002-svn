/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2004-2006, Ali Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU General Public License 
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public 
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

//! Emplode a given std string into vector elements, borrowed from evoluioN engine
std::string::size_type explode( const std::string& str, const std::string& separators, std::vector< std::string >* p_result );

//! Given a full path this function extracts the path cutting away the file name
std::string extractPath( const std::string& fullpath );

//! Given a full path this function extracts the file name
std::string extractFileName( const std::string& fullpath );

//! Given a directory this function retrieves all files inside for given extension
void getDirectoryListing( std::vector< std::string >& listing, const std::string& dir, const std::string& extension );

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

//! Update texture matrix for cubemaps ( see osg's VertexProgram example )
struct TexMatCallback : public osg::NodeCallback
{
    public:

                                            TexMatCallback( osg::TexMat& tm ) : _texMat( tm )
                                            {
                                            }

        virtual void                        operator()( osg::Node* node, osg::NodeVisitor* nv )
                                            {
                                                osgUtil::CullVisitor* cv = dynamic_cast< osgUtil::CullVisitor* >( nv );
                                                if (cv)
                                                {
                                                    const osg::Matrix& MV = cv->getModelViewMatrix();
                                                    const osg::Matrix R = osg::Matrix::rotate( osg::DegreesToRadians( 112.0f ), 0.0f, 0.0f, 1.0f )*
                                                                        osg::Matrix::rotate( osg::DegreesToRadians( 90.0f ), 1.0f, 0.0f, 0.0f );

                                                    osg::Quat q;
                                                    MV.get(q);
                                                    const osg::Matrix C = osg::Matrix::rotate( q.inverse() );

                                                    _texMat.setMatrix( C*R );
                                                }
                                                traverse( node, nv );
                                            }

        osg::TexMat&                        _texMat;
};

} // namespace CTD

#endif //_CTD_UTILS_H_
