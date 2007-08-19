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

        //! Base map file. This map is spanned over the entire terrain.
        std::string                                 _fileBasemap;

        //! Detail map 0 file. This map is repeated over the terrain patches.
        std::string                                 _fileDetailmap0;

        //! Repeat factor in X/Y direction for detailmap 0
        osg::Vec2f                                  _detailmap0Repeat;

        //! Detail map 1 file. This map is repeated over the terrain patches.
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

//! Class declarations
class TerrainPatch;
class TerrainSection;

/*! Terrain manager */
class TerrainManager : public Singleton< TerrainManager >
{
    public:

        //! Setup a terrain section and return its ID. The IDs begin with 1 and are incremented for every new section.
        unsigned int                                addSection( TerrainConfig& config ) throw ( TerrainException );

        //! Given a section ID return its scenegraph node.
        osg::ref_ptr< osg::Group >                  getSectionNode( unsigned int id ) throw ( TerrainException );

        //! Release a terrain section given its ID.
        void                                        releaseSection( unsigned int id ) throw ( TerrainException );

    protected:


                                                    TerrainManager();

        virtual                                     ~TerrainManager();

        //! Shutdown level manager
        void                                        shutdown();

        //! Setup the shaders
        osg::ref_ptr<osg::StateSet >                setupShaders();

        //! Build a quad tree for the given list of patches
        osg::ref_ptr< osg::Group >                  buildQuadTree( std::vector< TerrainPatch* >& patches );

        //! Recursively split the node into a quad tree
        void                                        split( unsigned int maxdepth, unsigned int depth, osg::Group* p_node, std::vector< TerrainPatch* >& patches, unsigned int& tileX, unsigned int& tileY );

        //! Count of sections
        unsigned int                                _sections;

        //! Count of tiles in a row
        unsigned int                                _tilesX;
        
        //! Count of tiles in a column
        unsigned int                                _tilesY;

        //! The depth of quad tree
        unsigned int                                _quadTreeDepth;

        //! Internal map of < terrain ID / section object >
        std::map< unsigned int, TerrainSection* >   _sectionMap;

    friend class Singleton< TerrainManager >;
    friend class Application;
};

} // namespace yaf3d

#endif // _TERRAINMANAGER_H_
