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

#include "base.h"
#include "log.h"
#include "terrainpatch.h"
#include "terraintesselator.h"


namespace yaf3d
{

BasePatchTesselator::BasePatchTesselator()
{
}

BasePatchTesselator::~BasePatchTesselator()
{
}

// screen space tesselator
TesselatorScreenSpace::TesselatorScreenSpace()
{
}

TesselatorScreenSpace::~TesselatorScreenSpace()
{
}

void TesselatorScreenSpace::tesselate( unsigned int level, float morph )
{
    //! TODO
}


// world space tesselator
TesselatorWorldSpace::TesselatorWorldSpace()
{
}

TesselatorWorldSpace::~TesselatorWorldSpace()
{
}

void TesselatorWorldSpace::tesselate( unsigned int level, float morph )
{
    //! TODO
}

} // namespace yaf3d
