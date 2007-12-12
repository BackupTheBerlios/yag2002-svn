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
 # node renderer is responsible for visualizing a node
 #
 #   date of creation:  07/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _NODERENDERER_H_
#define _NODERENDERER_H_

#include <main.h>
#include <core/boundingvolume.h>

namespace beditor
{

class BaseNode;

//! Node renderer
class NodeRenderer
{
    public:

        //! Construct the node renderer
                                                NodeRenderer();

        //! Render the node shape
        void                                    render( const Eigen::Matrix4f& view );

        //! Set node position
        void                                    setPosition( const Eigen::Vector3f& pos );

        //! Get node position
        const Eigen::Vector3f&                  getPosition() const;

        //! Set node scale
        void                                    setScale( const Eigen::Vector3f& scale );

        //! Get node scale
        const Eigen::Vector3f&                  getScale() const;

        //! 3D hit test
        bool                                    hit( const Eigen::Vector3f& hitpos ) const;

        //! Enable / disable highlighting the node.
        void                                    highlight( bool en );

    protected:

        virtual                                 ~NodeRenderer();

        //! Setup the visualization geometry, this should be implemented by derived classes
        virtual void                            setupGeometry() = 0;

        //! Render method, this should be implemented by derived classes
        virtual void                            render( const Eigen::Matrix4f& view, const Eigen::Matrix4f& model ) = 0;

        //! Position of the node
        Eigen::Vector3f                         _position;

        //! Scale of the node
        Eigen::Vector3f                         _scale;

        //! Bounding volume
        BoundingVolumePtr                       _bounds;

        //! GL object list, which must be setup by every concrete NodeRenderer
        GLint                                   _objectList;

        //! Flag showing if the node is to be drawn with highlight
        bool                                    _isHighlighted;

    friend class BaseNode;
};

} // namespace beditor

#endif // _NODERENDERER_H_
