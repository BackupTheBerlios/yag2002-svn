/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2006, A. Botorabi
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
 # graphical user interface renderer
 #
 #  this code is basing on CEGUI's opengl renderer implementation
 #
 #   date of creation:  04/04/2005
 #
 #   author:            ali botorabi (boto)
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>
#include "log.h"
#include "guitexture.h"
#include "guirenderer.h"
#include <GL/glu.h>
#include <osg/GLExtensions>

namespace yaf3d
{

const int GuiRenderer::VERTEX_PER_QUAD           = 6;
const int GuiRenderer::VERTEX_PER_TRIANGLE       = 3;
const int GuiRenderer::VERTEXBUFFER_CAPACITY     = OGLRENDERER_VBUFF_CAPACITY;

GuiRenderer::GuiRenderer( CEGUI::uint /*max_quads*/ ) :
_queueing( true ),
_currTexture( 0 ),
_bufferPos( 0 ),
_maxTextureSize( 0 )
{
    GLint vp[4];

    // initialise renderer size
    glGetIntegerv(GL_VIEWPORT, vp);
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &_maxTextureSize);
    _display_area.d_left    = 0;
    _display_area.d_top     = 0;
    _display_area.d_right   = (float)vp[2];
    _display_area.d_bottom  = (float)vp[3];
}

GuiRenderer::GuiRenderer( CEGUI::uint /*max_quads*/, int width, int height ) :
_queueing(true),
_currTexture(0),
_bufferPos(0)
{
    glGetIntegerv( GL_MAX_TEXTURE_SIZE, &_maxTextureSize );
    _display_area.d_left    = 0;
    _display_area.d_top     = 0;
    _display_area.d_right   = static_cast<float>( width );
    _display_area.d_bottom  = static_cast<float>( height );
}

GuiRenderer::~GuiRenderer()
{
    this->destroyAllTextures();
}

void GuiRenderer::changeDisplayResolution( float width, float height )
{
    _display_area.d_right   = width;
    _display_area.d_bottom  = height;
}

void GuiRenderer::addQuad( const CEGUI::Rect& dest_rect, float z, const CEGUI::Texture* tex, const CEGUI::Rect& texture_rect, const CEGUI::ColourRect& colours, CEGUI::QuadSplitMode quad_split_mode )
{
    // if not queuing, render directly (as in, right now!)
    if (!_queueing)
    {
        renderQuadDirect(dest_rect, z, tex, texture_rect, colours, quad_split_mode);
    }
    else
    {
        QuadInfo quad;
        quad.position           = dest_rect;
        quad.position.d_bottom  = _display_area.d_bottom - dest_rect.d_bottom;
        quad.position.d_top     = _display_area.d_bottom - dest_rect.d_top;
        quad.z                  = z;
        quad.texid              = ((GuiTexture*)tex)->getOGLTexid();
        quad.texPosition        = texture_rect;
        quad.topLeftCol         = colourToOGL(colours.d_top_left);
        quad.topRightCol        = colourToOGL(colours.d_top_right);
        quad.bottomLeftCol      = colourToOGL(colours.d_bottom_left);
        quad.bottomRightCol     = colourToOGL(colours.d_bottom_right);

        // set quad split mode
        quad.splitMode = quad_split_mode;

        _quadlist.insert(quad);
    }
}

