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
 # entity water
 #
 #   date of creation:  03/26/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_base.h>
#include <ctd_application.h>
#include "ctd_water.h"

#include <osg/BlendFunc>
#include <osg/AlphaFunc>
#include <osg/Material>

using namespace std;
using namespace CTD; 
using namespace osg; 


//! Implement and register the skybox entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( WaterEntityFactory );



float refract = 1.01;          // ratio of indicies of refraction
float fresnel = 1.1;           // Fresnel multiplier

const char vpstr[] =
    "!!ARBvp1.0 # Refraction                                    \n"
    "                                                           \n"
    "ATTRIB iPos         = vertex.position;                     \n"
    "#ATTRIB iCol        = vertex.color.primary;                \n"
    "ATTRIB iNormal      = vertex.normal;                       \n"
    "PARAM  esEyePos     = { 0, 0, 0, 1 };                      \n"
    "PARAM  const0123    = { 0, 1, 2, 3 };                      \n"
    "PARAM  fresnel      = program.local[0];                    \n"
    "PARAM  refract      = program.local[1];                    \n"
    "PARAM  itMV[4]      = { state.matrix.modelview.invtrans }; \n"
    "PARAM  MVP[4]       = { state.matrix.mvp };                \n"
    "PARAM  MV[4]        = { state.matrix.modelview };          \n"
    "PARAM  texmat[4]    = { state.matrix.texture[0] };         \n"
    "TEMP   esPos;        # position in eye-space               \n"
    "TEMP   esNormal;     # normal in eye-space                 \n"
    "TEMP   tmp, IdotN, K;                                      \n"
    "TEMP   esE;          # eye vector                          \n"
    "TEMP   esI;          # incident vector (=-E)               \n"
    "TEMP   esR;          # first refract- then reflect-vector  \n"
    "OUTPUT oPos         = result.position;                     \n"
    "OUTPUT oColor       = result.color;                        \n"
    "OUTPUT oRefractMap  = result.texcoord[0];                  \n"
    "OUTPUT oReflectMap  = result.texcoord[1];                  \n"
    "                                                           \n"
    "# transform vertex to clip space                           \n"
    "DP4    oPos.x, MVP[0], iPos;                               \n"
    "DP4    oPos.y, MVP[1], iPos;                               \n"
    "DP4    oPos.z, MVP[2], iPos;                               \n"
    "DP4    oPos.w, MVP[3], iPos;                               \n"
    "                                                           \n"
    "# Transform the normal to eye space.                       \n"
    "DP3    esNormal.x, itMV[0], iNormal;                       \n"
    "DP3    esNormal.y, itMV[1], iNormal;                       \n"
    "DP3    esNormal.z, itMV[2], iNormal;                       \n"
    "                                                           \n"
    "# normalize normal                                         \n"
    "DP3    esNormal.w, esNormal, esNormal;                     \n"
    "RSQ    esNormal.w, esNormal.w;                             \n"
    "MUL    esNormal, esNormal, esNormal.w;                     \n"
    "                                                           \n"
    "# transform vertex position to eye space                   \n"
    "DP4    esPos.x, MV[0], iPos;                               \n"
    "DP4    esPos.y, MV[1], iPos;                               \n"
    "DP4    esPos.z, MV[2], iPos;                               \n"
    "DP4    esPos.w, MV[3], iPos;                               \n"
    "                                                           \n"
    "# vertex to eye vector                                     \n"
    "ADD    esE, -esPos, esEyePos;                              \n"
    "#MOV   esE, -esPos;                                        \n"
    "                                                           \n"
    "# normalize eye vector                                     \n"
    "DP3    esE.w, esE, esE;                                    \n"
    "RSQ    esE.w, esE.w;                                       \n"
    "MUL    esE, esE, esE.w;                                    \n"
    "                                                           \n"
    "# calculate some handy values                              \n"
    "MOV    esI, -esE;                                          \n"
    "DP3    IdotN, esNormal, esI;                               \n"
    "                                                           \n"
    "# calculate refraction vector, Renderman style             \n"
    "                                                           \n"
    "# k = 1-index*index*(1-(I dot N)^2)                        \n"
    "MAD    tmp, -IdotN, IdotN, const0123.y;                    \n"
    "MUL    tmp, tmp, refract.y;                                \n"
    "ADD    K.x, const0123.y, -tmp;                             \n"
    "                                                           \n"
    "# k<0,  R = [0,0,0]                                        \n"
    "# k>=0, R = index*I-(index*(I dot N) + sqrt(k))*N          \n"
    "RSQ    K.y, K.x;                                           \n"
    "RCP    K.y, K.y;                           # K.y = sqrt(k) \n"
    "MAD    tmp.x, refract.x, IdotN, K.y;                       \n"
    "MUL    tmp, esNormal, tmp.x;                               \n"
    "MAD    esR, refract.x, esI, tmp;                           \n"
    "                                                           \n"
    "# transform refracted ray by cubemap transform             \n"
    "DP3    oRefractMap.x, texmat[0], esR;                      \n"
    "DP3    oRefractMap.y, texmat[1], esR;                      \n"
    "DP3    oRefractMap.z, texmat[2], esR;                      \n"
    "                                                           \n"
    "# calculate reflection vector                              \n"
    "# R = 2*N*(N dot E)-E                                      \n"
    "MUL    tmp, esNormal, const0123.z;                         \n"
    "DP3    esR.w, esNormal, esE;                               \n"
    "MAD    esR, esR.w, tmp, -esE;                              \n"
    "                                                           \n"
    "# transform reflected ray by cubemap transform             \n"
    "DP3    oReflectMap.x, texmat[0], esR;                      \n"
    "DP3    oReflectMap.y, texmat[1], esR;                      \n"
    "DP3    oReflectMap.z, texmat[2], esR;                      \n"
    "                                                           \n"
    "# Fresnel approximation = fresnel*(1-(N dot I))^2          \n"
    "ADD    tmp.x, const0123.y, -IdotN;                         \n"
    "MUL    tmp.x, tmp.x, tmp.x;                                \n"
    "MUL    oColor, tmp.x, fresnel;                             \n"
    "                                                           \n"
    "END                                                        \n";


