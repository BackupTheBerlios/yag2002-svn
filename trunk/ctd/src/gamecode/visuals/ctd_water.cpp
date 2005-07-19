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
 # entity water
 #
 # the rendering code is basing on osg examples osgreflect and 
 #   osgvertexprogram 
 #
 #   date of creation:  03/26/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_water.h"

#include <osg/Program>
#include <osg/Shader>
#include <osg/Uniform>
#include <osg/Texture3D>

#include "../extern/Noise.h"

using namespace std;
using namespace CTD; 


//! Implement and register the skybox entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( WaterEntityFactory );


// the water shader is basing on RenderMonkey's Reflection/Refraction example
#define LOCATION_CUBEMAP_SAMPLER    4
#define LOCATION_NOISE_SAMPLER      3

const char glsl_vp[] =
    "uniform vec4 viewPosition;                                                                 \n"
    "uniform vec4 scale;                                                                        \n"
    "                                                                                           \n"
    "varying vec3 vTexCoord;                                                                    \n"
    "varying vec3 vNormal;                                                                      \n"
    "varying vec3 vViewVec;                                                                     \n"
    "                                                                                           \n"
    "void main(void)                                                                            \n"
    "{                                                                                          \n"
    "   vec4 Position = gl_Vertex.xyzw;                                                         \n"
    "   vTexCoord     = Position.xyz * scale.xyz;                                               \n"
    "   vViewVec      = Position.xyz - viewPosition.xyz;                                        \n"
    "   vNormal       = gl_Normal;                                                              \n"
    "   gl_Position   = gl_ModelViewProjectionMatrix * Position;                                \n"
    "}                                                                                          \n"
;

const char glsl_fp[] =
    "uniform sampler3D   samplerNoise;                                                          \n"
    "uniform samplerCube samplerSkyBox;                                                         \n"
    "                                                                                           \n"
    "uniform float time;                                                                        \n"
    "uniform float transparency;                                                                \n"
    "uniform vec4  waterColor;                                                                  \n"
    "uniform float fadeExp;                                                                     \n"
    "uniform float fadeBias;                                                                    \n"
    "uniform float noiseSpeed;                                                                  \n"
    "uniform vec4  scale;                                                                       \n"
    "                                                                                           \n"
    "uniform float waveSpeed;                                                                   \n"
    "                                                                                           \n"
    "varying vec3 vTexCoord;                                                                    \n"
    "varying vec3 vNormal;                                                                      \n"
    "varying vec3 vViewVec;                                                                     \n"
    "                                                                                           \n"
    "                                                                                           \n"
    "void main(void)                                                                            \n"
    "{                                                                                          \n"
    "   vec3 tcoord = vTexCoord;                                                                \n"
    "   tcoord.x += waveSpeed  * time;                                                          \n"
    "   tcoord.z += noiseSpeed * time;                                                          \n"
    "                                                                                           \n"
    "   vec4 noisy = texture3D(samplerNoise, tcoord);                                           \n"
    "                                                                                           \n"
    "   // Signed noise                                                                         \n"
    "   vec3 bump = 2.0 * noisy.xyz - 1.0;                                                      \n"
    "   bump.xy *= 0.15;                                                                        \n"
    "                                                                                           \n"
    "   // Make sure the normal always points upwards                                           \n"
    "   bump.z = 0.8 * abs(bump.z) + 0.2;                                                       \n"
    "                                                                                           \n"
    "   // Offset the surface normal with the bump                                              \n"
    "   bump = normalize(vNormal + bump);                                                       \n"
    "                                                                                           \n"
    "   // Find the reflection vector                                                           \n"
    "   vec3 reflVec = reflect(vViewVec, bump);                                                 \n"
    "   vec4 refl = textureCube(samplerSkyBox, reflVec.yzx);                                    \n"
    "                                                                                           \n"
    "   float lrp = 1.0 - dot(-normalize(vViewVec), bump);                                      \n"
    "                                                                                           \n"
    "   // Interpolate between the water color and reflection                                   \n"
    "   vec4 col = mix(waterColor, refl, clamp(fadeBias + pow(lrp, fadeExp),0.0, 1.0));         \n"
    "   col.w = transparency;                                                                   \n"
    "   gl_FragColor = col;                                                                     \n"
    "}                                                                                          \n"
