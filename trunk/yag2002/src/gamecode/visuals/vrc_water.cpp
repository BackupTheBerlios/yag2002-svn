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
 # entity water
 #
 # the water shader is basing on RenderMonkey's Reflection/Refraction
 #  example
 #
 #   date of creation:  03/26/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include <osg/Program>
#include <osg/Shader>
#include <osg/Uniform>
#include <osg/Texture3D>
#include <osgUtil/UpdateVisitor>
#include "vrc_water.h"
#include "../extern/Noise.h"

namespace vrc
{
// shader sampler locations
#define LOCATION_CUBEMAP_SAMPLER    0
#define LOCATION_NOISE_SAMPLER      1
// take care that this bin number is the highest in the scene in order to correctly render the water transparency
#define WATER_RENDERBIN_NUMBER      30

// helper classes and shader code
//---------------------------------------------------
static const char glsl_vp[] =
    "/*                                                                                         \n"
    "* Vertex shader for ocean water simulation                                                 \n"
    "* http://yag2002.sf.net                                                                    \n"
    "* 03/26/2005                                                                               \n"
    "*/                                                                                         \n"
    "uniform vec4 viewPosition;                                                                 \n"
    "uniform vec4 scale;                                                                        \n"
    "                                                                                           \n"
    "varying vec3  vTexCoord;                                                                   \n"
    "varying vec3  vViewVec;                                                                    \n"
    "varying float fog;                                                                         \n"
    "                                                                                           \n"
    "void main(void)                                                                            \n"
    "{                                                                                          \n"
    "   vec4 Position = gl_Vertex.xyzw;                                                         \n"
    "   vTexCoord     = Position.xyz * scale.xyz;                                               \n"
    "   vViewVec      = Position.xyz - viewPosition.xyz;                                        \n"
    "   gl_Position   = gl_ModelViewProjectionMatrix * Position;                                \n"

    "   // depth value for fog computation                                                      \n"
    "   gl_FogFragCoord = length(vViewVec);                                                     \n"
    "   // linear fog computation                                                               \n"
    "   float fogScale = 1.0 / (gl_Fog.end - gl_Fog.start);                                     \n"
    "   fog = (gl_Fog.end - gl_FogFragCoord) * fogScale;                                        \n"
    "   fog = clamp(fog, 0.0, 1.0);                                                             \n"
    "}                                                                                          \n"
;

static const char glsl_fp[] =
    "/*                                                                                         \n"
    "* Fragment shader for ocean water simulation                                               \n"
    "* http://yag2002.sf.net                                                                    \n"
    "* 03/26/2005                                                                               \n"
    "*/                                                                                         \n"
    "uniform sampler3D   samplerNoise;                                                          \n"
    "uniform samplerCube samplerSkyBox;                                                         \n"
    "                                                                                           \n"
    "uniform float transparency;                                                                \n"
    "uniform vec4  waterColor;                                                                  \n"
    "uniform float fadeExp;                                                                     \n"
    "uniform float fadeBias;                                                                    \n"
    "uniform float deltaNoise;                                                                  \n"
    "uniform float deltaWave;                                                                   \n"
    "uniform vec4  scale;                                                                       \n"
    "                                                                                           \n"
    "varying vec3  vTexCoord;                                                                   \n"
    "varying vec3  vViewVec;                                                                    \n"
    "varying float fog;                                                                         \n"
    "                                                                                           \n"
    "void main(void)                                                                            \n"
    "{                                                                                          \n"
    "   vec3 tcoord = vTexCoord;                                                                \n"
    "   tcoord.x += deltaNoise;                                                                 \n"
    "   tcoord.y += deltaNoise;                                                                 \n"
    "   tcoord.z += deltaWave;                                                                  \n"
    "                                                                                           \n"
    "   vec4 noisy = texture3D(samplerNoise, tcoord);                                           \n"
    "   vec3 bump = vec3(0, 0, 2.0 * noisy.z + 1.0);                                            \n"
    "   // find the reflection vector                                                           \n"
    "   vec3 reflVec = reflect(vViewVec, bump);                                                 \n"
    "   vec4 refl = textureCube(samplerSkyBox, reflVec.yzx);                                    \n"
    "                                                                                           \n"
    "   // interpolate between the water color and reflection                                   \n"
    "   float lrp = 1.0 - dot(-normalize(vViewVec), bump);                                      \n"
    "   vec4 col = mix(waterColor, refl, clamp(fadeBias + pow(lrp, fadeExp),0.0, 1.0));         \n"
    "                                                                                           \n"
    "   // linear fog                                                                           \n"
    "   col.xyz = mix(gl_Fog.color.xyz, col.xyz, fog);                                          \n"
    "   col.w = transparency;                                                                   \n"
    "   gl_FragColor = col;                                                                     \n"
    "}                                                                                          \n"
;

// callback for water's wave uniform deltaWave
class DeltaWaveUpdateCallback: public osg::Uniform::Callback
{
    public:

        explicit                            DeltaWaveUpdateCallback( const EnWater* p_ent ) :
                                             _p_waterEntity( p_ent )
                                            {}

        virtual                             ~DeltaWaveUpdateCallback() {}

        virtual void                        operator() ( osg::Uniform* p_uniform, osg::NodeVisitor* p_nv )
                                            {
                                                float time = static_cast< float >( p_nv->getFrameStamp()->getReferenceTime() );
                                                p_uniform->set( _p_waterEntity->_waveSpeed * time );
                                            }

    protected:

        const EnWater*                      _p_waterEntity;
};

// callback for water's noise uniform deltaNoise
class DeltaNoiseUpdateCallback: public osg::Uniform::Callback
{
    public:

         explicit                           DeltaNoiseUpdateCallback( const EnWater* p_ent ) :
                                             _p_waterEntity( p_ent )
                                            {}

        virtual                             ~DeltaNoiseUpdateCallback() {}

        virtual void                        operator() ( osg::Uniform* p_uniform, osg::NodeVisitor* p_nv )
                                            {
                                                float time = static_cast< float >( p_nv->getFrameStamp()->getReferenceTime() );
                                                p_uniform->set( _p_waterEntity->_noiseSpeed * time );
                                            }

    protected:

        const EnWater*                      _p_waterEntity;
};

// callback for water's view position update
class ViewPositionUpdateCallback: public osg::Uniform::Callback
{
    public:

                                            ViewPositionUpdateCallback()
                                            {
                                                _p_sceneView = yaf3d::Application::get()->getSceneView();
                                            }

        virtual                             ~ViewPositionUpdateCallback() {}

        virtual void                        operator() ( osg::Uniform* p_uniform, osg::NodeVisitor* /*p_nv*/ )
                                            {
                                                // get current camera position and feed it into shader
                                                osg::Vec3f viewpos;
                                                osg::Vec3f center;
                                                osg::Vec3f up;
                                                _p_sceneView->getViewMatrixAsLookAt( viewpos, center, up );
                                                osg::Vec4f pos( viewpos, 1.0f );
                                                p_uniform->set( pos );
                                            }

    protected:

        osgUtil::SceneView*                  _p_sceneView;
};


// Entity water

// shared water effect state set
osg::ref_ptr< osg::StateSet > EnWater::s_stateSet;
osg::Uniform* EnWater::s_fadeBias       = NULL;
osg::Uniform* EnWater::s_waveSpeed      = NULL;
osg::Uniform* EnWater::s_fadeExp        = NULL;
osg::Uniform* EnWater::s_noiseSpeed     = NULL;
osg::Uniform* EnWater::s_waterColor     = NULL;
osg::Uniform* EnWater::s_scale          = NULL;
osg::Uniform* EnWater::s_trans          = NULL;

//---------------------------------------------------
//! Implement and register the water entity factory
YAF3D_IMPL_ENTITYFACTORY( WaterEntityFactory )

// Implementation of water entity
EnWater::EnWater() :
_sizeX( 1000.0f ),
_sizeY( 1000.0f ),
_fadeBias( 0.3f ),
_noiseSpeed( 0.10f ),
_waveSpeed( 0.14f ),
_fadeExp( 6.0f ),
_scale( osg::Vec3f( 1.0f, 1.0f, 1.0f ) ),
_waterColor( osg::Vec3f( 0.2f, 0.25f, 0.6f ) ),
_transparency( 0.5f ),
_usedInMenu( false ),
_enable( true )
{
    // register entity attributes
    getAttributeManager().addAttribute( "usedInMenu"    , _usedInMenu           );
    getAttributeManager().addAttribute( "enable"        , _enable               );
    getAttributeManager().addAttribute( "meshFile"      , _meshFile             );
    getAttributeManager().addAttribute( "position"      , _position             );
    getAttributeManager().addAttribute( "sizeX"         , _sizeX                );
    getAttributeManager().addAttribute( "sizeY"         , _sizeY                );
    getAttributeManager().addAttribute( "fadeBias"      , _fadeBias             );
    getAttributeManager().addAttribute( "noiseSpeed"    , _noiseSpeed           );
    getAttributeManager().addAttribute( "waveSpeed"     , _waveSpeed            );
    getAttributeManager().addAttribute( "fadeExp"       , _fadeExp              );
    getAttributeManager().addAttribute( "waterColor"    , _waterColor           );
    getAttributeManager().addAttribute( "transparency"  , _transparency         );
    getAttributeManager().addAttribute( "scale"         , _scale                );

    getAttributeManager().addAttribute( "right"         , _cubeMapTextures[ 0 ] );
    getAttributeManager().addAttribute( "left"          , _cubeMapTextures[ 1 ] );
    getAttributeManager().addAttribute( "front"         , _cubeMapTextures[ 2 ] );
    getAttributeManager().addAttribute( "back"          , _cubeMapTextures[ 3 ] );
    getAttributeManager().addAttribute( "up"            , _cubeMapTextures[ 4 ] );
    getAttributeManager().addAttribute( "down"          , _cubeMapTextures[ 5 ] );
}

EnWater::~EnWater()
{
    if ( _water.get() )
    {
        removeFromTransformationNode( _water.get() );
        _water = NULL;
    }
}

void EnWater::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle notifications
    switch( notification.getId() )
    {
        // disable water rendering when in menu
        case YAF3D_NOTIFY_MENU_ENTER:

            if ( _usedInMenu )
            {
                addToTransformationNode( _water.get() );
                // refresh the shader parameters
                setShaderParams( _water.get() );
            }
            else
            {
                removeFromTransformationNode( _water.get() );
            }

            break;

        // enable water entity when get back in game
        case YAF3D_NOTIFY_MENU_LEAVE:

            if ( _usedInMenu )
            {
                removeFromTransformationNode( _water.get() );
            }
            else
            {
                addToTransformationNode( _water.get() );
                // refresh the shader parameters
                setShaderParams( _water.get() );
            }

            break;

        case YAF3D_NOTIFY_ENTITY_ATTRIBUTE_CHANGED:

            // re-create the water
            //--------------------

            removeFromTransformationNode( _water.get() );

            // delete stateset before re-setup
            s_stateSet = NULL;

            _water = setupWater();
            if ( _enable )
                addToTransformationNode( _water.get() );

            break;

        case YAF3D_NOTIFY_UNLOAD_LEVEL:

            // release the shader state set
            if ( !_usedInMenu )
                removeFromTransformationNode( _water.get() );

            break;

        // if used in menu then this entity is persisten, so we have to trigger its deletion on shutdown
        case YAF3D_NOTIFY_SHUTDOWN:

            if ( _usedInMenu )
                yaf3d::EntityManager::get()->deleteEntity( this );

            // release the shader state set
            s_stateSet = NULL;

            break;

        default:
            ;
    }
}

void EnWater::initialize()
{
    if ( _transparency > 1.0f )
        _transparency = 1.0f;

    // the water is added to entity's transform node in notification call-back, when entering game ( leaving menu )!
    _water = setupWater();

    yaf3d::EntityManager::get()->registerNotification( this, true );   // register entity in order to get notifications (e.g. from menu entity)
}

// this function is taken from osg example Shaders ( osg version 0.9.9 )
osg::Image* make3DNoiseImage(int texSize)
{
    osg::Image* image = new osg::Image;
    image->setImage(
                     texSize, texSize, texSize,
                     4, GL_RGBA, GL_UNSIGNED_BYTE,
                     new unsigned char[4 * texSize * texSize * texSize],
                     osg::Image::USE_NEW_DELETE
                   );

    const int startFrequency = 4;
    const int numOctaves = 4;

    int f, i, j, k, inc;
    double ni[3];
    double inci, incj, inck;
    int frequency = startFrequency;
    GLubyte *ptr;
    double amp = 0.5;


    for (f = 0, inc = 0; f < numOctaves; ++f, frequency *= 2, ++inc, amp *= 0.5)
    {
        SetNoiseFrequency(frequency);
        ptr = image->data();
        ni[0] = ni[1] = ni[2] = 0;

        inci = 1.0 / (texSize / frequency);
        for (i = 0; i < texSize; ++i, ni[0] += inci)
        {
            incj = 1.0 / (texSize / frequency);
            for (j = 0; j < texSize; ++j, ni[1] += incj)
            {
                inck = 1.0 / (texSize / frequency);
                for (k = 0; k < texSize; ++k, ni[2] += inck, ptr += 4)
                {
                    *(ptr+inc) = (GLubyte) (((noise3(ni) + 1.0) * amp) * 128.0);
                }
            }
        }
    }

    return image;
}

