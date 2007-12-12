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

#include <main.h>
#include "eventrenderer.h"

namespace beditor
{

NodeRendererEvent::NodeRendererEvent()
{
    // setup the bbox
    BBox* p_bbox = new BBox;
    p_bbox->setDimensions( Eigen::Vector3f( -10.0f, -10.0f, 0.0f ), Eigen::Vector3f( 10.0f, 10.0f, 0.0f ) );
    _bounds = p_bbox;
}

NodeRendererEvent::~NodeRendererEvent()
{
}

void NodeRendererEvent::setupGeometry()
{
}

void NodeRendererEvent::render( const Eigen::Matrix4f& view, const Eigen::Matrix4f& model )
{
    glPushMatrix();
    glMultMatrixf( ( model * view ).array() );

    glColor3f( 0.2f, 0.6f, 0.2f );
    glBegin( GL_QUADS );
                glVertex3f( -10.0f, 10.0f, 0.0f );
                glVertex3f(  10.0f, 10.0f, 0.0f );
                glVertex3f(  10.0f,-10.0f, 0.0f );
                glVertex3f( -10.0f,-10.0f, 0.0f );
    glEnd();

    if ( _isHighlighted )
    {
        glColor3f( 1.0f, 0.0f, 0.0f );
        float restorelw;
        glGetFloatv( GL_LINE_WIDTH, &restorelw );
        glLineWidth( 2.0f );
        glBegin( GL_LINE_STRIP );
                    glVertex3f( -10.0f, 10.0f, 0.0f );
                    glVertex3f(  10.0f, 10.0f, 0.0f );
                    glVertex3f(  10.0f,-10.0f, 0.0f );
                    glVertex3f( -10.0f,-10.0f, 0.0f );
                    glVertex3f( -10.0f, 10.0f, 0.0f );
        glEnd();
        glLineWidth( restorelw );
    }

    glPopMatrix();
}

} // namespace beditor
