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
 #   date of creation:  06/27/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <base.h>
#include "shadercontainer.h"
#include "log.h"

// Implementation of ShadowManager
YAF3D_SINGLETON_IMPL( yaf3d::ShaderContainer )

namespace yaf3d
{

ShaderContainer::ShaderContainer()
{
}

ShaderContainer::~ShaderContainer()
{
}

void ShaderContainer::shutdown()
{
    log_info << "ShaderContainer: shutting down" << std::endl;

    // destroy the singleton
    destroy();
}


// vertex shader
static const char _glslCommonV[] =
    "/*\n"
    "* Common vertex shader functions\n"
    "* http://yag2002.sf.net\n"
    "* 09/10/2007\n"
    "*/\n"
    "\n"
    "// calculate the light parameters\n"
    "void calcLightingParams( in int lightnum, out vec3 normal, out vec3 lightdir, out vec3 halfvec, out vec4 ambient, out vec4 diffuse )\n"
    "{\n"
    "   normal      = normalize( gl_NormalMatrix * gl_Normal );\n"
    "   lightdir    = normalize( vec3( gl_LightSource[ lightnum ].position ) );\n"
    "   halfvec     = normalize( gl_LightSource[ lightnum ].halfVector.xyz );\n"
    "   diffuse     = gl_FrontMaterial.diffuse * gl_LightSource[ lightnum ].diffuse;\n"
    "   ambient     = gl_FrontMaterial.ambient * gl_LightSource[ lightnum ].ambient;\n"
    "   ambient     += gl_LightModel.ambient * gl_FrontMaterial.ambient;\n"
    "}\n"
;


// Shadow map shader. It needs the common vertex functions.
static char _glslShadowMapV[] =
    "/*\n"
    "* Vertex shader for shadow mapping\n"
    "* http://yag2002.sf.net\n"
    "* 06/27/2006\n"
    "*/\n"
    "uniform mat4 texgenMatrix;\n"
    "varying vec4 diffuse, ambient;\n"
    "varying vec3 normal, lightDir, halfVector;\n"
    "varying vec2 baseTexCoords;\n"
    "const   int  shadowTexChannel = 1;\n"
    "\n"
    "// declarations for common functions\n"
    "void calcLightingParams( in int lightnum, out vec3 normal, out vec3 lightdir, out vec3 halfvec, out vec4 ambient, out vec4 diffuse );\n"
    "\n"
    "void main( void )\n"
    "{\n"
    "   // calcualte the lighting parameters\n"
    "   calcLightingParams( 0, normal, lightDir, halfVector, ambient, diffuse );\n"
    "   // calculate the tex coords for shadow map\n"
    "   vec4 pos    =  gl_ModelViewMatrix * gl_Vertex;\n"
    "   gl_TexCoord[ shadowTexChannel ] = texgenMatrix * pos;\n"
    "   gl_Position   = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
    "   baseTexCoords = gl_MultiTexCoord0.st;\n"
    "}\n"
;

// Terrain vertex shader without dynamic shadows
static const char _glslTerrainV[] =
    "/*\n"
    "* Vertex shader for terrain renderer\n"
    "* http://yag2002.sf.net\n"
    "* 08/28/2007\n"
    "*/\n"
    "varying vec2 baseTexCoords;\n"
    "varying vec2 detail0TexCoords;\n"
    "varying vec2 detail1TexCoords;\n"
    "varying vec2 detail2TexCoords;\n"
    "varying vec2 detail3TexCoords;\n"
    "\n"
    "void main()\n"
    "{\n"
    "   gl_Position      = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
    "   baseTexCoords    = gl_MultiTexCoord0.st;\n"
    "   detail0TexCoords = gl_MultiTexCoord1.st;\n"
    "   detail1TexCoords = gl_MultiTexCoord2.st;\n"
    "   detail2TexCoords = gl_MultiTexCoord3.st;\n"
    "   detail3TexCoords = gl_MultiTexCoord4.st;\n"
    "}\n"
;

// Vegetation vertex shader
static const char _glslVegetationV[] =
    "/*\n"
    "* Vertex shader for vegetation animation\n"
    "* http://yag2002.sf.net\n"
    "* 08/10/2005\n"
    "*/\n"
    "/* Note: color's w component must contain the height bias if the mesh is not positioned at Z = 0 */\n"
    "\n"
    "uniform float osg_FrameTime;\n"
    "const float   amplitude = 0.002;\n"
    "\n"
    "varying vec4  vertColor;\n"
    "\n"
    "void main( void )\n"
    "{\n"
    "   float fac  = amplitude * ( gl_Vertex.z - gl_Color.w );\n"
    "   vec4  vert = gl_Vertex;\n"
    "\n"
    "   float sint = fac * sin( osg_FrameTime + gl_Vertex.x );\n"
    "   float cost = fac * cos( osg_FrameTime + gl_Vertex.y );\n"
    "   vert.x     += cost;\n"
    "   vert.y     += sint;\n"
    "   vert.z     += sint;\n"
    "   vert.w     = 1.0;\n"
    "\n"
    "   gl_Position = gl_ModelViewProjectionMatrix * vert;\n"
    "\n"
    "   gl_TexCoord[ 0 ] = gl_MultiTexCoord0;\n"
    "   vertColor        = gl_Color;\n"
    "}\n"
;

// Common fragment shader functions
static char _glslCommonF[] =
    "/*\n"
    "* Common fragment shader functions\n"
    "* http://yag2002.sf.net\n"
    "* 09/10/2007\n"
    "*/\n"
    "\n"
    "// phong lighting\n"
    "vec4 calcLighting( in vec4 diffuse, in vec4 ambient, in vec3 normal, in vec3 lightdir, in vec3 halfvector ) \n"
    "{\n"
    "   vec3  n, halfV;\n"
    "   float NdotL, NdotHV;\n"
    "   // calculate lighting\n"
    "   vec4 color = ambient;\n"
    "   n = normalize( normal );\n"
    "   NdotL = max( dot( n, lightdir ), 0.0 );\n"
    "   if ( NdotL > 0.0 )\n"
    "   {\n"
    "       color += diffuse * NdotL;\n"
    "       halfV = normalize( halfvector );\n"
    "       NdotHV = max( dot( n,halfV ), 0.0 );\n"
    "//     color += gl_FrontMaterial.specular *\n"
    "//              gl_LightSource[0].specular *\n"
    "//              pow( NdotHV, gl_FrontMaterial.shininess );\n"
    "   }\n"
    "   return color;\n"
    "}\n"
    "\n"
    "// calculate the shadow fragment colour. the tex coords must be calculated by the vertex shader.\n"
    "const int  shadowTexChannel = 1;\n"
    "vec3 calcShadowColor( in float shadowTexSize, in sampler2DShadow shadowTexture ) \n"
    "{\n"
    "   // smooth the shadow texel\n"
    "   // number of neighboring textels\n"
    "   float cells = 1.0 / 9.0;\n"
    "   // coordinate offset depending on shadow texture size\n"
    "   float co = 1.0 / shadowTexSize;\n"
    "   vec3 shadowCoord0 =\n"
    "      gl_TexCoord[ shadowTexChannel ].xyz / gl_TexCoord[ shadowTexChannel ].w;\n"
    "\n"
    "   // soften the shadow \n"
    "   vec3 shadowCoord1 = shadowCoord0 + vec3( -co,  co, 0.0 );\n"
    "   vec3 shadowCoord2 = shadowCoord0 + vec3(   0,  co, 0.0 );\n"
    "   vec3 shadowCoord3 = shadowCoord0 + vec3(  co,  co, 0.0 );\n"
    "   vec3 shadowCoord4 = shadowCoord0 + vec3(  co,   0, 0.0 );\n"
    "   vec3 shadowCoord5 = shadowCoord0 + vec3(  co, -co, 0.0 );\n"
    "   vec3 shadowCoord6 = shadowCoord0 + vec3( 0.0, -co, 0.0 );\n"
    "   vec3 shadowCoord7 = shadowCoord0 + vec3( -co, -co, 0.0 );\n"
    "   vec3 shadowCoord8 = shadowCoord0 + vec3( -co,   0, 0.0 );\n"
    "   vec3 shadowColor  = shadow2D( shadowTexture, shadowCoord0 ).rgb * cells +\n"
    "                       shadow2D( shadowTexture, shadowCoord1 ).rgb * cells +\n"
    "                       shadow2D( shadowTexture, shadowCoord2 ).rgb * cells + \n"
    "                       shadow2D( shadowTexture, shadowCoord3 ).rgb * cells + \n"
    "                       shadow2D( shadowTexture, shadowCoord4 ).rgb * cells + \n"
    "                       shadow2D( shadowTexture, shadowCoord5 ).rgb * cells + \n"
    "                       shadow2D( shadowTexture, shadowCoord6 ).rgb * cells + \n"
    "                       shadow2D( shadowTexture, shadowCoord7 ).rgb * cells + \n"
    "                       shadow2D( shadowTexture, shadowCoord8 ).rgb * cells;  \n"
    "   return shadowColor;\n"
    "   //return shadow2D( shadowTexture, shadowCoord0 ).rgb;\n"
    "}\n"
;

// Shadow map shader. It needs the common fragment functions.
static char _glslShadowMapF[] =
    "/*\n"
    "* Fragment shader for shadow mapping\n"
    "* http://yag2002.sf.net\n"
    "* 06/27/2006\n"
    "*/\n"
    "uniform sampler2D       baseTexture;\n"
    "uniform sampler2DShadow shadowTexture;\n"
    "uniform vec2            ambientBias;\n"
    "varying vec4            diffuse, ambient;\n"
    "varying vec3            normal, lightDir, halfVector;\n"
    "varying vec2            baseTexCoords;\n"
    "const   float           shadowTexSize    = 1024.0;\n"
    "\n"
    "// declarations for common functions\n"
    "vec4 calcLighting( in vec4 diffuse, in vec4 ambient, in vec3 normal, in vec3 lightdir, in vec3 halfvector ); \n"
    "vec3 calcShadowColor( in float shadowTexSize, in sampler2DShadow shadowTexture ); \n"
    "\n"
    "void main( void )\n"
    "{\n"
    "   vec4 color       = calcLighting( diffuse, ambient, normal, lightDir, halfVector );\n"
    "   vec3 shadowColor = calcShadowColor( shadowTexSize, shadowTexture );\n"
    "\n"
    "   vec4 texcolor = color * texture2D( baseTexture, baseTexCoords );\n"
    "   gl_FragColor  = vec4(\n"
    "                         texcolor.rgb * ( ambientBias.x + shadowColor * ambientBias.y ),\n"
    "                         step( 0.5, texcolor.a )\n"
    "                       );\n"
    "}\n"
;

// Terrain fragment shader without dynamic shadows
static const char _glslTerrainF[] =
    "/*\n"
    "* Fragment shader for terrain renderer\n"
    "* http://yag2002.sf.net\n"
    "* 08/28/2007\n"
    "*/\n"
    "uniform sampler2D baseTexture;\n"
    "uniform sampler2D detailTexture0;\n"
    "uniform sampler2D detailTexture1;\n"
    "uniform sampler2D detailTexture2;\n"
    "uniform sampler2D detailTexture3;\n"
    "uniform sampler2D layerMask;\n"
    "uniform float     baseTextureBlend;\n"
    "varying vec2      baseTexCoords;\n"
    "varying vec2      detail0TexCoords;\n"
    "varying vec2      detail1TexCoords;\n"
    "varying vec2      detail2TexCoords;\n"
    "varying vec2      detail3TexCoords;\n"
    "\n"
    "void main(void)\n"
    "{\n"
    "   vec4 fetch = texture2D( baseTexture, baseTexCoords );\n"
    "   vec3 color = fetch.rgb;\n"
    "   float alpha = fetch.a;\n"
    "   color *= baseTextureBlend;\n"
    "   vec4 mask  = texture2D( layerMask, baseTexCoords ).rgba;\n"
    "   color += mask.r * texture2D( detailTexture0, detail0TexCoords ).rgb;\n"
    "   color += mask.g * texture2D( detailTexture1, detail1TexCoords ).rgb;\n"
    "   color += mask.b * texture2D( detailTexture2, detail2TexCoords ).rgb;\n"
    "   color += mask.a * texture2D( detailTexture3, detail3TexCoords ).rgb;\n"
    "   gl_FragColor  = vec4( color, alpha );\n"
    "}\n"
;

// Vegetation fragment shader
static const char _glslVegetationF[] =
    "/*\n"
    "* Fragment shader for vegetation animation\n"
    "* http://yag2002.sf.net\n"
    "* 08/10/2005\n"
    "*/\n"
    "varying vec4      vertColor;\n"
    "uniform sampler2D baseMap;\n"
    "\n"
    "void main( void )\n"
    "{\n"
    "   vec4 mapColor  = texture2D( baseMap, gl_TexCoord[ 0 ].xy );\n"
    "   vec3 fragColor = mapColor.rgb * vertColor.rgb;\n"
    "   float trans    = step( 0.5, mapColor.w );\n"
    "   gl_FragColor   = vec4( fragColor.rgb, trans );\n"
    "}\n"
;

osg::Shader* ShaderContainer::getVertexShader( unsigned int type )
{
    // look up the cache first
    if ( _vsCache.find( type ) != _vsCache.end() )
        return _vsCache[ type ].get();

    std::string code;
    switch( type )
    {
        case eCommonV:
            code = _glslCommonV;
            break;

        case eShadowMapV:
            code = _glslShadowMapV;
            break;

        case eTerrainV:
            code = _glslTerrainV;
            break;

        //case eTerrainShadowMapV:
        //    code = _glslTerrainShadowMapV;
        //    break;

        case eVegetationV:
            code = _glslVegetationV;
            break;

        default:
            log_error << "ShaderContainer: invalid vertex shader type: " << type << std::endl;
            return NULL;
    }

    osg::Shader* p_shader = new osg::Shader( osg::Shader::VERTEX, code );
    _vsCache[ type ] = p_shader;

    return p_shader;
}

osg::Shader* ShaderContainer::getFragmentShader( unsigned int type )
{
    // look up the cache first
    if ( _fsCache.find( type ) != _fsCache.end() )
        return _fsCache[ type ].get();

    std::string code;
    switch( type )
    {
        case eCommonF:
            code = _glslCommonF;
            break;

        case eShadowMapF:
            code = _glslShadowMapF;
            break;

        case eTerrainF:
            code = _glslTerrainF;
            break;

        //case eTerrainShadowMapF:
        //    code = _glslTerrainShadowMapF;
        //    break;

        case eVegetationF:
            code = _glslVegetationF;
            break;

        default:
            log_error << "ShaderContainer: invalid fragment shader type: " << type << std::endl;
            return NULL;
    }

    osg::Shader* p_shader = new osg::Shader( osg::Shader::FRAGMENT, code );
    _fsCache[ type ] = p_shader;

    return p_shader;
}

bool ShaderContainer::addShaderNode( const std::string& name, osg::ref_ptr< osg::Group > node, osg::Group* p_parent )
{
    if ( _shaderNodes.find( name ) != _shaderNodes.end() )
        return false;

    _shaderNodes[ name ] = node;

    if ( p_parent )
        p_parent->addChild( node.get() );

    return true;
}

bool ShaderContainer::removeShaderNode( const std::string& name, osg::ref_ptr< osg::Group > node )
{
    std::map< std::string, osg::ref_ptr< osg::Group > >::iterator p_shader = _shaderNodes.find( name );
    if ( p_shader == _shaderNodes.end() )
        return false;

    _shaderNodes.erase( p_shader );
    return true;
}

osg::ref_ptr< osg::Group > ShaderContainer::getShaderNode( const std::string& name )
{
    std::map< std::string, osg::ref_ptr< osg::Group > >::iterator p_shader = _shaderNodes.find( name );
    if ( p_shader == _shaderNodes.end() )
        return NULL;

    return p_shader->second;
}

} // namespace yaf3d
