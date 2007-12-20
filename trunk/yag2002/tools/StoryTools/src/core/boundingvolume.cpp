/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2007, A. Botorabi
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
 # bounding volume used for hit tests
 #
 #   date of creation:  07/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#include <main.h>
#include "boundingvolume.h"


namespace beditor
{

BaseBoundingVolume::BaseBoundingVolume()
{
}

BaseBoundingVolume::~BaseBoundingVolume()
{
}

BBox::BBox()
{
    _min.loadZero();
    _max.loadZero();
}

BBox::~BBox()
{
}

void BBox::setDimensions( const Eigen::Vector3f& min, const Eigen::Vector3f& max )
{
    _min = min;
    _max = max;
}

bool BBox::contains( const Eigen::Vector3f& scale, const Eigen::Vector3f& volumepos, const Eigen::Vector3f& pos ) const
{
    Eigen::Vector3f minpos( _min.x() * scale.x(), _min.y() * scale.y(), _min.z() * scale.z() );
    minpos += volumepos;

    Eigen::Vector3f maxpos( _max.x() * scale.x(), _max.y() * scale.y(), _max.z() * scale.z() );
    maxpos += volumepos;

    if ( 
        ( pos.x() >= minpos.x() ) && ( pos.x() <= maxpos.x() ) &&
        ( pos.y() >= minpos.y() ) && ( pos.y() <= maxpos.y() ) &&
        ( pos.z() >= minpos.z() ) && ( pos.z() <= maxpos.z() )
       )
       return true;

    return false;
}


BLine::BLine() :
 _width( 5.0f )
{
}

BLine::~BLine()
{
}

void BLine::setSourceDestination( BaseNodePtr src, BaseNodePtr dest )
{
    _src  = src;
    _dest = dest;
}

void BLine::setLineWidth( float width )
{
    _width = width;
}

bool BLine::contains( const Eigen::Vector3f& scale, const Eigen::Vector3f& volumepos, const Eigen::Vector3f& pos ) const
{
    if ( !_src.getRef() || !_dest.getRef() )
        return false;

    Eigen::Vector3f SP( pos - _src->getPosition() );
    Eigen::Vector3f SD( _dest->getPosition() - _src->getPosition() );

    float lenSD = sqrtf( SD.x() * SD.x() + SD.y() * SD.y() + SD.z() * SD.z() );
    if ( lenSD < 0.1f )
        return false;

    float beta = ( SD.x() * SP.x() + SD.y() * SP.y() + SD.z() * SP.z() ) / ( lenSD * lenSD );

    // quick check for line projection
    if ( ( beta < 0.1f ) || ( beta > 0.9f ) )
        return false;

    // calculate the distance from position to line
    Eigen::Vector3f Pproj( _src->getPosition() + ( SD * beta ) );
    Eigen::Vector3f PosPproj   = pos - Pproj;
    float           distToLine = sqrtf( PosPproj.x() * PosPproj.x() + PosPproj.y() * PosPproj.y() + PosPproj.z() * PosPproj.z() );

    if ( distToLine < _width )
        return true;

    return false;
}

} // namespace beditor
