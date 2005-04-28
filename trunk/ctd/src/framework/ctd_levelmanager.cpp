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
_topGroup( new osg::Group() ),
_nodeGroup( new osg::Group() ),
_entityGroup( new osg::Group() ),
_firstLoading( true )
{
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

osg::ref_ptr< osg::Group > LevelManager::load( const string& levelFile, bool keepPhysicsWorld, bool keepEntities )
{
    // this shows whether the new level has a map in order to decide rebuilding physics when necessary (keepPhysicsWorld == false)
    bool haveMap = false;

    log << Log::LogLevel( Log::L_INFO ) << "loading level ..." << endl;
    
    ifstream    file;
    file.open( string( Application::get()->getMediaPath() + levelFile ).c_str(), std::ios::in, std::ios::binary );
    if ( !file )
    {
        log << Log::LogLevel( Log::L_DEBUG ) << "*** cannot open level file: '" << levelFile << "'" << endl;
        return false;
    }

    // Note: all following notifications are sent to already existing entities, so they have the change to do 
    //       particular actions such as rebuilding their physics, etc.
    //       the entities of initial level won't get those notifications as they are not created yet (see below)

    // send loading notification
    {
        EntityNotification ennotify( CTD_NOTIFY_LOADING_LEVEL );
        EntityManager::get()->sendNotification( ennotify );
    }

    // delete existing entities?
    if ( !keepEntities )
    {
        // first send out a notification about deleting entities
        EntityNotification ennotify( CTD_NOTIFY_DELETING_ENTITIES );
        EntityManager::get()->sendNotification( ennotify );
        // now delete entities (note: no-autodelete entities will remain -- see BaseEntity::getAutoDelete() )
        EntityManager::get()->deleteAllEntities();
        // (re-)create groups
        _topGroup->removeChild( _nodeGroup.get() );
        _topGroup->removeChild( _entityGroup.get() );
        _nodeGroup = new osg::Group();
        _nodeGroup->setName( CTD_NODE_GROUP_NAME );
        _entityGroup = new osg::Group();
        _entityGroup->setName( CTD_ENTITY_GROUP_NAME );
        _topGroup->addChild( _nodeGroup.get() );
        _topGroup->addChild( _entityGroup.get() );
    }
    // init physics
    if( _firstLoading )
    {
        assert( Physics::get()->initialize() );
    }
    else if ( !keepPhysicsWorld ) // reinit physics world?
    {
        EntityNotification ennotify( CTD_NOTIFY_DELETING_PHYSICSWORLD );
        EntityManager::get()->sendNotification( ennotify );
         assert( Physics::get()->reinitialize() );
    }

    // read in the file into char buffer for tinyxml
    file.seekg( 0, ios_base::end );
    int filesize = ( int )file.tellg();
    char *p_buffer = new char[ filesize + 1 ];
    file.seekg( 0, ios_base::beg );
    file.read( p_buffer, filesize );
    // append an EOF at the end of buffer, else tiny xml parser causes an assertion
    p_buffer[ filesize ] = EOF;

    // use tiny xml to parser lvl file
    //  parse in the level configuration
    TiXmlDocument doc;
    doc.SetCondenseWhiteSpace( false );
    doc.Parse( p_buffer );
    delete p_buffer;

    if ( doc.Error() == true ) 
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** CTD (XML parser) error parsing level config file. " << endl;
        log << Log::LogLevel( Log::L_ERROR ) << doc.ErrorDesc() << endl;
        return false;
    }

    log << Log::LogLevel( Log::L_INFO ) << "initializing physics ..." << endl;

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
            log << Log::LogLevel( Log::L_ERROR ) << "**** could not find level element. " << endl;
            return false;
        }

        p_bufName = ( char* )p_levelElement->Attribute( CTD_LVL_ELEM_NAME );
        if ( p_bufName )
            staticNodeName = p_bufName;
        else 
            return false;  
    }

    // get map files, load them and add them to main group
    p_node = p_levelElement->FirstChild( CTD_LVL_ELEM_MAP );
    do {

        // a level file can also be without a map
        if ( !p_node )
            break;

        haveMap = true;
        p_mapElement = p_node->ToElement();
        p_bufName    = ( char* )p_mapElement->Attribute( CTD_LVL_ELEM_NAME );
        if ( p_bufName == NULL ) 
        {
            log << Log::LogLevel( Log::L_ERROR ) << "*** missing map name in MAP entry" << endl;
            return false;
        } 
        else 
        {
            log << Log::LogLevel( Log::L_INFO ) << " loading static geometry: " << p_bufName << endl;
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

    } while ( p_node = p_node->NextSiblingElement( CTD_LVL_ELEM_MAP ) );


    // read entity definitions
    //------------------------
    log << Log::LogLevel( Log::L_INFO ) << " creating entities ..." << endl;

    unsigned int entityCounter = 0;
    vector< BaseEntity* > entities; // list of all entities which are created during loading

    p_node = p_levelElement;
    for ( p_node = p_node->FirstChild( CTD_LVL_ELEM_ENTITY ); p_node; p_node = p_node->NextSiblingElement( CTD_LVL_ELEM_ENTITY ) ) 
    {
        TiXmlElement* p_entityElement = p_node->ToElement();
        string entitytype, instancename;
        // get entity name
        p_bufName = ( char* )p_entityElement->Attribute( CTD_LVL_ENTITY_TYPE );
        string enttype;
        if ( !p_bufName ) 
        {
            log << Log::LogLevel( Log::L_DEBUG ) << "  **** entity has no type, skipping" << endl;
            continue;       
        }
        else
            enttype = p_bufName;

        entitytype = p_bufName;
        p_bufName = ( char* )p_entityElement->Attribute( CTD_LVL_ENTITY_INST_NAME );
        if ( !p_bufName ) 
            instancename = p_bufName;

        // create entity
        BaseEntity* p_entity = EntityManager::get()->createEntity( entitytype, instancename );

        // could we find entity type
        if ( !p_entity ) 
        {
            log << Log::LogLevel( Log::L_ERROR ) << "*** could not find entity type ' " << entitytype << " ', skipping entity!" << endl;
            continue;
        }
        // add to list in order to get it set up later
        entities.push_back( p_entity );

        log << Log::LogLevel( Log::L_DEBUG ) << "  entity created, type: '" << enttype << " '" << endl;
        // get instance name if one provided
        p_bufName = ( char* )p_entityElement->Attribute( CTD_LVL_ENTITY_INST_NAME );
        if ( p_bufName ) {
            // set entity's instance name
            p_entity->setInstanceName( p_bufName );
            log << Log::LogLevel( Log::L_DEBUG ) << "  instance name: '" << p_bufName << " '" << endl;
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
                log << Log::LogLevel( Log::L_DEBUG ) << "****  cannot find entity parameter '" << p_paramName << "'" << endl;
            }
        }

        // create a transformation node if needed
        if ( p_entity->needTransformation() ) 
        {
            osg::PositionAttitudeTransform  *p_trans = new osg::PositionAttitudeTransform;
            //! TODO currently we add all entities into entity group, later a better solution may be implemented
            //  which chooses an appropriate group depending on spatial paritioning or something similar.
            p_entity->setTransformationNode( p_trans );
            _entityGroup->addChild( p_trans );
        }

    }

    log << Log::LogLevel( Log::L_INFO ) << endl;
    log << Log::LogLevel( Log::L_INFO ) << " total number of created entities: '" << entityCounter << "'" << endl;
    log << Log::LogLevel( Log::L_INFO ) << "entity setup completed" << endl;

    // store the main group in application and set it in viewer
    Application::get()->setSceneRootNode( _topGroup.get() );
    Application::get()->getViewer()->setSceneData( _topGroup.get() );


    // are we loading for first time?
    if ( _firstLoading )
    {
        if ( haveMap )
            buildPhysicsStaticGeometry();

        initializeFirstTime();

        // mark that we have done the first level loading
        _firstLoading = false;
    }
    else
    {
        // if the existing static physics world is not to be kept and the new level has a map then rebuild physics
        if ( !keepPhysicsWorld && haveMap )
            buildPhysicsStaticGeometry();
    }

    // init and post-init entities which has been created
    setupEntities( entities );

    return _topGroup;
}

