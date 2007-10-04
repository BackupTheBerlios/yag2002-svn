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
 # terrain manager
 #
 #   date of creation:  07/30/2007
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include "base.h"
#include "log.h"
#include "terrainmanager.h"
#include "terrainpatch.h"
#include "application.h"
#include "shadercontainer.h"
#include "utils.h"


//! Patch subdivisions in X and Y direction for different lod levels
#define L0_SUBDIV_X     8
#define L0_SUBDIV_Y     8
#define L1_SUBDIV_X     4
#define L1_SUBDIV_Y     4

namespace yaf3d
{

//! Implement the terrain manager singleton
YAF3D_SINGLETON_IMPL( TerrainManager )


//! Terrain section class for manager's internal house-keeping
class TerrainSection
{
    public:

        explicit                                    TerrainSection( float basetexblend ) :
                                                     _id ( 0 )
                                                    {
                                                        _baseTexBlend = new osg::Uniform( "baseTextureBlend", basetexblend );
                                                    }

        virtual                                     ~TerrainSection()
                                                    {
                                                        _baseTexBlend = NULL;
                                                        _node         = NULL;
                                                    }

        //! Set section ID.
        void                                        setID( unsigned int id )
                                                    {
                                                        _id = id;
                                                    }

        //! Set scenegraph node.
        void                                        setMainNode( osg::Group* p_node )
                                                    {
                                                        _node = p_node;
                                                    }

        //! Get scenegraph node.
        osg::ref_ptr< osg::Group >                  getMainNode()
                                                    {
                                                        return _node;
                                                    }

        //! Get the patch list.
        std::vector< osg::ref_ptr< osg::LOD > >&    getLodNodes()
                                                    {
                                                        return _lodNodes;
                                                    }

        osg::ref_ptr< osg::Uniform >                getBaseTexUniform()
                                                    {
                                                        return _baseTexBlend;
                                                    }

    protected:

        //! Section ID
        unsigned int                                _id;

        //! List of patches organized as LOD nodes
        std::vector< osg::ref_ptr< osg::LOD > >     _lodNodes;

        //! Scenegraph node
        osg::ref_ptr< osg::Group >                  _node;

        //! Shader uniform for blend factor of base texture
        osg::ref_ptr < osg::Uniform >               _baseTexBlend;
};

//! Implemention of terrain manager
TerrainManager::TerrainManager() :
_sections( 0 ),
_tilesX( 16 ),
_tilesY( 16 ),
_quadTreeDepth( 3 )
{
}

TerrainManager::~TerrainManager()
{
}

void TerrainManager::shutdown()
{
    log_info << "TerrainManager: shutting down" << std::endl;

    //release all patches
    std::map< unsigned int, TerrainSection* >::iterator beg = _sectionMap.begin(), end = _sectionMap.end();
    for ( ; beg != end; ++beg )
        delete beg->second;

    destroy();
}

unsigned int TerrainManager::addSection( const TerrainConfig& config ) throw ( TerrainException )
{
    std::string mediapath = Application::get()->getMediaPath();

    _tilesX = config._tilesX;
    _tilesY = config._tilesY;

    if ( !_tilesX || !_tilesY )
        throw TerrainException( "Terrain Manager: invalid tile count for X or Y!" );

    if ( !config._lodResolutions.size() )
        throw TerrainException( "Terrain Manager: no LOD resolution specified, you need at least 1!" );

    if ( config._lodRanges.size() != config._lodResolutions.size() )
        throw TerrainException( "Terrain Manager: unequal LOD resolution and lod range count!" );

    // check for GLSL extension
    bool glslavailable = isGlslAvailable();
    if ( !glslavailable )
        log_verbose << "Terrain Manager: GLSL not available, using base map only!" << std::endl;

    // currently we accept only tga files as heightmap
    ImageTGA tga;
    if ( !tga.load( mediapath + config._fileHeightmap ) )
        throw TerrainException( std::string( "cannot load heightmap image" ) + mediapath + config._fileHeightmap );

    unsigned int sizeX = 0, sizeY = 0;
    tga.getSize( sizeX, sizeY );

    // calculate the quadtree depth
    _quadTreeDepth = ::log( float( std::min( _tilesX, _tilesY ) ) ) / ::log( float( 2 ) );
    // check the depth
    if ( !( _tilesX >> _quadTreeDepth ) || !( _tilesY >> _quadTreeDepth ) )
        throw TerrainException( "Terrain Manager: quad tree depth is too high!" );

    // base texture map
    osg::Texture2D* p_basetex = new osg::Texture2D;

    {
        osg::Image* p_baseimage = osgDB::readImageFile( mediapath + config._fileBasemap );

        if ( !p_baseimage )
        {
            log_warning << "Terrain Manager: cannot load basemap image" << std::endl;
        }
        else
        {
            p_basetex->setImage( p_baseimage );
            p_basetex->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
            p_basetex->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );
            p_basetex->setInternalFormatMode( osg::Texture::USE_IMAGE_DATA_FORMAT );
            p_basetex->setUnRefImageDataAfterApply( true );
        }
    }

