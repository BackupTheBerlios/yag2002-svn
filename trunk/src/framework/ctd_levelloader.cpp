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
    TiXmlNode       *pkNode                 = 0;
    TiXmlNode       *pkNode2                = 0;
    TiXmlNode       *pkNode3                = 0;
    TiXmlNode       *pkNode4                = 0;
    TiXmlElement    *pkLevelElement         = 0;
    TiXmlElement    *pkMapElement           = 0;
    TiXmlElement    *pkModuleElement        = 0;
    TiXmlElement    *pkEntityElement        = 0;
    TiXmlElement    *pkLoadingpicElement    = 0;

    char            *pcBufName, *pcBufType, *pcBufValue;
    string          strLevelName;
    string          strMapName;
    string          strMapType;
    Room            *pkRoom                 = NULL;
    bool            bHasMap                 = false;

    // get the level entry if one exists
    pkNode = doc.FirstChild( CTD_LVL_ELEM_LEVEL );
    if ( pkNode ) {
    
        // get level name
        pkLevelElement = pkNode->ToElement();
        if ( !pkLevelElement ) {

            neolog << LogLevel( ERROR ) << "**** CTD (XML parser) could not find level element. " << endl;
            return false;

        }

        pcBufName = (char*)pkLevelElement->Attribute( CTD_LVL_ATTR_NAME, NULL );
        if ( pcBufName == NULL ) {

            strLevelName = "noname";

        } else {

            strLevelName = pcBufName;

        }
        neolog << LogLevel( INFO ) << "  name: ' " << strLevelName.c_str() << " '" << endl;

        // set the level set name
        m_pkLevelSet->m_strName = strLevelName;

    } else {

        neolog << LogLevel( ERROR ) << "*** CTD (XML parser) error, file has no 'Level' node. " << endl;
        return NULL;

    }

    // get map information if it exists
    pkNode = pkLevelElement->FirstChild( CTD_LVL_ELEM_MAP );
    if ( pkNode ) {

        pkMapElement = pkNode->ToElement();
        pcBufName = ( char* )pkMapElement->Attribute( CTD_LVL_ATTR_NAME, NULL );
        if ( pcBufName == NULL ) {

            neolog << LogLevel( ERROR ) << "**** CTD (XML parser) could not find map element. " << endl;
            return false;

        } else {

            strMapName = pcBufName;

            // get map type
            pcBufName = ( char* )pkMapElement->Attribute( CTD_LVL_ELEM_MAP_TYPE, NULL );
            if ( pcBufName == NULL ) {

                neolog << LogLevel( WARNING ) << "**** CTD (XML parser) missing map type, assuming ABT " << endl;
                strMapType  = CTD_LVL_ELEM_MAP_TYPE_ABT;

            } else {

                strMapType = pcBufName;

            }

        }

        // load abt module
        if ( strMapType == CTD_LVL_ELEM_MAP_TYPE_ABT ) {

            pkRoom = NeoEngine::Core::Get()->GetRoomManager()->CreateRoom( "abt" );

        } else {

            // load bsp module
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
        m_pkLevelSet->m_pkRoom  = pkRoom;

        bHasMap                 = true;

    } else {

        pkRoom = m_pkLevelSet->m_pkRoom;

    }

    // look for loading picture definition
    pkNode = pkLevelElement->FirstChild( CTD_LVL_ELEM_LOADINGPIC );
    if ( pkNode ) {

        pkLoadingpicElement = pkNode->ToElement();
        pcBufName = (char*)pkLoadingpicElement->Attribute( CTD_LVL_ATTR_NAME, NULL );
        // show loading picture
        ShowLoadingPicture( pcBufName );

     }

    // load map if desired
    //-------------------------------------------------------//
    if ( bHasMap ) {

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

    }

    // load and setup plugins
    //----------------------------------------------------------------------------//

    // create a new plugin manager
    if ( !m_pkLevelSet->m_pkPluginMgr ) {
    
        m_pkLevelSet->m_pkPluginMgr = new PluginManager;

    }

    unsigned int uiPluginCount = 0;
    unsigned int uiEntityCount = 0;
    // get plugin information
    for ( pkNode = pkLevelElement->FirstChild( CTD_LVL_ELEM_MODULE ); pkNode; pkNode = pkNode->NextSiblingElement( CTD_LVL_ELEM_MODULE ) ) {

        pkModuleElement = pkNode->ToElement();
        pcBufName = (char*)pkModuleElement->Attribute( CTD_LVL_ATTR_NAME, NULL );

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
            for ( pkNode2 = pkModuleElement->FirstChild( CTD_LVL_ELEM_PARAM ); pkNode2; pkNode2 = pkNode2->NextSiblingElement( CTD_LVL_ELEM_PARAM ) ) {

                pkModuleElement = pkNode2->ToElement();
                pcBufName  = (char*)pkModuleElement->Attribute( CTD_LVL_ATTR_NAME, NULL );
                pcBufType  = (char*)pkModuleElement->Attribute( CTD_LVL_ATTR_TYPE, NULL );
                pcBufValue = (char*)pkModuleElement->Attribute( CTD_LVL_ATTR_VALUE, NULL );

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
            pkNode3 = pkNode; // module pkNode
            for ( pkNode3 = pkNode3->FirstChild( CTD_LVL_ELEM_ENTITY ); pkNode3; pkNode3 = pkNode3->NextSiblingElement( CTD_LVL_ELEM_ENTITY ) ) {

                pkEntityElement = pkNode3->ToElement();

                // get entity name
                pcBufName = ( char* )pkEntityElement->Attribute( CTD_LVL_ATTR_NAME, NULL );

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
                pcBufName = (char*)pkEntityElement->Attribute( CTD_LVL_ATTR_INST_NAME, NULL );
                if ( pcBufName ) {
                    // set entity's instance name
                    ( pkEntity)->SetInstanceName( pcBufName );
                    neolog << LogLevel( INFO ) << "  CTD instance name: '" << pcBufName << " '" << endl;
                }

                for ( pkNode4 = pkEntityElement->FirstChild( CTD_LVL_ELEM_PARAM ); pkNode4; pkNode4 = pkNode4->NextSiblingElement( CTD_LVL_ELEM_PARAM ) ) {

                    pkEntityElement = pkNode4->ToElement();
                    pcBufName  = (char*)pkEntityElement->Attribute( CTD_LVL_ATTR_NAME, NULL );
                    pcBufType  = (char*)pkEntityElement->Attribute( CTD_LVL_ATTR_TYPE, NULL );
                    pcBufValue = (char*)pkEntityElement->Attribute( CTD_LVL_ATTR_VALUE, NULL );

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

                pkRoom->AttachNode( ( SceneNode* )pkEntity );
                // add the entity into plugin's entity list
                m_pkLevelSet->m_pkPluginMgr->AddEntity( pkPlugin, pkEntity );

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
