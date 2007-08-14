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


EnTerrain::EnTerrain() :
_enable( true ),
_scale( osg::Vec3f( 100.0f, 100.0f, 100.0f ) )
{
    // register entity attributes
    getAttributeManager().addAttribute( "enable"           , _enable           );
    getAttributeManager().addAttribute( "scale"            , _scale            );
    getAttributeManager().addAttribute( "heightmap"        , _fileHeightmap    );
    getAttributeManager().addAttribute( "basemap"          , _fileBasemap      );
    getAttributeManager().addAttribute( "detailmap0"       , _fileDetailmap0   );
    getAttributeManager().addAttribute( "detailmap0Repeat" , _detailmap0Repeat );
    getAttributeManager().addAttribute( "detailmap1"       , _fileDetailmap1   );
    getAttributeManager().addAttribute( "detailmap1Repeat" , _detailmap1Repeat );
}

EnTerrain::~EnTerrain()
{
}

void EnTerrain::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle notifications
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_MENU_ENTER:

            if ( _enable )
            {
                removeFromTransformationNode( _p_terrainGrp.get() );
            }
            break;

        case YAF3D_NOTIFY_MENU_LEAVE:

            if ( _enable )
            {
                addToTransformationNode( _p_terrainGrp.get() );
            }
            break;

        // re-create the skybox whenever an attribute changed
        case YAF3D_NOTIFY_ENTITY_ATTRIBUTE_CHANGED:
            removeFromTransformationNode( _p_terrainGrp.get() );
            _p_terrainGrp = setup();
            if ( _enable )
                addToTransformationNode( _p_terrainGrp.get() );

        // if used in menu then this entity is persisten, so we have to trigger its deletion on shutdown
        case YAF3D_NOTIFY_SHUTDOWN:

            break;

        default:
            ;
    }
}

void EnTerrain::initialize()
{
    // register entity in order to get notifications   
    yaf3d::EntityManager::get()->registerNotification( this, true );   

    // setup the terrain
    _p_terrainGrp = setup();
}

osg::ref_ptr< osg::Group > EnTerrain::setup()
{
    osg::ref_ptr< osg::Group > group;

    yaf3d::TerrainConfig conf;
    conf._scale             = _scale;
    conf._fileHeightmap     = _fileHeightmap;
    conf._fileBasemap       = _fileBasemap;
    conf._fileDetailmap0    = _fileDetailmap0;
    conf._detailmap0Repeat  = _detailmap0Repeat;
    conf._fileDetailmap1    = _fileDetailmap1;
    conf._detailmap1Repeat  = _detailmap1Repeat;

    try
    {
        group = yaf3d::TerrainManager::get()->setup( conf );
    }
    catch( const yaf3d::TerrainException& e )
    {
        log_error << "cannot create terrain entity." << std::endl;
        log_error << " reason: " << e.what() << std::endl;
    }

    return group;
}

void EnTerrain::enable( bool en )
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