EnWater::EnWater() :
_sizeX( 100.0f ),
_sizeY( 100.0f ),
_subDevisionsX( 10 ),
_subDevisionsY( 10 ),
_height( 0.0f )
{
    // register entity attributes
    _attributeManager.addAttribute( "sizeX"    , _sizeX                 );
    _attributeManager.addAttribute( "sizeY"    , _sizeY                 );
    _attributeManager.addAttribute( "subdivX"  , _subDevisionsX         );
    _attributeManager.addAttribute( "subdivY"  , _subDevisionsY         );
    _attributeManager.addAttribute( "height"   , _height                );
    _attributeManager.addAttribute( "texture"  , _texFile               );

    _attributeManager.addAttribute( "right"    , _cubeMapTextures[ 0 ]  );
    _attributeManager.addAttribute( "left"     , _cubeMapTextures[ 1 ]  );
    _attributeManager.addAttribute( "front"    , _cubeMapTextures[ 2 ]  );
    _attributeManager.addAttribute( "back"     , _cubeMapTextures[ 3 ]  );
    _attributeManager.addAttribute( "up"       , _cubeMapTextures[ 4 ]  );
    _attributeManager.addAttribute( "down"     , _cubeMapTextures[ 5 ]  );
}

EnWater::~EnWater()
{
}

void EnWater::initialize()
{
     static_cast< Group* >( Application::get()->getSceneRootNode() )->addChild( makeMesh() );
}

