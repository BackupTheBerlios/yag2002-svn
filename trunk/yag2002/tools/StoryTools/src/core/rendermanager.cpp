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
 # render manager
 #
 #   date of creation:  08/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#include <main.h>
#include "settings.h"
#include "rendermanager.h"
#include "elements/link.h"

#define FONT_DEFAULT        "kimbalt_.ttf"
#define FONT_DEPTH          10.0f
#define FONT_DEFAULT_SIZE   12.0f

//! Implement the render manager singleton
BEDITOR_SINGLETON_IMPL( beditor::RenderManager )


namespace beditor
{

RenderManager::RenderManager() :
 _p_font( NULL ),
 _fontSize( FONT_DEFAULT_SIZE ),
 _highlightColour( Eigen::Vector3f( 1.0f, 0.0f, 0.0f ) )
{
}

RenderManager::~RenderManager()
{
}

void RenderManager::initialize()
{
    assert( _p_canvas && "the canvas is not set!" );
    assert( ( _p_font == NULL ) && "render manager seems to be already intitialized!" );

    std::string fontfile;
    CFG_GET_VALUE( CFG_FONT, fontfile );
    if ( !fontfile.length() )
    {
        fontfile = FONT_DEFAULT;
        CFG_SET_VALUE( CFG_FONT, fontfile );
    }

    std::auto_ptr< FTGLPixmapFont > p_font( new FTGLPixmapFont( fontfile.c_str() ) );
    if ( p_font->Error() )
    {
        log_error << "RenderManager: failed to open font: " << fontfile << std::endl;
        return;
    }
    else
    {
        if( !p_font->FaceSize( 12 ) )
        {
            log_error << "RenderManager: failed to set font size: " << fontfile << std::endl;
            return;
        }

        p_font->CharMap( ft_encoding_unicode );
    }

    _p_font = p_font.release();
}

void RenderManager::shutdown()
{
    log_info << "RenderManager: shutting down" << std::endl;

    // remove the children before destroying
    if ( _topNode.getRef() )
        _topNode->removeChildren();

    if ( _p_font )
        delete _p_font;

    destroy();
}

BaseNodePtr RenderManager::getTopNode()
{
    return _topNode;
}

void RenderManager::setTopNode( BaseNodePtr topnode )
{
    _topNode = topnode;
}

unsigned int RenderManager::selectNodesByHit( const Eigen::Vector3f& hitposition, std::vector< BaseNodePtr >& nodes )
{
    if ( !_topNode.getRef() )
        return 0;

    std::size_t nodecount = nodes.size();
    if ( !_topNode->hit( hitposition, &nodes ) )
        return 0;

    return ( unsigned int )( nodes.size() - nodecount );
}

void RenderManager::deleteNodes( std::vector< BaseNodePtr >& nodes )
{
    if ( !_topNode.getRef() )
        return;

    std::vector< BaseNodePtr >::iterator p_node = nodes.begin(), p_end = nodes.end();
    for ( ; p_node != p_end; ++p_node )
    {
        // check if any links are asscociated with node
        std::vector< BaseNodePtr > childrenlist = _topNode->getChildren();
        std::vector< BaseNodePtr >::iterator p_childnode = childrenlist.begin(), p_childEnd = childrenlist.end();
        for ( ; p_childnode != p_childEnd; ++p_childnode )
        {
            if ( ( *p_childnode )->getType() == BaseNode::eTypeLink )
            {
                NodeLink* p_link = dynamic_cast< NodeLink* >( p_childnode->getRef() );
                if ( p_link->isAssociated( *p_node ) )
                {
                    _topNode->removeChild( p_link );
                }
            }
        }

        // remove the node
        _topNode->removeChild( p_node->getRef() );
    }


    nodes.clear();
}

void RenderManager::highlightNodes( std::vector< BaseNodePtr >& nodes, const Eigen::Vector3f& colour )
{
    _highlightColour = colour;

    if ( !_topNode.getRef() )
        return;

    _topNode->highlight( nodes );
}

const Eigen::Vector3f& RenderManager::getHeighlightColour() const
{
    return _highlightColour;
}

void RenderManager::renderScene( const wxPoint& panPosition, const wxSize& viewSize )
{
    if ( !_topNode.getRef() )
        return;

    // set the GL context
    _p_canvas->SetCurrent();

    glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
    glEnable( GL_DEPTH_TEST );
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // setup the view port
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glViewport( 0, 0, viewSize.x, viewSize.y );

    // setup the orthogonal view
    float left   = -viewSize.x / 2.0f;
    float right  =  viewSize.x / 2.0f;
    float buttom = -viewSize.y / 2.0f;
    float top    =  viewSize.y / 2.0f;
    glOrtho( left, right, buttom, top, -100.0, 100.0 );

    // reset the model matrix
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    // render the current top node name
    {
        std::string text( "Story: " );
        text += _topNode->getName();
        Eigen::Vector2f namepos( -float( viewSize.x / 2.0f ) + 5.0, float( viewSize.y / 2.0f ) - 15.0f );
        glColor3f( 0.42f, 0.4f, 0.4f );
        fontRender( text, namepos, 14.0f );
    }

    // set the camera position
    Eigen::Matrix4f view;
    view.loadIdentity();
    view( 0, 3 ) = panPosition.x;
    view( 1, 3 ) = panPosition.y;
    glLoadMatrixf( view.array() );

    // recursively render the nodes
    view.loadIdentity();
    _topNode->render( view );

    // flush the render pipeline and swap buffers
    glFlush();
    _p_canvas->SwapBuffers();
}

void RenderManager::fontGetDims( const std::string& text, Eigen::Vector3f& min, Eigen::Vector3f& max )
{
    if ( !_p_font )
        return;

    _p_font->BBox( text.c_str(), min.x(), min.y(), min.z(), max.x(), max.y(), max.z() );
}

void RenderManager::fontRender( const std::string& text, const Eigen::Vector2f& pos, float size )
{
    if ( !_p_font )
        return;

    if ( size > 0.0f )
        _p_font->FaceSize( size );
    else
        _p_font->FaceSize( _fontSize );

    glRasterPos3f( pos.x(), pos.y(), FONT_DEPTH );
    _p_font->Render( text.c_str() );
}

void RenderManager::setDefaultFontSize( float size )
{
    _fontSize = size;
}

float RenderManager::getDefaultFontSize() const
{
    return _fontSize;
}

void RenderManager::setGLCanvas( wxGLCanvas* p_canvas )
{
    _p_canvas = p_canvas;
}

} // namespace beditor
