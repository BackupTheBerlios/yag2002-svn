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

#include <main.h>
#include "noderenderer.h"

namespace beditor
{

NodeRenderer::NodeRenderer() :
 _objectList( -1 ),
_isHighlighted( false )
{
    _position.loadZero();
    _scale = Eigen::Vector3f( 1.0f, 1.0f, 1.0f );
}

NodeRenderer::~NodeRenderer()
{
}

void NodeRenderer::render( const Eigen::Matrix4f& view )
{
    // setup the model matrix
    Eigen::Matrix4f model;
    model.loadIdentity();
    Eigen::Vector4f scale( _scale.x(), _scale.y(), _scale.z(), 1.0f );
    Eigen::Vector4f pos( _position.x(), _position.y(), _position.z(), 1.0f );
    model.loadScaling( scale );
    model.setColumn( 3, pos );

    render( view, model );
}

void NodeRenderer::setPosition( const Eigen::Vector3f& pos )
{
    _position = pos;
}

const Eigen::Vector3f& NodeRenderer::getPosition() const
{
    return _position;
}

void NodeRenderer::setScale( const Eigen::Vector3f& scale )
{
    _scale = scale;
}

const Eigen::Vector3f& NodeRenderer::getScale() const
{
    return _scale;
}

bool NodeRenderer::hit( const Eigen::Vector3f& hitpos ) const
{
    if ( _bounds.getRef() )
        return _bounds->contains( _scale, _position, hitpos );

    return false;
}

void NodeRenderer::highlight( bool en )
{
    _isHighlighted = en;
}

} // namespace beditor
