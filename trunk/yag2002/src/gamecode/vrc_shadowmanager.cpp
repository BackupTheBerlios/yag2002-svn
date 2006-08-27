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
 #   author:            ali botorabi (boto)
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_shadowmanager.h"

#include <osg/CullFace>
#include <osg/TexEnvCombine>
#include <osg/TexEnv>
#include <osg/TexGenNode>
#include <osg/CameraNode>
#include <osg/PolygonOffset>

// Implementation of ShadowManager
YAF3D_SINGLETON_IMPL( vrc::ShadowManager )

namespace vrc
{

// Update callback class for camera and tex generation
class UpdateCameraAndTexGenCallback : public osg::NodeCallback
{
    public:

                                                UpdateCameraAndTexGenCallback( osg::Vec3f lightpos, osg::CameraNode* p_cameraNode, osg::Uniform* p_texgenMatrix ):
                                                 _lightPosition( lightpos ),
                                                 _cameraNode( p_cameraNode ),
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
                                                    // first update subgraph to make sure objects are all moved into position
                                                    traverse( p_node, p_nv );

                                                    if ( _updateNodes )
                                                    {
                                                        _shadowBB.init();
                                                        // now compute the camera's view and projection matrix to point at the shadower (the camera's children)
                                                        for( unsigned int i = 0; i < _cameraNode->getNumChildren(); ++i )
                                                            _shadowBB.expandBy( _cameraNode->getChild(i)->getBound() );

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
                                                        _cameraNode->setReferenceFrame( osg::CameraNode::ABSOLUTE_RF );
                                                        _cameraNode->setProjectionMatrixAsFrustum( -_frustomCorner, _frustomCorner, -_frustomCorner, _frustomCorner, _nearZ, _farZ );
                                                        _cameraNode->setViewMatrixAsLookAt( _lightPosition, _shadowBB.center(), osg::Vec3( 0.0f, 1.0f, 0.0f ) );

                                                        // compute the matrix which takes a vertex from local coords into tex coords
                                                        // will use this later to specify osg::TexGen
                                                        _MVPT = _cameraNode->getViewMatrix() *
                                                                _cameraNode->getProjectionMatrix() *
                                                                osg::Matrix::translate( 1.0, 1.0, 1.0 ) *
                                                                osg::Matrix::scale( 0.5f, 0.5f, 0.5f );

                                                        
                                                         _updateLightPosition = false;                                                        
                                                    }

                                                    // update the texture generation matrix
                                                    _p_texgenMatrix->set( osg::Matrixf::inverse( yaf3d::Application::get()->getSceneView()->getViewMatrix() )* _MVPT );
                                                }

    protected:

        virtual                                 ~UpdateCameraAndTexGenCallback() {}

        osg::Vec3f                              _lightPosition;

        osg::ref_ptr< osg::CameraNode >         _cameraNode;

        osg::Uniform*                           _p_texgenMatrix;

        osg::BoundingSphere                     _shadowBB;

        bool                                    _updateNodes;

        bool                                    _updateLightPosition;

        float                                   _nearZ;

        float                                   _farZ;

        float                                   _frustomCorner;

        osg::Matrix                             _MVPT;
};

class CameraCullCallback : public osg::NodeCallback
{
    public:
                                                CameraCullCallback() {}

        void                                    operator()( osg::Node* p_node, osg::NodeVisitor* p_nv )
                                                {

                                                }

    protected:

        virtual                                 ~CameraCullCallback() {}
};


//TODO: substitude shadow channel and shadow texture size in shader code given the params in setup

