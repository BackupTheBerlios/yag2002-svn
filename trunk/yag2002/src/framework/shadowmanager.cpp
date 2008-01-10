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
 # a manager for handling dynamic shadows
 #
 #   date of creation:  06/27/2006
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <base.h>
#include "log.h"
#include "levelmanager.h"
#include "shadowmanager.h"
#include "shadercontainer.h"

#include <osg/ComputeBoundsVisitor>
#include <osg/TexEnvCombine>
#include <osg/TexGenNode>
#include <osg/Transform>
#include <osg/CullFace>
#include <osg/TexEnv>
#include <osg/Camera>
#include <osgDB/WriteFile>

// Implementation of ShadowManager
YAF3D_SINGLETON_IMPL( yaf3d::ShadowManager )

namespace yaf3d
{

// Update callback class for camera and tex generation
class ShadowSceneCullCallback : public osg::NodeCallback
{
    public:

                                                ShadowSceneCullCallback( osg::Vec3f lightpos, osg::Camera* p_camera, osg::Uniform* p_texgenMatrix, osg::StateSet* p_stateset ):
                                                 _lightPosition( lightpos ),
                                                 _p_camera( p_camera ),
                                                 _p_stateSet( p_stateset ),
                                                 _p_texgenMatrix( p_texgenMatrix ),
                                                 _updateNodes( true ),
                                                 _updateLightPosition( true ),
                                                 _nearZ( 0.0f ),
                                                 _farZ( 0.0f ),
                                                 _frustomCorner( 0.0f )
                                                {
                                                }

        void                                    setLightPosition( const osg::Vec3f& pos )
                                                {
                                                    _lightPosition = pos;
                                                    // set update flag so next callback will handle new light position
                                                    _updateLightPosition = true;
                                                }

        void                                    updateNodes()
                                                {
                                                    _updateNodes = true;
                                                }

