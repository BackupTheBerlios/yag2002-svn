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
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
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
 _enable( true ),
 _usedInMenu( false ),
 _shadowEnable( true ),
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
 _cgfShadow( false )
{
    // register entity attributes
    getAttributeManager().addAttribute( "enable"           , _enable           );
    getAttributeManager().addAttribute( "usedInMenu"       , _usedInMenu       );
    getAttributeManager().addAttribute( "shadowEnable"     , _shadowEnable     );
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
    getAttributeManager().addAttribute( "detailmap3"       , _fileDetailmap3   );
    getAttributeManager().addAttribute( "detailmap3Repeat" , _detailmap3Repeat );

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
    // remove shadow from shadow manager
    if ( ( _shadowEnable ) && getTransformationNode() )
    {
        yaf3d::ShadowManager::get()->removeShadowNode( getTransformationNode() );
    }

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
        {
            if ( _enable )
            {
                if ( _usedInMenu )
                {
                    addToSceneGraph();
                }
                else
                {
                    removeFromSceneGraph();
                }
            }
        }
        break;

        case YAF3D_NOTIFY_MENU_LEAVE:
        {
            if ( _enable )
            {
                if ( _usedInMenu )
                {
                    removeFromSceneGraph();
                }
                else
                {
                    addToSceneGraph();
                }
            }
        }
        break;

        //! Note: by default the level manager re-adds persistent entity transformation nodes to its entity group while unloading a level.
        //        thus we have to remove shadow nodes from that entity group on unloading a level; addToSceneGraph() does this job.
        case YAF3D_NOTIFY_ENTITY_TRANSNODE_CHANGED:
        {
            if ( _usedInMenu )
            {
                removeFromSceneGraph();
                addToSceneGraph();
            }
        }
        break;

        case YAF3D_NOTIFY_ENTITY_ATTRIBUTE_CHANGED:
        {
            // re-setup mesh
            if ( _p_terrainGrp.valid() )
                removeFromTransformationNode( _p_terrainGrp.get() );

            _p_terrainGrp = setup();

            if ( _p_terrainGrp.valid() && _enable )
                addToTransformationNode( _p_terrainGrp.get() );
        }
        break;

        case YAF3D_NOTIFY_UNLOAD_LEVEL:

            // release the shader state set
            if ( !_usedInMenu )
                removeFromTransformationNode( _p_terrainGrp.get() );

            break;

        // if used in menu then this entity is persisten, so we have to trigger its deletion on shutdown
        case YAF3D_NOTIFY_SHUTDOWN:
        {
            removeFromTransformationNode( _p_terrainGrp.get() );

            if ( _usedInMenu )
                yaf3d::EntityManager::get()->deleteEntity( this );
        }
        break;

        default:
            ;
    }
}

void EnTerrainSection::initialize()
{
    // setup the terrain
    _p_terrainGrp = setup();

    yaf3d::Configuration::get()->getSettingValue( YAF3D_GS_SHADOW_ENABLE, _cgfShadow );

    if ( _p_terrainGrp.valid() )
        addToTransformationNode( _p_terrainGrp.get() );

    // the node is added and removed by notification callback!
    removeFromSceneGraph();

    // register entity in order to get notifications
    yaf3d::EntityManager::get()->registerNotification( this, true );
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
    conf._fileDetailmap3    = _fileDetailmap3;
    conf._detailmap3Repeat  = _detailmap3Repeat;

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

void EnTerrainSection::removeFromSceneGraph()
{
    if ( !_p_terrainGrp.valid() )
        return;

    yaf3d::EntityManager::get()->removeFromScene( this );
}

void EnTerrainSection::addToSceneGraph()
{
    if ( !_p_terrainGrp.valid() )
        return;

    // enable shadow only if it is enabled in configuration
    if ( _cgfShadow && _shadowEnable )
    {
        yaf3d::ShadowManager::get()->addShadowNode( getTransformationNode(), yaf3d::ShadowManager::eReceiveShadow );
        yaf3d::ShadowManager::get()->updateShadowArea();
    }
    else
    {
        yaf3d::EntityManager::get()->addToScene( this );
    }
}

} // namespace vrc
