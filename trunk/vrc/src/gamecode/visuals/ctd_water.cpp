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
#include <ctd_log.h>
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


// Implementation of water entity

EnWater::EnWater() :
_sizeX( 5000.0f ),
_sizeY( 5000.0f ),
_subDevisionsX( 100 ),
_subDevisionsY( 100 ),
_viscosity( 0.005f ), 
_speed( 100.0f ),
_stimulationRate( 3 ),
_amplitude( 0.02f ),
_pastTime( 0 ),
_primaryPosBuffer( true ),
_p_geom( NULL )
{
    EntityManager::get()->registerUpdate( this );     // register entity in order to get updated per simulation step

    // register entity attributes
    _attributeManager.addAttribute( "sizeX"                 , _sizeX                 );
    _attributeManager.addAttribute( "sizeY"                 , _sizeY                 );
    _attributeManager.addAttribute( "subdivX"               , _subDevisionsX         );
    _attributeManager.addAttribute( "subdivY"               , _subDevisionsY         );
    _attributeManager.addAttribute( "position"              , _position              );
    _attributeManager.addAttribute( "viscosity"             , _viscosity             );
    _attributeManager.addAttribute( "waveSpeed"             , _speed                 );
    _attributeManager.addAttribute( "stimulationAmplitude"  , _amplitude             );
    _attributeManager.addAttribute( "stimulationRate"       , _stimulationRate       );

    _attributeManager.addAttribute( "texture"  , _texFile                );

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
    addToTransformableNode( makeMesh() );
    setPosition( _position );

    _stimulationPeriod = 1.0f / _stimulationRate;
    // calculate the liquid equation constants
    calcConstants( 0.03f );
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

    if ( imagePosX && imageNegX && imagePosY && imageNegY && imagePosZ && imageNegZ )
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
    else
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** Entity Water: could not setup all cubemap images" << endl;
    }

    return p_cubemap;
}

// this code is basing on osg's vertexprogram example
osg::Node* EnWater::addRefractStateSet( osg::Node* node )
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

    //! TODO: implementation of a gpu program cache
    static osg::VertexProgram* p_vp = 0;
    if ( !p_vp )
    {
        p_vp = new osg::VertexProgram();
        p_vp->setVertexProgram( vpstr );
        p_vp->setProgramLocalParameter( 0, osg::Vec4( fresnel, fresnel, fresnel, 1.0f ) );
        p_vp->setProgramLocalParameter( 1, osg::Vec4( refract, refract*refract, 0.0f, 0.0f ) );
    }
    stateset->setAttributeAndModes( p_vp, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

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

    // set blending for transparency
    BlendFunc* p_blend = new osg::BlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    stateset->setAttribute( p_blend );
    stateset->setMode( GL_BLEND,osg::StateAttribute::ON );      
    stateset->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    //stateset->setRenderBinDetails( 1, "RenderBin" );

    osg::Group* group = new osg::Group;
    group->addChild( node );
    group->setCullCallback( new TexMatCallback( *texMat ) );
    group->setStateSet( stateset );

    return group;
}

Node* EnWater::makeMesh()
{
    _geode  = new Geode();
    _p_geom = new Geometry();

    unsigned int gridx = ( unsigned int )( _sizeX / _subDevisionsX );
    unsigned int gridy = ( unsigned int )( _sizeY / _subDevisionsY );

    _posArray1     = new Vec3Array();
    _posArray2     = new Vec3Array();
    _normArray     = new Vec3Array();
    Vec3Array*  p_posArray1    = _posArray1.get();
    Vec3Array*  p_posArray2    = _posArray2.get();
    Vec3Array*  p_normArray    = _normArray.get();
    Vec2Array*  p_tcoordArray  = new Vec2Array();
    p_posArray1->resize( _subDevisionsX * _subDevisionsY );
    p_posArray2->resize( _subDevisionsX * _subDevisionsY );
    p_normArray->resize( _subDevisionsX * _subDevisionsY );
    p_tcoordArray->resize( _subDevisionsX * _subDevisionsY );

    // create vertex coordinates along x and y axis
    for( int y = 0; y < _subDevisionsY; ++y )
    {
        for( int x = 0; x < _subDevisionsX; ++x )
        {
            ( *p_posArray1 )[ y * _subDevisionsX + x ].set
                (
                    ( float( x ) - float( _subDevisionsX - 1 ) * 0.5f ) * gridx, 
                    ( float( y ) - float( _subDevisionsY - 1 ) * 0.5f ) * gridy,
                    0
                );

            ( *p_posArray2 )[ y * _subDevisionsX + x ] = ( *p_posArray1 )[ y * _subDevisionsX + x ];
            ( *p_normArray )[ y * _subDevisionsX + x ].set( 0, 0, -1.0f );
            ( *p_tcoordArray)[ y * _subDevisionsX + x ].set( float( gridx * x ) / _sizeX, float( gridy * y ) / _sizeY );  
        }
    }
    _p_geom->setVertexArray( p_posArray1 );
    _p_geom->setNormalArray( p_normArray );
    _p_geom->setTexCoordArray( 2, p_tcoordArray );

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
        _p_geom->addPrimitiveSet( new DrawElementsUInt( PrimitiveSet::TRIANGLE_STRIP, _subDevisionsX * 2, p_indices ) );
    }

    _p_geom->setNormalBinding( Geometry::AttributeBinding::BIND_PER_VERTEX );
    _p_geom->setUseDisplayList( false );     
    _p_geom->dirtyBound();
    _geode->addDrawable( _p_geom );

    return addRefractStateSet( _geode.get() );
}

