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
 # terrain manager
 #
 #   date of creation:  07/30/2007
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _TERRAINMANAGER_H_
#define _TERRAINMANAGER_H_

#include "base.h"
#include "singleton.h"


namespace yaf3d
{

/*! Terrain configuration class */
class TerrainConfig
{
    public:

        //! Scale the terrain
        osg::Vec3f                                  _scale;

        //!Height map file (grey scale)
        std::string                                 _fileHeightmap;

        //! Base map file
        std::string                                 _fileBasemap;

        //! Repeat factor in X/Y direction for basemap
        osg::Vec2f                                  _basemapRepeat;

        //! Detail map 0 file
        std::string                                 _fileDetailmap0;

        //! Repeat factor in X/Y direction for detailmap 0
        osg::Vec2f                                  _detailmap0Repeat;

        //! Detail map 1 file
        std::string                                 _fileDetailmap1;

        //! Repeat factor in X/Y direction for detailmap 1
        osg::Vec2f                                  _detailmap1Repeat;
};

//! Class for terrain related exceptions
class TerrainException : public std::runtime_error
{
    public:
                                                    TerrainException( const std::string& reason ) :
                                                     std::runtime_error( reason )
                                                    {
                                                    }

        virtual                                     ~TerrainException() throw() {}

                                                    TerrainException( const TerrainException& e ) :
                                                     std::runtime_error( e )
                                                    {
                                                    }

    protected:

                                                    TerrainException();

        TerrainException&                           operator = ( const TerrainException& );
};

/*! Terrain manager */
class TerrainManager : public Singleton< TerrainManager >
{
    public:

        //! Setup the terrain and return a group containing the entire terrain
        osg::ref_ptr< osg::Group >                  setup( TerrainConfig& config ) throw ( TerrainException );

        //! Update the patch tesselation and visibility
        void                                        update( osg::CameraNode* p_cam );

        //! Render the patch
        void                                        render();

    protected:


                                                    TerrainManager();

        virtual                                     ~TerrainManager();

        //! Shutdown level manager
        void                                        shutdown();

    friend class Singleton< TerrainManager >;
    friend class Application;
};

} // namespace yaf3d

#endif // _TERRAINMANAGER_H_