    // layer mask
    osg::Texture2D* p_layermask = new osg::Texture2D;

    {
        osg::Image* p_maskimage = osgDB::readImageFile( mediapath + config._fileLayerMask );

        if ( !p_maskimage )
        {
            log_warning << "Terrain Manager: cannot load layer mask image" << std::endl;
        }
        else
        {
            p_layermask->setImage( p_maskimage );
            p_layermask->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
            p_layermask->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );
            p_layermask->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR );
            p_layermask->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );            
            p_layermask->setInternalFormatMode( osg::Texture::USE_IMAGE_DATA_FORMAT );
            p_layermask->setUnRefImageDataAfterApply( true );
        }
    }

    // detailed texture map 0
    osg::Texture2D* p_detailtex0 = new osg::Texture2D;

    {
        osg::Image* p_detailimage0 = osgDB::readImageFile( mediapath + config._fileDetailmap0 );

        if ( !p_detailimage0 )
        {
            log_warning << "Terrain Manager: cannot load detail map 0 image" << std::endl;
        }
        else
        {
            p_detailtex0->setImage( p_detailimage0 );
            p_detailtex0->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
            p_detailtex0->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );
            p_detailtex0->setUnRefImageDataAfterApply( true );
        }
    }

    // detailed texture map 1
    osg::Texture2D* p_detailtex1 = new osg::Texture2D;

    {
        osg::Image* p_detailimage1 = osgDB::readImageFile( mediapath + config._fileDetailmap1 );

        if ( !p_detailimage1 )
        {
            log_warning << "Terrain Manager: cannot load detail map 1 image" << std::endl;
        }
        else
        {
            p_detailtex1->setImage( p_detailimage1 );
            p_detailtex1->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
            p_detailtex1->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );
            p_detailtex1->setUnRefImageDataAfterApply( true );
        }
    }

    // detailed texture map 2
    osg::Texture2D* p_detailtex2 = new osg::Texture2D;

    {
        osg::Image* p_detailimage2 = osgDB::readImageFile( mediapath + config._fileDetailmap2 );

        if ( !p_detailimage2 )
        {
            log_warning << "Terrain Manager: cannot load detail map 2 image" << std::endl;
        }
        else
        {
            p_detailtex2->setImage( p_detailimage2 );
            p_detailtex2->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
            p_detailtex2->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );
            p_detailtex2->setUnRefImageDataAfterApply( true );
        }
    }

    // create the state set
    _p_stateSet = new osg::StateSet;
    _p_stateSet->setGlobalDefaults();
    _p_stateSet->setDataVariance( osg::Object::STATIC );
    _p_stateSet->setRenderingHint( osg::StateSet::OPAQUE_BIN );
    _p_stateSet->setMode( GL_CULL_FACE, osg::StateAttribute::ON );

    // set base texture
    _p_stateSet->setTextureAttributeAndModes( 0, p_basetex, osg::StateAttribute::ON );
    // set detail textures only when glsl is available
    if ( glslavailable )
    {
        _p_stateSet->setTextureAttributeAndModes( 1, p_detailtex0, osg::StateAttribute::ON );
        _p_stateSet->setTextureAttributeAndModes( 2, p_detailtex1, osg::StateAttribute::ON );
        _p_stateSet->setTextureAttributeAndModes( 3, p_detailtex2, osg::StateAttribute::ON );
        // setup the layer mask
        if ( p_layermask )
            _p_stateSet->setTextureAttributeAndModes( 4, p_layermask, osg::StateAttribute::ON );
    }

    // create a new terrain section
    TerrainSection* p_section = new TerrainSection( config._blendBasemap );

    unsigned short patchPixelsX = sizeX / _tilesX;
    unsigned short patchPixelsY = sizeY / _tilesY;

    // create a patch object for building the section patches
    TerrainPatchBuilder*  p_patch    = new TerrainPatchBuilder();
    unsigned int          numPatches = 0;

    for ( unsigned int cntY = 0; cntY < sizeY; cntY += patchPixelsY )
    {
        for ( unsigned int cntX = 0; cntX < sizeX; cntX += patchPixelsX )
        {
            // create a lod node
            osg::ref_ptr< osg::LOD > lodnode = new osg::LOD;

            // build all specified lod nodes
            // note: the count of resolution and range definitions must match!
            TerrainConfig::ListLodResolution::const_iterator p_lodresolution = config._lodResolutions.begin(), p_lodresolutionEnd = config._lodResolutions.end();
            TerrainConfig::ListLodRange::const_iterator      p_lodrange      = config._lodRanges.begin();
            for ( ; p_lodresolution != p_lodresolutionEnd; ++p_lodresolution, ++p_lodrange )
            {
                // build LOD
                if ( !p_patch->build( tga, config._scale, cntX, cntY, patchPixelsX, patchPixelsY, ( *p_lodresolution ).first, ( *p_lodresolution ).second ) )
                {
                    log_error << "Terrain Manager: could not build patch LOD level 0!" << std::endl;
                    continue;
                }

                // build the base texture map coordinates
                if ( !p_patch->buildTexCoords( 0 ) )
                    log_error << "Terrain Manager: could not build base texture coordinates!" << std::endl;

                // build detail maps only when glsl is available
                if ( glslavailable )
                {
                    // build the detail texture map 0 coordinates
                    if ( !p_patch->buildTexCoords( 1, config._detailmap0Repeat ) )
                        log_error << "Terrain Manager: could not build deatial texture 0 coordinates!" << std::endl;

                    // build the detail texture map 1 coordinates
                    if ( !p_patch->buildTexCoords( 2, config._detailmap1Repeat ) )
                        log_error << "Terrain Manager: could not build deatial texture 1 coordinates!" << std::endl;

                    // build the detail texture map 2 coordinates
                    if ( !p_patch->buildTexCoords( 3, config._detailmap2Repeat ) )
                        log_error << "Terrain Manager: could not build deatial texture 2 coordinates!" << std::endl;
                }

                // add lod node and set its range
                lodnode->addChild( p_patch->getSceneNode().get(), ( *p_lodrange ).first, ( *p_lodrange ).second );

                //! NOTE: we may also allow the setting up the range mode. atm, we use a fixed distance from eye mode
                lodnode->setRangeMode( osg::LOD::DISTANCE_FROM_EYE_POINT /*PIXEL_SIZE_ON_SCREEN*/ );

                // reset the patch for next lod creation
                p_patch->reset();
            }

            // add the patch to section
            p_section->getLodNodes().push_back( lodnode.get() );

            numPatches++;
        }
    }

    log_debug << "Terrain Manager: created a total count of " << numPatches << " patches (including LODs)" << std::endl;

    // delete the patch object
    delete p_patch;

    // increment the count of terrain sections
    _sections++;

    // build quad tree
    osg::ref_ptr< osg::Group > group = buildQuadTree( p_section->getLodNodes() );
    std::stringstream secname;
    secname << "_terrainSection" << _sections << "_";
    group->setName( secname.str() );

    // setup the terrain shaders if glsl is available
    if ( glslavailable )
    {
        setupShaders( config, _p_stateSet.get() );
        _p_stateSet->addUniform( p_section->getBaseTexUniform().get() );
    }

    // set the group state set
    group->setStateSet( _p_stateSet.get() );

    // store the terrain section into internal map
    p_section->setID( _sections );
    p_section->setMainNode( group.get() );
    assert( ( _sectionMap.find( _sections ) == _sectionMap.end() ) && "section ID already exists in map!" );
    _sectionMap[ _sections ] = p_section;

    return _sections;
}

