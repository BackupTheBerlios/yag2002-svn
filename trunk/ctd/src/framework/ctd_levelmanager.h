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
 # level manager is responsible for loading level configuration 
 #
 #   date of creation:  02/17/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_LEVELMANAGER_H_
#define _CTD_LEVELMANAGER_H_

#include "ctd_singleton.h"

namespace CTD
{

class Application;
class BaseEntity;

//! Level manager used only by class Application
class LevelManager : public Singleton< LevelManager >
{
    public:

        //! Load a level given its file name. Returns top most node in scenegraph.
        /*!
        * The 'keep' flags care about destroying entities and physics world of an already loaded level.
        */
        osg::ref_ptr< osg::Group >                  load( const std::string& levelName, bool keepPhysicsWorld = false, bool keepEntities = false );

        //! Load mesh. Return NULL if somethings goes wrong.
        osg::Node*                                  loadMesh( const std::string& fileName, bool useCache = true );

        //! Get root node of loaded static world geometry in the case you want to do some crazy things such as post-processings ;-)
        osg::Node*                                  getStaticMesh();

    protected:


                                                    LevelManager();

                                                    LevelManager( const LevelManager& );

        virtual                                     ~LevelManager();

        LevelManager&                               operator = ( const LevelManager& );

        //! Shutdown level manager
        void                                        shutdown();

        //! Load static world geometry. Return NULL if somethings goes wrong.
        osg::Node*                                  loadStaticWorld( const std::string& fileName );

        //! This method initializes other cores such as sound, gui, etc. (it is called during first level loading)
        void                                        initializeFirstTime();

        //! Initialize and Post-Initialize given entities
        void                                        setupEntities( std::vector< BaseEntity* >& entities );

        //! Build the physics's static collision geometry
        void                                        buildPhysicsStaticGeometry();

        //! Static level mesh
        osg::ref_ptr< osg::Node >                   _staticMesh;

        //! The node group where all nodes reside
        osg::ref_ptr< osg::Group >                  _nodeGroup;

        //! Entity group for all those entities with transformation node
        osg::ref_ptr< osg::Group >                  _entityGroup;

        //! The top group containing entity and node groups
        osg::ref_ptr< osg::Group >                  _topGroup;

        //! Cache for loaded meshes ( filename / node pairs )
        std::map< std::string, osg::ref_ptr< osg::Node > > _meshCache;

        //! This flag shows whether we are loading a level for first time
        bool                                        _firstLoading;

    friend class Singleton< LevelManager >;
    friend class Application;
};

inline osg::Node* LevelManager::getStaticMesh()
{
    return _staticMesh.get();
}

}

#endif // _CTD_LEVELMANAGER_H_