void GuiRenderer::doRender()
{
    initPerFrameStates();
    glInterleavedArrays( GL_T2F_C4UB_V3F , 0, _buff );

    _currTexture = 0;

    // iterate over each quad in the list
    for (QuadList::iterator i = _quadlist.begin(); i != _quadlist.end(); ++i)
    {
        const QuadInfo& quad = (*i);

        if(_currTexture != quad.texid)
        {
            renderVBuffer();
            glBindTexture(GL_TEXTURE_2D, quad.texid);
            _currTexture = quad.texid;
        }

        //vert0
        _buff[_bufferPos].vertex[0] = quad.position.d_left;
        _buff[_bufferPos].vertex[1] = quad.position.d_top;
        _buff[_bufferPos].vertex[2] = quad.z;
        _buff[_bufferPos].color     = quad.topLeftCol;
        _buff[_bufferPos].tex[0]    = quad.texPosition.d_left;
        _buff[_bufferPos].tex[1]    = quad.texPosition.d_top;
        ++_bufferPos;

        //vert1
        _buff[_bufferPos].vertex[0] = quad.position.d_left;
        _buff[_bufferPos].vertex[1] = quad.position.d_bottom;
        _buff[_bufferPos].vertex[2] = quad.z;
        _buff[_bufferPos].color     = quad.bottomLeftCol;
        _buff[_bufferPos].tex[0]    = quad.texPosition.d_left;
        _buff[_bufferPos].tex[1]    = quad.texPosition.d_bottom;
        ++_bufferPos;

        //vert2

        // top-left to bottom-right diagonal
        if (quad.splitMode == CEGUI::TopLeftToBottomRight)
        {
            _buff[_bufferPos].vertex[0] = quad.position.d_right;
            _buff[_bufferPos].vertex[1] = quad.position.d_bottom;
            _buff[_bufferPos].vertex[2] = quad.z;
            _buff[_bufferPos].color     = quad.bottomRightCol;
            _buff[_bufferPos].tex[0]    = quad.texPosition.d_right;
            _buff[_bufferPos].tex[1]    = quad.texPosition.d_bottom;
        }
        // bottom-left to top-right diagonal
        else
        {
            _buff[_bufferPos].vertex[0] = quad.position.d_right;
            _buff[_bufferPos].vertex[1] = quad.position.d_top;
            _buff[_bufferPos].vertex[2] = quad.z;
            _buff[_bufferPos].color     = quad.topRightCol;
            _buff[_bufferPos].tex[0]    = quad.texPosition.d_right;
            _buff[_bufferPos].tex[1]    = quad.texPosition.d_top;
        }
        ++_bufferPos;

        //vert3
        _buff[_bufferPos].vertex[0] = quad.position.d_right;
        _buff[_bufferPos].vertex[1] = quad.position.d_top;
        _buff[_bufferPos].vertex[2] = quad.z;
        _buff[_bufferPos].color     = quad.topRightCol;
        _buff[_bufferPos].tex[0]    = quad.texPosition.d_right;
        _buff[_bufferPos].tex[1]    = quad.texPosition.d_top;
        ++_bufferPos;

        //vert4

        // top-left to bottom-right diagonal
        if (quad.splitMode == CEGUI::TopLeftToBottomRight)
        {
            _buff[_bufferPos].vertex[0] = quad.position.d_left;
            _buff[_bufferPos].vertex[1] = quad.position.d_top;
            _buff[_bufferPos].vertex[2] = quad.z;
            _buff[_bufferPos].color     = quad.topLeftCol;
            _buff[_bufferPos].tex[0]    = quad.texPosition.d_left;
            _buff[_bufferPos].tex[1]    = quad.texPosition.d_top;
        }
        // bottom-left to top-right diagonal
        else
        {
            _buff[_bufferPos].vertex[0] = quad.position.d_left;
            _buff[_bufferPos].vertex[1] = quad.position.d_bottom;
            _buff[_bufferPos].vertex[2] = quad.z;
            _buff[_bufferPos].color     = quad.bottomLeftCol;
            _buff[_bufferPos].tex[0]    = quad.texPosition.d_left;
            _buff[_bufferPos].tex[1]    = quad.texPosition.d_bottom;
        }
        ++_bufferPos;

        //vert 5
        _buff[_bufferPos].vertex[0] = quad.position.d_right;
        _buff[_bufferPos].vertex[1] = quad.position.d_bottom;
        _buff[_bufferPos].vertex[2] = quad.z;
        _buff[_bufferPos].color     = quad.bottomRightCol;
        _buff[_bufferPos].tex[0]    = quad.texPosition.d_right;
        _buff[_bufferPos].tex[1]    = quad.texPosition.d_bottom;
        ++_bufferPos;

        if(_bufferPos > (VERTEXBUFFER_CAPACITY - VERTEX_PER_QUAD))
        {
            renderVBuffer();
        }
    }

    //Render
    renderVBuffer();
    exitPerFrameStates();
}

void GuiRenderer::clearRenderList()
{
    _quadlist.clear();
}

CEGUI::Texture* GuiRenderer::createTexture()
{
    GuiTexture* p_tex = new GuiTexture( this );
    _texturelist.push_back( p_tex );
    //log_debug << "GuiRenderer: creating texture, id " << p_tex->getOGLTexid() << std::endl;

    return p_tex;
}

CEGUI::Texture* GuiRenderer::createTexture( const CEGUI::String& filename, const CEGUI::String& resourceGroup )
{
    GuiTexture* p_tex = static_cast< GuiTexture* >( this->createTexture() );
    p_tex->loadFromFile( filename, resourceGroup );
    //log_debug << "GuiRenderer: creating texture from file " << filename.c_str() << ", id " << p_tex->getOGLTexid() << std::endl;
    return p_tex;
}