        void                                    operator()( osg::Node* p_node, osg::NodeVisitor* p_nv )
                                                {

                                                    // traverse the receiving shadow nodes
                                                    {
                                                        osgUtil::CullVisitor* p_cullvisitor = static_cast< osgUtil::CullVisitor* >( p_nv );
                                                        p_cullvisitor->pushStateSet( _p_stateSet.get() );
                                                        //p_cullvisitor->setTraversalMask( ShadowManager::eReceiveShadow );
                                                        traverse( p_node, p_cullvisitor );
                                                        p_cullvisitor->popStateSet();
                                                    }
#if 1
                                                    if ( _updateNodes )
                                                    {
                                                        _shadowBB.init();

                                                        // get the bounds of receiving nodes
                                                        osg::ComputeBoundsVisitor bv( osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN );
                                                        //bv.setTraversalMask( ShadowManager::eReceiveShadow );
                                                        p_node->traverse( bv );
                                                        _shadowBB = bv.getBoundingBox();

                                                        if ( !_shadowBB.valid() )
                                                            return;

                                                        float centerDistance = ( _lightPosition - _shadowBB.center() ).length();

                                                        _nearZ = centerDistance - _shadowBB.radius();
                                                        _farZ  = centerDistance + _shadowBB.radius();
                                                        float zNearRatio = 0.001f;
                                                        if ( _nearZ < _farZ * zNearRatio )
                                                            _nearZ = _farZ * zNearRatio;

                                                        _frustomCorner   = ( _shadowBB.radius() / centerDistance ) * _nearZ;

                                                        _updateNodes = false;
                                                        // when updating nodes the MVPT must be updated too!
                                                        _updateLightPosition = true;
                                                    }

                                                    if ( _updateLightPosition )
                                                    {
                                                        _p_camera->setReferenceFrame( osg::Camera::ABSOLUTE_RF );
                                                        _p_camera->setProjectionMatrixAsFrustum( -_frustomCorner, _frustomCorner, -_frustomCorner, _frustomCorner, _nearZ, _farZ );
                                                        _p_camera->setViewMatrixAsLookAt( _lightPosition, _shadowBB.center(), osg::Vec3( 0.0f, 0.0f, 1.0f ) );

                                                        // compute the matrix which takes a vertex from local coords into tex coords
                                                        // will use this later to specify osg::TexGen
                                                        _MVPT = _p_camera->getViewMatrix() *
                                                                _p_camera->getProjectionMatrix() *
                                                                osg::Matrix::translate( 1.0, 1.0, 1.0 ) *
                                                                osg::Matrix::scale( 0.5f, 0.5f, 0.5f );

                                                         _updateLightPosition = false;
                                                    }

                                                    // collect the shadow throwing nodes in camera group
//                                                    p_nv->setTraversalMask( ShadowManager::eThrowShadow );
                                                    _p_camera->accept( *p_nv );

                                                    // update the texture generation matrix
                                                    _p_texgenMatrix->set( osg::Matrixf::inverse( yaf3d::Application::get()->getSceneView()->getViewMatrix() )* _MVPT );

// psm implementation is broken :-(
#else
                                                    {
                                                        _shadowBB.init();

                                                        // get the bounds of receiving nodes
                                                        osg::ComputeBoundsVisitor bv( osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN );
                                                        //bv.setTraversalMask( ShadowManager::eReceiveShadow );
                                                        p_node->traverse( bv );
                                                        _shadowBB = bv.getBoundingBox();

                                                        if ( !_shadowBB.valid() )
                                                            return;

                                                        float centerDistance = ( _lightPosition - _shadowBB.center() ).length();

                                                        _nearZ = centerDistance - _shadowBB.radius();
                                                        _farZ  = centerDistance + _shadowBB.radius();
                                                        float zNearRatio = 0.001f;
                                                        if ( _nearZ < _farZ * zNearRatio )
                                                            _nearZ = _farZ * zNearRatio;

                                                        _frustomCorner   = ( _shadowBB.radius() / centerDistance ) * _nearZ;
                                                    }

                                                    osg::Vec4f lightpp( _lightPosition, 1.0f );
                                                    osg::Matrixd& MV = yaf3d::Application::get()->getSceneView()->getViewMatrix();
                                                    osg::Matrixd& P  = yaf3d::Application::get()->getSceneView()->getProjectionMatrix();
                                                    osg::Matrixd  M  = P * MV;
                                                    lightpp = lightpp * M;
                                                    lightpp /= lightpp._v[ 3 ];
                                                    osg::Matrixf  MVL;
                                                    MVL.makeLookAt( osg::Vec3( lightpp._v [ 0 ], lightpp._v [ 1 ], lightpp._v [ 2 ] ) , osg::Vec3( 0.0f, 0.0f, 0.0f ), osg::Vec3( 0.0f, 0.0f, 1.0f ) );

                                                    _p_camera->setReferenceFrame( osg::Camera::ABSOLUTE_RF );
                                                    _p_camera->setProjectionMatrixAsFrustum( -_frustomCorner, _frustomCorner, -_frustomCorner, _frustomCorner, _nearZ, _farZ );
                                                    _p_camera->setViewMatrix( M * MVL );

                                                    // compute the matrix which takes a vertex from local coords into tex coords
                                                    // will use this later to specify osg::TexGen
                                                    _MVPT = osg::Matrix::translate( 1.0, 1.0, 1.0 ) * osg::Matrix::scale( 0.5f, 0.5f, 0.5f );
                                                    _MVPT = _p_camera->getProjectionMatrix() * _p_camera->getViewMatrix() * P * MV * _MVPT;
                                                    // update the texture generation matrix
                                                    _p_texgenMatrix->set( osg::Matrixf::inverse( yaf3d::Application::get()->getSceneView()->getViewMatrix() ) * _MVPT );

                                                    // collect the shadow throwing nodes in camera group
                                                    //p_nv->setTraversalMask( ShadowManager::eThrowShadow );
                                                    _p_camera->accept( *p_nv );
#endif

                                                }

    protected:

        virtual                                 ~ShadowSceneCullCallback() {}

        osg::Vec3f                              _lightPosition;

        osg::ref_ptr< osg::Camera >             _p_camera;

        osg::Uniform*                           _p_texgenMatrix;

        osg::ref_ptr< osg::StateSet >           _p_stateSet;