TextureCubeMap* EnWater::readCubeMap()
{
    TextureCubeMap* p_cubemap = new TextureCubeMap;
    string mediapath = Application::get()->getMediaPath();
    Image* imagePosX = osgDB::readImageFile( mediapath + _cubeMapTextures[ 0 ] );
    Image* imageNegX = osgDB::readImageFile( mediapath + _cubeMapTextures[ 1 ] );
    Image* imagePosY = osgDB::readImageFile( mediapath + _cubeMapTextures[ 2 ] );
    Image* imageNegY = osgDB::readImageFile( mediapath + _cubeMapTextures[ 3 ] );
    Image* imagePosZ = osgDB::readImageFile( mediapath + _cubeMapTextures[ 4 ] );
    Image* imageNegZ = osgDB::readImageFile( mediapath + _cubeMapTextures[ 5 ] );

    if (imagePosX && imageNegX && imagePosY && imageNegY && imagePosZ && imageNegZ)
    {
        p_cubemap->setImage(TextureCubeMap::POSITIVE_X, imagePosX);
        p_cubemap->setImage(TextureCubeMap::NEGATIVE_X, imageNegX);
        p_cubemap->setImage(TextureCubeMap::POSITIVE_Y, imagePosY);
        p_cubemap->setImage(TextureCubeMap::NEGATIVE_Y, imageNegY);
        p_cubemap->setImage(TextureCubeMap::POSITIVE_Z, imagePosZ);
        p_cubemap->setImage(TextureCubeMap::NEGATIVE_Z, imageNegZ);

        p_cubemap->setWrap(Texture::WRAP_S, Texture::CLAMP_TO_EDGE);
        p_cubemap->setWrap(Texture::WRAP_T, Texture::CLAMP_TO_EDGE);
        p_cubemap->setWrap(Texture::WRAP_R, Texture::CLAMP_TO_EDGE);

        p_cubemap->setFilter(Texture::MIN_FILTER, Texture::LINEAR_MIPMAP_LINEAR);
        p_cubemap->setFilter(Texture::MAG_FILTER, Texture::LINEAR);
    }

    return p_cubemap;
}

