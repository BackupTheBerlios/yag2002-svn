/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2004-2006, Ali Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU General Public License 
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public 
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
#include "ctd_log.h"
#include "ctd_levelmanager.h"
#include "ctd_application.h"
#include "ctd_attributemanager.h"
#include "ctd_entitymanager.h"
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



CTD_SINGLETON_IMPL( LevelManager );

LevelManager::LevelManager()
{
}

LevelManager::~LevelManager()
{
}

osg::ref_ptr< osg::Group > LevelManager::load( const string& levelFile )
{
    ifstream    file;
    file.open( levelFile.c_str(), std::ios::in, std::ios::binary );
    if ( !file )
    {
        cout << "*** cannot open level file." << endl;
        return false;
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
        cout << "*** CTD (XML parser) error parsing level config file. " << endl;
        cout << doc.ErrorDesc() << endl;
        return false;
    }

    // create the top of scene node ( root of all nodes )
    osg::ref_ptr< osg::Group >  mainGroup = new osg::Group();

    // start evaluating the xml structure
    //---------------------------------//
    TiXmlNode       *p_node                 = NULL;
    TiXmlElement    *p_levelElement         = NULL;
    TiXmlElement    *p_mapElement           = NULL;
    char            *p_bufName              = NULL;

    // get the level entry if one exists
    p_node = doc.FirstChild( CTD_LVL_ELEM_LEVEL );
    if ( p_node ) 
    {
    
        // get level name
        p_levelElement = p_node->ToElement();
        if ( !p_levelElement ) 
        {
            cout << "**** could not find level element. " << endl;
            return false;
        }

        p_bufName = ( char* )p_levelElement->Attribute( CTD_LVL_ELEM_NAME );
        if ( p_bufName )
            mainGroup->setName( p_bufName );
        else 
            return false;  
    }

    // get map files, load them and add them to main group
    p_node = p_levelElement->FirstChild( CTD_LVL_ELEM_MAP );
    do {

        if ( p_node ) {

            p_mapElement = p_node->ToElement();
            p_bufName    = ( char* )p_mapElement->Attribute( CTD_LVL_ELEM_NAME );
            if ( p_bufName == NULL ) 
            {
                cout << "*** could not find map element." << endl;
                return false;
            } 
            else 
            {
                osg::Node *p_staticnode = loadStaticWorld( p_bufName );
                if ( p_staticnode )
                {
                    p_staticnode->setName( p_bufName );
                    mainGroup->addChild( p_staticnode );
                    _staticMesh = p_staticnode;
                }
                else
                    return false;
            }
        } 

    } while ( p_node = p_node->NextSiblingElement( CTD_LVL_ELEM_MAP ) );


    // read entity definitions
    //------------------------
    cout << "setup entities" << endl;

    unsigned int entityCounter = 0;

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
            cout << "  **** entity has no type, skipping" << endl;
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
            cout << "  *** could not find entity type ' " << p_bufName << " '" << endl;
            continue;
        }
        cout << "entity created, type: '" << enttype << " '" << endl;
        // get instance name if one provided
        p_bufName = ( char* )p_entityElement->Attribute( CTD_LVL_ENTITY_INST_NAME );
        if ( p_bufName ) {
            // set entity's instance name
            p_entity->setInstanceName( p_bufName );
            cout << "  instance name: '" << p_bufName << " '" << endl;
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
                cout << "****  incomplete entity parameter entry, skipping" << endl;
                continue;   
            }

            AttributeManager& attrMgr = p_entity->getAttributeManager();
            if ( !attrMgr.setAttributeValue( p_paramName, p_paramType, p_paramValue ) )
            {
                cout << "****  cannot find entity parameter '" << p_paramName << "'" << endl;
            }
        }

        // create a transformation node if needed
        if ( p_entity->needTransformation() ) 
        {
            osg::PositionAttitudeTransform  *p_trans = new osg::PositionAttitudeTransform;
            //! TODO currently we add all entities into main group, later a better solution may be implemented
            //  which chooses an appropriate group depending on spatial paritioning or something similar.
            p_entity->setTransformationNode( p_trans );
            mainGroup->addChild( p_trans );
        }

    }

    cout << endl;
    cout << "total number of created entities: '" << entityCounter << "'" << endl;

    return mainGroup;
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
    cout << "Time to load = "<< osg::Timer::instance()->delta_s( start_tick, end_tick ) << endl;

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
    _meshCache.insert( make_pair( fileName, p_loadedModel ) );

    return p_loadedModel;
}

void LevelManager::shutdown()
{
    // destroy singleton
    destroy();
}
