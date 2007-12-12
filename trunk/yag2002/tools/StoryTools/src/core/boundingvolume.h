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

#ifndef _BOUNDINGVOLUME_H_
#define _BOUNDINGVOLUME_H_

#include <main.h>

namespace beditor
{

//! Base class for bounding volumes
class BaseBoundingVolume : public RefCount< BaseBoundingVolume >
{
    DECLARE_SMARTPTR_ACCESS( BaseBoundingVolume )

    public:

        //! Construct a bounding volume
                                                    BaseBoundingVolume();

        //! Check if the given point (pos) is contained in scaled (scale) and positioned (boxpos) volume). Implementation must be done in derived classes.
        virtual bool                                contains( const Eigen::Vector3f& scale, const Eigen::Vector3f& boxpos, const Eigen::Vector3f& pos ) const = 0;

    protected:

        //! The object can be destroyed only by the smart pointer
        virtual                                     ~BaseBoundingVolume();
};

//! Typedef for the smart pointer
typedef SmartPtr< BaseBoundingVolume >  BoundingVolumePtr;


//! Box volume
class BBox : public BaseBoundingVolume
{
    public:

        //! Construct a bounding volume of box type
                                                    BBox();

        //! Set the box dimensions
        void                                        setDimensions( const Eigen::Vector3f& min, const Eigen::Vector3f& max );

        //! Containment test
        bool                                        contains( const Eigen::Vector3f& scale, const Eigen::Vector3f& boxpos, const Eigen::Vector3f& pos ) const;

    protected:

        //! The object can be destroyed only by the smart pointer
        virtual                                     ~BBox();

        //! Min point of bbox dimensions
        Eigen::Vector3f                             _min;

        //! Max point of bbox dimensions
        Eigen::Vector3f                             _max;
};

} // namespace beditor

#endif // _BOUNDINGVOLUME_H_
