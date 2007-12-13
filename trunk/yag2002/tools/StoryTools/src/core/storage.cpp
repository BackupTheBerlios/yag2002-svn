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
 # io manager
 #
 #   date of creation:  12/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#include <main.h>
#include "storage.h"
#include "elementfactory.h"
#include <3rdparty/tinyxml/tinyxml.h>

//! File format tags
#define FF_ATTR_NAME            "Name"
#define FF_ATTR_TYPE            "Type"
#define FF_ATTR_VALUE           "Value"

#define FF_NODE_MAIN            "StoryCollection"
#define FF_NODE_STORY           "Story"
#define FF_NODE_STORY_ELEMENT   "Element"

#define FF_NODE_RENDER          "Render"
#define FF_ATTR_RENDER_POS      "Position"
#define FF_ATTR_RENDER_SCALE    "Scale"
#define FF_NODE_PROPS           "Properties"


//! Implement the render manager singleton
BEDITOR_SINGLETON_IMPL( beditor::Storage )

namespace beditor
{

Storage::Storage()
{
}

Storage::~Storage()
{
}

void Storage::initialize() throw( ... )
{
}

void Storage::shutdown()
{
    log_info << "Storage: shutting down" << std::endl;
    destroy();
}

void Storage::read( const std::string& filename, std::vector< BaseNodePtr >& stories ) throw( ... )
{
    // setup an xml document
    TiXmlDocument doc;
    doc.SetCondenseWhiteSpace( false );
    if ( !doc.LoadFile( filename.c_str() ) )
    {
        int row = doc.ErrorRow();
        int col = doc.ErrorCol();
        std::stringstream msg;
        msg << doc.ErrorDesc() << " Line " << row << " column " << col;
        log_error << "Storage: cannot load level file: '" << filename << "'" << std::endl;
        log_error << " reason: " << msg.str() << "." << std::endl;

        throw StorageException( "Problem reading file '" + filename + "'. " + msg.str() + "." );
    }

    // start evaluating the xml structure
    //---------------------------------//
    TiXmlNode*     p_node    = NULL;
    TiXmlElement*  p_element = NULL;
    char*          p_bufName = NULL;

    // check if any nodes exist in the file
    p_node = doc.FirstChild( FF_NODE_MAIN );
    if ( !p_node )
    {
        log_warning << "Storage: empty file" << std::endl;
        throw StorageException( "Empty story collection file '" + filename + "'" );
    }

    p_element = p_node->ToElement();
    p_bufName = ( char* )p_element->Attribute( FF_ATTR_NAME );
    if ( p_bufName == NULL ) 
    {
        log_error << "Storage: missing story collection name" << std::endl;
        throw StorageException( "Problem reading file '" + filename + "'. No valid story collection name." );
    }
    else
    {
        // find the first story
        p_node = p_node->FirstChild( FF_NODE_STORY );
    }

    TiXmlNode*      p_nodestory = p_node;
    unsigned int    storycnt = 0;
    do
    {
        if ( !p_nodestory )
            break;

        // increment the story count
        storycnt++;
        std::stringstream  storycntstr;
        storycntstr << storycnt;

        p_element = p_nodestory->ToElement();
        p_bufName = ( char* )p_element->Attribute( FF_ATTR_NAME );
        if ( p_bufName == NULL ) 
        {
            log_error << "Storage: Story " << storycntstr.str() << " has no name!" << std::endl;
            throw StorageException( "Problem reading file '" + filename + "'. Story " + storycntstr.str() + " has no valid name." );
        }

        // create a new story node
        BaseNodePtr story = new BaseNode( BaseNode::eTypeStory );

        try
        {
            story = ElementFactory::get()->createNode( ELEM_TYPE_STORY );
            story->setName( p_bufName );
        }
        catch ( std::exception& e )
        {
            log_error << "Storage: Story element " << storycntstr.str() << " has no name!" << std::endl;
            log_error << "  " << e.what() << std::endl;
            throw StorageException( "Problem reading file '" + filename + "'. Story element " + storycntstr.str() + " has no valid element type '" + std::string( ELEM_TYPE_STORY ) + "'." );
        }

        // get the story elements
        TiXmlNode* p_nodestoryelement = p_nodestory->FirstChild( FF_NODE_STORY_ELEMENT );

        do
        {
            // check for empty story
            if ( !p_nodestoryelement )
                break;

            // get element type and name
            p_element = p_nodestoryelement->ToElement();
            char* p_bufType = ( char* )p_element->Attribute( FF_ATTR_TYPE );
            char* p_bufName = ( char* )p_element->Attribute( FF_ATTR_NAME );
            if ( !p_bufName || !p_bufType || !std::string( p_bufName ).length() )
            {
                log_error << "Storage: Incomplete story element " << storycntstr.str() << std::endl;
                //throw StorageException( "Problem reading file '" + filename + "'. Incomplete story element " + storycntstr.str() + "." );
                continue;
            }

            // create a new story element of given type
            BaseNodePtr storyelement;
            try
            {
                storyelement = ElementFactory::get()->createNode( p_bufType );
                storyelement->setName( p_bufName );
            }
            catch ( std::exception& e )
            {
                log_error << "Storage: Story element " << storycntstr.str() << " has no name!" << std::endl;
                log_error << "  " << e.what() << std::endl;
                throw StorageException( "Problem reading file '" + filename + "'. Story element " + storycntstr.str() + " has no valid element type '" + std::string( p_bufType ) + "'." );
            }

            // append the element to story
            story->addChild( storyelement.getRef() );

            TiXmlNode* p_elementrender = p_nodestoryelement->FirstChild( FF_NODE_RENDER );
            if ( !p_elementrender )
            {
                log_warning << "Storage: Story element " << storycntstr.str() << " has no rendering information!" << std::endl;
            }
            else
            {
                float posX = 0.0f, posY = 0.0f, scaleX = 1.0f, scaleY = 1.0f;

                TiXmlElement* p_render = p_elementrender->ToElement();
                char* p_bufPos   = ( char* )p_render->Attribute( FF_ATTR_RENDER_POS );
                char* p_bufScale = ( char* )p_render->Attribute( FF_ATTR_RENDER_SCALE );
                if ( !p_bufPos || !p_bufScale ) 
                {
                    log_warning << "Storage: Story element " << storycntstr.str() << " has no rendering details!" << std::endl;
                }
                else
                {
                    // pick up the position and scale
                    std::stringstream attrpos;
                    attrpos << p_bufPos;
                    attrpos >> posX >> posY;

                    std::stringstream attrscale;
                    attrscale << p_bufScale;
                    attrscale >> scaleX >> scaleY;
                }

                storyelement->setPosition( Eigen::Vector3f( posX, posY, 0.0f ) );
                storyelement->setScale( Eigen::Vector3f( scaleX, scaleY, 1.0f ) );

            }

            //! TODO: implement the serialization of element properties
            TiXmlNode* p_elementprops = p_nodestoryelement->FirstChild( FF_NODE_PROPS );


            // get the next story element
            p_nodestoryelement = p_nodestoryelement->NextSiblingElement( FF_NODE_STORY_ELEMENT );
        }
        while( p_nodestoryelement );

        p_nodestory = p_nodestory->NextSiblingElement( FF_NODE_STORY );
        // append the top node to stories container
        stories.push_back( story );
    }
    while ( p_nodestory );

    // clear the xml document
    doc.Clear();
}

void Storage::write( const std::string& filename, std::vector< BaseNodePtr >& stories ) throw( ... )
{
}

} // namespace beditor
