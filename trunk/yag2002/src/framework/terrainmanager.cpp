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
#include "utils.h"

namespace yaf3d
{

//! Implement the terrain manager singleton
YAF3D_SINGLETON_IMPL( TerrainManager )

//! Terrain section class for manager's internal house-keeping
class TerrainSection
{
    public:

                                                    TerrainSection() :
                                                     _id ( 0 )
                                                    {}

        virtual                                     ~TerrainSection()
                                                    {
                                                        // delete the patches
                                                        std::vector< TerrainPatch* >::iterator pp_patch    = _patches.begin(),
                                                                                               pp_patchEnd = _patches.end();
                                                        for ( ; pp_patch != pp_patchEnd; ++pp_patch )
                                                            delete ( *pp_patch );
                                                    }

        //! Get the patch list.
        std::vector< TerrainPatch* >&               getPatchList()
                                                    {
                                                        return _patches;
                                                    }

        //! Set section ID.
        void                                        setID( unsigned int id )
                                                    {
                                                        _id = id;
                                                    }

        //! Set scenegraph node.
        void                                        setNode( osg::Group* p_node )
                                                    {
                                                        _node = p_node;
                                                    }

        //! Get scenegraph node.
        osg::ref_ptr< osg::Group >                  getNode()
                                                    {
                                                        return _node;
                                                    }

    protected:

        //! Section ID
        unsigned int                                _id;

        //! List of patches
        std::vector< TerrainPatch* >                _patches;

        //! Scenegraph node
        osg::ref_ptr< osg::Group >                  _node; 
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
    //release all patches
    std::map< unsigned int, TerrainSection* >::iterator beg = _sectionMap.begin(), end = _sectionMap.end();
    for ( ; beg != end; ++beg )
        delete beg->second;

    destroy();
}

unsigned int TerrainManager::addSection( TerrainConfig& config ) throw ( TerrainException )
{
    std::string mediapath = Application::get()->getMediaPath();

    if ( !_tilesX || !_tilesY )
        throw TerrainException( "Terrain Manager: invalid tile count for X or Y!" );

    if ( !( _tilesX >> _quadTreeDepth ) || !( _tilesY >> _quadTreeDepth ) )
        throw TerrainException( "Terrain Manager: quad tree depth is too high!" );

    // check for GLSL extension
    bool glslavailable = false;
    {
        static const osg::GL2Extensions* p_extensions = NULL;
        if ( !p_extensions )
        {
            p_extensions = osg::GL2Extensions::Get( 0, true );
            if ( p_extensions )
            {
                glslavailable = p_extensions->isGlslSupported();
                if ( !glslavailable )
                    log_verbose << "Terrain Manager: GLSL not available, using base map only!" << std::endl;
            }
        }
    }

    // currently we accept only tga files as heightmap
    ImageTGA tga;
    if ( !tga.load( mediapath + config._fileHeightmap ) )
        throw TerrainException( std::string( "cannot load heightmap image" ) + mediapath + config._fileHeightmap );

    unsigned int sizeX = 0, sizeY = 0;
    tga.getSize( sizeX, sizeY );

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
            p_basetex->setFilter( osg::Texture::MIN_FILTER, osg::Texture::LINEAR );
            p_basetex->setFilter( osg::Texture::MAG_FILTER, osg::Texture::LINEAR );            
            p_basetex->setInternalFormatMode( osg::Texture::USE_IMAGE_DATA_FORMAT );
            p_basetex->setUnRefImageDataAfterApply( true );
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

            unsigned int channels = p_detailimage0->computeNumComponents( p_detailimage0->getPixelFormat() );
            if ( channels != 4 )
                log_error << "Terrain Manager: no alpha channel in detail tex map 0!" << std::endl;
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

            unsigned int channels = p_detailimage1->computeNumComponents( p_detailimage1->getPixelFormat() );
            if ( channels != 4 )
                log_error << "Terrain Manager: no alpha channel in detail tex map 1!" << std::endl;
        }
    }

    // create a new terrain section
    TerrainSection* p_section = new TerrainSection;

    for ( unsigned int cntY = 0; cntY < sizeY; cntY += sizeY / _tilesY )
    {
        for ( unsigned int cntX = 0; cntX < sizeX; cntX += sizeX / _tilesX )
        {
            TerrainPatch* p_patch = new TerrainPatch();
            if ( !p_patch->build( tga, config._scale, cntX, cntY, sizeX / _tilesX, sizeY / _tilesY ) )
            {
                delete p_patch;
                continue;
            }

            // build the base texture map coordinates
            if ( p_patch->buildTexCoords( 0 ) )
                p_patch->getStateSet()->setTextureAttributeAndModes( 0, p_basetex, osg::StateAttribute::ON );

            // build detail maps only when glsl is available
            if ( glslavailable )
            {
                // build the detail texture map 0 coordinates
                if ( p_patch->buildTexCoords( 1, config._detailmap0Repeat ) )
                p_patch->getStateSet()->setTextureAttributeAndModes( 1, p_detailtex0, osg::StateAttribute::ON );

                // build the detail texture map 1 coordinates
                if ( p_patch->buildTexCoords( 2, config._detailmap1Repeat ) )
                    p_patch->getStateSet()->setTextureAttributeAndModes( 2, p_detailtex1, osg::StateAttribute::ON );
            }

            // add the patch to list
            p_section->getPatchList().push_back( p_patch );            
        }
    }

    // increment the count of terrain sections
    _sections++;

    // build quad tree
    osg::ref_ptr< osg::Group > group = buildQuadTree( p_section->getPatchList() );
    std::stringstream secname;
    secname << "_terrainSection" << _sections << "_";
    group->setName( secname.str() );

    // setup the terrain shaders
    if ( glslavailable )
        group->setStateSet( setupShaders().get() );

    // store the terrain section into internal map
    p_section->setID( _sections );
    p_section->setNode( group.get() );
    assert( ( _sectionMap.find( _sections ) == _sectionMap.end() ) && "section ID already exists in map!" );
    _sectionMap[ _sections ] = p_section;

    return _sections;
}