;

// callback for water's time animation
class TimeUpdateCallback: public osg::Uniform::Callback
{
    public:
        
                                            TimeUpdateCallback() {}

        virtual                             ~TimeUpdateCallback() {}

        virtual void                        operator() ( osg::Uniform* p_uniform, osg::NodeVisitor* p_nv )
                                            {
                                                p_uniform->set( static_cast< float >( p_nv->getFrameStamp()->getReferenceTime() ) );
                                            }
};

// callback for water's view position update
class ViewPositionUpdateCallback: public osg::Uniform::Callback
{
    public:
        
                                            ViewPositionUpdateCallback() 
                                            {
                                                _p_sceneView = Application::get()->getSceneView();
                                            }

        virtual                             ~ViewPositionUpdateCallback() {}

        virtual void                        operator() ( osg::Uniform* p_uniform, osg::NodeVisitor* p_nv )
                                            {
                                                osg::Matrixd::value_type* mat = _p_sceneView->getViewMatrix().ptr();  
                                                osg::Vec4f viewpos
                                                    ( 
                                                    static_cast< float >( mat[ 13 ] ), 
                                                    static_cast< float >( mat[ 14 ] ), 
                                                    static_cast< float >( mat[ 15 ] ),
                                                    1.0f
                                                    );                                                    

                                                p_uniform->set( viewpos );
                                            }

    protected:

        osgUtil::SceneView*                  _p_sceneView;
};

// Implementation of water entity                                                           

EnWater::EnWater() :
_fadeBias( 0.3f ),
_noiseSpeed( 0.10f ),
_waveSpeed( 0.14f ),
_fadeExp( 6.0f ),
_sizeX( 1000.0f ),
_sizeY( 1000.0f ),
_scale( osg::Vec3f( 1.0f, 1.0f, 1.0f ) ),
_waterColor( osg::Vec3f( 0.2f, 0.25f, 0.6f ) ),
_transparency( 0.5f )
{
    // register entity attributes
    getAttributeManager().addAttribute( "position"              , _position              );
    getAttributeManager().addAttribute( "sizeX"                 , _sizeX                 );
    getAttributeManager().addAttribute( "sizeY"                 , _sizeY                 );
    getAttributeManager().addAttribute( "fadeBias"              , _fadeBias              );
    getAttributeManager().addAttribute( "noiseSpeed"            , _noiseSpeed            );
    getAttributeManager().addAttribute( "waveSpeed"             , _waveSpeed             );
    getAttributeManager().addAttribute( "fadeExp"               , _fadeExp               );
    getAttributeManager().addAttribute( "waterColor"            , _waterColor            );
    getAttributeManager().addAttribute( "transparency"          , _transparency          );
    getAttributeManager().addAttribute( "scale"                 , _scale                 );

    getAttributeManager().addAttribute( "right"                 , _cubeMapTextures[ 0 ]  );
    getAttributeManager().addAttribute( "left"                  , _cubeMapTextures[ 1 ]  );
    getAttributeManager().addAttribute( "front"                 , _cubeMapTextures[ 2 ]  );
    getAttributeManager().addAttribute( "back"                  , _cubeMapTextures[ 3 ]  );
    getAttributeManager().addAttribute( "up"                    , _cubeMapTextures[ 4 ]  );
    getAttributeManager().addAttribute( "down"                  , _cubeMapTextures[ 5 ]  );
}

EnWater::~EnWater()
{
}

