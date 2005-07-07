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

#include <ctd_base.h>
#include <ctd_log.h>
#include <ctd_physics.h>
#include <ctd_levelmanager.h>
#include <ctd_application.h>
#include <ctd_attributemanager.h>
#include <ctd_entitymanager.h>
#include <ctd_guimanager.h>
#include <ctd_gamestate.h>

#define TIXML_USE_STL
#include <tinyxml.h>

using namespace std;
using namespace CTD;

// xml item names
//-------------------------------------//
#define CTD_LVL_ELEM_LEVEL                  "Level"
#define CTD_LVL_ELEM_NAME                   "Name"
#define CTD_LVL_ELEM_MAP                    "Map"

#define CTD_LVL_ELEM_ENTITY                 "Entity"
#define CTD_LVL_ENTITY_TYPE                 "Type"
#define CTD_LVL_ENTITY_INST_NAME            "InstanceName"
#define CTD_LVL_ENTITY_PARAM                "Parameter"
#define CTD_LVL_ENTITY_PARAM_NAME           "Name"
#define CTD_LVL_ENTITY_PARAM_TYPE           "Type"
#define CTD_LVL_ENTITY_PARAM_VALUE          "Value"


#define CTD_TOP_GROUP_NAME                  "_topGrp_"
#define CTD_ENTITY_GROUP_NAME               "_entityGrp_"
#define CTD_NODE_GROUP_NAME                 "_nodeGrp_"

CTD_SINGLETON_IMPL( LevelManager );

LevelManager::LevelManager() :
_firstLoading( true ),
_levelHasMap( false ),
_staticMesh( NULL )
{
    _topGroup    = new osg::Group();
    _nodeGroup   = new osg::Group();
    _entityGroup = new osg::Group();

    // set name of top group
    _topGroup->setName( CTD_TOP_GROUP_NAME );

    // set name of node group
    _nodeGroup->setName( CTD_NODE_GROUP_NAME );

    // all entities with transformation node are placed in this entity group
    _entityGroup->setName( CTD_ENTITY_GROUP_NAME );

    _topGroup->addChild( _nodeGroup.get() );
    _topGroup->addChild( _entityGroup.get() );
}

LevelManager::~LevelManager()
{
}

bool LevelManager::unloadLevel( bool clearPhysics, bool clearEntities )
{
    if ( _firstLoading )
        return false;

    // first send out notification so the entities can do appropriate actions before level unloading happens
    if ( clearEntities )
    {
        // first send out a notification about deleting entities
        EntityNotification ennotify( CTD_NOTIFY_DELETING_ENTITIES );
        EntityManager::get()->sendNotification( ennotify );
    }
    if ( clearPhysics )
    {
        EntityNotification ennotify( CTD_NOTIFY_DELETING_PHYSICSWORLD );
        EntityManager::get()->sendNotification( ennotify );
    }
    // flush the notification queue so perior and new entities get the notification
    EntityManager::get()->flushNotificationQueue();

    if ( clearEntities )
    {
        // now delete entities (note: no-autodelete entities will remain -- see BaseEntity::getAutoDelete() )
        EntityManager::get()->deleteAllEntities();
        // (re-)create entiy transform node group
        _topGroup->removeChild( _entityGroup.get() );
        _entityGroup = new osg::Group();
        _entityGroup->setName( CTD_ENTITY_GROUP_NAME );
        _topGroup->addChild( _entityGroup.get() );

        // save the persistent entities which need transformation. they must be copied into new created entity group.
        std::vector< BaseEntity* > perentities;
        EntityManager::get()->getPersistentEntities( perentities );
        vector< BaseEntity* >::iterator pp_entity = perentities.begin(), pp_entityEnd = perentities.end();
        for( ; pp_entity != pp_entityEnd; pp_entity++ )
        {
            if ( ( *pp_entity )->isTransformable() )
            {
                _entityGroup->addChild( ( *pp_entity )->getTransformationNode() );
            }
        }
    }

    if ( clearPhysics )
    {
        // (re-)create static geom node group
        _topGroup->removeChild( _nodeGroup.get() );
        _nodeGroup = new osg::Group();
        _nodeGroup->setName( CTD_NODE_GROUP_NAME );
        _topGroup->addChild( _nodeGroup.get() );

        bool reinitphysics = Physics::get()->reinitialize();
        assert( reinitphysics && "could not re-init physics" );

        _staticMesh = NULL;
    }

    return true;
}

