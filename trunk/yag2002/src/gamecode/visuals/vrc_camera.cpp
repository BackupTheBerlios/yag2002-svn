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
 # entity camera
 #  this entity can be used by other entities (e.g. player) for 
 #  controling the rendering view.
 #
 #   date of creation:  04/21/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_camera.h"

namespace vrc
{

//! Input handler for setting the view matrix in 'frame' callback phase
class CameraFrameHandler : public vrc::gameutils::GenericInputHandler< EnCamera >
{
    public:

        explicit                        CameraFrameHandler( EnCamera* p_camEntity ) : 
                                          vrc::gameutils::GenericInputHandler< EnCamera >( p_camEntity ),
                                          _enable( true )
                                        {}

        virtual                         ~CameraFrameHandler() {};

        bool                            handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

        void                            setEnable( bool enable )
                                        {
                                            _enable = enable;
                                        }

    protected:

        bool                            _enable;
};

bool CameraFrameHandler::handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa )
{
    if ( !_enable )
        return false;

    if ( ea.getEventType() == osgGA::GUIEventAdapter::FRAME )
    {
        // update camera matrix if needed
        if ( getUserObject()->_needUpdate )
        {
            // setup the view matrix basing on position and transformation
            osg::Matrixf trans;
            trans.makeTranslate( _p_userObject->_curPosition.x(), _p_userObject->_curPosition.y(), _p_userObject->_curPosition.z() );
            osg::Matrix rot;
            rot.makeRotate( _p_userObject->_curRotation );

            // add rotation offset (note: we do not rotate about world coord system)
            rot = _p_userObject->_offsetMatrixRotation * rot;

            osg::Matrixf mat;
            mat = rot * trans;

            // add position offset here
            mat = _p_userObject->_offsetMatrixPosition * mat;

            //  inverse the matrix
            osg::Matrixf inv = osg::Matrixf::inverse( mat );

            // adjust Z-UP
            static osg::Matrixf adjustZ_Up ( osg::Matrixf::rotate( -osg::PI / 2.0f, 1.0f, 0.0f, 0.0f ) );

            // set view matrix
            yaf3d::Application::get()->getSceneView()->setViewMatrix( osg::Matrixf( inv.ptr() ) * adjustZ_Up  );

            // reset update flag
            getUserObject()->_needUpdate = false;
         }
    }
    return false;
}

//! Implement and register the camera entity factory
YAF3D_IMPL_ENTITYFACTORY( CameraEntityFactory );

EnCamera::EnCamera() :
_fov( 60.00 ),
_nearClip( 0.5f ),
_farClip( 5000.0f ),
_backgroundColor( osg::Vec3f( 0.2f, 0.2f, 0.2f ) ),
_isPersistent( false ),
_pitch( 0 ),
_yaw( 0 ),
_needUpdate ( false ),
_p_cameraHandler( NULL )
{
    // register entity attributes
    getAttributeManager().addAttribute( "position"          , _position         );
    getAttributeManager().addAttribute( "rotation"          , _rotation         );
    getAttributeManager().addAttribute( "fov"               , _fov              );
    getAttributeManager().addAttribute( "nearClip"          , _nearClip         );
    getAttributeManager().addAttribute( "farClip"           , _farClip          );
    getAttributeManager().addAttribute( "backgroundColor"   , _backgroundColor  );
}

EnCamera::~EnCamera()
{
    // destroy the input handler, this will deregister and delete our handler from viewer's handler list
    if ( _p_cameraHandler )
        _p_cameraHandler->destroyHandler();
}

void EnCamera::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle notifications
    switch( notification.getId() )
    {
        // re-setup camera when an attribute changed
        case YAF3D_NOTIFY_ENTITY_ATTRIBUTE_CHANGED:

            setupCamera();
            break;
             
        // we have to trigger the deletion ourselves! ( this entity can be peristent )
        case YAF3D_NOTIFY_SHUTDOWN:

            if ( _isPersistent )
                yaf3d::EntityManager::get()->deleteEntity( this );
            break;

        default:
            ;
    }
}

void EnCamera::initialize()
{
    // setup the camera, fov, near / far plane etc.
    setupCamera();

    // setup the event handler for handling 'frame' callbacks ( for setting the view matrix )
    _p_cameraHandler = new CameraFrameHandler( this );

    // register entity in order to get notifications
    yaf3d::EntityManager::get()->registerNotification( this, true );
}

void EnCamera::setupCamera()
{
    unsigned int width, height;
    yaf3d::Application::get()->getScreenSize( width, height );
    yaf3d::Application::get()->getSceneView()->setProjectionMatrixAsPerspective( _fov, ( float( width ) / float( height ) ), _nearClip, _farClip );
    yaf3d::Application::get()->getSceneView()->setClearColor( osg::Vec4f( _backgroundColor, 1.0f ) );
    // avoid overriding our near and far plane setting by scene viewer
    yaf3d::Application::get()->getSceneView()->setComputeNearFarMode( osgUtil::CullVisitor::DO_NOT_COMPUTE_NEAR_FAR );

    _curPosition = _position;
    _curRotation = osg::Quat( 
                                osg::DegreesToRadians( _rotation.x() ), osg::Vec3f( 0.0f, 1.0f, 0.0f ), // roll
                                osg::DegreesToRadians( _rotation.y() ), osg::Vec3f( 1.0f, 0.0f, 0.0f ), // pitch
                                osg::DegreesToRadians( _rotation.z() ), osg::Vec3f( 0.0f, 0.0f, 1.0f )  // yaw
                            );

    setCameraTranslation( _curPosition, _curRotation );
}

void EnCamera::setEnable( bool enable )
{
    _p_cameraHandler->setEnable( enable );
}

} // namespace vrc