// vertex shader
static const char glsl_vp[] =
    "/*                                                                                 \n"
    "* Vertex shader for shadow mapping                                                 \n"
    "* http://yag2002.sf.net                                                            \n"
    "* 06/27/2006                                                                       \n"
    "*/                                                                                 \n"
    "uniform mat4 texgenMatrix;                                                         \n"
    "varying vec4 diffuse, ambient;                                                     \n"
    "varying vec3 normal, lightDir, halfVector;                                         \n"
    "varying vec2 baseTexCoords;                                                        \n"
    "const   int  shadowTexChannel = 1;                                                 \n"
    "                                                                                   \n"
    "void main()                                                                        \n"
    "{                                                                                  \n"
    "   normal      = normalize( gl_NormalMatrix * gl_Normal );                         \n"
    "   lightDir    = normalize( vec3( gl_LightSource[ 0 ].position ) );                \n"
    "   halfVector  = normalize( gl_LightSource[ 0 ].halfVector.xyz );                  \n"
    "   diffuse     = gl_FrontMaterial.diffuse * gl_LightSource[ 0 ].diffuse;           \n"
    "   ambient     = gl_FrontMaterial.ambient * gl_LightSource[ 0 ].ambient;           \n"
    "   ambient     += gl_LightModel.ambient * gl_FrontMaterial.ambient;                \n"
    "                                                                                   \n"
    "   vec4 pos    =  gl_ModelViewMatrix * gl_Vertex;                                  \n"
    "   gl_TexCoord[ shadowTexChannel ] = texgenMatrix * pos;                           \n"
    "   gl_Position   = gl_ModelViewProjectionMatrix * gl_Vertex;                       \n"
    "   baseTexCoords = gl_MultiTexCoord0.st;                                           \n"
    "}                                                                                  \n"
;

static char glsl_fp[] =
    "/*                                                                                 \n"
    "* Fragment shader for shadow mapping                                               \n"
    "* http://yag2002.sf.net                                                            \n"
    "* 06/27/2006                                                                       \n"
    "*/                                                                                 \n"
    "uniform sampler2D       baseTexture;                                               \n"
    "uniform sampler2DShadow shadowTexture;                                             \n"
    "uniform vec2            ambientBias;                                               \n"
    "varying vec4            diffuse, ambient;                                          \n"
    "varying vec3            normal, lightDir, halfVector;                              \n"
    "varying vec2            baseTexCoords;                                             \n"
    "const   int             shadowTexChannel = 1;                                      \n"
    "const   float           shadowTexSize    = 1024.0;                                 \n"
    "                                                                                   \n"
    "void main(void)                                                                    \n"
    "{                                                                                  \n"
    "   vec3 n,halfV;                                                                   \n"
    "   float NdotL,NdotHV;                                                             \n"
    "   // calculate lighting                                                           \n"
    "   vec4 color = ambient;                                                           \n"
    "   n = normalize( normal );                                                        \n"
    "   NdotL = max(dot( n, lightDir ), 0.0 );                                          \n"
    "   if ( NdotL > 0.0 )                                                              \n"
    "   {                                                                               \n"
    "       color += diffuse * NdotL;                                                   \n"
    "       halfV = normalize( halfVector );                                            \n"
    "       NdotHV = max( dot( n,halfV ), 0.0 );                                        \n"
//  "       color += gl_FrontMaterial.specular *                                        \n"
//  "               gl_LightSource[0].specular *                                        \n"
//  "               pow(NdotHV, gl_FrontMaterial.shininess);                            \n"
    "   }                                                                               \n"
    "   // smooth the shadow texel                                                      \n"
    "   // number of neighboring textels                                                \n"
    "   float cells = 1.0 / 9.0;                                                        \n"
    "   // coordinate offset depending on shadow texture size                           \n"
    "   float co = 1.0 / shadowTexSize;                                                 \n"
    "   vec3 shadowCoord0 =                                                             \n"
    "      gl_TexCoord[ shadowTexChannel ].xyz / gl_TexCoord[ shadowTexChannel ].w;     \n"
    "                                                                                   \n"
    "   vec3 shadowCoord1 = shadowCoord0 + vec3( -co,  co, 0.0 );                       \n"
    "   vec3 shadowCoord2 = shadowCoord0 + vec3(   0,  co, 0.0 );                       \n"
    "   vec3 shadowCoord3 = shadowCoord0 + vec3(  co,  co, 0.0 );                       \n"
    "   vec3 shadowCoord4 = shadowCoord0 + vec3(  co,   0, 0.0 );                       \n"
    "   vec3 shadowCoord5 = shadowCoord0 + vec3(  co, -co, 0.0 );                       \n"
    "   vec3 shadowCoord6 = shadowCoord0 + vec3( 0.0, -co, 0.0 );                       \n"
    "   vec3 shadowCoord7 = shadowCoord0 + vec3( -co, -co, 0.0 );                       \n"
    "   vec3 shadowCoord8 = shadowCoord0 + vec3( -co,   0, 0.0 );                       \n"
    "   vec3 shadowColor  = shadow2D( shadowTexture, shadowCoord0 ).rgb * cells +       \n"
    "                       shadow2D( shadowTexture, shadowCoord1 ).rgb * cells +       \n"
    "                       shadow2D( shadowTexture, shadowCoord2 ).rgb * cells +       \n"
    "                       shadow2D( shadowTexture, shadowCoord3 ).rgb * cells +       \n"
    "                       shadow2D( shadowTexture, shadowCoord4 ).rgb * cells +       \n"
    "                       shadow2D( shadowTexture, shadowCoord5 ).rgb * cells +       \n"
    "                       shadow2D( shadowTexture, shadowCoord6 ).rgb * cells +       \n"
    "                       shadow2D( shadowTexture, shadowCoord7 ).rgb * cells +       \n"
    "                       shadow2D( shadowTexture, shadowCoord8 ).rgb * cells;        \n"
    "                                                                                   \n"
    "   vec4 texcolor = color * texture2D( baseTexture, baseTexCoords );                \n"
    "   gl_FragColor  = vec4(                                                                 \n"
    "                         texcolor.rgb * ( ambientBias.x + shadowColor * ambientBias.y ), \n"
    "                         step( 0.5, texcolor.a )                                         \n"
    "                       );                                                                \n"
    "}                                                                                  \n"
