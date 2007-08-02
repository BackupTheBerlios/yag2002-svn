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
 # terrain patch tesselator
 #
 #   date of creation:  07/30/2007
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _TERRAINTESSELATOR_H_
#define _TERRAINTESSELATOR_H_

#include "base.h"


namespace yaf3d
{

class TerrainPatch;

/*! Terrain patch tesselator */
class BasePatchTesselator
{
    public:

        //! Create patch tesselator
                                                    BasePatchTesselator();

        virtual                                     ~BasePatchTesselator();

        //! Tesselate the patch for given level and morph factor [0..1]
        virtual void                                tesselate( unsigned int level, float morph ) = 0;

        //! Set the four neighboring patches ( 0: up, 1: right, 2: down, 3: left )
        void                                        setNeighbors( TerrainPatch* p_neighbors ) { _p_neighbors = p_neighbors; }

        //! Get the four neighbors ( 0: up, 1: right, 2: down, 3: left )
        TerrainPatch*                               getNeighbors() { return _p_neighbors; }

    protected:

        TerrainPatch*                               _p_neighbors;
};

/*! Terrain patch tesselator using screen space error metrics */
class TesselatorScreenSpace: public BasePatchTesselator
{
    public:


        //! Create a patch tesselator using the screen space as metric
                                                    TesselatorScreenSpace();

        virtual                                     ~TesselatorScreenSpace();

        //! Tesselate the patch for given level and morph factor [0..1]
        void                                        tesselate( unsigned int level, float morph );
};

/*! Terrain patch tesselator using world space error metrics */
class TesselatorWorldSpace: public BasePatchTesselator
{
    public:


        //! Create a patch tesselator using the screen space as metric
                                                    TesselatorWorldSpace();

        virtual                                     ~TesselatorWorldSpace();

        //! Tesselate the patch for given level and morph factor [0..1]
        void                                        tesselate( unsigned int level, float morph );
};

} // namespace yaf3d

#endif // _TERRAINTESSELATOR_H_
