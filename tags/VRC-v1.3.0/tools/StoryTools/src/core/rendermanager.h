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

        //! Delete given nodes
        void                                    deleteNodes( std::vector< BaseNodePtr >& nodes );

        //! Traverse all nodes in scene and highlight the given nodes with given colour.
        void                                    highlightNodes( std::vector< BaseNodePtr >& nodes, const Eigen::Vector3f& colour = Eigen::Vector3f( 1.0f, 0.0f, 0.0f ) );

        //! Get the current highlight colour.
        const Eigen::Vector3f&                  getHeighlightColour() const;

        //! Render all nodes in scene
        void                                    renderScene( const wxPoint& panPosition, const wxSize& viewSize );

        //! Get the top node of render graph
        BaseNodePtr                             getTopNode();

        //! Set a new top node. The old one is released.
        void                                    setTopNode( BaseNodePtr topnode );

        //! Set the default font size
        void                                    setDefaultFontSize( float size );

        //! Get the default font size
        float                                   getDefaultFontSize() const;

        //! Given a text, get its needed dimensions for rendering
        void                                    fontGetDims( const std::string& text, Eigen::Vector3f& min, Eigen::Vector3f& max );

        //! Render the given text. If size is 0 then the default size is used.
        void                                    fontRender( const std::string& text, const Eigen::Vector2f& pos, float size = 0.0f );

        //! Set the GL canvas, no drawing takes place until the canvas is set. This method is used by draw panel class only!
        void                                    setGLCanvas( wxGLCanvas* p_canvas );

    protected:

                                                RenderManager();

        virtual                                 ~RenderManager();

        //! Initialize the render manager
        void                                    initialize();

        //! Shutdown the render manager
        void                                    shutdown();

        //! Render graph's top node
        BaseNodePtr                             _topNode;

        //! OpenGL canvas of the draw panel
        wxGLCanvas*                             _p_canvas;

        //! Font renderer object
        FTGLPixmapFont*                         _p_font;

        //! Font size
        float                                   _fontSize;

        //! Highlighting colour
        Eigen::Vector3f                         _highlightColour;

    friend class Core;
    friend class Singleton< RenderManager >;
};

} // namespace beditor

#endif // _RENDERMANAGER_H_