;

ShadowManager::ShadowManager() :
_shadowTextureWidth( 1024 ),
_shadowTextureHeight( 1024 ),
_shadowTextureUnit( 1 ),
_shadowAmbient( 0.2f ),
_enable( false ),
_lightPosition( osg::Vec3f( 20.0f, 20.0f, 280.0f ) ),
_shadowAmbientBias( osg::Vec2f( 0.3, 0.9f ) ),
_p_updateCallback( NULL ),
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
    osg::Geometry* p_geom = osg::createTexturedQuadGeometry( osg::Vec3( 0, 0, 0 ), osg::Vec3( size.x(), 0.0, 0.0 ), osg::Vec3( 0.0, size.y(), 0.0 ) );
    osg::StateSet* p_stateset = p_geom->getOrCreateStateSet();
    p_stateset->setTextureAttributeAndModes( 0, p_texture,osg::StateAttribute::ON );
    p_stateset->setTextureAttributeAndModes( 1, p_texture, osg::StateAttribute::ON );
    p_stateset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    p_geode->addDrawable( p_geom );

    osg::CameraNode* p_camera = new osg::CameraNode;

    // set the projection matrix
    p_camera->setProjectionMatrix(osg::Matrix::ortho2D( 0, size.x(), 0, size.y() ) );

    // set the view matrix
    p_camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    p_camera->setViewMatrix( osg::Matrix::identity() );

    p_camera->setViewport( static_cast< int >( pos.x() ), static_cast< int >( pos.y() ), static_cast< int >( size.x() ), static_cast< int >( size.y() ) );

    // only clear the depth buffer
    p_camera->setClearMask( GL_DEPTH_BUFFER_BIT );

    // draw subgraph after main camera view.
    p_camera->setRenderOrder( osg::CameraNode::POST_RENDER );

    p_camera->addChild( p_geode );

    return p_camera;
}

