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
#include "rendermanager.h"

//! Implement the render manager singleton
BEDITOR_SINGLETON_IMPL( beditor::RenderManager )

namespace beditor
{

RenderManager::RenderManager()
{
}

RenderManager::~RenderManager()
{
}

void RenderManager::setGLCanvas( wxGLCanvas* p_canvas )
{
    _p_canvas = p_canvas;
}

wxGLCanvas* RenderManager::getGLCanvas()
{
    return _p_canvas;
}

void RenderManager::initialize()
{
    assert( _p_canvas && "the canvas is not set!" );
}

void RenderManager::shutdown()
{
    log_info << "RenderManager: shutting down" << std::endl;

    // remove the children before destroying
    if ( _topNode.getRef() )
        _topNode->removeChildren();

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

void RenderManager::highlightNodes( std::vector< BaseNodePtr >& nodes )
{
    if ( !_topNode.getRef() )
        return;

    _topNode->highlight( nodes );
}

void RenderManager::renderScene( const wxPoint& panPosition, const wxSize& viewSize )
{
    if ( !_topNode.getRef() )
        return;

    // set the GL context
    _p_canvas->SetCurrent();

    glClearColor( 1.0f, 1.0f, 1.0f, 1.0f );
    glClear( GL_COLOR_BUFFER_BIT );

    // setup the view port
    glMatrixMode( GL_PROJECTION );
    glLoadIdentity();
    glViewport( 0, 0, viewSize.x, viewSize.y );

    // setup the orthogonal view
    float left   = -viewSize.x / 2.0f;
    float right  =  viewSize.x / 2.0f;
    float buttom = -viewSize.y / 2.0f;
    float top    =  viewSize.y / 2.0f;
    glOrtho( left, right, buttom, top, -1.0, 1.0 );

    // reset the model matrix
    glMatrixMode( GL_MODELVIEW );
    glLoadIdentity();

    //! TODO: any HUD here?
    //glColor3f( 0.0, 1.0, .0 );
    //glBegin( GL_QUADS );
    //    glVertex3f(left, top, 0.0f);     // Left And Up 1 Unit (Top Left) 
    //    glVertex3f(right, top, 0.0f);    // Right And Up 1 Unit (Top Right) 
    //    glVertex3f(right, buttom, 0.0f); // Right And Down One Unit (Bottom Right) 
    //    glVertex3f(left, buttom, 0.0f);  // Left And Down One Unit (Bottom Left) 
    //glEnd();

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

} // namespace beditor