void LevelManager::setupEntities( vector< BaseEntity* >& entities )
{
    osg::Timer      timer;
    osg::Timer_t    curTick         = 0;
    osg::Timer_t    startTick       = 0;
    float           time4Init       = 0;
    float           time4PostInit   = 0;

    vector< BaseEntity* >::iterator pp_beg = entities.begin(), pp_end = entities.end();

    log << Log::LogLevel( Log::L_INFO ) << "starting entity setup ..." << endl;
    // setup entities
    log << Log::LogLevel( Log::L_INFO ) << " initializing entities ..." << endl;
    
    {
        startTick  = timer.tick();

        for( ; pp_beg != pp_end; pp_beg++ )
            ( *pp_beg )->initialize();
        
        curTick    = timer.tick();
        time4Init  = timer.delta_s( startTick, curTick );
    }

    pp_beg = entities.begin(); pp_end = entities.end();
    log << Log::LogLevel( Log::L_INFO ) << " post-initializing entities ..." << endl;
    {
        startTick  = timer.tick();
    

        for( ; pp_beg != pp_end; pp_beg++ )
            ( *pp_beg )->postInitialize();

        curTick        = timer.tick();
        time4PostInit  = timer.delta_s( startTick, curTick );
    }
    log << Log::LogLevel( Log::L_INFO ) << "--------------------------------------------" << endl;
    log << Log::LogLevel( Log::L_INFO ) << "needed time for initialization: " << time4Init << " seconds" << endl;
    log << Log::LogLevel( Log::L_INFO ) << "needed time for post-initialization: " << time4PostInit << " seconds" <<  endl;
    log << Log::LogLevel( Log::L_INFO ) << "total time for setting up: " << time4Init + time4PostInit << " seconds" <<  endl;
    log << Log::LogLevel( Log::L_INFO ) << "--------------------------------------------" << endl;
    //--------------//
}

