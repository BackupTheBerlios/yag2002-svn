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

#include <main.h>
#include "linkrenderer.h"

namespace beditor
{

NodeRendererLink::NodeRendererLink()
{
    // setup the bounding volume
    _bounds = new BLine;
}

NodeRendererLink::~NodeRendererLink()
{
}

void NodeRendererLink::setSourceDestination( BaseNodePtr src, BaseNodePtr dest )
{
    _source      = src;
    _destination = dest;

    BLine* p_bline = dynamic_cast< BLine* >( _bounds.getRef() );
    p_bline->setSourceDestination( src, dest );
}

void NodeRendererLink::getSourceDestination( BaseNodePtr& src, BaseNodePtr& dest )
{
    src = _source;
    dest =_destination;
}

BaseNodePtr NodeRendererLink::getSource()
{
    return _source;
}

BaseNodePtr NodeRendererLink::getDestination()
{
    return _destination;
}

void NodeRendererLink::render( const Eigen::Matrix4f& view, const Eigen::Matrix4f& model )
{
    Eigen::Vector3f srcpos( 0.0f, 0.0f, 0.0f );
    Eigen::Vector3f destpos( 0.0f, 0.0f, 0.0f );

    if ( _source.getRef() )
        srcpos = _source->getPosition();

    if ( _destination.getRef() )
        destpos = _destination->getPosition();

    glPushMatrix();

    if ( _isHighlighted )
        glColor3f( 1.0f, 0.5f, 0.4f );
    else
        glColor3f( 0.0f, 0.0f, 0.0f );

    float restorelw;
    glGetFloatv( GL_LINE_WIDTH, &restorelw );
    glLineWidth( 2.0f );

    // draw the connection line
    glBegin( GL_LINES );
        glVertex3f( srcpos.x(), srcpos.y(), -1.0f );
        glVertex3f( destpos.x(), destpos.y(), -1.0f );
    glEnd();

    // draw the direction triangle
    Eigen::Vector3f dist( ( destpos - srcpos ) * 0.5f );
    Eigen::Vector3f tripos = srcpos + dist;
    float angle = atan2( dist.y(), dist.x() );
    glTranslatef( tripos.x(), tripos.y(), tripos.z() );
    glRotatef( angle * 180.0f / M_PI + 90.0f, 0.0f, 0.0f, 1.0f );
    glBegin( GL_TRIANGLES );
        glVertex3f( -5.0f, 0.0f, -1.0 );
        glVertex3f( 5.0f, 0.0f, -1.0 );
        glVertex3f( 0.0f, - 10.0f, -1.0 );
    glEnd();

    glLineWidth( restorelw );
    glPopMatrix();
}

} // namespace beditor
