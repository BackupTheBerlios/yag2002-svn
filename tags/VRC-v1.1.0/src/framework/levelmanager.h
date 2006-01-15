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
 # level manager is responsible for loading level configuration 
 #  and setting up the subsystems and entities
 #
 #   date of creation:  02/17/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _LEVELMANAGER_H_
#define _LEVELMANAGER_H_

#include "singleton.h"


//! Location of level file directory for Server, Client, and Standalone level
#define YAF3D_LEVEL_SERVER_DIR    "level/server/"
#define YAF3D_LEVEL_CLIENT_DIR    "level/client/"
#define YAF3D_LEVEL_SALONE_DIR    "level/standalone/"

namespace yaf3d
{

class Application;
class BaseEntity;

//! Level manager
/*!
* In order to load a level proceed as follows:
* --------------------------------------------
*
* // decide if you want to keep exsiting physics setup and entities
* LevelManager::get()->unloadLevel( true, true );
*
* // load the level given its file name relative to media folder
* LevelManager::get()->loadLevel( levelfile );
*
* // now optionally load other entity configuration files ( e.g for the player )
* std::vector< BaseEntity* > entities;
* LevelManager::get()->loadEntities( playercfgfile, entities );
*
* // complete level loading, it will properly setup physics and loaded entities
* LevelManager::get()->finalizeLoading();
*/
class LevelManager : public Singleton< LevelManager >
{
    public:

        //! Load a level given its file name. Returns top most node in scenegraph.
        osg::ref_ptr< osg::Group >                  loadLevel( const std::string& levelName );

        //! Load entities defined in specified file and put all created entities into given vector.
        /*!
        * Be aware that this method does not initialize the entities. instPostfix is appended to entities instance names for the case that the same file is loaded 
        * several times, so the instance names can differ using a postfix on every loading. If not NULL the new loaded entities are stored in p_entities.
        */
        bool                                        loadEntities( const std::string& levelFile, std::vector< BaseEntity* >* p_entities = NULL, const std::string& instPostfix = "" );

        //! After usage of loadLevel and loadEntities this method must be called to finalize the loading process
        void                                        finalizeLoading();

        //! Unload an already loaded level and free up allocated resources and entities (except persistent entities).
        bool                                        unloadLevel( bool clearPhysics = true, bool clearEntities = true );

        //! Load mesh. Return NULL if something goes wrong.
        osg::Node*                                  loadMesh( const std::string& fileName, bool useCache = true );

        //! Get node of loaded static world geometry.
        inline osg::Node*                           getStaticMesh();

        //! Replace the current static world node and return the old one.
        inline osg::Node*                           setStaticMesh( osg::Node* p_newnode );

    protected:


                                                    LevelManager();


        virtual                                     ~LevelManager();

        //! Shutdown level manager
        void                                        shutdown();

        //! Load static world geometry. Return NULL if somethings goes wrong.
        osg::Node*                                  loadStaticWorld( const std::string& fileName );

        //! This method initializes other cores such as sound, gui, etc. (it is called during first level loading)
        void                                        initializeFirstTime();

        //! Build the physics's static collision geometry
        void                                        buildPhysicsStaticGeometry( const std::string& levelFile );

        //! Static level mesh
        osg::Node*                                  _staticMesh;

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

        //! Internal flag showing that a level file contains a map entry
        bool                                        _levelHasMap;

        //! A queue for all entities which have been loaded before calling filanlizeLoading
        std::vector< BaseEntity* >                  _setupQueue;

        //! Level name which is loaded, this is used for physics serialization
        std::string                                 _levelFile;

    friend class Singleton< LevelManager >;
    friend class Application;
};

inline osg::Node* LevelManager::getStaticMesh()
{
    return _staticMesh;
}

inline osg::Node* LevelManager::setStaticMesh( osg::Node* p_newnode )
{
    osg::Node* p_oldnode = _staticMesh;

    // catch null nodes
    if ( !p_newnode )
    {
        if ( p_oldnode )
            _nodeGroup->removeChild( p_oldnode );

        _staticMesh = NULL;
        return NULL;
    }

    // catch trying to set the same node again
    if ( p_newnode == _staticMesh )
        return p_newnode;

    if ( p_oldnode )
        _nodeGroup->removeChild( p_oldnode );

    _staticMesh = p_newnode;
    _nodeGroup->addChild( p_newnode );
    return p_oldnode;
}

}

#endif // _LEVELMANAGER_H_