// this code is basing on osg's vertexprogram example
osg::Node* EnWater::addRefractStateSet(osg::Node* node)
{
    osg::StateSet* stateset = new osg::StateSet();

    osg::TextureCubeMap* reflectmap = readCubeMap();
    stateset->setTextureAttributeAndModes( 0, reflectmap, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
    stateset->setTextureAttributeAndModes( 1, reflectmap, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
    
    osg::TexMat* texMat = new osg::TexMat;
    stateset->setTextureAttribute(0, texMat);

    // ---------------------------------------------------
    // Vertex Program
    // ---------------------------------------------------
    osg::VertexProgram* vp = new osg::VertexProgram();
    vp->setVertexProgram( vpstr );
    vp->setProgramLocalParameter( 0, osg::Vec4( fresnel, fresnel, fresnel, 1.0f ) );
    vp->setProgramLocalParameter( 1, osg::Vec4( refract, refract*refract, 0.0f, 0.0f ) );
    stateset->setAttributeAndModes( vp, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

    // ---------------------------------------------------
    // fragment = refraction*(1-fresnel) + reflection*fresnel
    // T0 = texture unit 0, refraction map
    // T1 = texture unit 1, reflection map
    // C.rgb = primary color, water color
    // C.a   = primary color, fresnel factor
    // Cp    = result from previous texture environment
    // ---------------------------------------------------

    // REPLACE function: Arg0
    // = T0
    osg::TexEnvCombine *te0 = new osg::TexEnvCombine;    
    te0->setCombine_RGB(osg::TexEnvCombine::REPLACE);
    te0->setSource0_RGB(osg::TexEnvCombine::TEXTURE0);
    te0->setOperand0_RGB(osg::TexEnvCombine::SRC_COLOR);
    
    // INTERPOLATE function: Arg0 * (Arg2) + Arg1 * (1-Arg2)
    // = T1 * C0.a + Cp * (1-C0.a)
    osg::TexEnvCombine *te1 = new osg::TexEnvCombine;    

    // rgb = Cp + Ct
    te1->setCombine_RGB(osg::TexEnvCombine::INTERPOLATE);
    te1->setSource0_RGB(osg::TexEnvCombine::TEXTURE1);
    te1->setOperand0_RGB(osg::TexEnvCombine::SRC_COLOR);
    te1->setSource1_RGB(osg::TexEnvCombine::PREVIOUS);
    te1->setOperand1_RGB(osg::TexEnvCombine::SRC_COLOR);
    te1->setSource2_RGB(osg::TexEnvCombine::PRIMARY_COLOR);
    te1->setOperand2_RGB(osg::TexEnvCombine::SRC_COLOR);

    stateset->setTextureAttributeAndModes(0, te0, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);
    stateset->setTextureAttributeAndModes(1, te1, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    osg::Group* group = new osg::Group;
    group->addChild(node);
    group->setCullCallback(new TexMatCallback(*texMat));
    group->setStateSet( stateset );

    // set blending for transparency
    BlendFunc* blend = new osg::BlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    stateset->setAttribute( blend );
	stateset->setMode(GL_BLEND,osg::StateAttribute::ON);		
	stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

    return group;
}

Node* EnWater::makeMesh()
{
    _geode  = new Geode();
    Geometry* p_geom = new Geometry();

    unsigned int gridx = ( unsigned int )( _sizeX / _subDevisionsX );
    unsigned int gridy = ( unsigned int )( _sizeY / _subDevisionsY );

    _posArray     = new Vec3Array();
    _normArray    = new Vec3Array();
    Vec3Array*  p_posArray     = _posArray.get();
    Vec3Array*  p_normArray    = _normArray.get();
    Vec2Array*  p_tcoordArray  = new Vec2Array();
    p_posArray->resize( _subDevisionsX * _subDevisionsY );
    p_normArray->resize( _subDevisionsX * _subDevisionsY );
    p_tcoordArray->resize( _subDevisionsX * _subDevisionsY );

    // create vertex coordinates along x and y axis
    for( int y = 0; y < _subDevisionsY; ++y )
    {
        for( int x = 0; x < _subDevisionsX; ++x )
        {
            ( *p_posArray )[ y * _subDevisionsX + x ].set
                (
                    ( float( x ) - float( _subDevisionsX - 1 ) * 0.5f ) * gridx, 
                    ( float( y ) - float( _subDevisionsY - 1 ) * 0.5f ) * gridy,
                    _height + ( ( float )( rand() % 20 ) / 10.0f ) // add a little random, TODO, remove this later, the water equation will stimulate the vert positions
                );

            ( *p_normArray )[ y * _subDevisionsX + x ].set( 0, 0, -1.0f );
            ( *p_tcoordArray)[ y * _subDevisionsX + x ].set( float( gridx * x ) / _sizeX, float( gridy * y ) / _sizeY );  
        }
    }
    p_geom->setVertexArray( p_posArray );
    p_geom->setNormalArray( p_normArray );
    p_geom->setTexCoordArray( 2, p_tcoordArray );

    // create indices for triangle strips
    for( int y = 0; y < _subDevisionsY - 1; ++y )
    {
        unsigned int* p_indices   = new unsigned int[ _subDevisionsX * 2 ];
        unsigned int* p_curindex  = p_indices;
        for( int x = 0; x < _subDevisionsX; ++x )
        {
            *p_curindex = ( y + 1 ) * _subDevisionsX + x;
            p_curindex++;
            *p_curindex = ( y + 0 ) * _subDevisionsX + x;
            p_curindex++;
        }
        p_geom->addPrimitiveSet( new DrawElementsUInt( PrimitiveSet::TRIANGLE_STRIP, _subDevisionsX * 2, p_indices ) );
    }

    p_geom->setNormalBinding( Geometry::AttributeBinding::BIND_PER_VERTEX );
    p_geom->setUseDisplayList( false );     
    p_geom->dirtyBound();
    _geode->addDrawable( p_geom );

    return addRefractStateSet( _geode.get() );
}

void EnWater::updateEntity( float deltaTime )
{
    //! TODO implementation of wave equation
}
