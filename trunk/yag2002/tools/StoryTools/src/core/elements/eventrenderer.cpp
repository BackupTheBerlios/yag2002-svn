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

#define GEOM_WIDTH      40.0f
#define GEOM_HEIGHT     20.0f

namespace beditor
{

NodeRendererEvent::NodeRendererEvent()
{
    // setup the bbox
    BBox* p_bbox = new BBox;
    p_bbox->setDimensions( Eigen::Vector3f( -GEOM_WIDTH, -GEOM_HEIGHT, 0.0f ), Eigen::Vector3f( GEOM_WIDTH, GEOM_HEIGHT, 0.0f ) );
    _bounds = p_bbox;
}

NodeRendererEvent::~NodeRendererEvent()
{
}

void NodeRendererEvent::render( const Eigen::Matrix4f& view, const Eigen::Matrix4f& model )
{
    glPushMatrix();
    glMultMatrixf( ( model * view ).array() );

    glColor3f( 1.0f, 1.0f, 0.0f );

    // render a filled elipse
    glBegin( GL_POLYGON );
    for ( float step = 0.0f; step < 2.0f * M_PI; step += 0.1f )
    {
        float x = GEOM_WIDTH * sinf( step );
        float y = GEOM_HEIGHT * cosf( step );
        glVertex3f( x, y, 0.0f );
    }
    glEnd();

    // render the border
    if ( _isHighlighted )
    {
        glColor3f( 1.0f, 0.0f, 0.0f );
    }
    else
    {
        glColor3f( 0.3f, 0.3f, 0.0f );
    }

    glLineWidth( 8.0f );
    glBegin( GL_LINE_LOOP );
    for ( float step = 0.0f; step < 2.0f * M_PI; step += 0.05f )
    {
        float x = ( GEOM_WIDTH + 0.9f ) * sinf( step );
        float y = ( GEOM_HEIGHT + 0.9f ) * cosf( step );
        glVertex3f( x, y, 0.0f );
    }
    glEnd();

    // render the node text
    glColor3f( 0.0, 0.0, 0.0 );
    std::string text( getText() );
    if ( text.length() )
    {
        Eigen::Vector3f tmin, tmax;
        RenderManager::get()->fontGetDims( text.c_str(), tmin, tmax );

        float xpos = tmax.x() - tmin.x();
        float ypos = tmax.y() - tmin.y();
        if ( getScale().x() )
            xpos /= getScale().x();

        if ( getScale().y() )
            ypos /= getScale().y();

        Eigen::Vector2f pos( -xpos / 2.0f, -ypos / 2.0f );
        RenderManager::get()->fontRender( text.c_str(), pos );
    }

    glPopMatrix();
}

} // namespace beditor