void LevelManager::initializeFirstTime()
{
    // initialize sound manager
    osgAL::SoundManager* p_soundManager = osgAL::SoundManager::instance();
    try 
    {
        p_soundManager->init( 16 );
        p_soundManager->getEnvironment()->setDistanceModel( openalpp::InverseDistance );
        p_soundManager->getEnvironment()->setDopplerFactor( 1 );
        osgAL::SoundRoot* p_soundRoot = new osgAL::SoundRoot;
        _topGroup->addChild( p_soundRoot );
    }
    catch( openalpp::InitError e )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** cannot initialize sound device openAL. reason: '" << e.what() << "'" << endl;
        log << Log::LogLevel( Log::L_ERROR ) << "***   have you already installed the openAL drivers?" << endl;
        return;
    }

    log << Log::LogLevel( Log::L_INFO ) << "initializing gui system..." << endl;
    // initialize the gui system
    GuiManager::get()->initialize();

    // for first time we realize the viewer
    log << Log::LogLevel( Log::L_INFO ) << "starting viewer ..." << endl;
    // note: after initialization of gui system the viewer must be realized (in order to get gui's renderer
    //  initialized) before the entities are initialized
    // use single-threading (entity specific gl commands need it)
    Application::get()->getViewer()->realize( osgProducer::Viewer::ThreadingModel::SingleThreaded ); 
    Application::get()->getViewer()->sync();
}

void LevelManager::buildPhysicsStaticGeometry()
{
    // continue setting up physics
    //----------------------------
    log << Log::LogLevel( Log::L_INFO ) << "building pyhsics collision geometries ..." << endl;

    // send the notification about building static world
    {
        EntityNotification ennotify( CTD_NOTIFY_BUILDING_PHYSICSWORLD );
        EntityManager::get()->sendNotification( ennotify );
    }

    osg::Timer      timer;
    osg::Timer_t    curTick         = 0;
    osg::Timer_t    startTick       = 0;
    float           time4Init       = 0;

    startTick  = timer.tick();
    // build static geoms for physics collision nodes
    assert( Physics::get()->buildStaticGeometry( _nodeGroup.get() ) );
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
    // important: first of all we shut down all entities!
    EntityManager::get()->shutdown(); 
    // then we shut down all other libs managed by level loader
    osgAL::SoundManager::instance()->shutdown();
    Physics::get()->shutdown();
    GuiManager::get()->shutdown();

    // destroy singleton
    destroy();
}