CEGUI::Texture* GuiRenderer::createTexture( float size )
{
    GuiTexture* p_tex = static_cast< GuiTexture* >( this->createTexture() );
    p_tex->setOGLTextureSize( ( CEGUI::uint )size );
    //log_debug << "GuiRenderer: creating texture with given size " << size << ", id " << p_tex->getOGLTexid() << std::endl;
    return p_tex;
}

void GuiRenderer::destroyTexture( CEGUI::Texture* p_texture )
{
    if ( p_texture )
    {
        GuiTexture* p_tex = static_cast< GuiTexture* >( p_texture );
        //log_debug << "GuiRenderer: deleting texture, id " << p_tex->getOGLTexid() << std::endl;
        _texturelist.remove( p_tex );
        delete p_tex;
    }
}

void GuiRenderer::destroyAllTextures()
{
    while ( !_texturelist.empty() )
        this->destroyTexture( *( _texturelist.begin( )) );
}

void GuiRenderer::initPerFrameStates()
{
    //save current attributes
    glPushClientAttrib( GL_CLIENT_ALL_ATTRIB_BITS );
    glPushAttrib( GL_ALL_ATTRIB_BITS );

    glViewport( 0, 0, int( _display_area.d_right ), int( _display_area.d_bottom ) );

    glPolygonMode( GL_FRONT, GL_FILL );
    glMatrixMode( GL_PROJECTION );
    glPushMatrix();
    glLoadIdentity();

    glOrtho( 0.0, _display_area.d_right, 0.0, _display_area.d_bottom, -1, 1 );

    glMatrixMode( GL_MODELVIEW );
    glPushMatrix();
    glLoadIdentity();

    // setup the texture units
    //------------------------
    typedef void ( APIENTRY * ActiveTextureProc ) ( GLenum texture );
    static ActiveTextureProc s_glClientActiveTexture = ( ActiveTextureProc )osg::getGLExtensionFuncPtr( "glClientActiveTexture", "glClientActiveTextureARB" );
    static ActiveTextureProc s_glActiveTexture       = ( ActiveTextureProc )osg::getGLExtensionFuncPtr( "glActiveTexture",       "glActiveTextureARB"       );

    // disable the client-side tex unit 0 as some previous primitives may have used it and missed to disable it after their usage ( is this behaviour intented by osg or is it a bug? )
    //  missing this results in disappearing the gui when primitives mentioned above are rendered
    s_glClientActiveTexture( GL_TEXTURE0 + 0 );
    glDisable( GL_TEXTURE_2D );
    glBindTexture( GL_TEXTURE_2D, 0 );

    // enable the server-side tex unit 0
    s_glActiveTexture( GL_TEXTURE0 + 1 );
    glDisable( GL_TEXTURE_2D );

    s_glActiveTexture( GL_TEXTURE0 + 0 );
    glEnable( GL_TEXTURE_2D );
    //------------------------

    // setup alpha-blending
    glEnable( GL_BLEND );
    glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
    glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

    // disable depth testing and lighting
    glDisable( GL_LIGHTING );
    glDisable( GL_DEPTH_TEST );
}

void GuiRenderer::exitPerFrameStates()
{
    glDisable( GL_TEXTURE_2D );
    glPopMatrix();
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );

    //restore former attributes
    glPopClientAttrib();
    glPopAttrib();
}

void GuiRenderer::renderVBuffer()
{
    // if bufferPos is 0 there is no data in the buffer and nothing to render
    if (_bufferPos == 0)
    {
        return;
    }

    // render the sprites
    glDrawArrays(GL_TRIANGLES, 0, _bufferPos);

    // reset buffer position to 0...
    _bufferPos = 0;
}


/*************************************************************************
    sort quads list according to texture
*************************************************************************/
void GuiRenderer::sortQuads()
{
    // no need to do anything here.
}

