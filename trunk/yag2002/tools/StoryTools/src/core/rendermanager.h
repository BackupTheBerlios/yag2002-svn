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

#ifndef _RENDERMANAGER_H_
#define _RENDERMANAGER_H_

#include <main.h>
#include "node.h"

namespace beditor
{

//! Renderer manager is responsible for rendering the scene nodes.
class RenderManager : public Singleton< RenderManager >
{
    public:

//! TODO
/*
        //! Set the draw view size ( e.g. the panel size )
        void                                    setViewSize( unsigned int x, unsigned int y );

        //! Set the rectangular zoom area; set it to ( 0, 0, view size X, view size Y ) in order to get the full area, or use reset zoom
        void                                    setZoomRect( unsigned int left, unsigned int up, unsigned int right, unsigned int down );

        //! Reset the  current zoom to full view area
        void                                    resetZoom();
*/

        //! Select nodes by hit check at given position. Vector 'nodes' contains all hit nodes. Return count of hit nodes.
        unsigned int                            selectNodesByHit( const Eigen::Vector3f& hitposition, std::vector< BaseNodePtr >& nodes );

        //! Traverse all nodes in scene and highlight the given nodes.
        void                                    highlightNodes( std::vector< BaseNodePtr >& nodes );

        //! Render all nodes in scene
        void                                    renderScene( const wxPoint& panPosition, const wxSize& viewSize );

        //! Get the top node of render graph
        BaseNodePtr                             getTopNode();

        //! Set a new top node. The old one is released.
        void                                    setTopNode( BaseNodePtr topnode );

        //! Set the GL canvas, no drawing takes place until the canvas is set. This method is used by draw panel class only!
        void                                    setGLCanvas( wxGLCanvas* p_canvas );

        //! Get the GL canvas
        wxGLCanvas*                             getGLCanvas();

    protected:

                                                RenderManager();

        virtual                                 ~RenderManager();

        void                                    initialize();

        void                                    shutdown();

        //! Render graph's top node
        BaseNodePtr                             _topNode;

        //! OpenGL canvas of the draw panel
        wxGLCanvas*                             _p_canvas;

    friend class Core;
    friend class Singleton< RenderManager >;
};

} // namespace beditor

#endif // _RENDERMANAGER_H_