osg::ref_ptr< osg::Group > TerrainManager::buildQuadTree( std::vector< TerrainPatch* >& patches )
{
    osg::ref_ptr< osg::Group > group = new osg::Group();

    // build quadtree
    unsigned int tileX = 0, tileY = 0;
    split( _quadTreeDepth, 0, group.get(), patches, tileX, tileY );

    return group;
}

void TerrainManager::split( unsigned int maxdepth, unsigned int depth, osg::Group* p_node, std::vector< TerrainPatch* >& patches, unsigned int& tileX, unsigned int& tileY )
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
                p_nodeA->addChild( patches[ cntX + cntY * _tilesY ]->getSceneNode().get() );

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
                p_nodeB->addChild( patches[ cntX + cntY * _tilesY ]->getSceneNode().get() );

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
                p_nodeC->addChild( patches[ cntX + cntY * _tilesY ]->getSceneNode().get() );

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
                p_nodeD->addChild( patches[ cntX + cntY * _tilesY ]->getSceneNode().get() );

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

    return _sectionMap[ id ]->getNode();
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

osg::ref_ptr<osg::StateSet > TerrainManager::setupShaders()
{
    osg::ref_ptr<osg::StateSet > p_stateset = new osg::StateSet;

#if 0
    osg::Program* p_program = new osg::Program;
    p_stateset->setAttribute( p_program );

    osg::Shader* p_vertexshader = new osg::Shader( osg::Shader::VERTEX, glsl_vp );
    p_program->addShader( p_vertexshader );

    osg::Shader* p_fragmentshader = new osg::Shader( osg::Shader::FRAGMENT, glsl_fp );
    p_program->addShader( p_fragmentshader );

    osg::Uniform* p_baseTextureSampler = new osg::Uniform( "baseTexture", 0 );
    p_stateset->addUniform( p_baseTextureSampler );

    osg::Uniform* p_detialTexture0Sampler = new osg::Uniform( "detailTexture0", 1 );
    p_stateset->addUniform( p_detialTexture0Sampler );

    osg::Uniform* p_detialTexture1Sampler = new osg::Uniform( "detailTexture1", 2 );
    p_stateset->addUniform( p_detialTexture1Sampler );

#endif

    return p_stateset;
}

} // namespace yaf3d