osg::ref_ptr< osg::Group > TerrainManager::buildQuadTree( std::vector< osg::ref_ptr< osg::LOD > >& patches )
{
    osg::ref_ptr< osg::Group > group = new osg::Group();

    // build quadtree
    unsigned int tileX = 0, tileY = 0;
    split( _quadTreeDepth, 0, group.get(), patches, tileX, tileY );

    return group;
}

void TerrainManager::split( unsigned int maxdepth, unsigned int depth, osg::Group* p_node, std::vector< osg::ref_ptr< osg::LOD > >& patches, unsigned int& tileX, unsigned int& tileY )
{
    if ( depth > maxdepth )
        return;

    // increment the quadtree depth
    depth++;

    // create four new nodes
    osg::ref_ptr< osg::Group > p_nodeA = new osg::Group;
    p_node->addChild( p_nodeA.get() );
    osg::ref_ptr< osg::Group > p_nodeB = new osg::Group;
    p_node->addChild( p_nodeB.get() );
    osg::ref_ptr< osg::Group > p_nodeC = new osg::Group;
    p_node->addChild( p_nodeC.get() );
    osg::ref_ptr< osg::Group > p_nodeD = new osg::Group;
    p_node->addChild( p_nodeD.get() );

    // append the leave nodes, the patches it is
    if ( depth == maxdepth )
    {
        unsigned int numpatchesX = _tilesX >> _quadTreeDepth;
        unsigned int numpatchesY = _tilesY >> _quadTreeDepth;

        // append the patch nodes into leave A
        for ( unsigned int cntY = tileY; cntY < tileY + numpatchesY; cntY++ )
            for ( unsigned int cntX = tileX; cntX < tileX + numpatchesX; cntX++ )
                p_nodeA->addChild( patches[ cntX + cntY * _tilesY ].get() );

        // set leave node name
        {
            std::stringstream name;
            name << "_leave" << tileX << "x" << tileY << "_";
            p_nodeA->setName( name.str() );
        }

        // increment the tile index X
        tileX += numpatchesX;

        // append the patch nodes into leave B
        for ( unsigned int cntY = tileY; cntY < tileY + numpatchesY; cntY++ )
            for ( unsigned int cntX = tileX; cntX < tileX + numpatchesX; cntX++ )
                p_nodeB->addChild( patches[ cntX + cntY * _tilesY ].get() );

        // set leave node name
        {
            std::stringstream name;
            name << "_leave" << tileX << "x" << tileY << "_";
            p_nodeB->setName( name.str() );
        }

        // increment the tile index X
        tileX += numpatchesX;

        // append the patch nodes into leave C
        for ( unsigned int cntY = tileY; cntY < tileY + numpatchesY; cntY++ )
            for ( unsigned int cntX = tileX; cntX < tileX + numpatchesX; cntX++ )
                p_nodeC->addChild( patches[ cntX + cntY * _tilesY ].get() );

        // set leave node name
        {
            std::stringstream name;
            name << "_leave" << tileX << "x" << tileY << "_";
            p_nodeC->setName( name.str() );
        }

        // increment the tile index X
        tileX += numpatchesX;

        // append the patch nodes into leave D
        for ( unsigned int cntY = tileY; cntY < tileY + numpatchesY; cntY++ )
            for ( unsigned int cntX = tileX; cntX < tileX + numpatchesX; cntX++ )
                p_nodeD->addChild( patches[ cntX + cntY * _tilesY ].get() );

        // set leave node name
        {
            std::stringstream name;
            name << "_leave" << tileX << "x" << tileY << "_";
            p_nodeD->setName( name.str() );
        }

        // increment the tile index X
        tileX += numpatchesX;

        // have to increment the Y index?
        if ( tileX >= _tilesX )
        {
            tileX = 0;
            tileY += numpatchesY;
        }

        return;
    }

    // continue to split the quads
    split( maxdepth, depth, p_nodeA.get(), patches, tileX, tileY );
    split( maxdepth, depth, p_nodeB.get(), patches, tileX, tileY );
    split( maxdepth, depth, p_nodeC.get(), patches, tileX, tileY );
    split( maxdepth, depth, p_nodeD.get(), patches, tileX, tileY );
}

