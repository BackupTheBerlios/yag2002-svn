/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2002-2004, Ali Botorabi
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
 # neoengine, level loader
 #
 # this class implements reading level file '.lvl' and initializing
 #  all static and active objects in level
 #
 #
 #   date of creation:  11/16/2003
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 #
 ################################################################*/

#include "ctd_frbase.h"

#include "ctd_levelloader.h"
#include "ctd_framework_impl.h"
#include "ctd_pluginmanager.h"
#include <ctd_network.h>
#include <ctd_levelmanager.h>

#define TIXML_USE_STL
#include "tinyxml.h"

using namespace std;
using namespace NeoEngine;
using namespace NeoChunkIO;

namespace CTD 
{

// xml item names
//-------------------------------------//
#define CTD_LVL_ELEM_LEVEL                  "Level"

#define CTD_LVL_ELEM_MAP                    "Map"
#define CTD_LVL_ELEM_MAP_TYPE               "Type"
#define CTD_LVL_ELEM_MAP_TYPE_ABT           "ABT"
#define CTD_LVL_ELEM_MAP_TYPE_BSP           "BSP"

#define CTD_LVL_ELEM_LOADINGPIC             "LoadingPic"
#define CTD_LVL_ELEM_MODULE                 "Module"
#define CTD_LVL_ELEM_ENTITY                 "Entity"
#define CTD_LVL_ELEM_PARAM                  "Parameter"

#define CTD_LVL_ATTR_NAME                   "Name"
#define CTD_LVL_ATTR_INST_NAME              "InstanceName"
#define CTD_LVL_ATTR_TYPE                   "Type"
#define CTD_LVL_ATTR_VALUE                  "Value"
//-------------------------------------//

LevelLoader::LevelLoader( LevelSet* pkLevelSet, FrameworkImpl* pkFrameworkImpl )
{

    m_pkLevelSet      = pkLevelSet;
    m_pkFrameworkImpl = pkFrameworkImpl;

}

LevelLoader::~LevelLoader()
{
}

bool LevelLoader::LoadNode( unsigned int uiFlags )
{

    neolog << LogLevel( INFO ) << endl;
    neolog << LogLevel( INFO ) << "CTD loading level configuration file ' " << m_pkFile->GetName().c_str() << " '" << endl;

    if( !m_pkFile || !m_pkFile->IsValid() ) {

        return false;

    }

    string  strPicName;
    char    *pcBuffer;
    pcBuffer = new char[ m_pkFile->GetSize() + 1 ];

    m_pkFile->SetBinary( false );
    m_pkFile->Read( ( void* )pcBuffer, m_pkFile->GetSize() );
    // append an EOF at the end of buffer, else tiny xml parser causes an assertion
    pcBuffer[ m_pkFile->GetSize() ] = EOF;

    // use tiny xml to parser lvl file
    //  parse in the level configuration
    TiXmlDocument doc;
    doc.SetCondenseWhiteSpace(false);
    doc.Parse(pcBuffer);
    delete pcBuffer;

    if ( doc.Error() == true ) {

        neolog << LogLevel( ERROR ) << "*** CTD (XML parser) error parsing level config file. " << endl;
        neolog << LogLevel( ERROR ) << doc.ErrorDesc() << endl;
        return false;

    }

    // start evaluating the xml structure
    //---------------------------------//
    TiXmlNode       *node               = 0;
    TiXmlNode       *node2              = 0;
    TiXmlNode       *node3              = 0;
    TiXmlNode       *node4              = 0;
    TiXmlElement    *levelElement       = 0;
    TiXmlElement    *mapElement         = 0;
    TiXmlElement    *moduleElement      = 0;
    TiXmlElement    *entityElement      = 0;
    TiXmlElement    *loadingpicElement  = 0;


    char *pcBufName, *pcBufType, *pcBufValue;
    string strLevelName;
    string strMapName;
    string strMapType;

    // get the level entry
    node = doc.FirstChild( CTD_LVL_ELEM_LEVEL );
    if ( !node ) {

        neolog << LogLevel( ERROR ) << "**** CTD (XML parser) could not find level element: " << string( CTD_LVL_ELEM_LEVEL ) << endl;
        return false;    
    
    }

    // get level name
    levelElement = node->ToElement();
    if ( !levelElement ) {
 
        neolog << LogLevel( ERROR ) << "**** CTD (XML parser) could not find level element. " << endl;
        return false;

    }

    pcBufName = (char*)levelElement->Attribute( CTD_LVL_ATTR_NAME, NULL );
    if ( pcBufName == NULL ) {

        strLevelName = "noname";

    } else {

        strLevelName = pcBufName;

    }
    neolog << LogLevel( INFO ) << "  name: ' " << strLevelName.c_str() << " '" << endl;

    // set the level set name
    m_pkLevelSet->m_strName = strLevelName;

    // get map information
    node = levelElement->FirstChild( CTD_LVL_ELEM_MAP );
    mapElement = node->ToElement();
    pcBufName = ( char* )mapElement->Attribute( CTD_LVL_ATTR_NAME, NULL );
    if ( pcBufName == NULL ) {

        neolog << LogLevel( ERROR ) << "**** CTD (XML parser) could not find map element. " << endl;
        return false;

    } else {

        strMapName = pcBufName;

        // get map type
        pcBufName = ( char* )mapElement->Attribute( CTD_LVL_ELEM_MAP_TYPE, NULL );
        if ( pcBufName == NULL ) {

            neolog << LogLevel( WARNING ) << "**** CTD (XML parser) missing map type, assuming ABT " << endl;
            strMapType  = CTD_LVL_ELEM_MAP_TYPE_ABT;

        } else {

            strMapType = pcBufName;

        }

    }

    // look for loading picture definition
    node = levelElement->FirstChild( CTD_LVL_ELEM_LOADINGPIC );
    if ( node ) {

        loadingpicElement = node->ToElement();
        pcBufName = (char*)loadingpicElement->Attribute( CTD_LVL_ATTR_NAME, NULL );
        // show loading picture
        ShowLoadingPicture( pcBufName );

     }

    // load map
    //-------------------------------------------------------//

    Room *pkRoom = NULL;
    // load abt module
    if ( strMapType == CTD_LVL_ELEM_MAP_TYPE_ABT ) {

        pkRoom = NeoEngine::Core::Get()->GetRoomManager()->CreateRoom( "abt" );

    } else {

        if ( strMapType == CTD_LVL_ELEM_MAP_TYPE_BSP ) {

            pkRoom = NeoEngine::Core::Get()->GetRoomManager()->CreateRoom( "bsp" );


        } else {

            neolog << LogLevel( ERROR ) << "**** CTD (XML parser) unsupported map tye '" << strMapType << endl;
            neolog << LogLevel( ERROR ) << "****   use '" << CTD_LVL_ELEM_MAP_TYPE_ABT << "' or '"
                << CTD_LVL_ELEM_MAP_TYPE_BSP << "'" << endl;
            return false;

        }

    }


    // set level set's room
    m_pkLevelSet->m_pkRoom = pkRoom;

    Scene       *pkScene = new Scene;

    // load the scene
    pkScene->Load( strMapName.c_str(), Scene::STRIPIFYALL );

    // get all meshes and place them into room
    size_t uiMeshCount = pkScene->GetMeshes().size();
    for ( size_t uiMeshes = 0; uiMeshes < uiMeshCount; uiMeshes++ ) {

        MeshEntity  *pkMeshEnt = new MeshEntity( pkScene->GetMeshes()[ uiMeshes ] );

        if ( strMapType == CTD_LVL_ELEM_MAP_TYPE_ABT ) {

            // get all submeshes
            vector<SubMesh*>    vpkSubmeshes = pkMeshEnt->GetSubMeshes();
            SubMesh             *pkSubMesh = NULL;
            for ( size_t uiSubMeshes = 0; uiSubMeshes < vpkSubmeshes.size(); uiSubMeshes++) {

                // get geometry info and add it into room
                pkSubMesh = vpkSubmeshes[ uiSubMeshes ];

                PolygonBufferPtr    pkPolys = pkSubMesh->GetPolygonBuffer();
                VertexBufferPtr     pkVerts = pkSubMesh->GetVertexBuffer();
                Material            *pkMat  = pkSubMesh->m_pkMaterial;  

                pkRoom->AddGeometry( pkPolys, pkVerts, pkMat );
            }

        } else {
            
            if ( strMapType == CTD_LVL_ELEM_MAP_TYPE_BSP ) {

                pkMeshEnt->GenerateBoundingVolume();
                SceneNode *pkNode = new SceneNode;
                pkNode->SetEntity( pkMeshEnt );
                pkRoom->AttachNode( pkNode );

            }

        } 

    }

    pkScene->PrintHierarchy();
    delete pkScene;
    //-------------------------------------------------------//

    // load and setup plugins
    //----------------------------------------------------------------------------//

    // create a new plugin manager
    m_pkLevelSet->m_pkPluginMgr = new PluginManager;

    unsigned int uiPluginCount = 0;
    unsigned int uiEntityCount = 0;
    // get plugin information
    for ( node = levelElement->FirstChild( CTD_LVL_ELEM_MODULE ); node; node = node->NextSiblingElement( CTD_LVL_ELEM_MODULE ) ) {

        moduleElement = node->ToElement();
        pcBufName = (char*)moduleElement->Attribute( CTD_LVL_ATTR_NAME, NULL );

        if (pcBufName != NULL) {

            // try to load and bind plugin
            Plugin  *pkPlugin;
            pkPlugin = m_pkLevelSet->m_pkPluginMgr->LoadPlugin( pcBufName );

            if ( pkPlugin == NULL ) {

                // could not load plugin!
                continue;

            }

            // increase count of successfully loaded plugins
            uiPluginCount++;

            // setup plugin's global parameters
            neolog << LogLevel( INFO ) << "  CTD setup plugin's global parameters ..." << endl;
            for ( node2 = moduleElement->FirstChild( CTD_LVL_ELEM_PARAM ); node2; node2 = node2->NextSiblingElement( CTD_LVL_ELEM_PARAM ) ) {

                moduleElement = node2->ToElement();
                pcBufName  = (char*)moduleElement->Attribute( CTD_LVL_ATTR_NAME, NULL );
                pcBufType  = (char*)moduleElement->Attribute( CTD_LVL_ATTR_TYPE, NULL );
                pcBufValue = (char*)moduleElement->Attribute( CTD_LVL_ATTR_VALUE, NULL );

                if ( ( pcBufName == NULL ) || 
                     ( pcBufType == NULL ) ||
                     ( pcBufValue == NULL ) )
                {
                    neolog << LogLevel( WARNING ) << " **** CTD (XML parser) incomplete plugin parameter entry" << endl;
                    continue;
                }

                // set parameter value
                pkPlugin->SetParameter( pcBufName, pcBufType, pcBufValue );
                //---------------------------------------------//

            }
            
            neolog << LogLevel( INFO ) << "  CTD setup entity's parameters..." << endl;

            // get entity information
            node3 = node; // module node
            for ( node3 = node3->FirstChild( CTD_LVL_ELEM_ENTITY ); node3; node3 = node3->NextSiblingElement( CTD_LVL_ELEM_ENTITY ) ) {

                entityElement = node3->ToElement();

                // get entity name
                pcBufName = ( char* )entityElement->Attribute( CTD_LVL_ATTR_NAME, NULL );

                if ( pcBufName == NULL ) {
                    neolog << LogLevel( WARNING ) << "  **** CTD (XML parser) entity has no name, skipping" << endl;
                    continue;       
                }



                // create an entity and assign it into plugin's entity list
                EntityDescriptor    *pkEntityDesc =  pkPlugin->GetEntityDescriptor( pcBufName );

                // could we find entity descriptor by its name?
                if ( pkEntityDesc == NULL ) {

                    neolog << LogLevel( WARNING ) << "  **** CTD (XML parser) could not find entity ' " << pcBufName << " '" << endl;
                    continue;

                }

                // create an entity instance
                BaseEntity  *pkEntity = pkEntityDesc->CreateEntityInstance();

                // get instance name if one provided
                pcBufName = (char*)entityElement->Attribute( CTD_LVL_ATTR_INST_NAME, NULL );
                if ( pcBufName ) {
                    // set entity's instance name
                    ( pkEntity)->SetInstanceName( pcBufName );
                    neolog << LogLevel( INFO ) << "  CTD instance name: '" << pcBufName << " '" << endl;
                }

                for ( node4 = entityElement->FirstChild( CTD_LVL_ELEM_PARAM ); node4; node4 = node4->NextSiblingElement( CTD_LVL_ELEM_PARAM ) ) {

                    entityElement = node4->ToElement();
                    pcBufName  = (char*)entityElement->Attribute( CTD_LVL_ATTR_NAME, NULL );
                    pcBufType  = (char*)entityElement->Attribute( CTD_LVL_ATTR_TYPE, NULL );
                    pcBufValue = (char*)entityElement->Attribute( CTD_LVL_ATTR_VALUE, NULL );

                    if ( ( pcBufName == NULL ) || 
                         ( pcBufType == NULL ) ||
                         ( pcBufValue == NULL ) )
                    {
                        neolog << LogLevel( WARNING ) << "  **** CTD (XML parser) incomplete entity parameter entry, skipping" << endl;
                        continue;   
                    }


                    // set parameter value
                    pkPlugin->SetParameter( pcBufName, pcBufType, pcBufValue, pkEntity );
                    //---------------------------------------------//

                }

                // enable networking if desired
                // attach node to room immediately only if in stand-alone mode or in server/client mode but no networking enabled for entity
                if ( ( m_pkFrameworkImpl->m_eGameMode != stateSTANDALONE ) && ( pkEntity->GetNetworkingType() != stateNONE ) ) {

                    // if server object then instantly create it
                    if ( pkEntity->GetNetworkingType() == stateSERVEROBJECT ) {

                        m_pkFrameworkImpl->GetNetworkDevice()->AddServerObject( pkEntity );
                        // add the entity into plugin's entity list
                        m_pkLevelSet->m_pkPluginMgr->AddEntity( pkPlugin, pkEntity );


                    } else {

                        // if it is a client object then it must be created later after negotiation with server
                        //  client objects can only be created as actors on clients; their ghosts are created on server and all other clients later
                        //  after negotiation with server
                        if ( ( pkEntity->GetNetworkingType() == stateCLIENTOBJECT ) &&
                              ( m_pkFrameworkImpl->m_eGameMode == stateCLIENT ) ) {

                            m_pkFrameworkImpl->GetNetworkDevice()->AddClientObject( pkEntity );

                              
                        }

                    }

                // add immediately the entity into room manager when in stand-alone mode
                //  otherwise they are added into room manager later ( after server/client connection is established )
                } else {

                    pkRoom->AttachNode( ( SceneNode* )pkEntity );
                    // add the entity into plugin's entity list
                    m_pkLevelSet->m_pkPluginMgr->AddEntity( pkPlugin, pkEntity );

                }

                uiEntityCount++;

            }
        }
    }

    neolog << LogLevel( INFO ) << " CTD count of successfully loaded plugins: " << uiPluginCount << endl;
    neolog << LogLevel( INFO ) << " CTD count of successfully created entities: " << uiEntityCount << endl;

    //----------------------------------------------------------------------------//

    return true;

}

// show up loading picture
void LevelLoader::ShowLoadingPicture( const string &strPicture )
{

    // in server mode we need not loading picture
    if ( m_pkFrameworkImpl->m_eGameMode == stateSERVER ) {

        return;

    }

    RenderDevice    *pkRenderDev = m_pkFrameworkImpl->GetRenderDevice();
    assert( pkRenderDev );

    pkRenderDev->SetPerspectiveProjection( 65.0f, 1.0f, 1600.0f );

    TexturePtr pkLoadingPicTexture = pkRenderDev->LoadTexture( strPicture, Texture::TEX2D, Texture::DEFAULT, Texture::NOMIPMAPS );
    if ( !pkLoadingPicTexture ) {
        return;
    }


    VertexBufferPtr pkVertexBuffer = pkRenderDev->CreateVertexBuffer( Buffer::STATIC, 4, &TexVertex::s_kDecl );

    pkVertexBuffer->Lock( Buffer::WRITE );

    TexVertex *pkVertex = (TexVertex*)pkVertexBuffer->GetVertex();

    pkVertex->m_kPosition.Set( 0.0f, 0.0f, 0.0f );
    pkVertex->m_afTexCoord[0] = 0.0f;
    pkVertex->m_afTexCoord[1] = 0.0f;
    ++pkVertex;

    pkVertex->m_kPosition.Set( 0.0f, 1.0f, 0.0f );
    pkVertex->m_afTexCoord[0] = 0.0f;
    pkVertex->m_afTexCoord[1] = 1.0f;
    ++pkVertex;

    pkVertex->m_kPosition.Set( 1.0f, 1.0f, 0.0f );
    pkVertex->m_afTexCoord[0] = 1.0f;
    pkVertex->m_afTexCoord[1] = 1.0f;
    ++pkVertex;

    pkVertex->m_kPosition.Set( 1.0f, 0.0f, 0.0f );
    pkVertex->m_afTexCoord[0] = 1.0f;
    pkVertex->m_afTexCoord[1] = 0.0f;

    pkVertexBuffer->Unlock();


    PolygonBufferPtr pkPolygonBuffer = pkRenderDev->CreatePolygonBuffer( Buffer::NORMAL, 2 );

    pkPolygonBuffer->Lock( Buffer::WRITE );

    (*pkPolygonBuffer)[0][0] = 0;
    (*pkPolygonBuffer)[0][1] = 1;
    (*pkPolygonBuffer)[0][2] = 2;

    (*pkPolygonBuffer)[1][0] = 0;
    (*pkPolygonBuffer)[1][1] = 2;
    (*pkPolygonBuffer)[1][2] = 3;

    pkPolygonBuffer->Unlock();


    MaterialPtr pkLoadingPicMat = new Material( "loadingpic", 0 );

    pkLoadingPicMat->m_pkTexture  = pkLoadingPicTexture;


    pkRenderDev->SetOrthographicProjection();

    pkRenderDev->Begin( Matrix::IDENTITY );
    {
        RenderPrimitive kPrim;

        kPrim.m_ePrimitive      = RenderPrimitive::TRIANGLES;
        kPrim.m_pkMaterial      = pkLoadingPicMat;
        kPrim.m_pkVertexBuffer  = pkVertexBuffer;
        kPrim.m_pkPolygonBuffer = pkPolygonBuffer;
        kPrim.m_uiNumPrimitives = 2;

        pkRenderDev->Render( kPrim, 0 );
    }
    pkRenderDev->End();

    pkRenderDev->Flip();

    pkRenderDev->SetPerspectiveProjection();

}

} // namespace CTD 