        osg::BoundingSphere                     _shadowBB;

        bool                                    _updateNodes;

        bool                                    _updateLightPosition;

        float                                   _nearZ;

        float                                   _farZ;

        float                                   _frustomCorner;

        osg::Matrix                             _MVPT;
};


//! Cull callback class for culling far shadow throwing nodes
class ShadowNodeCullCallback : public osg::NodeCallback
{
    public:

        //! culldistance is used for culling far shadows nodes.
        explicit                                ShadowNodeCullCallback( float culldistance )
                                                {
                                                    _cullDistance2 = culldistance * culldistance;
                                                }

        //! Update the current viewer position
        static void                             updateViewerPosition( const osg::Vec3f& pos )
                                                {
                                                    _viewerPosition = pos;
                                                }

        void                                    operator()( osg::Node* p_node, osg::NodeVisitor* p_nv )
                                                {
                                                    const osg::BoundingSphere& bb = p_node->getBound();
                                                    float dist = ( bb.center() - _viewerPosition ).length2();
                                                    if ( dist < _cullDistance2 )
                                                        traverse( p_node, p_nv );
                                                }

    protected:

        virtual                                 ~ShadowNodeCullCallback() {}

        //! Current viewer position ( main camera ) used for culling far shadow throwing nodes
        static osg::Vec3f                       _viewerPosition;

        //! Square of cull distance
        float                                   _cullDistance2;
};
osg::Vec3f ShadowNodeCullCallback::_viewerPosition;


//! Update callback class for the top shadow scene node
class ShadowSceneUpdateCallback : public osg::NodeCallback
{
    public:

                                                ShadowSceneUpdateCallback( osg::Group* p_camera ) :
                                                 _p_camera( p_camera )
                                                {
                                                }

        void                                    operator()( osg::Node* p_node, osg::NodeVisitor* p_nv )
                                                {
                                                    // get the current camera position
                                                    osg::Matrixd& mat = Application::get()->getSceneView()->getCamera()->getViewMatrix();
                                                    ShadowNodeCullCallback::updateViewerPosition( osg::Matrix::inverse( mat ).getTrans() );
                                                    // update all children of camera node
                                                    _p_camera->accept( *p_nv );
                                                }

    protected:

        virtual                                 ~ShadowSceneUpdateCallback() {}

