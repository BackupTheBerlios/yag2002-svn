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
 # a node describes any kind of logical block
 #
 #   date of creation:  07/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#include <main.h>
#include "node.h"
#include "noderenderer.h"


namespace beditor
{

BaseNode::BaseNode( unsigned int type ) :
 _type( type ),
 _p_nodeRenderer( NULL )
{
}

BaseNode::~BaseNode()
{
    if ( _p_nodeRenderer )
        delete _p_nodeRenderer;

    removeChildren();
}

void BaseNode::setName( const std::string& name )
{
    _name = name;
    if ( _p_nodeRenderer )
        _p_nodeRenderer->setText( name );
}

const std::string& BaseNode::getName() const
{
    return _name;
}

unsigned int BaseNode::getType() const
{
    return _type;
}

std::string BaseNode::getTypeAsString() const
{
    std::string type;
    switch( _type )
    {
        case eTypeStory:
            type = ELEM_TYPE_STORY;
            break;

        case eTypeLink:
            type = ELEM_TYPE_LINK;
            break;

        case eTypeExecution:
            type = ELEM_TYPE_EXECUTION;
            break;

        case eTypeOperation:
            type = ELEM_TYPE_OPERATION;
            break;

        case eTypeDialog:
            type = ELEM_TYPE_DIALOG;
            break;

        case eTypeEvent:
            type = ELEM_TYPE_EVENT;
            break;

        default:
            type = "__unknown__";
    }

    return type;
}

bool BaseNode::hit( const Eigen::Vector2f& pos, std::vector< BaseNodePtr >* p_nodes )
{
    Eigen::Vector3f hitpos( pos.x(), pos.y(), 0.0f );
    return hit( hitpos, p_nodes );
}

bool BaseNode::hit( const Eigen::Vector3f& pos, std::vector< BaseNodePtr >* p_nodes )
{
    bool hitfound = false;

    // first check this node
    if ( _p_nodeRenderer && _p_nodeRenderer->hit( pos ) )
    {
        if ( p_nodes && !findNode( *p_nodes ) )
            p_nodes->push_back( BaseNodePtr( this ) );

        hitfound = true;
    }

    // now traverse all children
    std::vector< BaseNodePtr >::iterator p_currchild = _children.begin(), p_end = _children.end();
    for ( ; p_currchild != p_end; ++p_currchild )
    {
        if ( ( *p_currchild )->hit( pos, p_nodes ) )
            hitfound = true;
    }

    return hitfound;
}

bool BaseNode::addChild( BaseNode* p_child )
{
    std::vector< BaseNodePtr >::iterator p_currchild = _children.begin(), p_end = _children.end();
    for ( ; p_currchild != p_end; ++p_currchild )
    {
        if ( p_currchild->getRef() == p_child )
        {
            assert( NULL && "child already exists!" );
            return false;
        }
    }

    _children.push_back( BaseNodePtr( p_child ) );
    p_child->addParent( this );

    return true;
}

bool BaseNode::removeChild( BaseNode* p_child )
{
    std::vector< BaseNodePtr >::iterator p_currchild = _children.begin(), p_end = _children.end();
    for ( ; p_currchild != p_end; ++p_currchild )
    {
        if ( p_currchild->getRef() == p_child )
            break;
    }

    if ( p_currchild != p_end )
    {
        ( *p_currchild )->removeParent( this );
        _children.erase( p_currchild );
    }
    else
    {
        assert( NULL && "cound not find child to remove!" );
        return false;
    }

    return true;
}

void BaseNode::removeChildren()
{
    std::vector< BaseNodePtr >::iterator p_currchild = _children.begin(), p_end = _children.end();
    for ( ; p_currchild != p_end; ++p_currchild )
        ( *p_currchild )->removeParent( this );

    _children.clear();
}

bool BaseNode::addParent( BaseNode* p_parent )
{
    std::vector< BaseNodePtr >::iterator p_currparent = _parents.begin(), p_end = _parents.end();
    for ( ; p_currparent != p_end; ++p_currparent )
    {
        if ( p_currparent->getRef() == p_parent )
        {
            assert( NULL && "parent already exists!" );
            return false;
        }
    }

    _parents.push_back( BaseNodePtr( p_parent ) );
    return true;
}

bool BaseNode::removeParent( BaseNode* p_node )
{
    std::vector< BaseNodePtr >::iterator p_currparent = _parents.begin(), p_end = _parents.end();
    for ( ; p_currparent != p_end; ++p_currparent )
    {
        if ( p_currparent->getRef() == p_node )
            break;
    }

    if ( p_currparent != p_end )
    {
        _parents.erase( p_currparent );
    }
    else
    {
        assert( NULL && "cound not find node in parent list to remove!" );
        return false;
    }

    return true;
}

std::vector< BaseNodePtr >& BaseNode::getChildren()
{
    return _children;
}

std::vector< BaseNodePtr >& BaseNode::getParents()
{
    return _parents;
}

void BaseNode::setPosition( const Eigen::Vector3f& pos )
{
    if ( _p_nodeRenderer )
        _p_nodeRenderer->setPosition( pos );
}

Eigen::Vector3f BaseNode::getPosition() const
{
    if ( _p_nodeRenderer )
    {
        return _p_nodeRenderer->getPosition();
    }

    Eigen::Vector3f pos;
    pos.loadZero();
    return pos;
}

void BaseNode::setScale( const Eigen::Vector3f& scale )
{
    if ( _p_nodeRenderer )
        _p_nodeRenderer->setScale( scale );
}

Eigen::Vector3f BaseNode::getScale() const
{
    if ( _p_nodeRenderer )
    {
        return _p_nodeRenderer->getScale();
    }

    Eigen::Vector3f scale;
    scale.loadZero();
    return scale;
}

void BaseNode::highlight( std::vector< BaseNodePtr >& nodes )
{
    if ( _p_nodeRenderer )
    {
        if ( findNode( nodes ) )
            _p_nodeRenderer->highlight( true );
        else
            _p_nodeRenderer->highlight( false );
    }

    // highlight the children
    std::vector< BaseNodePtr >::iterator p_currchild = _children.begin(), p_end = _children.end();
    for ( ; p_currchild != p_end; ++p_currchild )
    {
        ( *p_currchild )->highlight( nodes );
    }
}

bool BaseNode::findNode( std::vector< BaseNodePtr >& nodes )
{
    std::vector< BaseNodePtr >::iterator p_node = nodes.begin(), p_end = nodes.end();
    for ( ; p_node != p_end; ++p_node )
        if ( p_node->getRef() == this )
            return true;

    return false;
}

void BaseNode::render( const Eigen::Matrix4f& view, BaseNode* p_parent )
{
    if ( _p_nodeRenderer )
        _p_nodeRenderer->render( view, p_parent );

    // draw the children
    std::vector< BaseNodePtr >::iterator p_currchild = _children.begin(), p_end = _children.end();
    for ( ; p_currchild != p_end; ++p_currchild )
    {
        ( *p_currchild )->render( view, this );
    }
}

} // namespace beditor
