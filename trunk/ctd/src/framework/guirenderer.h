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

#ifndef _YAF3DGUIRENDERER_H_
#define _YAF3DGUIRENDERER_H_

#include <list>
#include <set>

#define OGLRENDERER_VBUFF_CAPACITY  4096

namespace yaf3d
{

class CTDGuiTexture;

//! Rendering adapter for CEGUI
class CTDGuiRenderer : public CEGUI::Renderer
{
    public:

        explicit                                CTDGuiRenderer( unsigned int max_quads );

                                                CTDGuiRenderer( unsigned int max_quads, int width, int height );

        virtual                                 ~CTDGuiRenderer(void);

        //! Change the client area, use this e.g. when the main app windows shape changed
        void                                    changeDisplayResolution( float width, float height );

        //! Interface methods ( for mode details look into CEGUI::OpenGLRenderer )
        //----------------------------------------------------------------------//
        virtual void                            addQuad( 
                                                        const CEGUI::Rect& dest_rect, 
                                                        float z, 
                                                        const CEGUI::Texture* p_tex, 
                                                        const CEGUI::Rect& texture_rect, 
                                                        const CEGUI::ColourRect& colours, 
                                                        CEGUI::QuadSplitMode quad_split_mode 
                                                        );

        virtual void                            doRender();

        virtual void                            clearRenderList();

        virtual void                            setQueueingEnabled( bool setting )  { _queueing = setting; }


        // create an empty texture
        virtual CEGUI::Texture*                 createTexture();

        // create a texture and load it with the specified file.
        virtual CEGUI::Texture*                 createTexture( const CEGUI::String& filename, const CEGUI::String& resourceGroup );

        // create a texture and set it to the specified size
        virtual CEGUI::Texture*                 createTexture( float size );

        // destroy the given texture
        virtual void                            destroyTexture( CEGUI::Texture* p_texture );

        // destroy all textures still active
        virtual void                            destroyAllTextures();

        virtual bool                            isQueueingEnabled() const   { return _queueing; }

        virtual float                           getWidth() const            { return _display_area.getWidth(); }

        virtual float                           getHeight() const           { return _display_area.getHeight(); }

        virtual CEGUI::Size                     getSize() const             { return _display_area.getSize(); }

        virtual CEGUI::Rect                     getRect() const             { return _display_area; }

        virtual CEGUI::uint                     getMaxTextureSize() const   { return _maxTextureSize; }

        virtual CEGUI::uint                     getHorzScreenDPI() const    { return 96; }

        virtual CEGUI::uint                     getVertScreenDPI() const    { return 96; }


    protected:

        const static int                        VERTEX_PER_QUAD;                            //!< number of vertices per quad

        const static int                        VERTEX_PER_TRIANGLE;                        //!< number of vertices for a triangle

        const static int                        VERTEXBUFFER_CAPACITY;                      //!< capacity of the allocated vertex buffer
        
        struct MyQuad
        {
            float tex[2];
            long color;
            float vertex[3];
        };

        struct QuadInfo
        {
            CEGUI::uint texid;  
            CEGUI::Rect position;
            float       z;
            CEGUI::Rect texPosition;
            long        topLeftCol;
            long        topRightCol;
            long        bottomLeftCol;
            long        bottomRightCol;

            CEGUI::QuadSplitMode   splitMode;

            bool operator<( const QuadInfo& other ) const
            {
                // this is intentionally reversed.
                return z > other.z;
            }

        };

        // setup states etc
        void                                    initPerFrameStates(void);

        // restore states
        void                                    exitPerFrameStates(void);

        // render whatever is in the vertex buffer
        void                                    renderVBuffer(void);

        // sort quads list according to texture
        void                                    sortQuads(void);

        // render a quad directly to the display
        void                                    renderQuadDirect(
                                                                const CEGUI::Rect& dest_rect, 
                                                                float z, 
                                                                const CEGUI::Texture* p_tex, 
                                                                const CEGUI::Rect& texture_rect, 
                                                                const CEGUI::ColourRect& colours, 
                                                                CEGUI::QuadSplitMode quad_split_mode 
                                                                );

        // convert colour value to whatever the OpenGL system is expecting.
        long                                    colourToOGL( const CEGUI::colour& col ) const;

        typedef std::multiset< QuadInfo >       QuadList;

        QuadList                                _quadlist;

        CEGUI::Rect                             _display_area;

        MyQuad                                  _buff[ OGLRENDERER_VBUFF_CAPACITY ];

        bool                                    _queueing;          //!< setting for queuing control.

        CEGUI::uint                             _currTexture;       //!< Currently bound texture.

        int                                     _bufferPos;         //!< index into buffer where next vertex should be put.

        std::list< CTDGuiTexture* >             _texturelist;       //!< List used to track textures.
        
        int                                     _maxTextureSize;    //!< Holds maximum supported texture size (in pixels).
};

} // namespace yaf3d

#endif // _YAF3DGUIRENDERER_H_