osg::ref_ptr< osg::Group > LevelManager::loadLevel( const string& levelFile )
{
    log << Log::LogLevel( Log::L_INFO ) << "start loading level: " << levelFile << endl;

    // send loading notification
    {
        EntityNotification ennotify( CTD_NOTIFY_LOADING_LEVEL );
        EntityManager::get()->sendNotification( ennotify );
        // flush the notification queue so perior and new entities get the notification
        EntityManager::get()->flushNotificationQueue();
    }

    std::vector< BaseEntity* > entities; // list of all entities which are created during loading
    if ( !loadEntities( levelFile, &entities ) )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "error loading entities" << endl;
        return NULL;
    }

    return _topGroup;
}

bool LevelManager::loadEntities( const string& levelFile, std::vector< BaseEntity* >* p_entities, const std::string& instPostfix )
{
    log << Log::LogLevel( Log::L_INFO ) << "loading entities ..." << endl;
    
    // use tiny xml to parser lvl file
    //  parse in the level configuration
    TiXmlDocument doc;
    doc.SetCondenseWhiteSpace( false );
    if ( !doc.LoadFile( string( Application::get()->getMediaPath() + levelFile ).c_str() ) )
    {
        log << Log::LogLevel( Log::L_DEBUG ) << "cannot load level file: '" << levelFile << "'" << endl;
        log << "  reason: " << doc.ErrorDesc() << endl;
        return false;
    }

    // start evaluating the xml structure
    //---------------------------------//
    TiXmlNode       *p_node                 = NULL;
    TiXmlElement    *p_levelElement         = NULL;
    TiXmlElement    *p_mapElement           = NULL;
    char            *p_bufName              = NULL;

    // get the level entry if one exists
    p_node = doc.FirstChild( CTD_LVL_ELEM_LEVEL );
    string staticNodeName;
    if ( p_node ) 
    {
    
        // get level name
        p_levelElement = p_node->ToElement();
        if ( !p_levelElement ) 
        {
            log << Log::LogLevel( Log::L_ERROR ) << "could not find level element" << endl;
            return false;
        }

        p_bufName = ( char* )p_levelElement->Attribute( CTD_LVL_ELEM_NAME );
        if ( p_bufName )
            staticNodeName = p_bufName;
        else 
            return false;  
    }
    else
    {
        log << Log::LogLevel( Log::L_ERROR ) << "empty level file" << endl;
        return false;
    }

    // get map files, load them and add them to main group
    _levelHasMap = false; // reset the map flag
    p_node = p_levelElement->FirstChild( CTD_LVL_ELEM_MAP );
    do 
    {
        // a level file can also be without a map
        if ( !p_node )
            break;

        _levelHasMap = true;
        p_mapElement = p_node->ToElement();
        p_bufName    = ( char* )p_mapElement->Attribute( CTD_LVL_ELEM_NAME );
        if ( p_bufName == NULL ) 
        {
            log << Log::LogLevel( Log::L_ERROR ) << "missing map name in MAP entry" << endl;
            return false;
        } 
        else 
        {
            log << Log::LogLevel( Log::L_INFO ) << "loading static geometry: " << p_bufName << endl;
            osg::Node *p_staticnode = loadStaticWorld( p_bufName );
            if ( p_staticnode )
            {
                p_staticnode->setName( p_bufName );
                _nodeGroup->addChild( p_staticnode );
                _staticMesh = p_staticnode;
                _staticMesh->setName( staticNodeName );
            }
            else
                return false;
        }

        p_node = p_node->NextSiblingElement( CTD_LVL_ELEM_MAP );

    } while ( p_node );


    // read entity definitions
    //------------------------
    log << Log::LogLevel( Log::L_INFO ) << "creating entities ..." << endl;

    unsigned int entityCounter = 0;
    p_node = p_levelElement;
    for ( p_node = p_levelElement->FirstChild( CTD_LVL_ELEM_ENTITY ); p_node; p_node = p_node->NextSiblingElement( CTD_LVL_ELEM_ENTITY ) ) 
    {
        TiXmlElement* p_entityElement = p_node->ToElement();
        string entitytype, instancename;
        // get entity name
        p_bufName = ( char* )p_entityElement->Attribute( CTD_LVL_ENTITY_TYPE );
        string enttype;
        if ( !p_bufName ) 
        {
            log << Log::LogLevel( Log::L_DEBUG ) << "entity has no type, skipping" << endl;
            continue;       
        }
        else
            enttype = p_bufName;

        entitytype = p_bufName;
        p_bufName = ( char* )p_entityElement->Attribute( CTD_LVL_ENTITY_INST_NAME );
        if ( p_bufName )
            instancename = p_bufName;

        instancename += instPostfix;

        // create entity, considering the game mode and entity's creation policy
        BaseEntityFactory* p_entfac = EntityManager::get()->getEntityFactory( entitytype );
        if ( !p_entfac )
        {
            log << Log::LogLevel( Log::L_ERROR ) << "unknown entity type '" << entitytype << "', skipping" << endl;
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
            log << Log::LogLevel( Log::L_ERROR ) << "could not find entity type '" << entitytype << "', skipping entity!" << endl;
            continue;
        }
        // add to given list if it was desired
        if ( p_entities )
            p_entities->push_back( p_entity );

        log << Log::LogLevel( Log::L_DEBUG ) << "entity created, type: '" << enttype << "'" << endl;
        // get instance name if one provided
        p_bufName = ( char* )p_entityElement->Attribute( CTD_LVL_ENTITY_INST_NAME );
        if ( p_bufName ) {
            // set entity's instance name
            p_entity->setInstanceName( instancename );
            log << Log::LogLevel( Log::L_DEBUG ) << "instance name: '" << instancename << "'" << endl;
        }

        entityCounter++;

        TiXmlElement* p_entityParam = NULL;
        TiXmlNode*    p_entityNode  = NULL;
        for ( p_entityNode = p_entityElement->FirstChild( CTD_LVL_ENTITY_PARAM ); p_entityNode; p_entityNode = p_entityParam->NextSiblingElement( CTD_LVL_ENTITY_PARAM ) )
        {
            p_entityParam = p_entityNode->ToElement();
            char *p_paramName, *p_paramType, *p_paramValue;
            p_paramName  = ( char* )p_entityParam->Attribute( CTD_LVL_ENTITY_PARAM_NAME  );
            p_paramType  = ( char* )p_entityParam->Attribute( CTD_LVL_ENTITY_PARAM_TYPE  );
            p_paramValue = ( char* )p_entityParam->Attribute( CTD_LVL_ENTITY_PARAM_VALUE );

            if ( !p_paramName || !p_paramType || !p_paramValue )
            {
                log << Log::LogLevel( Log::L_ERROR ) << "****  incomplete entity parameter entry, skipping" << endl;
                continue;   
            }

            AttributeManager& attrMgr = p_entity->getAttributeManager();
            if ( !attrMgr.setAttributeValue( p_paramName, p_paramType, p_paramValue ) )
            {
                log << Log::LogLevel( Log::L_DEBUG ) << "cannot find entity parameter '" << p_paramName << "'" << endl;
            }
        }

        // append the transformation node if the entity has one
        if ( p_entity->isTransformable() ) 
            _entityGroup->addChild( p_entity->getTransformationNode() );

        // enqueue entitiy for later setup in finalizeLoading
        _setupQueue.push_back( p_entity );
    }

    log << Log::LogLevel( Log::L_INFO ) << endl;
    log << Log::LogLevel( Log::L_INFO ) << "total number of created entities: '" << entityCounter << "'" << endl;
    log << Log::LogLevel( Log::L_INFO ) << "entity loading completed" << endl;

    return true;
}

