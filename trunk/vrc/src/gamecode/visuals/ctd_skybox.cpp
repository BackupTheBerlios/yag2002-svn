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
 # entity skybox
 #      this code is basing on Delta3D's implementation of skybox
 #                  ( http://www.delta3d.org/ )
 #
 #   date of creation:  03/24/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_base.h>
#include <ctd_application.h>
#include "ctd_skybox.h"

using namespace std;
using namespace CTD; 
using namespace osg; 


//! Implement and register the skybox entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( SkyBoxEntityFactory );


EnSkyBox::EnSkyBox()
{
    // this entity does not need a periodic update
    activate( false );

    // register entity attributes
    _attributeManager.addAttribute( "front"    , _texNames[ 0 ] );
    _attributeManager.addAttribute( "right"    , _texNames[ 1 ] );
    _attributeManager.addAttribute( "back"     , _texNames[ 2 ] );
    _attributeManager.addAttribute( "left"     , _texNames[ 3 ] );
    _attributeManager.addAttribute( "up"       , _texNames[ 4 ] );
    _attributeManager.addAttribute( "down"     , _texNames[ 5 ] );
}

EnSkyBox::~EnSkyBox()
{
}

void EnSkyBox::initialize()
{
    // setup texture side names
    _textureFilenameMap.insert( make_pair( 0, _texNames[ 0 ] ) );
    _textureFilenameMap.insert( make_pair( 1, _texNames[ 1 ] ) );
    _textureFilenameMap.insert( make_pair( 2, _texNames[ 2 ] ) );
    _textureFilenameMap.insert( make_pair( 3, _texNames[ 3 ] ) );
    _textureFilenameMap.insert( make_pair( 4, _texNames[ 4 ] ) );
    _textureFilenameMap.insert( make_pair( 5, _texNames[ 5 ] ) );

    osg::Group* p_group = new osg::Group();
    p_group->setName( "_skybox_" );
    _p_transformEyePoint = new EyeTransform();
    _p_transformEyePoint->setCullingActive( false );
    _p_transformEyePoint->addChild( makeBox() );
    p_group->addChild( _p_transformEyePoint );

    static_cast< Group* >( Application::get()->getSceneRootNode() )->addChild( p_group );
}

// thanks to delta3d team for the code ;-)
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
        osg::Vec2(0,0),
            osg::Vec2(1,0),
            osg::Vec2(1,1),
            osg::Vec2(0,1)
    };
    osg::Geometry *polyGeom[6];

    osg::Image *image[6];
    osg::Texture2D *texture[6];

    osg::Vec3Array *vArray[6];
    vArray[0] = new osg::Vec3Array(4, coords0);
    vArray[1] = new osg::Vec3Array(4, coords1);
    vArray[2] = new osg::Vec3Array(4, coords2);
    vArray[3] = new osg::Vec3Array(4, coords3);
    vArray[4] = new osg::Vec3Array(4, coords4);
    vArray[5] = new osg::Vec3Array(4, coords5);

    for (int side=0; side<6; side++)
    {
        polyGeom[side] = new osg::Geometry();

        polyGeom[side]->setVertexArray(vArray[side]);
        polyGeom[side]->setTexCoordArray(0, new osg::Vec2Array(4, tCoords));
        polyGeom[side]->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS, 0, 4));

        osg::StateSet *dstate = new osg::StateSet;
        dstate->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
        dstate->setMode( GL_CULL_FACE, osg::StateAttribute::ON );

        // clear the depth to the far plane.
        osg::Depth* depth = new osg::Depth;
        depth->setFunction(osg::Depth::ALWAYS);
        depth->setRange(1.0,1.0);   
        dstate->setAttributeAndModes(depth,osg::StateAttribute::ON );
        dstate->setMode(GL_FOG, osg::StateAttribute::OFF );
        dstate->setRenderBinDetails(-2,"RenderBin");

        //for wireframe rendering
        //   osg::PolygonMode *polymode = new osg::PolygonMode;
        //   polymode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
        //   dstate->setAttributeAndModes(polymode, osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
        if (!_textureFilenameMap[side].empty())
        {
            string filename = Application::get()->getMediaPath() + _textureFilenameMap[side].c_str();
            image[side] = osgDB::readImageFile(filename);

            if (image[side])
            {
                texture[side] = new osg::Texture2D;
                texture[side]->setImage(image[side]);
                texture[side]->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
                texture[side]->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
                dstate->setTextureAttributeAndModes(0, texture[side], osg::StateAttribute::ON);
            }
        }

        polyGeom[side]->setStateSet( dstate );
        _geode->addDrawable(polyGeom[side]);
    }
    return _geode.get();
}