void EnWater::handleNotification( const EntityNotification& notification )
{
    // handle notifications
    switch( notification.getId() )
    {
        // disable water rendering when in menu
        case CTD_NOTIFY_MENU_ENTER:

            removeFromTransformationNode( _water.get() );
            break;

        // enable water entity when get back in game
        case CTD_NOTIFY_MENU_LEAVE:

            addToTransformationNode( _water.get() );
            break;

        case CTD_NOTIFY_ENTITY_ATTRIBUTE_CHANGED:
            // re-create the water
            removeFromTransformationNode( _water.get() );
            _water = setupWater();
            addToTransformationNode( _water.get() );
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

    EntityManager::get()->registerNotification( this, true );   // register entity in order to get notifications (e.g. from menu entity)
}

// this function is taken from osg example Shaders ( osg version 0.9.9 )
osg::Image* make3DNoiseImage(int texSize)
{
    osg::Image* image = new osg::Image;
    image->setImage( texSize, texSize, texSize,
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
    osg::Geode* p_geode = new osg::Geode();
    // create water plane
    {
        osg::Vec3 coords[] =
        {
            osg::Vec3( _position.x() - _sizeX * 0.5f, _position.y() + _sizeY * 0.5f, _position.z() ),
            osg::Vec3( _position.x() - _sizeX * 0.5f, _position.y() - _sizeY * 0.5f, _position.z() ),
            osg::Vec3( _position.x() + _sizeX * 0.5f, _position.y() - _sizeY * 0.5f, _position.z() ),
            osg::Vec3( _position.x() + _sizeX * 0.5f, _position.y() + _sizeY * 0.5f, _position.z() )
        };
        osg::Geometry* p_polyGeom = new osg::Geometry;
        p_polyGeom->setVertexArray( new osg::Vec3Array( 4, coords) );
        osg::DrawArrays* p_drawarray = new osg::DrawArrays( osg::PrimitiveSet::QUADS, 0, 4 );        
        p_polyGeom->addPrimitiveSet( p_drawarray );

        p_geode->addDrawable( p_polyGeom );
    }

    // setup the shaders and uniforms
    osg::StateSet* p_stateSet = new osg::StateSet;
    {
        osg::Program* p_program = new osg::Program;
        p_program->setName( "_waterShaderGLSL_" );

        p_program->addShader( new osg::Shader( osg::Shader::VERTEX, glsl_vp ) );
        p_program->addShader( new osg::Shader( osg::Shader::FRAGMENT, glsl_fp ) );
        p_stateSet->setAttributeAndModes( p_program, osg::StateAttribute::ON );

        osg::Uniform* p_fadeBias    = new osg::Uniform( "fadeBias",     _fadeBias                       );
        osg::Uniform* p_waveSpeed   = new osg::Uniform( "waveSpeed",    _waveSpeed                      );
        osg::Uniform* p_fadeExp     = new osg::Uniform( "fadeExp",      _fadeExp                        );
        osg::Uniform* p_noiseSpeed  = new osg::Uniform( "noiseSpeed",   _noiseSpeed                     );
        osg::Uniform* p_waterColor  = new osg::Uniform( "waterColor",   osg::Vec4f( _waterColor, 1.0f ) );
        osg::Uniform* p_scale       = new osg::Uniform( "scale",        osg::Vec4f( _scale, 1.0f )      );
        osg::Uniform* p_trans       = new osg::Uniform( "transparency", _transparency                   );

        p_stateSet->addUniform( p_fadeBias   );
        p_stateSet->addUniform( p_waveSpeed  );
        p_stateSet->addUniform( p_fadeExp    );
        p_stateSet->addUniform( p_scale      );
        p_stateSet->addUniform( p_noiseSpeed );
        p_stateSet->addUniform( p_waterColor );
        p_stateSet->addUniform( p_trans      );

        osg::Uniform* p_viewPosition = new osg::Uniform( "viewPosition", osg::Vec4f() );
        p_stateSet->addUniform( p_viewPosition );
        p_viewPosition->setUpdateCallback( new ViewPositionUpdateCallback() ); // set time view position update callback for the shader

        osg::Uniform* p_uniformTime = new osg::Uniform( "time", 0.0f );
        p_stateSet->addUniform( p_uniformTime );
        p_uniformTime->setUpdateCallback( new TimeUpdateCallback() ); // set time update callback for the shader

        // create a noise texture
        osg::Texture3D* p_noiseTexture = new osg::Texture3D;
        p_noiseTexture->setFilter( osg::Texture3D::MIN_FILTER, osg::Texture3D::LINEAR );
        p_noiseTexture->setFilter( osg::Texture3D::MAG_FILTER, osg::Texture3D::LINEAR );
        p_noiseTexture->setWrap( osg::Texture3D::WRAP_S, osg::Texture3D::REPEAT );
        p_noiseTexture->setWrap( osg::Texture3D::WRAP_T, osg::Texture3D::REPEAT );
        p_noiseTexture->setWrap( osg::Texture3D::WRAP_R, osg::Texture3D::REPEAT );
        p_noiseTexture->setImage( make3DNoiseImage( 64 ) );

        p_stateSet->setTextureAttribute( LOCATION_NOISE_SAMPLER, p_noiseTexture );
        p_stateSet->addUniform( new osg::Uniform( "samplerNoise", LOCATION_NOISE_SAMPLER ) );

        // create skybox texture
        std::vector< std::string > texfiles;
        texfiles.push_back( _cubeMapTextures[ 0 ] );
        texfiles.push_back( _cubeMapTextures[ 1 ] );
        texfiles.push_back( _cubeMapTextures[ 2 ] );
        texfiles.push_back( _cubeMapTextures[ 3 ] );
        texfiles.push_back( _cubeMapTextures[ 4 ] );
        texfiles.push_back( _cubeMapTextures[ 5 ] );
        osg::ref_ptr< osg::TextureCubeMap > reflectmap = readCubeMap( texfiles );

        p_stateSet->setTextureAttribute( LOCATION_CUBEMAP_SAMPLER, reflectmap.get() );
        p_stateSet->addUniform( new osg::Uniform( "samplerSkyBox", LOCATION_CUBEMAP_SAMPLER ) );

        // set lighting to diabled and culling to enabled
        p_stateSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
        p_stateSet->setMode( GL_CULL_FACE, osg::StateAttribute::OFF );

        // diable depth test
        p_stateSet->setMode( GL_DEPTH, osg::StateAttribute::OFF );

        // set up transparency
        osg::TexEnvCombine* p_te0 = new osg::TexEnvCombine;    
        p_te0->setCombine_RGB( osg::TexEnvCombine::REPLACE );
        p_te0->setSource0_RGB( osg::TexEnvCombine::TEXTURE0 );
        p_te0->setOperand0_RGB( osg::TexEnvCombine::SRC_COLOR );

        osg::TexEnvCombine *p_te1 = new osg::TexEnvCombine;    
        p_te1->setCombine_RGB( osg::TexEnvCombine::INTERPOLATE );
        p_te1->setSource0_RGB( osg::TexEnvCombine::TEXTURE1 );
        p_te1->setOperand0_RGB( osg::TexEnvCombine::SRC_COLOR );
        p_te1->setSource1_RGB( osg::TexEnvCombine::PREVIOUS );
        p_te1->setOperand1_RGB( osg::TexEnvCombine::SRC_COLOR );
        p_te1->setSource2_RGB( osg::TexEnvCombine::PRIMARY_COLOR );
        p_te1->setOperand2_RGB( osg::TexEnvCombine::SRC_COLOR );

        p_stateSet->setTextureAttributeAndModes( 0, p_te0, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
        p_stateSet->setTextureAttributeAndModes( 1, p_te1, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

        // append state set to geode
        p_geode->setStateSet( p_stateSet );
    }

    osg::Group* p_group = new osg::Group;

    p_group->addChild( p_geode );
    return p_group;
}
