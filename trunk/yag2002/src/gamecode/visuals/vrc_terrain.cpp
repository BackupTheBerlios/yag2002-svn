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
_enable( true ),
_scale( osg::Vec3f( 1.0f, 1.0f, 1.0f ) )
{
    // register entity attributes
    getAttributeManager().addAttribute( "enable"           , _enable           );
    getAttributeManager().addAttribute( "position"         , _position         );
    getAttributeManager().addAttribute( "rotation"         , _rotation         );
    getAttributeManager().addAttribute( "scale"            , _scale            );
    getAttributeManager().addAttribute( "heightmap"        , _fileHeightmap    );
    getAttributeManager().addAttribute( "basemap"          , _fileBasemap      );
    getAttributeManager().addAttribute( "detailmap0"       , _fileDetailmap0   );
    getAttributeManager().addAttribute( "detailmap0Repeat" , _detailmap0Repeat );
    getAttributeManager().addAttribute( "detailmap1"       , _fileDetailmap1   );
    getAttributeManager().addAttribute( "detailmap1Repeat" , _detailmap1Repeat );
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
                removeFromTransformationNode( _p_terrainGrp.get() );
            }
            break;

        case YAF3D_NOTIFY_MENU_LEAVE:

            if ( _enable )
            {
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
    conf._fileHeightmap     = _fileHeightmap;
    conf._fileBasemap       = _fileBasemap;
    conf._fileDetailmap0    = _fileDetailmap0;
    conf._detailmap0Repeat  = _detailmap0Repeat;
    conf._fileDetailmap1    = _fileDetailmap1;
    conf._detailmap1Repeat  = _detailmap1Repeat;

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