void LevelManager::finalizeLoading()
{
    Application::get()->setSceneRootNode( _topGroup.get() );
    Application::get()->getViewer()->setSceneData( _topGroup.get() );

    // are we loading for first time?
    if ( _firstLoading )
    {
        bool initphysics = Physics::get()->initialize();
        assert( initphysics && "could not init physics" );

        if ( _levelHasMap )
            buildPhysicsStaticGeometry();

        initializeFirstTime();
    }
    else
    {
        // build the physics static geometry on every finalizing
        buildPhysicsStaticGeometry();
    }

    // init and post-init entities which have been created before
    EntityManager::get()->setupEntities( _setupQueue );
    _setupQueue.clear();

    // send the notification that the a new level has been loaded and initialized
    if ( !_firstLoading )
    {
        EntityNotification ennotify( CTD_NOTIFY_NEW_LEVEL_INITIALIZED );
        EntityManager::get()->sendNotification( ennotify );
        // flush the notification queue so perior and new entities get the notification
        EntityManager::get()->flushNotificationQueue();
    }
    else
    {
        // mark that we have done the first level loading
        _firstLoading = false;
    }
}

void LevelManager::initializeFirstTime()
{
    // initialize sound manager
    log << Log::LogLevel( Log::L_INFO ) << "initializing sound system..." << endl;
    osgAL::SoundManager* p_soundManager = osgAL::SoundManager::instance();
    try 
    {
        p_soundManager->init( 16 );
        p_soundManager->getEnvironment()->setDistanceModel( openalpp::InverseDistance );
        p_soundManager->getEnvironment()->setDopplerFactor( 1.0f );
        osg::ref_ptr< osgAL::SoundRoot > soundRoot = new osgAL::SoundRoot;
        _topGroup->addChild( soundRoot.get() );
    }
    catch( const openalpp::InitError& e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "cannot initialize sound device openAL. reason: '" << e.what() << "'" << endl;
        log << Log::LogLevel( Log::L_ERROR ) << " reason: " << e.what() << endl;
        log << Log::LogLevel( Log::L_ERROR ) << " have you already installed the openAL drivers?" << endl;
    }

    log << Log::LogLevel( Log::L_INFO ) << "initializing gui system..." << endl;
    // initialize the gui system
    GuiManager::get()->initialize();

    // for first time we realize the viewer
    log << Log::LogLevel( Log::L_INFO ) << "starting viewer ..." << endl;
    // note: after initialization of gui system the viewer must be realized (in order to get gui's renderer
    //  initialized) before the entities are initialized
    // use single-threading (entity specific gl commands need it)
    Application::get()->getViewer()->realize( Producer::CameraGroup::SingleThreaded ); 
    Application::get()->getViewer()->sync();
}