osg::Node* EnWater::setupWater()
{
    osg::Node* p_node = NULL;

    // check if a water mesh is given, if so load it and place it into level
    if ( _meshFile.length() )
    {
        p_node = yaf3d::LevelManager::get()->loadMesh( _meshFile );
        if ( !p_node )
        {
            log_warning << "could not load water mesh file: " << _meshFile << ", in '" << getInstanceName() << "'" << std::endl;
            log_warning << " creating a simple plane for water mesh." << std::endl;
        }

        setPosition( _position );
    }

    // create a simple quadratic water plane if no mesh given
    if ( !p_node )
    {
        osg::Geode* p_geode = new osg::Geode;

        osg::Vec3f coords[] =
        {
            osg::Vec3( _position.x() - _sizeX * 0.5f, _position.y() + _sizeY * 0.5f, _position.z() ),
            osg::Vec3( _position.x() - _sizeX * 0.5f, _position.y() - _sizeY * 0.5f, _position.z() ),
            osg::Vec3( _position.x() + _sizeX * 0.5f, _position.y() - _sizeY * 0.5f, _position.z() ),
            osg::Vec3( _position.x() + _sizeX * 0.5f, _position.y() + _sizeY * 0.5f, _position.z() )
        };

        osg::Vec3f normals[] =
        {
            osg::Vec3( 0.0f, 0.0f, 1.0f ),
            osg::Vec3( 0.0f, 0.0f, 1.0f ),
            osg::Vec3( 0.0f, 0.0f, 1.0f ),
            osg::Vec3( 0.0f, 0.0f, 1.0f )
        };

        osg::Geometry* p_polyGeom = new osg::Geometry;
        p_polyGeom->setVertexArray( new osg::Vec3Array( 4, coords ) );
        p_polyGeom->setNormalArray( new osg::Vec3Array( 4, normals ) );

        osg::DrawArrays* p_drawarray = new osg::DrawArrays( osg::PrimitiveSet::QUADS, 0, 4 );
        p_polyGeom->addPrimitiveSet( p_drawarray );

        p_geode->addDrawable( p_polyGeom );
        p_node = p_geode;
    }

    // check if glsl is supported before setting up the shaders ( gl context 0  is assumed )
    if ( !yaf3d::isGlslAvailable() )
        return p_node;

    // create skybox cube map texture
    std::vector< std::string > texfiles;
    texfiles.push_back( _cubeMapTextures[ 0 ] );
    texfiles.push_back( _cubeMapTextures[ 1 ] );
    texfiles.push_back( _cubeMapTextures[ 2 ] );
    texfiles.push_back( _cubeMapTextures[ 3 ] );
    texfiles.push_back( _cubeMapTextures[ 4 ] );
    texfiles.push_back( _cubeMapTextures[ 5 ] );
    _reflectmap = vrc::gameutils::readCubeMap( texfiles );

    // setup the shaders and uniforms, share the stateset
    if ( !s_stateSet.valid() )
    {
        s_stateSet = new osg::StateSet;
        osg::Program* p_program = new osg::Program;
        p_program->setName( "_waterShaderGLSL_" );

        p_program->addShader( new osg::Shader( osg::Shader::VERTEX, glsl_vp ) );
        p_program->addShader( new osg::Shader( osg::Shader::FRAGMENT, glsl_fp ) );
        s_stateSet->setAttributeAndModes( p_program, osg::StateAttribute::ON );

        s_fadeBias    = new osg::Uniform( "fadeBias",     _fadeBias                       );
        s_waveSpeed   = new osg::Uniform( "waveSpeed",    _waveSpeed                      );
        s_fadeExp     = new osg::Uniform( "fadeExp",      _fadeExp                        );
        s_noiseSpeed  = new osg::Uniform( "noiseSpeed",   _noiseSpeed                     );
        s_waterColor  = new osg::Uniform( "waterColor",   osg::Vec4f( _waterColor, 1.0f ) );
        s_scale       = new osg::Uniform( "scale",        osg::Vec4f( _scale, 1.0f )      );
        s_trans       = new osg::Uniform( "transparency", _transparency                   );

        s_stateSet->addUniform( s_fadeBias   );
        s_stateSet->addUniform( s_waveSpeed  );
        s_stateSet->addUniform( s_fadeExp    );
        s_stateSet->addUniform( s_scale      );
        s_stateSet->addUniform( s_noiseSpeed );
        s_stateSet->addUniform( s_waterColor );
        s_stateSet->addUniform( s_trans      );

        osg::Uniform* p_viewPosition = new osg::Uniform( "viewPosition", osg::Vec4f() );
        s_stateSet->addUniform( p_viewPosition );
        p_viewPosition->setUpdateCallback( new ViewPositionUpdateCallback() ); // set time view position update callback for the shader

        osg::Uniform* p_uniformDeltaWave = new osg::Uniform( "deltaWave", 0.0f );
        s_stateSet->addUniform( p_uniformDeltaWave );
        p_uniformDeltaWave->setUpdateCallback( new DeltaWaveUpdateCallback( this ) ); // set time update callback for the shader

        osg::Uniform* p_uniformDeltaNoise = new osg::Uniform( "deltaNoise", 0.0f );
        s_stateSet->addUniform( p_uniformDeltaNoise );
        p_uniformDeltaNoise->setUpdateCallback( new DeltaNoiseUpdateCallback( this ) ); // set time update callback for the shader

        // create a noise texture
        osg::Texture3D* p_noiseTexture = new osg::Texture3D;
        p_noiseTexture->setFilter( osg::Texture3D::MIN_FILTER, osg::Texture3D::LINEAR );
        p_noiseTexture->setFilter( osg::Texture3D::MAG_FILTER, osg::Texture3D::LINEAR );
        p_noiseTexture->setWrap( osg::Texture3D::WRAP_S, osg::Texture3D::REPEAT );
        p_noiseTexture->setWrap( osg::Texture3D::WRAP_T, osg::Texture3D::REPEAT );
        p_noiseTexture->setWrap( osg::Texture3D::WRAP_R, osg::Texture3D::REPEAT );
        p_noiseTexture->setImage( make3DNoiseImage( 32 ) );

        s_stateSet->setTextureAttribute( LOCATION_NOISE_SAMPLER, p_noiseTexture );
        s_stateSet->addUniform( new osg::Uniform( "samplerNoise", LOCATION_NOISE_SAMPLER ) );

        s_stateSet->setTextureAttribute( LOCATION_CUBEMAP_SAMPLER, _reflectmap.get() );
        s_stateSet->addUniform( new osg::Uniform( "samplerSkyBox", LOCATION_CUBEMAP_SAMPLER ) );

        // set lighting and culling
        s_stateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
        s_stateSet->setMode( GL_CULL_FACE, osg::StateAttribute::OFF );

        // disable depth test
        s_stateSet->setMode( GL_DEPTH, osg::StateAttribute::OFF );

        // set up transparency
        s_stateSet->setMode( GL_BLEND, osg::StateAttribute::ON );
        s_stateSet->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
        // make sure that the water is rendered at first, then all other transparent primitives
        s_stateSet->setBinNumber( WATER_RENDERBIN_NUMBER );

        // append state set to geode
        p_node->setStateSet( s_stateSet.get() );
    }

    // set shader parameters
    setShaderParams( p_node );
    // disable culling
    p_node->setCullingActive( false );

    return p_node;
}

void EnWater::setShaderParams( osg::Node* p_node )
{
    if ( !s_stateSet.valid() )
        return;

    // update / refresh the shader uniforms and texture cubemap
    s_fadeBias->set( _fadeBias );
    s_waveSpeed->set( _waveSpeed );
    s_fadeExp->set( _fadeExp );
    s_noiseSpeed->set( _noiseSpeed );
    s_waterColor->set( osg::Vec4f( _waterColor, 1.0f ) );
    s_scale->set( osg::Vec4f( _scale, 1.0f ) );
    s_trans->set( _transparency );
    s_stateSet->setTextureAttribute( LOCATION_CUBEMAP_SAMPLER, _reflectmap.get() );

    p_node->setStateSet( s_stateSet.get() );
}

}
