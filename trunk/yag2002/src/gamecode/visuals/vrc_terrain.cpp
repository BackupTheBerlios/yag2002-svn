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
 # entity terrain
 #
 #   date of creation:  07/27/2007
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_terrain.h"
#include <terrainmanager.h>

namespace vrc
{

//! Implement and register the terrain entity factory
YAF3D_IMPL_ENTITYFACTORY( TerrainEntityFactory )


EnTerrainSection::EnTerrainSection() :
 _sectionID( 0 ),
 _scale( osg::Vec3f( 1.0f, 1.0f, 1.0f ) ),
 _tilesX( 16 ),
 _tilesY( 16 ),
 _blendBasemap( 0.3f ),
 _lod0ResolutionX( 8 ),
 _lod0ResolutionY( 8 ),
 _lod0RangeMin( 0.0f ),
 _lod0RangeMax( 200.0f ),
 _lod1ResolutionX( 4 ),
 _lod1ResolutionY( 4 ),
 _lod1RangeMin( 180.0f ),
 _lod1RangeMax( 400.0f ),
 _lod2ResolutionX( 1 ),
 _lod2ResolutionY( 1 ),
 _lod2RangeMin( 380.0f ),
 _lod2RangeMax( 100000.0f ),
 _enable( true )
{
    // register entity attributes
    getAttributeManager().addAttribute( "enable"           , _enable           );
    getAttributeManager().addAttribute( "position"         , _position         );
    getAttributeManager().addAttribute( "rotation"         , _rotation         );
    getAttributeManager().addAttribute( "scale"            , _scale            );
    getAttributeManager().addAttribute( "tilesX"           , _tilesX           );
    getAttributeManager().addAttribute( "tilesY"           , _tilesY           );
    getAttributeManager().addAttribute( "heightmap"        , _fileHeightmap    );
    getAttributeManager().addAttribute( "basemap"          , _fileBasemap      );
    getAttributeManager().addAttribute( "blendBasemap"     , _blendBasemap     );
    getAttributeManager().addAttribute( "layerMask"        , _fileLayerMask    );
    getAttributeManager().addAttribute( "detailmap0"       , _fileDetailmap0   );
    getAttributeManager().addAttribute( "detailmap0Repeat" , _detailmap0Repeat );
    getAttributeManager().addAttribute( "detailmap1"       , _fileDetailmap1   );
    getAttributeManager().addAttribute( "detailmap1Repeat" , _detailmap1Repeat );
    getAttributeManager().addAttribute( "detailmap2"       , _fileDetailmap2   );
    getAttributeManager().addAttribute( "detailmap2Repeat" , _detailmap2Repeat );

    getAttributeManager().addAttribute( "lod0ResolutionX" , _lod0ResolutionX   );
    getAttributeManager().addAttribute( "lod0ResolutionY" , _lod0ResolutionY   );
    getAttributeManager().addAttribute( "lod0RangeMin"    , _lod0RangeMin      );
    getAttributeManager().addAttribute( "lod0RangeMax"    , _lod0RangeMax      );

    getAttributeManager().addAttribute( "lod1ResolutionX" , _lod1ResolutionX   );
    getAttributeManager().addAttribute( "lod1ResolutionY" , _lod1ResolutionY   );
    getAttributeManager().addAttribute( "lod1RangeMin"    , _lod1RangeMin      );
    getAttributeManager().addAttribute( "lod1RangeMax"    , _lod1RangeMax      );

    getAttributeManager().addAttribute( "lod2ResolutionX" , _lod2ResolutionX   );
    getAttributeManager().addAttribute( "lod2ResolutionY" , _lod2ResolutionY   );
    getAttributeManager().addAttribute( "lod2RangeMin"    , _lod2RangeMin      );
    getAttributeManager().addAttribute( "lod2RangeMax"    , _lod2RangeMax      );
}

EnTerrainSection::~EnTerrainSection()
{
    try
    {
        // note: 0 is an invalid terrain section ID
        if ( _sectionID )
            yaf3d::TerrainManager::get()->releaseSection( _sectionID );
    }
    catch( const yaf3d::TerrainException& e )
    {
        log_error << "deleting terrain entity: cannot release terrain section." << std::endl;
        log_error << " reason: " << e.what() << std::endl;
    }
}

void EnTerrainSection::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle notifications
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_MENU_ENTER:

            if ( _enable )
            {
                if ( _p_terrainGrp.valid() )
                    removeFromTransformationNode( _p_terrainGrp.get() );

                //// remove the transformation node from its parents
                //unsigned int parents = getTransformationNode()->getNumParents();
                //for ( unsigned int cnt = 0; cnt < parents; ++cnt )
                //    getTransformationNode()->getParent( 0 )->removeChild( getTransformationNode() );
            }
            break;