void LevelManager::buildPhysicsStaticGeometry()
{
    // continue setting up physics
    //----------------------------
    log << Log::LogLevel( Log::L_INFO ) << "building pyhsics collision geometries ..." << endl;

    // send the notification about building static world, here physics related stuff such as
    //  definition of own physics materials can take place
    {
        EntityNotification ennotify( CTD_NOTIFY_BUILDING_PHYSICSWORLD );
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
    bool buildphysics = Physics::get()->buildStaticGeometry( _nodeGroup.get() );
    assert( buildphysics && "could not build physics" );

    curTick    = timer.tick();
    time4Init  = timer.delta_s( startTick, curTick );
    log << Log::LogLevel( Log::L_INFO ) << "needed time for building pyhsics geometries: " << time4Init << " seconds" << endl;
}

osg::Node* LevelManager::loadStaticWorld( const string& fileName )
{
    // start timer
    osg::Timer_t start_tick = osg::Timer::instance()->tick();

    // load given file
    osg::Node* p_loadedModel = loadMesh( fileName, false );
    // if no model has been successfully loaded report failure.
    if ( !p_loadedModel ) 
    {
        return NULL;
    }
    // stop timer and give out the time messure
    osg::Timer_t end_tick = osg::Timer::instance()->tick();
    log << Log::LogLevel( Log::L_DEBUG ) << "  - elapsed time for loading '" << fileName << "' : " << osg::Timer::instance()->delta_s( start_tick, end_tick ) << endl;
    return p_loadedModel;
}

osg::Node* LevelManager::loadMesh( const string& fileName, bool useCache )
{
    // first lookup in cache
    if ( useCache )
    {
        map< std::string, osg::ref_ptr< osg::Node > >::iterator pp_mesh = _meshCache.find( fileName );
        if ( pp_mesh != _meshCache.end() )
            return pp_mesh->second.get();
    }

    // read given file
    osg::Node* p_loadedModel = osgDB::readNodeFile( Application::get()->getMediaPath() + fileName );

    // if no model has been successfully loaded report failure.
    if ( !p_loadedModel ) 
    {
        log << Log::LogLevel( Log::L_WARNING ) << "*** could not load mesh: " << fileName << endl;
        return NULL;
    }

    // optimize the scene graph, remove rendundent nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize( p_loadedModel );

    // cache the loaded mesh
    if ( useCache )
        _meshCache.insert( make_pair( fileName, p_loadedModel ) );

    return p_loadedModel;
}

void LevelManager::shutdown()
{
    // clean up entity manager
    EntityManager::get()->shutdown(); 
    // shutdown all other libs managed by level loader
    osgAL::SoundManager::instance()->shutdown();
    Physics::get()->shutdown();
    GuiManager::get()->shutdown();

    // destroy singleton
    destroy();
}
