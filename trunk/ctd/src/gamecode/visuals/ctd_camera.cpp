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

#include <ctd_main.h>
#include "ctd_camera.h"

using namespace std;
using namespace osg; 

namespace CTD
{

//! Input handler for setting the view matrix in 'frame' callback phase
class CameraFrameHandler : public GenericInputHandler< EnCamera >
{
    public:

        explicit                        CameraFrameHandler( EnCamera* p_camEntity ) : 
                                          GenericInputHandler< EnCamera >( p_camEntity ),
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
            osg::Matrixf inv = Matrixf::inverse( mat );

            // adjust Z-UP
            static osg::Matrixf adjustZ_Up ( osg::Matrixf::rotate( -M_PI / 2.0f, 1.0f, 0.0f, 0.0f ) );

            // set view matrix
            Application::get()->getSceneView()->setViewMatrix( osg::Matrixf( inv.ptr() ) * adjustZ_Up  );

            // reset update flag
            getUserObject()->_needUpdate = false;
         }
    }
    return false;
}

//! Implement and register the camera entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( CameraEntityFactory );

EnCamera::EnCamera() :
_isPersistent( false ),
_fov( 60.00 ),
_nearClip( 0.1f ),
_farClip( 1000.0f ),
_backgroudColor( Vec3f( 0.2f, 0.2f, 0.2f ) ),
_needUpdate ( false ),
_p_cameraHandler( NULL ),
_pitch( 0 ),
_yaw( 0 )
{
    // register entity attributes
    getAttributeManager().addAttribute( "position"          , _position         );
    getAttributeManager().addAttribute( "rotation"          , _rotation         );
    getAttributeManager().addAttribute( "fov"               , _fov              );
    getAttributeManager().addAttribute( "nearClip"          , _nearClip         );
    getAttributeManager().addAttribute( "farClip"           , _farClip          );
    getAttributeManager().addAttribute( "backgroundColor"   , _backgroudColor   );
}

EnCamera::~EnCamera()
{
    // destroy the input handler, this will deregister and delete our handler from viewer's handler list
    _p_cameraHandler->destroyHandler();
}

void EnCamera::handleNotification( const EntityNotification& notification )
{
    // handle notifications
    switch( notification.getId() )
    {
        // for every subsequent level loading we must register outself again for getting updating
        case CTD_NOTIFY_NEW_LEVEL_INITIALIZED:
            break;

        // we have to trigger the deletion ourselves! ( this entity can be peristent )
        case CTD_NOTIFY_SHUTDOWN:

            if ( _isPersistent )
                EntityManager::get()->deleteEntity( this );
            break;

        default:
            ;
    }
}

void EnCamera::initialize()
{
    unsigned int width, height;
    Application::get()->getScreenSize( width, height );
    Application::get()->getSceneView()->setProjectionMatrixAsPerspective( _fov, ( float( width ) / float( height ) ), _nearClip, _farClip );
    Application::get()->getSceneView()->setClearColor( osg::Vec4f( _backgroudColor, 1.0f ) );

    _curPosition = _position;
    _curRotation = osg::Quat( 
                                osg::DegreesToRadians( _rotation.x() ), osg::Vec3f( 0.0f, 1.0f, 0.0f ), // roll
                                osg::DegreesToRadians( _rotation.y() ), osg::Vec3f( 1.0f, 0.0f, 0.0f ), // pitch
                                osg::DegreesToRadians( _rotation.z() ), osg::Vec3f( 0.0f, 0.0f, 1.0f )  // yaw
                            );

    setCameraTranslation( _curPosition, _curRotation );

    // setup the event handler for handling 'frame' callbacks (for setting the view matrix)
    _p_cameraHandler = new CameraFrameHandler( this );

    EntityManager::get()->registerUpdate( this, true );         // register entity in order to get updated per simulation step
    EntityManager::get()->registerNotification( this, true );   // register entity in order to get notifications (e.g. from menu entity)
}

void EnCamera::setEnable( bool enable )
{
    _p_cameraHandler->setEnable( enable );
}

void EnCamera::updateEntity( float deltaTime )
{
    // we may add camera fx here later
}

} // namespace CTD
