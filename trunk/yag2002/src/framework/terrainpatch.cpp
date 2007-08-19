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
 # all things needed for a terrain patch
 #
 #   date of creation:  07/30/2007
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include "base.h"
#include "log.h"
#include "terrainpatch.h"

//! Patch subdivisions in X and Y direction
//! TODO: make these parameters of a patch
#define L0_SUBDIV_X 8
#define L0_SUBDIV_Y 8


namespace yaf3d
{

TerrainPatch::TerrainPatch()
{
}

TerrainPatch::~TerrainPatch()
{
}

osg::ref_ptr< osg::PositionAttitudeTransform > TerrainPatch::getSceneNode()
{
    return _p_node;
}

bool TerrainPatch::build( const ImageTGA& image, const osg::Vec3f& scale, unsigned int column , unsigned int row, unsigned int sizeS, unsigned int sizeT )
{
    unsigned int imgSizeX = 0, imgSizeY = 0;
    image.getSize( imgSizeX, imgSizeY );

    // auto-correct the exceeding patch size
    if ( imgSizeX < column + sizeS )
        sizeS = imgSizeX - column;

    if ( imgSizeY < row + sizeT )
        sizeT = imgSizeY - row;

    // store relative patch position in terrain
    _relativePosition._v[ 0 ] = float( column ) / float( imgSizeX );
    _relativePosition._v[ 1 ] = float( row )    / float( imgSizeY );

    const unsigned char* p_data    = NULL;
    unsigned int         pixelsize = image.getNumChannels();

    // calc the pixel distance in the patch element
    unsigned int pixdiffX = sizeS / L0_SUBDIV_X;
    unsigned int pixdiffY = sizeT / L0_SUBDIV_Y;

    // check the patch subdivision, in particular for size 2^N + 1 images
    if ( pixdiffX < 2 )
    {
        log_verbose << "Terrain Patch: cannot subdevide the patch in Y direction, skipping the patch" << std::endl;
        return false;
    }
    if ( pixdiffY < 2 )
    {
        log_verbose << "Terrain Patch: cannot subdevide the patch in X direction, skipping the patch" << std::endl;
        return false;
    }

    // create the geode
    _p_node = new osg::PositionAttitudeTransform();
    std::stringstream patchname;
    patchname << "_patch" << column << "x" << row << "_";
    _p_node->setName( patchname.str() );
    osg::Geode* p_geode = new osg::Geode();
    _p_node->addChild( p_geode );

    // setup the drawable
    _p_drawable = new osg::Geometry();
    osg::Vec3Array* p_vertarray = new osg::Vec3Array();
    _p_drawable->setVertexArray( p_vertarray );

    // setup the colour binding
    _p_drawable->setColorBinding( osg::Geometry::BIND_OVERALL );

    float inv24bits = scale.z() / float( 0x10000000 );
    float minheight = float( 0xffffffff );
    float height    = 0.0f;

    for ( unsigned int cntY = 0; cntY <= sizeT; cntY += pixdiffY )
    {
        // for the case that the image has not the size 2^N + 1
        if ( column + cntY >=  imgSizeY )
        {
            log_verbose << "Terrain Patch: height map image Y size is not 2^N + 1, correcting last column!" << std::endl;
            cntY = imgSizeY - column - 1;
        }

        // go to next line in the image data
        p_data = image.getData( row, column + cntY );
        assert( p_data && "Terrain Patch: internal error while creating a terrain patch!" );        

        for ( unsigned int cntX = 0; cntX <= sizeS; cntX += pixdiffX )
        {
            // for the case that the image has not the size 2^N + 1
            if ( row + cntX >= imgSizeX )
            {
                log_verbose << "Terrain Patch: height map image X size is not 2^N + 1, correcting last row!" << std::endl;
                cntX = imgSizeX - row - 1;
            }

            // build the height value out of rgb ( 24 bits )
            height = float( ( ( unsigned int )p_data[ cntX * pixelsize ] << 16 )  | ( ( unsigned int )p_data[ cntX * pixelsize + 1 ] << 8 ) | ( ( unsigned int )p_data[ cntX * pixelsize + 2 ] ) );
            height *= inv24bits;

            osg::Vec3f pos; 
            pos._v[ 0 ] = float( cntX ) * scale.x();
            pos._v[ 1 ] = float( cntY ) * scale.y();
            pos._v[ 2 ] = height;

            // store the minimal height for later calculation of proper transformation
            if ( height < minheight )
                minheight = height;

            // add the point to vertex array
            p_vertarray->push_back( pos );
        }
    }

    // adjust the z values
    {
        osg::Vec3Array::iterator vert = p_vertarray->begin(), vertEnd = p_vertarray->end();
        for ( ; vert != vertEnd; ++vert )
            vert->_v[ 2 ] -= minheight;
    }

    //set the patch transformation
    _p_node->setPosition( osg::Vec3f( float( row ) * scale.x(), float( column ) * scale.y(), minheight ) );  

    // setup the draw elements array
    osg::DrawElementsUByte* p_drawElements = new osg::DrawElementsUByte( osg::PrimitiveSet::TRIANGLE_STRIP );

    // create triangle strips
    unsigned int indexdist = L0_SUBDIV_X + 1;
    for ( unsigned int indexY = 0; indexY < L0_SUBDIV_Y; indexY++ )
    {
        for ( int indexX = L0_SUBDIV_X; indexX >= 0; indexX-- )
        {
            if ( indexY % 2 )  // odd rows
            {
                p_drawElements->push_back( indexdist * ( indexY + 1 ) - ( indexX + 1 ) );
                p_drawElements->push_back( indexdist * ( indexY + 2 ) - ( indexX + 1 ) );
            }
            else               // even rows
            {
                p_drawElements->push_back( indexdist * indexY + ( indexX ) );
                p_drawElements->push_back( indexdist * ( indexY + 1 ) + ( indexX ) );
            }
        }
        // create a degenerated triangle in order to continue with next row, but not for last row
        if ( indexY < L0_SUBDIV_Y - 1 )
        {
            if ( indexY % 2 )  // odd rows
                p_drawElements->push_back( ( indexY + 2 ) * indexdist - 1 );
            else               // even rows
                p_drawElements->push_back( ( indexY + 1 ) * indexdist );
        }
    }

    _p_drawable->addPrimitiveSet( p_drawElements );

    _p_stateSet = new osg::StateSet;
    _p_stateSet->setGlobalDefaults();
    _p_stateSet->setDataVariance( osg::Object::DYNAMIC );
    _p_stateSet->setRenderingHint( osg::StateSet::OPAQUE_BIN/*TRANSPARENT_BIN*/ );
    _p_stateSet->setMode( GL_CULL_FACE, osg::StateAttribute::ON );

    _p_drawable->setStateSet( _p_stateSet.get() );

    p_geode->addDrawable( _p_drawable.get() );

    // store the patch x/y dimensions
    _relativeSize._v[ 0 ] = float( sizeS ) / float( imgSizeX );
    _relativeSize._v[ 1 ] = float( sizeT ) / float( imgSizeY );

    return true;
}

bool TerrainPatch::buildTexCoords( unsigned int channel, const osg::Vec2f& scale )
{
    if ( !_relativeSize.x() || !_relativeSize.y() )
    {
        log_error << "Terrain Patch: invalid terrain size! Has the patch been already built?" << std::endl;
        return false;
    }

    if ( channel >= MAX_TEX_CHANNELS )
    {
        log_error << "Terrain Patch: invalid texture channel. Supporting max channels: " << MAX_TEX_CHANNELS << std::endl;
        return false;
    }

    float coordS = 0.0f;
    float coordT = 1.0f;
    float coordStrideS = -1.0f / float( L0_SUBDIV_X );
    float coordStrideT =  1.0f / float( L0_SUBDIV_Y );

    // span over the entire terrain?
    if ( !scale.x() || !scale.y() )
    {
        coordStrideS *= -_relativeSize.x();
        coordStrideT *=  _relativeSize.y();
        coordS        =  _relativePosition.x();
        coordT        =  1.0f - _relativePosition.y() - _relativeSize.y();
    }
    else
    {
        coordStrideS *= scale.x();
        coordStrideT *= scale.y();
    }

    osg::Vec2Array* p_coordarray = new osg::Vec2Array();

    // calculate the texture coordinates of all patch vertices
    for ( unsigned int stepY = 0; stepY <= L0_SUBDIV_Y; stepY++ )
    {
        for ( unsigned int stepX = 0; stepX <= L0_SUBDIV_X; stepX++ )
        {
            osg::Vec2f coord( coordStrideS * float( L0_SUBDIV_X - stepX ), coordStrideT * float( stepY ) );
            // rotate the local texture coordinates by -90 degree
            p_coordarray->push_back( osg::Vec2f( coordS + coord.y(), coordT + coord.x() ) );
        }
    }

    _p_drawable->setTexCoordArray( channel, p_coordarray );

    return true;
}

osg::ref_ptr< osg::StateSet > TerrainPatch::getStateSet()
{
    return _p_stateSet;
}

} // namespace yaf3d