osg::ref_ptr< osg::Group > TerrainManager::getSectionNode( unsigned int id ) throw ( TerrainException )
{
    if ( _sectionMap.find( id ) == _sectionMap.end() )
        throw TerrainException( "Terrain Manager: Section ID does not exist!" );

    return _sectionMap[ id ]->getMainNode();
}

void TerrainManager::releaseSection( unsigned int id ) throw ( TerrainException )
{
    if ( _sectionMap.find( id ) == _sectionMap.end() )
        throw TerrainException( "Terrain Manager: Section ID does not exist!" );

    // delete section
    delete _sectionMap[ id ];
    // remove section from internal map
    _sectionMap.erase( _sectionMap.find( id ) );
}

void TerrainManager::setBlendBasemap( unsigned int id, float blend )
{
    if ( _sectionMap.find( id ) == _sectionMap.end() )
        throw TerrainException( "Terrain Manager: cannot set base texture blend; Section ID does not exist!" );

    _sectionMap[ id ]->getBaseTexUniform()->set( blend );
}

float TerrainManager::getBlendBasemap( unsigned int id )
{
    float blend = 0.0f;

    if ( _sectionMap.find( id ) == _sectionMap.end() )
        throw TerrainException( "Terrain Manager: cannot set base texture blend; Section ID does not exist!" );

    _sectionMap[ id ]->getBaseTexUniform()->get( blend );

    return blend;
}