        case YAF3D_NOTIFY_MENU_LEAVE:

            if ( _enable )
            {
//                yaf3d::Application::get()->getSceneRootNode()->addChild( getTransformationNode() );
                if ( _p_terrainGrp.valid() )
                   addToTransformationNode( _p_terrainGrp.get() );

            }
            break;

        // re-build the terrain whenever an attribute changed
        case YAF3D_NOTIFY_ENTITY_ATTRIBUTE_CHANGED:

            if ( _sectionID )
                yaf3d::TerrainManager::get()->releaseSection( _sectionID );

            // reset the section id
            _sectionID = 0;

            if ( _p_terrainGrp.valid() )
                removeFromTransformationNode( _p_terrainGrp.get() );

            _p_terrainGrp = NULL;

            if ( _enable )
            {
                _p_terrainGrp = setup();
                addToTransformationNode( _p_terrainGrp.get() );
            }

            break;

        // remove the node on shutdown. in normal case the removal happens when entering the menu, however
        //  the entity can also be in the menu itself.
        case YAF3D_NOTIFY_SHUTDOWN:

            removeFromTransformationNode( _p_terrainGrp.get() );
            break;

        default:
            ;
    }
}

void EnTerrainSection::initialize()
{
    // register entity in order to get notifications
    yaf3d::EntityManager::get()->registerNotification( this, true );

    // setup the terrain
    _p_terrainGrp = setup();
}

osg::ref_ptr< osg::Group > EnTerrainSection::setup()
{
    osg::ref_ptr< osg::Group > group;

    yaf3d::TerrainConfig conf;
    conf._scale             = _scale;
    conf._tilesX            = _tilesX;
    conf._tilesY            = _tilesY;
    conf._fileHeightmap     = _fileHeightmap;
    conf._fileBasemap       = _fileBasemap;
    conf._blendBasemap      = _blendBasemap;
    conf._fileLayerMask     = _fileLayerMask;
    conf._fileDetailmap0    = _fileDetailmap0;
    conf._detailmap0Repeat  = _detailmap0Repeat;
    conf._fileDetailmap1    = _fileDetailmap1;
    conf._detailmap1Repeat  = _detailmap1Repeat;
    conf._fileDetailmap2    = _fileDetailmap2;
    conf._detailmap2Repeat  = _detailmap2Repeat;

    // configure the LODs
    
    if ( _lod0ResolutionX && _lod0ResolutionY )
    {
        conf._lodResolutions.push_back( std::make_pair( _lod0ResolutionX, _lod0ResolutionY ) );
        conf._lodRanges.push_back( std::make_pair( _lod0RangeMin, _lod0RangeMax ) );
    }

    if ( _lod1ResolutionX && _lod1ResolutionY )
    {
        conf._lodResolutions.push_back( std::make_pair( _lod1ResolutionX, _lod1ResolutionY ) );
        conf._lodRanges.push_back( std::make_pair( _lod1RangeMin, _lod1RangeMax ) );
    }

    if ( _lod2ResolutionX && _lod2ResolutionY )
    {
        conf._lodResolutions.push_back( std::make_pair( _lod2ResolutionX, _lod2ResolutionY ) );
        conf._lodRanges.push_back( std::make_pair( _lod2RangeMin, _lod2RangeMax ) );
    }

    try
    {
        _sectionID = yaf3d::TerrainManager::get()->addSection( conf );
        group = yaf3d::TerrainManager::get()->getSectionNode( _sectionID );
    }
    catch( const yaf3d::TerrainException& e )
    {
        log_error << "cannot create terrain entity." << std::endl;
        log_error << " reason: " << e.what() << std::endl;
    }

    // set position and rotation of the terrain section
    setPosition( _position );
    osg::Quat   rot(
                     osg::DegreesToRadians( _rotation.x() ), osg::Vec3f( 1.0f, 0.0f, 0.0f ),
                     osg::DegreesToRadians( _rotation.y() ), osg::Vec3f( 0.0f, 1.0f, 0.0f ),
                     osg::DegreesToRadians( _rotation.z() ), osg::Vec3f( 0.0f, 0.0f, 1.0f )
                    );

    setRotation( rot );

    return group;
}

void EnTerrainSection::enable( bool en )
{
    if ( en == _enable )
        return;

    if ( !_p_terrainGrp.valid() )
        return;

    _enable = en;
    if ( en )
        addToTransformationNode( _p_terrainGrp.get() );
    else
        removeFromTransformationNode( _p_terrainGrp.get() );
}

} // namespace vrc
