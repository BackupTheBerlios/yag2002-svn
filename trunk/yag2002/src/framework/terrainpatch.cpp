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
#include "terraintesselator.h"


namespace yaf3d
{


TerrainPatch::TerrainPatch( unsigned int tesselatortype ) :
 _p_tesselator( NULL )
{
    switch ( tesselatortype )
    {
        case eTesselatorScreenSpace:

            _p_tesselator = new TesselatorScreenSpace;
            break;

        case eTesselatorWorldSpace:

            _p_tesselator = new TesselatorWorldSpace;
            break;

        default:

            log_error << "unknown tesselator type! set to screen space tesselator." << std::endl;
            _p_tesselator = new TesselatorScreenSpace;
    }
}

TerrainPatch::~TerrainPatch()
{
}

osg::ref_ptr< osg::PositionAttitudeTransform > TerrainPatch::getSceneNode()
{
    return _p_node;
}

bool TerrainPatch::build( ImageTGA& image, const osg::Vec3f& scale, unsigned int column , unsigned int row, unsigned int sizeS, unsigned int sizeT )
{
    unsigned int imgSizeX = 0, imgSizeY = 0;
    image.getSize( imgSizeX, imgSizeY );

    // auto-correct the exceeding patch size
    if ( imgSizeX < column + sizeS )
        sizeS = imgSizeX - column;

    if ( imgSizeY < row + sizeT )
        sizeT = imgSizeY - row;

    unsigned char* p_data     = NULL;
    unsigned int   pixelsize  = image.getNumChannels();
    unsigned int   height     = 0;

    // calc the subdivision in the patch element
    unsigned int   vertsX = sizeS / 4;
    unsigned int   vertsY = sizeT / 4;

    // check the patch subdivision, in particular for size 2^N + 1 images
    if ( vertsX < 2 )
    {
        log_verbose << "cannot subdevide the patch in Y direction, skipping the patch" << std::endl;
        return false;
    }
    if ( vertsY < 2 )
    {
        log_verbose << "cannot subdevide the patch in X direction, skipping the patch" << std::endl;
        return false;
    }

    osg::ref_ptr< osg::Geometry >  p_drawable  = new osg::Geometry();
    osg::Vec3Array*                p_vertarray = new osg::Vec3Array();

    log_info << "new patch" << std::endl;

    float inv24bits = 1.0f / float( 0x10000000 );
    for ( unsigned int cntY = 0; cntY <= sizeT; cntY += vertsY )
    {
        // for the case that the image has not the size 2^N + 1
        if ( column + cntY >=  imgSizeY )
        {
            log_verbose << "height map image Y size is not 2^N + 1, correcting last column!" << std::endl;
            cntY = imgSizeY - column - 1;
        }

        // go to next line in the image data
        p_data = image.getData( row, column + cntY );
        assert( p_data && "internal error while creating a terrain patch!" );        

        for ( unsigned int cntX = 0; cntX <= sizeS; cntX += vertsX )
        {
            // for the case that the image has not the size 2^N + 1
            if ( row + cntX >= imgSizeX )
            {
                log_verbose << "height map image X size is not 2^N + 1, correcting last row!" << std::endl;
                cntX = imgSizeX - row - 1;
            }

            // build the height value out of rgb ( 24 bits )
            height = ( ( unsigned int )p_data[ cntX * pixelsize ] << 16 )  | ( ( unsigned int )p_data[ cntX * pixelsize + 1 ] << 8 ) | ( ( unsigned int )p_data[ cntX * pixelsize + 2 ] );

            osg::Vec3f pos;
            pos._v[ 0 ] = ( float( row )    + float( cntX ) ) * scale.x();
            pos._v[ 1 ] = ( float( column ) + float( cntY ) ) * scale.y();
            pos._v[ 2 ] = ( float( height ) * inv24bits )     * scale.z();

            // add the point to vertex array
            p_vertarray->push_back( pos );

            log_verbose << pos.x() << "," << pos.y() << "," << pos.z() << " ";
        }

        log_info << std::endl;
    }

    // create the geode
    _p_node = new osg::PositionAttitudeTransform();
    std::stringstream patchname;
    patchname << "_patch" << column << "x" << row << "_";
    _p_node->setName( patchname.str() );
    osg::Geode* p_geode = new osg::Geode();
    _p_node->addChild( p_geode );

    p_geode->addDrawable( p_drawable.get() );
    p_drawable->setVertexArray( p_vertarray );

    //set the patch transformation
    //! TODO: do we need a positioning at all? we can use absolute vertex positions
    //_p_node->setPosition( osg::Vec3f( float( column ) * scale._v[ 0 ], float( row )  * scale._v[ 1 ], 0.0f ) );  

    //! TODO: setup the index array
    osg::UByteArray* p_indexarray = new osg::UByteArray;
    p_drawable->setVertexIndices( p_indexarray );

    // create triangle strips
    unsigned int indexdist = vertsX + 1;
    for ( unsigned int indexY = 0; indexY < vertsY; indexY++ )
    {
        for ( unsigned int indexX = 0; indexX < vertsX + 1; indexX++ )
        {
            if ( indexY % 2 )  // odd rows
            {
                p_indexarray->push_back( indexdist * ( indexY + 1 ) - indexX - 1 );
                p_indexarray->push_back( indexdist * ( indexY + 2 ) - indexX - 1 );
            }
            else               // even rows
            {
                p_indexarray->push_back( indexdist * indexY + indexX );
                p_indexarray->push_back( indexdist * indexY + indexdist + indexX );
            }
        }

        // create a degenerated triangle in order to continue with next row, but not for last vertex
        if ( indexY < vertsY )
        {
            if ( indexY % 2 )  // odd rows
                p_indexarray->push_back( ( indexY + 1 ) * indexdist );
            else               // even rows
                p_indexarray->push_back( ( indexY + 2 ) * indexdist - 1 );
        }
    }
    p_drawable->addPrimitiveSet( new osg::DrawArrays( osg::PrimitiveSet::TRIANGLE_STRIP, 0, p_indexarray->size() ) );

    //! TODO: check why the polys are two sided!
    osg::StateSet* p_stateset = new osg::StateSet;
    p_stateset->setDataVariance( osg::Object::STATIC );
    p_stateset->setRenderingHint( osg::StateSet::TRANSPARENT_BIN );
    p_stateset->setGlobalDefaults();

    p_drawable->setStateSet( p_stateset );

    return true;
}

void TerrainPatch::reset()
{
    // release the node
    _p_node = NULL;
}

void TerrainPatch::update( osg::CameraNode* p_cam )
{
    //! TODO
}

bool TerrainPatch::isVisible()
{
    //! TODO
    return true;
}

void TerrainPatch::render()
{
    //! TODO
}

} // namespace yaf3d
