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
 # node renderer is responsible for visualizing an event node
 #
 #   date of creation:  08/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _NODERENDEREREVENT_H_
#define _NODERENDEREREVENT_H_

#include <main.h>
#include <core/noderenderer.h>

namespace beditor
{

//! Node renderer for event node
class NodeRendererEvent : public NodeRenderer
{
    public:

        //! Construct the renderer
                                                NodeRendererEvent();

    protected:

        virtual                                 ~NodeRendererEvent();

        //! Setup the visualization geometry, this should be implemented by derived classes
        virtual void                            setupGeometry();

        //! Render method, this should be implemented by derived classes
        virtual void                            render( const Eigen::Matrix4f& view, const Eigen::Matrix4f& model );

    friend class BaseNode;
};

} // namespace beditor

#endif // _NODERENDEREREVENT_H_
