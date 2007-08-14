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

TerrainManager::TerrainManager()
{
}

TerrainManager::~TerrainManager()
{
}

void TerrainManager::shutdown()
{
    //!TODO: release all patches and other terrain resources

    destroy();
}

osg::ref_ptr< osg::Group > TerrainManager::setup( TerrainConfig& config ) throw ( TerrainException )
{
    std::string mediapath = Application::get()->getMediaPath();

    // currently we accept only tga files as heightmap
    ImageTGA tga;
    if ( !tga.load( mediapath + config._fileHeightmap ) )
        throw TerrainException( std::string( "cannot load heightmap image" ) + mediapath + config._fileHeightmap );

    //! TODO: move these to object interface
    std::vector< TerrainPatch* > patchlist;
    unsigned int tilesX = 16, tilesY = 16;
    
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
        p_basetex->setImage( p_baseimage );
        p_basetex->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
        p_basetex->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );
    }

    // detailed texture map 0
    osg::Texture2D* p_detailtex0 = new osg::Texture2D;

    {
        osg::Image* p_detailimage0 = osgDB::readImageFile( mediapath + config._fileDetailmap0 );

        if ( !p_detailimage0 )
        {
            log_warning << "Terrain Manager: cannot load detail map 0 image" << std::endl;
        }
        p_detailtex0->setImage( p_detailimage0 );
        p_detailtex0->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
        p_detailtex0->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );
    }

    // detailed texture map 1
    osg::Texture2D* p_detailtex1 = new osg::Texture2D;

    {
        osg::Image* p_detailimage1 = osgDB::readImageFile( mediapath + config._fileDetailmap1 );

        if ( !p_detailimage1 )
        {
            log_warning << "Terrain Manager: cannot load detail map 1 image" << std::endl;
        }
        p_detailtex1->setImage( p_detailimage1 );
        p_detailtex1->setWrap( osg::Texture::WRAP_S, osg::Texture::REPEAT );
        p_detailtex1->setWrap( osg::Texture::WRAP_T, osg::Texture::REPEAT );
    }

    osg::ref_ptr< osg::Group > group = new osg::Group();
    group->setName( "_terrain_" );

    for ( unsigned int cntY = 0; cntY < sizeY; cntY += tilesY )
    {
        for ( unsigned int cntX = 0; cntX < sizeX; cntX += tilesX )
        {
            TerrainPatch* p_patch = new TerrainPatch( TerrainPatch::eTesselatorScreenSpace );
            if ( !p_patch->build( tga, config._scale, cntX, cntY, tilesX, tilesY ) )
            {
                delete p_patch;
                continue;
            }

            // build the base texture map
            if ( p_patch->buildTexCoords( 0 ) )
                p_patch->getStateSet()->setTextureAttributeAndModes( 0, p_basetex, osg::StateAttribute::ON );

            // build the detail texture map 0
            if ( p_patch->buildTexCoords( 1, config._detailmap0Repeat ) )
                p_patch->getStateSet()->setTextureAttributeAndModes( 1, p_detailtex0, osg::StateAttribute::ON );

            // build the detail texture map 1
            if ( p_patch->buildTexCoords( 2, config._detailmap1Repeat ) )
                p_patch->getStateSet()->setTextureAttributeAndModes( 2, p_detailtex1, osg::StateAttribute::ON );

            patchlist.push_back( p_patch );
            group->addChild( p_patch->getSceneNode().get() );
        }
    }

    return group;
}

void TerrainManager::update( osg::CameraNode* p_cam )
{
    //! TODO
}

void TerrainManager::render()
{
    //! TODO
}

} // namespace yaf3d
