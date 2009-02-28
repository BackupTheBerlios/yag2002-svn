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
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include "base.h"
#include "log.h"
#include "physics.h"
#include "levelmanager.h"
#include "application.h"
#include "attributemanager.h"
#include "entitymanager.h"
#include "soundmanager.h"
#include "guimanager.h"
#include "gamestate.h"
#include "filesystem.h"
#include "yaf3dtinyxml/tinyxml.h"
#include <osgUtil/GLObjectsVisitor>

// XML item names
//---------------------------------------------------------//
#define YAF3D_LVL_ELEM_LEVEL                  "Level"
#define YAF3D_LVL_ELEM_NAME                   "Name"
#define YAF3D_LVL_ELEM_MAP                    "Map"

#define YAF3D_LVL_ELEM_ENTITY                 "Entity"
#define YAF3D_LVL_ENTITY_TYPE                 "Type"
#define YAF3D_LVL_ENTITY_INST_NAME            "InstanceName"
#define YAF3D_LVL_ENTITY_PARAM                "Parameter"
#define YAF3D_LVL_ENTITY_PARAM_NAME           "Name"
#define YAF3D_LVL_ENTITY_PARAM_TYPE           "Type"
#define YAF3D_LVL_ENTITY_PARAM_VALUE          "Value"


#define YAF3D_TOP_GROUP_NAME                  "_topGrp_"
#define YAF3D_ENTITY_GROUP_NAME               "_entityGrp_"
#define YAF3D_NODE_GROUP_NAME                 "_nodeGrp_"

//! The file type extension for osg's plugin reading from physfs (see filesystem.cpp)
#define VRC_OSGVFS_EXT                        ".physfs"

