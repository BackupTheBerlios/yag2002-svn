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
 # this node renderer is responsible for visualizing a link node
 #
 #   date of creation:  18/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _NODERENDERERLINK_H_
#define _NODERENDERERLINK_H_

#include <main.h>
#include <core/noderenderer.h>

namespace beditor
{

//! Node renderer for link node
class NodeRendererLink : public NodeRenderer
{
    public:

        //! Construct the renderer
                                                NodeRendererLink();

        //! Set the source and destination nodes
        void                                    setSourceDestination( BaseNodePtr src, BaseNodePtr dest );

        //! Get the source and destination nodes
        void                                    getSourceDestination( BaseNodePtr& src, BaseNodePtr& dest );

        //! Get source node
        BaseNodePtr                             getSource();

        // Get destination node
        BaseNodePtr                             getDestination();

    protected:

        virtual                                 ~NodeRendererLink();

        //! Render method
        virtual void                            render( const Eigen::Matrix4f& view, const Eigen::Matrix4f& model );

        //! Source of the link
        BaseNodePtr                             _source;

        //! Destination of the link
        BaseNodePtr                             _destination;

    friend class BaseNode;
};

} // namespace beditor

#endif // _NODERENDERERLINK_H_
