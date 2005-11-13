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
 # graphical user interface: texture creation interface
 #
 #	this code is basing	on CEGUI's opengl renderer implementation
 #
 #	 date of creation:	04/03/2005
 #
 #	 author:			ali	botorabi (boto)	
 #		e-mail:			botorabi@gmx.net
 #
 ################################################################*/

#ifndef	_GUITEXTURE_H_
#define	_GUITEXTURE_H_

#include <guirenderer.h>

namespace yaf3d
{

//!	Adapter	for	CEGUI's	texture	creation / manipulation
/**
    This class is used by our CEGUI's renderer backend.
    In normal case you may not need to use it!
*/
class CTDGuiTexture	: public CEGUI::Texture
{
    public:

        explicit                                CTDGuiTexture( CEGUI::Renderer* p_owner );

	    virtual	                                ~CTDGuiTexture();

	    virtual	CEGUI::ushort	                getWidth() const  { return _width; }

	    virtual	CEGUI::ushort	                getHeight()	const { return _height; }

	    virtual	void	                        loadFromFile( const CEGUI::String& filename, const CEGUI::String& resourceGroup );

	    virtual	void	                        loadFromMemory( const void* buffPtr, CEGUI::uint buffWidth, CEGUI::uint buffHeight );

	    GLuint	                                getOGLTexid() const	{ return _ogltexture; }

	    void	                                setOGLTextureSize( CEGUI::uint size );

    protected:

        // These defines are used to tell us about the type	of TARGA file it is
        #	define TGA_RGB		 2		// This	tells us it's a	normal RGB (really BGR)	file
        #	define TGA_A		 3		// This	tells us it's a	ALPHA file
        #	define TGA_RLE		10		// This	tells us that the targa	is Run-Length Encoded (RLE)

	    /*************************************************************************
		    Implementation Struct
	    *************************************************************************/
	    // This	is our image structure for our targa data
	    struct tImageTGA
	    {
		    int	            channels;			// The channels	in the image (3	= RGB :	4 =	RGBA)
		    int	            sizeX;				// The width of	the	image in pixels
		    int	            sizeY;				// The height of the image in pixels
		    unsigned char*  data;	            // The image pixel data
	    };

	    // flips data for tImageTGA	'img'
	    static void	                            flipImageTGA( tImageTGA* img );
    	
	    // Took	this code from http://www.gametutorials.com	still ne
	    tImageTGA*                              LoadTGA( const unsigned char* buffer, std::size_t buffer_size );

        GLuint				                    _ogltexture;		//!< The 'real'	texture.

	    CEGUI::ushort		                    _width;			    //!< cached	width of the texture
	    
        CEGUI::ushort		                    _height;			//!< cached	height of the texture

        friend	CEGUI::Texture*	                CTDGuiRenderer::createTexture();
    	
        friend	CEGUI::Texture*	                CTDGuiRenderer::createTexture( const CEGUI::String& filename, const CEGUI::String& resourceGroup );

    	friend	CEGUI::Texture*	                CTDGuiRenderer::createTexture( float size );

	    friend	void	                        CTDGuiRenderer::destroyTexture( CEGUI::Texture* texture );

};

} // namespace yaf3d


#endif	// _GUITEXTURE_H_