        osg::ref_ptr< osg::Group >              _p_camera;
};



ShadowManager::ShadowManager() :
_shadowTextureWidth( 1024 ),
_shadowTextureHeight( 1024 ),
_shadowTextureUnit( 1 ),
_shadowAmbient( 0.2f ),
_enable( false ),
_lightPosition( osg::Vec3f( 20.0f, 20.0f, 280.0f ) ),
_shadowAmbientBias( osg::Vec2f( 0.3, 0.9f ) ),
_p_cullCallback( NULL ),
_p_colorGainAndBiasParam( NULL ),
_p_shadowMapTexture( NULL )
{
}

ShadowManager::~ShadowManager()
{
}

osg::Node* ShadowManager::createDebugDisplay( osg::Texture* p_texture )
{
    // position and size of overlay on screen
    osg::Vec2f pos( 20.0f, 20.0f );
    osg::Vec2f size( 200.0f, 200.0f );

    osg::Geode* p_geode = new osg::Geode;
    p_geode->setName( "_shadowMapOverlay" );
    osg::StateSet* p_stateset = p_geode->getOrCreateStateSet();
    p_stateset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    {
        osg::Geometry* p_geom = new osg::Geometry;
        osg::Vec3Array* p_vertices = new osg::Vec3Array;
        p_vertices->push_back( osg::Vec3( pos.x(), pos.y(), -10 ) );
        p_vertices->push_back( osg::Vec3( pos.x(), pos.y() + size.y(), -10 ) );
        p_vertices->push_back( osg::Vec3( pos.x() + size.x(), pos.y() + size.y(), -10 ) );
        p_vertices->push_back( osg::Vec3( pos.x() + size.x(), pos.y(), -10 ) );
        p_geom->setVertexArray( p_vertices );

        osg::Vec3Array* p_normals = new osg::Vec3Array;
        p_normals->push_back( osg::Vec3( 0.0f, 0.0f, 1.0f ) );
        p_geom->setNormalArray( p_normals );
        p_geom->setNormalBinding( osg::Geometry::BIND_OVERALL );

        osg::Vec4Array* p_colors = new osg::Vec4Array;
        p_colors->push_back( osg::Vec4( 1.0f, 1.0, 1.0f, 1.0f ) );
        p_geom->setColorArray( p_colors );
        p_geom->setColorBinding( osg::Geometry::BIND_OVERALL );

        osg::Vec2Array* p_tcoords = new osg::Vec2Array( 4 );
        ( *p_tcoords )[ 0 ].set( 0.0f, 1.0f );
        ( *p_tcoords )[ 1 ].set( 0.0f, 0.0f );
        ( *p_tcoords )[ 2 ].set( 1.0f, 0.0f );
        ( *p_tcoords )[ 3 ].set( 1.0f, 1.0f );
        p_geom->setTexCoordArray( 0, p_tcoords );

        p_geom->addPrimitiveSet( new osg::DrawArrays( GL_QUADS, 0, 4 ) );
        p_geode->addDrawable( p_geom );
        
        osg::StateSet* p_stateset = p_geom->getOrCreateStateSet();
        p_stateset->setTextureAttributeAndModes( 0, p_texture, osg::StateAttribute::ON );
    }

    osg::Camera* p_camera = new osg::Camera;
    p_camera->addChild( p_geode );

    // set the projection matrix
    p_camera->setProjectionMatrixAsOrtho2D( 0, size.x(), 0, size.y() );
    // set the view matrix
    p_camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    p_camera->setViewMatrix( osg::Matrix::identity() );
    p_camera->setViewport( static_cast< int >( pos.x() ), static_cast< int >( pos.y() ), static_cast< int >( size.x() ), static_cast< int >( size.y() ) );
    // only clear the depth buffer
    p_camera->setClearMask( GL_DEPTH_BUFFER_BIT );
    // draw subgraph after main camera view.
    p_camera->setRenderOrder( osg::Camera::POST_RENDER );

    return p_camera;
}

void ShadowManager::setup( unsigned int shadowTextureWidth, unsigned int shadowTextureHeight, unsigned int shadowTextureUnit, float shadowAmbient )
{
    log_info << "ShadowManager: setting up the shadow scene graph" << std::endl;

    if ( _enable )
    {
        log_error << "ShadowManager has already been initialized! Skipping initialization." << std::endl;
        return;
    }

    const osg::GL2Extensions* p_extensions = osg::GL2Extensions::Get( 0, true );
    if ( !p_extensions->isGlslSupported() )
    {
        log_info << "ShadowManager: GLSL is not available, dynamic shadows are disabled." << std::endl;
        _enable = false;
        return;
    }

    _shadowTextureWidth  = shadowTextureWidth;
    _shadowTextureHeight = shadowTextureHeight;
    _shadowTextureUnit   = shadowTextureUnit;
    _shadowAmbient       = shadowAmbient;

    _shadowSceneGroup = new osg::Group;
    _shadowSceneGroup->setName( "_shadowGroup" );

    osg::Texture2D* p_texture = new osg::Texture2D;
    p_texture->setTextureSize( shadowTextureWidth, shadowTextureHeight );

    p_texture->setInternalFormat( GL_DEPTH_COMPONENT );
    p_texture->setShadowComparison( true );
    p_texture->setShadowAmbient( _shadowAmbient );
    p_texture->setShadowTextureMode( osg::Texture::LUMINANCE );
    p_texture->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
    p_texture->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );

    // set up the render to texture camera.
    {
        // create the camera node
        _shadowCameraGroup = new osg::Camera;
        _shadowCameraGroup->setName( "_shadowCameraGroup" );

        _shadowCameraGroup->setClearMask( GL_DEPTH_BUFFER_BIT );
        _shadowCameraGroup->setClearColor( osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
        _shadowCameraGroup->setComputeNearFarMode( osg::Camera::DO_NOT_COMPUTE_NEAR_FAR );

        // set viewport
        _shadowCameraGroup->setViewport( 0, 0, shadowTextureWidth, shadowTextureHeight );

        osg::StateSet* p_localstateset = _shadowCameraGroup->getOrCreateStateSet();
        p_localstateset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

        float factor = 2.0f;
        float units  = 4.0f;

        _polygonOffset = new osg::PolygonOffset;
        _polygonOffset->setFactor( factor );
        _polygonOffset->setUnits( units );
        p_localstateset->setAttribute( _polygonOffset.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
        p_localstateset->setMode( GL_POLYGON_OFFSET_FILL, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

        // using culling is not good for non-closed geoms!
#if 0
        osg::ref_ptr< osg::CullFace > p_cullface = new osg::CullFace;
        p_cullface->setMode( osg::CullFace::FRONT );
        p_localstateset->setAttribute( p_cullface.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
        p_localstateset->setMode( GL_CULL_FACE, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
#else
        p_localstateset->setMode( GL_CULL_FACE, osg::StateAttribute::OFF | osg::StateAttribute::OVERRIDE );
#endif

        // set the camera to render before the main camera.
        _shadowCameraGroup->setRenderOrder( osg::Camera::PRE_RENDER );

        // tell the camera to use OpenGL frame buffer object where supported.
        _shadowCameraGroup->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );

        // attach the texture and use it as the depth buffer.
        _shadowCameraGroup->attach( osg::Camera::DEPTH_BUFFER, p_texture );

        // store the shadow map texture for debug display
        _p_shadowMapTexture = p_texture;
    }

    // set the shadowed subgraph so that it uses the p_texture and tex gen settings.
    {
        _shadowedGroup = new osg::Group;
        _shadowedGroup->setName( "_shadowedNodesGroup" );
        _shadowSceneGroup->addChild( _shadowedGroup.get() );

        osg::StateSet* p_stateset = new osg::StateSet();
        p_stateset->setTextureAttributeAndModes( shadowTextureUnit, p_texture, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

        osg::Program* p_program = new osg::Program;
        p_stateset->setAttribute( p_program );

        // setup the vertex shaders
        osg::Shader* p_vcommon = ShaderContainer::get()->getVertexShader( ShaderContainer::eCommonV );
        p_program->addShader( p_vcommon );
        osg::Shader* p_vshadowmap = ShaderContainer::get()->getVertexShader( ShaderContainer::eShadowMapV );
        p_program->addShader( p_vshadowmap );

        // setup the fragment shaders
        osg::Shader* p_fcommon = ShaderContainer::get()->getFragmentShader( ShaderContainer::eCommonF );
        p_program->addShader( p_fcommon );
        osg::Shader* p_fshadowmap = ShaderContainer::get()->getFragmentShader( ShaderContainer::eShadowMapF );
        p_program->addShader( p_fshadowmap );

        osg::Uniform* p_baseTextureSampler = new osg::Uniform( "baseTexture", 0 );
        p_stateset->addUniform( p_baseTextureSampler );

        osg::Uniform* p_shadowTextureSampler = new osg::Uniform( "shadowTexture", static_cast< int >( shadowTextureUnit ) );
        p_stateset->addUniform( p_shadowTextureSampler );

        _p_colorGainAndBiasParam = new osg::Uniform( "ambientBias", _shadowAmbientBias );
        p_stateset->addUniform( _p_colorGainAndBiasParam );

        // the texture generation matrix and its uniform are updated in camera callback
        osg::Matrixf   texgenMatrix;
        osg::Uniform* p_texgenMatrix = new osg::Uniform( "texgenMatrix", texgenMatrix );
        p_stateset->addUniform( p_texgenMatrix );

        // set an update callback to keep moving the camera and tex gen in the right direction.
        _p_cullCallback = new ShadowSceneCullCallback( _lightPosition, _shadowCameraGroup.get(), p_texgenMatrix, p_stateset );
        _shadowedGroup->setCullCallback( _p_cullCallback );

        ShadowSceneUpdateCallback* p_updateCallback = new ShadowSceneUpdateCallback( _shadowCameraGroup.get() );
        _shadowedGroup->setUpdateCallback( p_updateCallback );
    }

    // append the shadow group to top node group
    yaf3d::LevelManager::get()->getTopNodeGroup()->addChild( _shadowSceneGroup.get() );

    _enable = true;
}

void ShadowManager::enable( bool en )
{
    // free up resources on disabling
    if ( !en && _enable )
    {
        // free up the shadow scene group
        if ( _shadowSceneGroup.valid() )
        {
            yaf3d::LevelManager::get()->getTopNodeGroup()->removeChild( _shadowSceneGroup.get() );
            _shadowCameraGroup = NULL;
            _shadowedGroup     = NULL;
            _shadowSceneGroup  = NULL;
        }
    }
    else if ( en && !_enable )
    {
        setup( _shadowTextureWidth, _shadowTextureHeight, _shadowTextureUnit );
    }

    _enable = en;
}

void ShadowManager::shutdown()
{
    log_info << "ShadowManager: shutting down" << std::endl;

    // clean up resources
    enable( false );

    // destroy singleton
    destroy();
}

void ShadowManager::displayShadowMap( bool enable )
{
    // first check if the shadow manager is enabled
    if ( !_enable )
        return;

    //! FIXME: the display does not work properly, there is a problem with assigning
    //         the shadow texture to the display quad!
    if ( enable )
    {
        if ( !_debugDisplay.valid() )
        {
            _debugDisplay = createDebugDisplay( _p_shadowMapTexture );
            // add the preview pic for shadow map
            _shadowedGroup->addChild( _debugDisplay.get() );
//            Application::get()->getSceneRootNode()->addChild( _debugDisplay.get() );
        }
    }
    else
    {
        if ( !_debugDisplay.valid() )
            return;

        _shadowedGroup->removeChild( _debugDisplay.get() );
//        Application::get()->getSceneRootNode()->removeChild( _debugDisplay.get() );
        _debugDisplay = NULL;
    }
}

void ShadowManager::addShadowNode( osg::Node* p_node, unsigned int shadowmode, float culldistance )
{
    if ( !_enable )
        return;

    // setup the node mask identifying receive/throw shadow mode
    unsigned int nodemask = p_node->getNodeMask();
    nodemask &= ~( eThrowShadow | eReceiveShadow );
    p_node->setNodeMask( nodemask );
    if ( shadowmode & eThrowShadow )
    {
        nodemask |= eThrowShadow;
        p_node->setNodeMask( nodemask );

        // create a lod node in order to cull away far shadow throwing nodes
        ShadowNodeCullCallback* p_cullcallback = new ShadowNodeCullCallback( culldistance );
        osg::Group* p_cullnode = new osg::Group;
        p_cullnode->addChild( p_node );
        p_cullnode->setNodeMask( nodemask );
        p_cullnode->setCullCallback( p_cullcallback );

        _shadowCameraGroup->addChild( p_cullnode );
    }

    if ( shadowmode & eReceiveShadow )
    {
        p_node->setNodeMask( nodemask | eReceiveShadow );
        _shadowedGroup->addChild( p_node );
    }

    // force updating shadow nodes in next callback
    _p_cullCallback->updateNodes();
}

void ShadowManager::removeShadowNode( osg::Node* p_node )
{
    if ( !_enable )
        return;

    _shadowedGroup->removeChild( p_node );
    _shadowCameraGroup->removeChild( p_node );
    // force updating shadow nodes in next callback
    _p_cullCallback->updateNodes();
}

void ShadowManager::updateShadowArea()
{
    if ( !_enable )
        return;

    // force updating shadow nodes in next callback
    _p_cullCallback->updateNodes();
}

void ShadowManager::setLightPosition( const osg::Vec3f& position )
{
    if ( !_enable )
        return;

    _lightPosition = position;
    _p_cullCallback->setLightPosition( _lightPosition );
}

void ShadowManager::setShadowColorGainAndBias( float gain, float bias )
{
    if ( !_enable )
        return;

    _shadowAmbientBias._v[ 0 ] = gain;
    _shadowAmbientBias._v[ 1 ] = bias;

    _p_colorGainAndBiasParam->set( _shadowAmbientBias );
}

} // namespace yaf3d
