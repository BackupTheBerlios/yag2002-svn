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
 # entity skybox
 #
 #   date of creation:  03/24/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_skybox.h"

namespace vrc
{

//! Implement and register the skybox entity factory
YAF3D_IMPL_ENTITYFACTORY( SkyBoxEntityFactory )


EnSkyBox::EnSkyBox() :
_usedInMenu( false ),
_enable( true )
{
    // register entity attributes
    getAttributeManager().addAttribute( "usedInMenu"    , _usedInMenu    );
    getAttributeManager().addAttribute( "enable"        , _enable        );
    getAttributeManager().addAttribute( "right"         , _texNames[ 0 ] );
    getAttributeManager().addAttribute( "left"          , _texNames[ 1 ] );
    getAttributeManager().addAttribute( "front"         , _texNames[ 2 ] );
    getAttributeManager().addAttribute( "back"          , _texNames[ 3 ] );
    getAttributeManager().addAttribute( "up"            , _texNames[ 4 ] );
    getAttributeManager().addAttribute( "down"          , _texNames[ 5 ] );
}

EnSkyBox::~EnSkyBox()
{
    if ( _p_skyGrp.get() )
        _p_skyGrp = NULL;
}

void EnSkyBox::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle notifications
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_MENU_ENTER:

            if ( _enable )
            {
                if ( !_usedInMenu ) 
                    removeFromTransformationNode( _p_skyGrp.get() );
                else
                    addToTransformationNode( _p_skyGrp.get() );
            }
            break;

        case YAF3D_NOTIFY_MENU_LEAVE:

            if ( _enable )
            {
                if ( !_usedInMenu )
                    addToTransformationNode( _p_skyGrp.get() );
                else
                    removeFromTransformationNode( _p_skyGrp.get() );
            }
            break;

        // re-create the skybox whenever an attribute changed
        case YAF3D_NOTIFY_ENTITY_ATTRIBUTE_CHANGED:
            removeFromTransformationNode( _p_skyGrp.get() );
            _p_skyGrp = setupSkybox();
            if ( _enable )
                addToTransformationNode( _p_skyGrp.get() );

        // if used in menu then this entity is persisten, so we have to trigger its deletion on shutdown
        case YAF3D_NOTIFY_SHUTDOWN:

            if ( _usedInMenu )
                yaf3d::EntityManager::get()->deleteEntity( this );
            break;

        default:
            ;
    }
}

void EnSkyBox::initialize()
{
    // register entity in order to get notifications   
    yaf3d::EntityManager::get()->registerNotification( this, true );   

    // setup the skybox
    _p_skyGrp = setupSkybox();
}

osg::ref_ptr< osg::Group > EnSkyBox::setupSkybox()
{
    _textureFilenameMap.clear();

    // setup texture side names
    _textureFilenameMap.insert( make_pair( osg::TextureCubeMap::POSITIVE_X, _texNames[ 0 ] ) );
    _textureFilenameMap.insert( make_pair( osg::TextureCubeMap::NEGATIVE_X, _texNames[ 1 ] ) );
    _textureFilenameMap.insert( make_pair( osg::TextureCubeMap::POSITIVE_Y, _texNames[ 2 ] ) );
    _textureFilenameMap.insert( make_pair( osg::TextureCubeMap::NEGATIVE_Y, _texNames[ 3 ] ) );
    _textureFilenameMap.insert( make_pair( osg::TextureCubeMap::POSITIVE_Z, _texNames[ 4 ] ) );
    _textureFilenameMap.insert( make_pair( osg::TextureCubeMap::NEGATIVE_Z, _texNames[ 5 ] ) );

    osg::ref_ptr< osg::Group > skyboxgrp = new osg::Group();
    skyboxgrp->setName( "_skybox_" );
    _p_transformEyePoint = new vrc::gameutils::EyeTransform();
    _p_transformEyePoint->setCullingActive( false );
    _p_transformEyePoint->addChild( makeBox() );
    skyboxgrp->addChild( _p_transformEyePoint );

    return skyboxgrp;
}

void EnSkyBox::enable( bool en )
{
    if ( en == _enable )
        return;

    _enable = en;
    if ( en )
        addToTransformationNode( _p_skyGrp.get() );
    else
        removeFromTransformationNode( _p_skyGrp.get() );
}