void EnWater::updateEntity( float deltaTime )
{// reference: ISBN 1-58450-277-0, thanks to eric
    
    Vec3Array*  p_posArray1 = _primaryPosBuffer ? _posArray1.get() : _posArray2.get();
    Vec3Array*  p_posArray2 = _primaryPosBuffer ? _posArray2.get() : _posArray1.get();
    Vec3Array*  p_normArray = _normArray.get();

    // calculate vertex positions
    for( int y = 1; y < _subDevisionsY - 1; ++y )
    {
        Vec3f* p_curPos   = &( ( *p_posArray1 )[ y * _subDevisionsX ] );
        Vec3f* p_prevPos  = &( ( *p_posArray2 )[ y * _subDevisionsX ] );
        for( int x = 1; x < _subDevisionsX - 1; ++x )
        {
           float newZ =
                _k1 * p_curPos[ x ]._v[ 2 ]  + 
                _k2 * p_prevPos[ x ]._v[ 2 ] + 
                _k3 * 
                ( 
                    p_curPos[ x + 1 ]._v[ 2 ] + p_curPos[ x - 1 ]._v[ 2 ] + 
                    p_curPos[ x + _subDevisionsX ]._v[ 2 ] + p_curPos[ x - _subDevisionsX ]._v[ 2 ]
                );

           p_prevPos[ x ]._v[ 2 ] = newZ;
        }
    }

    // calculate vertex normals
    for( int y = 1; y < _subDevisionsY - 1; ++y )
    {
        Vec3f* p_norm     = &( ( *p_normArray )[ y * _subDevisionsX ] );
        Vec3f* p_nextPos  = &( ( *p_posArray2 )[ y * _subDevisionsX ] );
        for( int x = 1; x < _subDevisionsX - 1; ++x )
        {
            p_norm[ x ]._v[ 0 ] = p_nextPos[ x - 1 ]._v[ 2 ] - p_nextPos[ x + 1 ]._v[ 2 ];
            p_norm[ x ]._v[ 1 ] = p_nextPos[ x - _subDevisionsX ]._v[ 2 ] - p_nextPos[ x + _subDevisionsX ]._v[ 2 ];
            p_norm[ x ].normalize();
            Vec3f n = p_norm[ x ];
            int i = 0;
        }
    }

    // adapt the liquid equation constants to changing framerate
    static float lastDeltaTime = 0;
    if ( abs( lastDeltaTime - deltaTime ) > 0.015f ) 
        calcConstants( deltaTime );
    lastDeltaTime = deltaTime;

    // create random z-position offset
    static float ltime = 0;
    ltime += deltaTime;
    float stimulus = _amplitude * sinf( ltime );
    if ( ltime > 2.0f * PI )
        ltime -= 2.0f * PI;

    // create stimulations considering the rate
    _pastTime += deltaTime;
    while ( _pastTime > 0 ) 
    {
        if ( ( _pastTime - _stimulationPeriod ) < 0 )
        {
            break;
        }
        _pastTime -= _stimulationPeriod;
        // create a random coordinate
        unsigned int randcoord = ( unsigned int )( ( rand() % _subDevisionsY ) * _subDevisionsY + ( rand() % _subDevisionsX ) );
        ( *p_posArray2 )[ randcoord ]._v[ 2 ] += stimulus;
    }

    // swap vertex position buffers
    _p_geom->setVertexArray( _primaryPosBuffer ? p_posArray2 : p_posArray1 );
    _primaryPosBuffer = !_primaryPosBuffer;
}

void EnWater::calcConstants( float stepWidth )
{
    float distance = _sizeX / ( float )_subDevisionsX;
    // setup water equation constants
    float f1 = _speed * _speed * stepWidth * stepWidth / ( distance * distance );
    float f2 = 1.0f / ( _viscosity * stepWidth + 2 );
    _k1 = ( 4.0f - 8.0f * f1 ) * f2;
    _k2 = ( _viscosity * stepWidth - 2 ) * f2;
    _k3 = 2.0f * f1 * f2;
}