void ShadowManager::setup( unsigned int shadowTextureWidth, unsigned int shadowTextureHeight, unsigned int shadowTextureUnit, float shadowAmbient )
{
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
        _shadowCameraGroup = new osg::CameraNode;
        _shadowCameraGroup->setName( "_shadowCameraGroup" );

        _shadowCameraGroup->setClearMask( GL_DEPTH_BUFFER_BIT );
        _shadowCameraGroup->setClearColor( osg::Vec4( 1.0f, 1.0f, 1.0f, 1.0f ) );
        _shadowCameraGroup->setComputeNearFarMode( osg::CameraNode::DO_NOT_COMPUTE_NEAR_FAR );

        // set viewport
        _shadowCameraGroup->setViewport( 0, 0, shadowTextureWidth, shadowTextureHeight );

        osg::StateSet* p_localstateset = _shadowCameraGroup->getOrCreateStateSet();
        p_localstateset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

        float factor = 2.0f;
        float units  = 2.0f;

        osg::ref_ptr< osg::PolygonOffset > p_polygonoffset = new osg::PolygonOffset;
        p_polygonoffset->setFactor( factor );
        p_polygonoffset->setUnits( units );
        p_localstateset->setAttribute( p_polygonoffset.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
        p_localstateset->setMode( GL_POLYGON_OFFSET_FILL, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

        osg::ref_ptr< osg::CullFace > p_cullface = new osg::CullFace;
        p_cullface->setMode( osg::CullFace::FRONT );
        p_localstateset->setAttribute( p_cullface.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
        p_localstateset->setMode( GL_CULL_FACE, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );


        // set the camera to render before the main camera.
        _shadowCameraGroup->setRenderOrder( osg::CameraNode::PRE_RENDER );

        // tell the camera to use OpenGL frame buffer object where supported.
        _shadowCameraGroup->setRenderTargetImplementation( osg::CameraNode::FRAME_BUFFER_OBJECT );

        // attach the texture and use it as the depth buffer.
        _shadowCameraGroup->attach( osg::CameraNode::DEPTH_BUFFER, p_texture );

        _shadowSceneGroup->addChild( _shadowCameraGroup.get() );

        // store the shadow map texture for debug display
        _p_shadowMapTexture = p_texture;

        // set camera's cull callback
        //_p_cullCallback = new CameraCullCallback;
        //_shadowSceneGroup->setCullCallback( _p_cullCallback );
    }

    // set the shadowed subgraph so that it uses the p_texture and tex gen settings.
    {
        _shadowedGroup = new osg::Group;
        _shadowedGroup->setName( "_shadowedNodesGroup" );
        _shadowSceneGroup->addChild( _shadowedGroup.get() );

        osg::StateSet* p_stateset = _shadowedGroup->getOrCreateStateSet();
        p_stateset->setTextureAttributeAndModes( shadowTextureUnit, p_texture, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

        osg::Program* p_program = new osg::Program;
        p_stateset->setAttribute( p_program );

        osg::Shader* p_vertexshader = new osg::Shader( osg::Shader::VERTEX, glsl_vp );
        p_program->addShader( p_vertexshader );

        osg::Shader* p_fragmentshader = new osg::Shader( osg::Shader::FRAGMENT, glsl_fp );
        p_program->addShader( p_fragmentshader );

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
        _p_updateCallback = new UpdateCameraAndTexGenCallback( _lightPosition, _shadowCameraGroup.get(), p_texgenMatrix );
        _shadowedGroup->setUpdateCallback( _p_updateCallback );
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
            _shadowSceneGroup = NULL;
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
        }
    }
    else
    {
        if ( !_debugDisplay.valid() )
            return;

        _shadowedGroup->removeChild( _debugDisplay.get() );
        _debugDisplay = NULL;
    }
}

void ShadowManager::addShadowNode( osg::Node* p_node )
{
    if ( !_enable )
        return;

    _shadowedGroup->addChild( p_node );
    _shadowCameraGroup->addChild( p_node );
    // force updating shadow nodes in next callback
    _p_updateCallback->updateNodes();
}

void ShadowManager::removeShadowNode( osg::Node* p_node )
{
    if ( !_enable )
        return;

    _shadowedGroup->removeChild( p_node );
    _shadowCameraGroup->removeChild( p_node );
    // force updating shadow nodes in next callback
    _p_updateCallback->updateNodes();
}

void ShadowManager::updateShadowArea()
{
    if ( !_enable )
        return;

    // force updating shadow nodes in next callback
    _p_updateCallback->updateNodes();
}

void ShadowManager::setLightPosition( const osg::Vec3f& position )
{
    if ( !_enable )
        return;

    _lightPosition = position;
    _p_updateCallback->setLightPosition( _lightPosition );
}

void ShadowManager::setShadowColorGainAndBias( float gain, float bias )
{
    if ( !_enable )
        return;

    _shadowAmbientBias._v[ 0 ] = gain;
    _shadowAmbientBias._v[ 1 ] = bias;

    _p_colorGainAndBiasParam->set( _shadowAmbientBias );
}

} // namespace vrc