osg::Node* EnSkyBox::makeBox()
{
    _geode  = new osg::Geode();
    float x = 1.f;
    float y = 1.f;
    float z = 1.f;
    osg::Vec3 coords0[] = //front
    {
        osg::Vec3(-x, y, -z),
        osg::Vec3( x, y, -z),
        osg::Vec3( x, y, +z),
        osg::Vec3(-x, y, +z),
    };

    osg::Vec3 coords1[] = //right
    {
        osg::Vec3(x,  y, -z),
        osg::Vec3(x, -y, -z),
        osg::Vec3(x, -y,  z),
        osg::Vec3(x,  y,  z)
    };

    osg::Vec3 coords2[] = //back
    {
        osg::Vec3( x, -y, -z),
        osg::Vec3(-x, -y, -z),
        osg::Vec3(-x, -y,  z),
        osg::Vec3( x, -y,  z)
    };

    osg::Vec3 coords3[] = //left
    {
        osg::Vec3(-x, -y, -z),
        osg::Vec3(-x,  y, -z),
        osg::Vec3(-x,  y,  z),
        osg::Vec3(-x, -y,  z)
    };

    osg::Vec3 coords4[] = //top
    {
        osg::Vec3(-x,  y, z),
        osg::Vec3( x,  y, z),
        osg::Vec3( x, -y, z),
        osg::Vec3(-x, -y, z)
    };
    osg::Vec3 coords5[] = //bottom
    {
        osg::Vec3(-x,  y, -z),
        osg::Vec3(-x, -y, -z),
        osg::Vec3( x, -y, -z),
        osg::Vec3( x,  y, -z)
    };

    osg::Vec2 tCoords[] =
    {
        osg::Vec2(0.0f,0.0f),
        osg::Vec2(1.0f,0.0f),
        osg::Vec2(1.0f,1.0f),
        osg::Vec2(0.0f,1.0f)
    };
    osg::Geometry *polyGeom[ 6 ];

    osg::Image *image[ 6 ];
    osg::Texture2D *texture[ 6 ];

    osg::Vec3Array *vArray[ 6 ];
    vArray[ 0 ] = new osg::Vec3Array( 4, coords0 );
    vArray[ 1 ] = new osg::Vec3Array( 4, coords1 );
    vArray[ 2 ] = new osg::Vec3Array( 4, coords2 );
    vArray[ 3 ] = new osg::Vec3Array( 4, coords3 );
    vArray[ 4 ] = new osg::Vec3Array( 4, coords4 );
    vArray[ 5 ] = new osg::Vec3Array( 4, coords5 );

    for ( int side = 0; side < 6; ++side )
    {
        polyGeom[ side ] = new osg::Geometry();

        polyGeom[ side ]->setVertexArray( vArray[ side ] );
        osg::ref_ptr< osg::Vec2Array > tcoords = new osg::Vec2Array( 4, tCoords );
        polyGeom[ side ]->setTexCoordArray( 0, tcoords.get() );
        osg::ref_ptr< osg::DrawArrays > drawarray = new osg::DrawArrays( osg::PrimitiveSet::QUADS, 0, 4 );        
        polyGeom[ side ]->addPrimitiveSet( drawarray.get() );

        osg::StateSet *dstate = new osg::StateSet;
        dstate->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
        dstate->setMode( GL_CULL_FACE, osg::StateAttribute::ON );

        // clear the depth to the far plane.
        osg::Depth* depth = new osg::Depth;
        depth->setFunction( osg::Depth::ALWAYS );
        depth->setRange( 1.0f, 1.0f );   
        dstate->setAttributeAndModes( depth,osg::StateAttribute::ON );
        dstate->setMode( GL_FOG, osg::StateAttribute::OFF );
        dstate->setRenderBinDetails( -2, "RenderBin" );

        //for wireframe rendering
        //   osg::PolygonMode *polymode = new osg::PolygonMode;
        //   polymode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
        //   dstate->setAttributeAndModes(polymode, osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
        if ( !_textureFilenameMap[ side ].empty() )
        {
            std::string filename = yaf3d::Application::get()->getMediaPath() + _textureFilenameMap[ side ].c_str();
            image[ side ] = osgDB::readImageFile( filename );

            if ( image[ side ])
            {
                texture[ side ] = new osg::Texture2D;
                texture[ side ]->setImage( image[ side ] );
                texture[ side ]->setWrap( osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE );
                texture[ side ]->setWrap( osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE );
                dstate->setTextureAttributeAndModes( 0, texture[ side ], osg::StateAttribute::ON );
            }
        }

        polyGeom[side]->setStateSet( dstate );
        _geode->addDrawable( polyGeom[ side ] );
    }
    return _geode.get();
}

} // namespace vrc