void GuiRenderer::renderQuadDirect(const CEGUI::Rect& dest_rect, float z, const CEGUI::Texture* tex, const CEGUI::Rect& texture_rect, const CEGUI::ColourRect& colours, CEGUI::QuadSplitMode quad_split_mode)
{
    QuadInfo quad;
    quad.position.d_left    = dest_rect.d_left;
    quad.position.d_right   = dest_rect.d_right;
    quad.position.d_bottom  = _display_area.d_bottom - dest_rect.d_bottom;
    quad.position.d_top     = _display_area.d_bottom - dest_rect.d_top;
    quad.texPosition        = texture_rect;

    quad.topLeftCol     = colourToOGL(colours.d_top_left);
    quad.topRightCol    = colourToOGL(colours.d_top_right);
    quad.bottomLeftCol  = colourToOGL(colours.d_bottom_left);
    quad.bottomRightCol = colourToOGL(colours.d_bottom_right);

    MyQuad myquad[VERTEX_PER_QUAD];

    initPerFrameStates();
    glInterleavedArrays(GL_T2F_C4UB_V3F , 0, myquad);
    glBindTexture(GL_TEXTURE_2D, ((GuiTexture*)tex)->getOGLTexid());

    //vert0
    myquad[0].vertex[0] = quad.position.d_left;
    myquad[0].vertex[1] = quad.position.d_top;
    myquad[0].vertex[2] = z;
    myquad[0].color     = quad.topLeftCol;
    myquad[0].tex[0]    = quad.texPosition.d_left;
    myquad[0].tex[1]    = quad.texPosition.d_top;

    //vert1
    myquad[1].vertex[0] = quad.position.d_left;
    myquad[1].vertex[1] = quad.position.d_bottom;
    myquad[1].vertex[2] = z;
    myquad[1].color     = quad.bottomLeftCol;
    myquad[1].tex[0]    = quad.texPosition.d_left;
    myquad[1].tex[1]    = quad.texPosition.d_bottom;

    //vert2

    // top-left to bottom-right diagonal
    if (quad_split_mode == CEGUI::TopLeftToBottomRight)
    {
        myquad[2].vertex[0] = quad.position.d_right;
        myquad[2].vertex[1] = quad.position.d_bottom;
        myquad[2].vertex[2] = z;
        myquad[2].color     = quad.bottomRightCol;
        myquad[2].tex[0]    = quad.texPosition.d_right;
        myquad[2].tex[1]    = quad.texPosition.d_bottom;
    }
    // bottom-left to top-right diagonal
    else
    {
        myquad[2].vertex[0] = quad.position.d_right;
        myquad[2].vertex[1] = quad.position.d_top;
        myquad[2].vertex[2] = z;
        myquad[2].color     = quad.topRightCol;
        myquad[2].tex[0]    = quad.texPosition.d_right;
        myquad[2].tex[1]    = quad.texPosition.d_top;
    }

    //vert3
    myquad[3].vertex[0] = quad.position.d_right;
    myquad[3].vertex[1] = quad.position.d_top;
    myquad[3].vertex[2] = z;
    myquad[3].color     = quad.topRightCol;
    myquad[3].tex[0]    = quad.texPosition.d_right;
    myquad[3].tex[1]    = quad.texPosition.d_top;

    //vert4

    // top-left to bottom-right diagonal
    if (quad_split_mode == CEGUI::TopLeftToBottomRight)
    {
        myquad[4].vertex[0] = quad.position.d_left;
        myquad[4].vertex[1] = quad.position.d_top;
        myquad[4].vertex[2] = z;
        myquad[4].color     = quad.topLeftCol;
        myquad[4].tex[0]    = quad.texPosition.d_left;
        myquad[4].tex[1]    = quad.texPosition.d_top;
    }
    // bottom-left to top-right diagonal
    else
    {
        myquad[4].vertex[0] = quad.position.d_left;
        myquad[4].vertex[1] = quad.position.d_bottom;
        myquad[4].vertex[2] = z;
        myquad[4].color     = quad.bottomLeftCol;
        myquad[4].tex[0]    = quad.texPosition.d_left;
        myquad[4].tex[1]    = quad.texPosition.d_bottom;
    }

    //vert5
    myquad[5].vertex[0] = quad.position.d_right;
    myquad[5].vertex[1] = quad.position.d_bottom;
    myquad[5].vertex[2] = z;
    myquad[5].color     = quad.bottomRightCol;
    myquad[5].tex[0]    = quad.texPosition.d_right;
    myquad[5].tex[1]    = quad.texPosition.d_bottom;

    glDrawArrays(GL_TRIANGLES, 0, 6);

    exitPerFrameStates();
}

long GuiRenderer::colourToOGL(const CEGUI::colour& col) const
{
    CEGUI::ulong cval;
    cval =  (static_cast<CEGUI::ulong>(255 * col.getAlpha())) << 24;
    cval |= (static_cast<CEGUI::ulong>(255 * col.getBlue())) << 16;
    cval |= (static_cast<CEGUI::ulong>(255 * col.getGreen())) << 8;
    cval |= (static_cast<CEGUI::ulong>(255 * col.getRed()));

    return cval;
}

} // namespace yaf3d

