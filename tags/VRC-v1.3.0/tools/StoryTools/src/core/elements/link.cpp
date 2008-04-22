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
 # link node used for building the node relationships
 #
 #   date of creation:  18/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#include <main.h>
#include <core/node.h>
#include "link.h"
#include "linkrenderer.h"


ELEMENT_REGISTRY_IMPL( beditor::RegistryLink, ELEM_TYPE_LINK )

namespace beditor
{

NodeLink::NodeLink() :
 BaseNode( BaseNode::eTypeLink )
{
    _p_nodeRenderer = new NodeRendererLink;
}

NodeLink::~NodeLink()
{
}

void NodeLink::setSourceDestination( BaseNodePtr src, BaseNodePtr dest )
{
    if ( _p_nodeRenderer )
        dynamic_cast< NodeRendererLink* >( _p_nodeRenderer )->setSourceDestination( src, dest );
}

void NodeLink::getSourceDestination( BaseNodePtr& src, BaseNodePtr& dest )
{
    if ( _p_nodeRenderer )
        dynamic_cast< NodeRendererLink* >( _p_nodeRenderer )->getSourceDestination( src, dest );
}

bool NodeLink::isAssociated( BaseNodePtr node ) const
{
    if ( !_p_nodeRenderer )
        return false;

    NodeRendererLink* p_renderer = dynamic_cast< NodeRendererLink* >( _p_nodeRenderer );
    assert( p_renderer );

    if ( ( p_renderer->getSource().getRef() == node.getRef() ) || 
         ( p_renderer->getDestination().getRef() == node.getRef() ) )
         return true;

    return false;
}

} // namespace beditor