namespace yaf3d
{

//! Implement the level manager singleton
YAF3D_SINGLETON_IMPL( LevelManager )

LevelManager::LevelManager() :
_staticMesh( NULL ),
_firstLoading( true ),
_levelHasMap( false )
{
    _topGroup        = new osg::Group();
    _staticMeshGroup = new osg::Group();
    _entityGroup     = new osg::Group();

    // set name of top group
    _topGroup->setName( YAF3D_TOP_GROUP_NAME );

    // set name of node group
    _staticMeshGroup->setName( YAF3D_NODE_GROUP_NAME );

    // all entities with transformation node are placed in this entity group
    _entityGroup->setName( YAF3D_ENTITY_GROUP_NAME );

    _topGroup->addChild( _staticMeshGroup.get() );
    _topGroup->addChild( _entityGroup.get() );
}

LevelManager::~LevelManager()
{
}

bool LevelManager::unloadLevel( bool clearPhysics, bool clearEntities )
{
    if ( _firstLoading )
        return false;

    _levelFile = "";

    // send out unload level notification
    EntityNotification ennotify( YAF3D_NOTIFY_UNLOAD_LEVEL );
    EntityManager::get()->sendNotification( ennotify );
    EntityManager::get()->flushNotificationQueue();

    // first send out notification so the entities can do appropriate actions before level unloading happens
    if ( clearEntities )
    {
        // first send out a notification about deleting entities
        EntityNotification ennotify( YAF3D_NOTIFY_DELETING_ENTITIES );
        EntityManager::get()->sendNotification( ennotify );
    }
    if ( clearPhysics )
    {
        EntityNotification ennotify( YAF3D_NOTIFY_DELETING_PHYSICSWORLD );
        EntityManager::get()->sendNotification( ennotify );
    }
    // flush the notification queue so perior and new entities get the notification
    EntityManager::get()->flushNotificationQueue();

    if ( clearEntities )
    {
        // now delete all non-persistent entities
        EntityManager::get()->deleteAllEntities();
        // (re-)create entiy transform node group
        _topGroup->removeChild( _entityGroup.get() );
        _entityGroup = new osg::Group();
        _entityGroup->setName( YAF3D_ENTITY_GROUP_NAME );
        _topGroup->addChild( _entityGroup.get() );

        // save the persistent entities which need transformation. they must be copied into new created entity group.
        EntityNotification ennotify( YAF3D_NOTIFY_ENTITY_TRANSNODE_CHANGED );
        std::vector< BaseEntity* > perentities;
        EntityManager::get()->getPersistentEntities( perentities );
        std::vector< BaseEntity* >::iterator p_entity = perentities.begin(), p_entityEnd = perentities.end();
        for ( ; p_entity != p_entityEnd; ++p_entity )
        {
            if ( ( *p_entity )->isTransformable() )
            {
                _entityGroup->addChild( ( *p_entity )->getTransformationNode() );
                EntityManager::get()->sendNotification( ennotify, *p_entity );
            }
        }
    }

    if ( clearPhysics )
    {
        // (re-)create static geom node group
        _topGroup->removeChild( _staticMeshGroup.get() );
        _staticMeshGroup = new osg::Group();
        _staticMeshGroup->setName( YAF3D_NODE_GROUP_NAME );
        _topGroup->addChild( _staticMeshGroup.get() );

        bool reinitphysics = Physics::get()->reinitialize();
        assert( reinitphysics && "could not re-init physics" );

        _staticMesh = NULL;
    }

    // clear the mesh cache now
    _meshCache.clear();

    return true;
}

osg::ref_ptr< osg::Group > LevelManager::loadLevel( const std::string& levelFile )
{
    log_info << "LevelManager: start loading level: " << levelFile << std::endl;

    // store the level file name, we will use it for physics serialization
    _levelFile = levelFile;

    // clear the entity list used for loading process
    _setupQueue.clear();

    // send loading notification
    {
        EntityNotification ennotify( YAF3D_NOTIFY_LOADING_LEVEL );
        EntityManager::get()->sendNotification( ennotify );
        // flush the notification queue so perior and new entities get the notification
        EntityManager::get()->flushNotificationQueue();
    }

    std::vector< BaseEntity* > entities; // list of all entities which are created during loading
    if ( !loadEntities( levelFile, &entities ) )
    {
        log_error << "LevelManager: error loading entities" << std::endl;
        return NULL;
    }

    return _topGroup;
}

bool LevelManager::loadEntities( const std::string& levelFile, std::vector< BaseEntity* >* p_entities, const std::string& instPostfix )
{
    log_info << "LevelManager: loading entities ..." << std::endl;

    // use tiny xml to parser lvl file
    yaf3dTinyXml::TiXmlDocument doc;
    doc.SetCondenseWhiteSpace( false );

    FilePtr file = FileSystem::get()->getFile( levelFile );
    if ( !file.valid() )
    {
        log_error << "LevelManager: file '" << levelFile << "' does not exit" << std::endl;
        return false;
    }

    char* p_filebuf = file->getBuffer();
    if ( !doc.Parse( p_filebuf ) )
    {
        log_error << "LevelManager: cannot load level file: '" << levelFile << "'" << std::endl;
        log_error << "              reason: " << doc.ErrorDesc() << std::endl;
        return false;
    }

    // start evaluating the xml structure
    //---------------------------------//
    yaf3dTinyXml::TiXmlNode*      p_node          = NULL;
    yaf3dTinyXml::TiXmlElement*   p_levelElement  = NULL;
    yaf3dTinyXml::TiXmlElement*   p_mapElement    = NULL;
    char*                         p_bufName       = NULL;

    // get the level entry if one exists
    p_node = doc.FirstChild( YAF3D_LVL_ELEM_LEVEL );
    std::string staticNodeName;
    if ( p_node )
    {
        // get level name
        p_levelElement = p_node->ToElement();
        if ( !p_levelElement )
        {
            log_error << "LevelManager: could not find level element" << std::endl;
            return false;
        }

        p_bufName = ( char* )p_levelElement->Attribute( YAF3D_LVL_ELEM_NAME );
        if ( p_bufName )
            staticNodeName = p_bufName;
        else
            return false;
    }
    else
    {
        log_error << "LevelManager: empty level file" << std::endl;
        return false;
    }

    // get map files, load them and add them to main group
    _levelHasMap = false; // reset the map flag
    p_node = p_levelElement->FirstChild( YAF3D_LVL_ELEM_MAP );
    do
    {
        // a level file can also be without a map
        if ( !p_node )
            break;

        _levelHasMap = true;
        p_mapElement = p_node->ToElement();
        p_bufName    = ( char* )p_mapElement->Attribute( YAF3D_LVL_ELEM_NAME );
        if ( p_bufName == NULL )
        {
            log_error << "LevelManager: missing map name in MAP entry" << std::endl;
            return false;
        }
        else
        {
            log_info << "LevelManager: loading static geometry: " << p_bufName << std::endl;
            osg::Node *p_staticnode = loadStaticWorld( p_bufName );
            if ( p_staticnode )
            {
                p_staticnode->setName( p_bufName );
                _staticMeshGroup->addChild( p_staticnode );
                _staticMesh = p_staticnode;
                _staticMesh->setName( staticNodeName );
            }
            else
                return false;
        }

        p_node = p_node->NextSiblingElement( YAF3D_LVL_ELEM_MAP );

    } while ( p_node );


    // read entity definitions
    //------------------------
    log_info << "LevelManager: creating entities ..." << std::endl;

    unsigned int entityCounter = 0;
    p_node = p_levelElement;
    for ( p_node = p_levelElement->FirstChild( YAF3D_LVL_ELEM_ENTITY ); p_node; p_node = p_node->NextSiblingElement( YAF3D_LVL_ELEM_ENTITY ) )
    {
        yaf3dTinyXml::TiXmlElement* p_entityElement = p_node->ToElement();
        std::string entitytype, instancename;
        // get entity name
        p_bufName = ( char* )p_entityElement->Attribute( YAF3D_LVL_ENTITY_TYPE );
        std::string enttype;
        if ( !p_bufName )
        {
            log_debug << "LevelManager: entity has no type, skipping" << std::endl;
            continue;
        }
        else
            enttype = p_bufName;

        entitytype = p_bufName;
        p_bufName = ( char* )p_entityElement->Attribute( YAF3D_LVL_ENTITY_INST_NAME );
        if ( p_bufName )
            instancename = p_bufName;

        instancename += instPostfix;

        // create entity, considering the game mode and entity's creation policy
        BaseEntityFactory* p_entfac = EntityManager::get()->getEntityFactory( entitytype );
        if ( !p_entfac )
        {
            log_error << "LevelManager: unknown entity type '" << entitytype << "', skipping" << std::endl;
            continue;
        }
        unsigned int creationpolicy = p_entfac->getCreationPolicy();
        bool         create         = false;
        switch ( GameState::get()->getMode() )
        {
            case GameState::Standalone:
                if ( creationpolicy & BaseEntityFactory::Standalone )
                    create = true;
                break;

            case GameState::Server:
                if ( creationpolicy & BaseEntityFactory::Server )
                    create = true;
                break;

            case GameState::Client:
                if ( creationpolicy & BaseEntityFactory::Client )
                    create = true;
                break;

            default:
                assert( NULL && "unsupported game mode" );
        }
        if ( !create )
            continue;

        BaseEntity* p_entity = EntityManager::get()->createEntity( entitytype, instancename );
        // could we find entity type
        if ( !p_entity )
        {
            log_error << "LevelManager: could not find entity type '" << entitytype << "', skipping entity!" << std::endl;
            continue;
        }
        // add to given list if it was desired
        if ( p_entities )
            p_entities->push_back( p_entity );

        log_debug << "LevelManager: entity created, type: '" << enttype << "'" << std::endl;
        // get instance name if one provided
        p_bufName = ( char* )p_entityElement->Attribute( YAF3D_LVL_ENTITY_INST_NAME );
        if ( p_bufName ) {
            // set entity's instance name
            p_entity->setInstanceName( instancename );
            log_debug << "LevelManager: instance name: '" << instancename << "'" << std::endl;
        }

        ++entityCounter;

        yaf3dTinyXml::TiXmlElement* p_entityParam = NULL;
        yaf3dTinyXml::TiXmlNode*    p_entityNode  = NULL;
        for ( p_entityNode = p_entityElement->FirstChild( YAF3D_LVL_ENTITY_PARAM ); p_entityNode; p_entityNode = p_entityParam->NextSiblingElement( YAF3D_LVL_ENTITY_PARAM ) )
        {
            p_entityParam = p_entityNode->ToElement();
            char *p_paramName, *p_paramType, *p_paramValue;
            p_paramName  = ( char* )p_entityParam->Attribute( YAF3D_LVL_ENTITY_PARAM_NAME  );
            p_paramType  = ( char* )p_entityParam->Attribute( YAF3D_LVL_ENTITY_PARAM_TYPE  );
            p_paramValue = ( char* )p_entityParam->Attribute( YAF3D_LVL_ENTITY_PARAM_VALUE );

            if ( !p_paramName || !p_paramType || !p_paramValue )
            {
                log_error << "LevelManager: incomplete entity parameter entry, skipping" << std::endl;
                continue;
            }

            AttributeManager& attrMgr = p_entity->getAttributeManager();
            if ( !attrMgr.setAttributeValue( p_paramName, p_paramType, p_paramValue ) )
            {
                log_error << "LevelManager: cannot find entity parameter '" << p_paramName << "'" << std::endl;
            }
        }

        // append the transformation node if the entity has one
        if ( p_entity->isTransformable() )
        {
            if ( instancename.length() )
                p_entity->getTransformationNode()->setName( instancename );
            _entityGroup->addChild( p_entity->getTransformationNode() );
        }

        // enqueue entitiy for later setup in finalizeLoading
        _setupQueue.push_back( p_entity );
    }

    log_info << "LevelManager: total number of created entities: '" << entityCounter << "'" << std::endl;
    log_info << "LevelManager: entity loading completed" << std::endl;

    return true;
}

void LevelManager::finalizeLoading()
{
    // are we loading for first time?
    if ( _firstLoading )
    {
        bool initphysics = Physics::get()->initialize();
        assert( initphysics && "could not init physics" );

        if ( _levelHasMap )
            buildPhysicsStaticGeometry( _levelFile );

        initializeFirstTime();
    }
    else
    {
        // build the physics static geometry on every finalizing
        buildPhysicsStaticGeometry( _levelFile );
    }

    // init and post-init entities which have been created before
    EntityManager::get()->setupEntities( _setupQueue );
    _setupQueue.clear();

    // send the notification that the a new level has been loaded and initialized
    {
        EntityNotification ennotify( YAF3D_NOTIFY_NEW_LEVEL_INITIALIZED );
        EntityManager::get()->sendNotification( ennotify );
        // flush the notification queue so perior and new entities get the notification
        EntityManager::get()->flushNotificationQueue();
    }

    // mark that we have done the first level loading
    _firstLoading = false;
}

void LevelManager::initializeFirstTime()
{
    if ( GameState::get()->getMode() != GameState::Server )
    {
        // initialize sound manager
        log_info << "LevelManager: initializing sound system..." << std::endl;
        try
        {
            SoundManager::get()->initialize();
        }
        catch( const SoundException& e )
        {
            log_error << "LevelManager: error initializing sound system. reason: '" << e.what() << "'" << std::endl;
        }

        log_info << "LevelManager: initializing gui system..." << std::endl;
        // initialize the gui system
        GuiManager::get()->initialize();

        // for first time we realize the viewer
        log_info << "LevelManager: starting viewer ..." << std::endl;
        // start viewer
        Application::get()->getViewer()->open();
        Application::get()->getViewer()->init();
        // call draw in order to clear the screen during loading
        Application::get()->getViewer()->draw();
        // set game state
        GameState::get()->setState( GameState::GraphicsInitialized );
    }
}

void LevelManager::buildPhysicsStaticGeometry( const std::string& levelFile )
{
    // continue setting up physics
    //----------------------------
    log_info << "LevelManager: building pyhsics collision geometries ..." << std::endl;

    // send the notification about building static world, here physics related stuff such as
    //  definition of own physics materials can take place
    {
        EntityNotification ennotify( YAF3D_NOTIFY_BUILDING_PHYSICSWORLD );
        EntityManager::get()->sendNotification( ennotify );
        // flush the notification queue so perior and new entities get the notification
        EntityManager::get()->flushNotificationQueue();
    }

    osg::Timer      timer;
    osg::Timer_t    curTick         = 0;
    osg::Timer_t    startTick       = 0;
    float           time4Init       = 0;

    startTick  = timer.tick();
    // build static geoms for physics collision nodes
    bool buildphysics = Physics::get()->buildStaticGeometry( _staticMeshGroup.get(), levelFile );
    assert( buildphysics && "could not build physics" );

    curTick    = timer.tick();
    time4Init  = timer.delta_s( startTick, curTick );
    log_info << "LevelManager: needed time for building pyhsics geometries: " << time4Init << " seconds" << std::endl;
}

osg::Node* LevelManager::loadStaticWorld( const std::string& fileName )
{
    // start timer
    osg::Timer_t start_tick = osg::Timer::instance()->tick();

    // load given file
    osg::Node* p_loadedModel = loadMesh( fileName, true );
    // if no model has been successfully loaded report failure.
    if ( !p_loadedModel )
    {
        return NULL;
    }
    // stop timer and give out the time messure
    osg::Timer_t end_tick = osg::Timer::instance()->tick();
    log_debug << "LevelManager: LevelManager: elapsed time for loading '" << fileName << "' : " << osg::Timer::instance()->delta_s( start_tick, end_tick ) << std::endl;
    return p_loadedModel;
}

osg::Node* LevelManager::loadMesh( const std::string& fileName, bool useCache )
{
    // first lookup in cache
    if ( useCache )
    {
        std::map< std::string, osg::ref_ptr< osg::Node > >::iterator p_mesh = _meshCache.find( fileName );
        if ( p_mesh != _meshCache.end() )
        {
            log_verbose << "LevelManager: using cached mesh '" << fileName << "'" << std::endl;
            return p_mesh->second.get();
        }
    }

    // read given file from virtual file system loader
    osg::Node* p_loadedModel = osgDB::readNodeFile( fileName + VRC_OSGVFS_EXT );

    // if no model has been successfully loaded report failure.
    if ( !p_loadedModel )
    {
        log_error << "LevelManager: could not load mesh: " << fileName << std::endl;
        return NULL;
    }

    // optimize the scene graph, remove rendundent nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize( p_loadedModel );

    // compile all gl textures in loaded level
    // this avoids delays when textures are loaded on-the-fly by osg only when they are visible for first time
    // check if glsl is supported before setting up the shaders ( gl context 0  is assumed )
    if ( GameState::get()->getMode() != GameState::Server )
    {
        log_debug << "LevelManager: compiling openGL resources ..." << std::endl;
        osgUtil::GLObjectsVisitor compiler;
        osg::RenderInfo renderinfo;
        renderinfo.setState( Application::get()->getSceneView()->getState() );
        compiler.setRenderInfo( renderinfo );
        p_loadedModel->accept( compiler );
    }

    // cache the loaded mesh
    if ( useCache )
        _meshCache.insert( make_pair( fileName, p_loadedModel ) );

    return p_loadedModel;
}

void LevelManager::shutdown()
{
    // clean up entity manager
    EntityManager::get()->shutdown();

    if ( GameState::get()->getMode() != GameState::Server )
    {
        // shutdown all other libs managed by level loader
        SoundManager::get()->shutdown();
        GuiManager::get()->shutdown();
    }

    // set game state
    GameState::get()->setState( GameState::Shutdown );

    // unload level
    unloadLevel( true, true );

    Physics::get()->shutdown();
    // destroy singleton
    destroy();
}

} // namespace yaf3d