void TerrainManager::setupShaders( const TerrainConfig& config, osg::StateSet* p_stateset )
{
    osg::Program* p_program = new osg::Program;
    p_stateset->setAttribute( p_program );

    osg::Shader* p_vcommon = ShaderContainer::get()->getVertexShader( ShaderContainer::eCommonV );
    p_program->addShader( p_vcommon );

    osg::Shader* p_vterrain = ShaderContainer::get()->getVertexShader( ShaderContainer::eTerrainV );
    p_program->addShader( p_vterrain );

    osg::Shader* p_fcommon = ShaderContainer::get()->getFragmentShader( ShaderContainer::eCommonF );
    p_program->addShader( p_fcommon );

    osg::Shader* p_fterrain = ShaderContainer::get()->getFragmentShader( ShaderContainer::eTerrainF );
    p_program->addShader( p_fterrain );

    osg::Uniform* p_baseTextureSampler = new osg::Uniform( "baseTexture", 0 );
    p_stateset->addUniform( p_baseTextureSampler );

    osg::Uniform* p_detailTexture0Sampler = new osg::Uniform( "detailTexture0", 1 );
    p_stateset->addUniform( p_detailTexture0Sampler );

    osg::Uniform* p_detailTexture1Sampler = new osg::Uniform( "detailTexture1", 2 );
    p_stateset->addUniform( p_detailTexture1Sampler );

    osg::Uniform* p_detailTexture2Sampler = new osg::Uniform( "detailTexture2", 3 );
    p_stateset->addUniform( p_detailTexture2Sampler );

    osg::Uniform* p_layerMaskSampler = new osg::Uniform( "layerMask", 4 );
    p_stateset->addUniform( p_layerMaskSampler );
}

} // namespace yaf3d
