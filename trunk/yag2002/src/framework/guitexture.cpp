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
 #  this code is basing on CEGUI's opengl renderer implementation
 #
 #   date of creation:  04/03/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <base.h>
#include "application.h"
#include "guitexture.h"
#include <CEGUIExceptions.h>
#include <CEGUISystem.h>

namespace yaf3d
{

CTDGuiTexture::CTDGuiTexture( CEGUI::Renderer* p_owner ) :
CEGUI::Texture( p_owner ),
_width( 0 ),
_height( 0 )
{
    // generate a OGL texture that we will use.
    glGenTextures( 1, &_ogltexture );
    // set some parameters for this texture.
    glBindTexture( GL_TEXTURE_2D, _ogltexture );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
}

CTDGuiTexture::~CTDGuiTexture()
{
    // delete OGL texture associated with this object.
    glDeleteTextures( 1, &_ogltexture );
}

void CTDGuiTexture::loadFromFile( const CEGUI::String& filename, const CEGUI::String& resourceGroup )
{
    glBindTexture( GL_TEXTURE_2D, _ogltexture );        

    // load file to memory via resource provider
    CEGUI::RawDataContainer texFile;
    CEGUI::System::getSingleton().getResourceProvider()->loadRawDataContainer( filename, texFile, resourceGroup );

    tImageTGA*  img = LoadTGA(texFile.getDataPtr(), texFile.getSize());
    if (img != 0)
    {
        _width = static_cast<CEGUI::ushort>(img->sizeX);
        _height = static_cast<CEGUI::ushort>(img->sizeY);

        // flip the image...
        flipImageTGA(img);

        // If the image is 32-bit (4 channels), then we need to specify GL_RGBA for an alpha, else we use GL_RGB.
        int textureType = (img->channels == 4) ? GL_RGBA : GL_RGB;

        glTexImage2D(GL_TEXTURE_2D, 0, textureType, _width, _height, 0, textureType, GL_UNSIGNED_BYTE, img->data);

        // Free texture data, we don't need it anymore
        if ( img->data )                          
        {
            delete[] img->data;
        }

        // Free the image structure
        delete img;
    }
    else
    {
        throw CEGUI::RendererException("CTDGuiTexture::loadFromFile - internal Targa loader failed to load image '" + filename + "'.");
    }
}

void CTDGuiTexture::loadFromMemory(const void* buffPtr, CEGUI::uint buffWidth, CEGUI::uint buffHeight)
{
	glBindTexture(GL_TEXTURE_2D, _ogltexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, buffWidth, buffHeight, 0, GL_RGBA ,GL_UNSIGNED_BYTE, buffPtr);

    _width  = static_cast<CEGUI::ushort>(buffWidth);
    _height = static_cast<CEGUI::ushort>(buffHeight);
}

void CTDGuiTexture::setOGLTextureSize(CEGUI::uint size)
{
    // if not power of 2
    if ((size & (size - 1)) || !size)
    {
        int l = 0;

        // get integer log of 'size' to base 2
        while (size >>= 1)
        {
            ++l;
        }

        // use log to calculate value to use as size.
        size = (2 << l);
    }

    // allocate temp buffer for texture
    CEGUI::uchar* buff = new CEGUI::uchar[size * size * 4];

    // load empty buffer to texture
    glBindTexture(GL_TEXTURE_2D, _ogltexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size, size, 0, GL_RGBA ,GL_UNSIGNED_BYTE, buff);

    // delete buffer
    delete[] buff;

    _height = _width = static_cast<CEGUI::ushort>(size);
}

void CTDGuiTexture::flipImageTGA(CTDGuiTexture::tImageTGA* img)
{
    int pitch = img->sizeX * img->channels;

    // flip the image bits...
    for (int line = 0; line < img->sizeY / 2; ++line)
    {
        int srcOffset = (line * pitch);
        int dstOffest = ((img->sizeY - line - 1) * pitch);

        for (int colBit = 0; colBit < pitch; ++colBit)
        {
            CEGUI::uchar tmp = img->data[dstOffest + colBit];
            img->data[dstOffest + colBit] = img->data[srcOffset + colBit];
            img->data[srcOffset + colBit] = tmp;
        }
    }
}

CTDGuiTexture::tImageTGA* CTDGuiTexture::LoadTGA( const unsigned char* buffer, size_t buffer_size )
{
    tImageTGA *pImageData = NULL;       // This stores our important image data
    short width = 0, height = 0;        // The dimensions of the image
    GLbyte length = 0;                  // The length in bytes to the pixels
    GLbyte imageType = 0;               // The image type (RLE, RGB, Alpha...)

    GLbyte bits = 0;                    // The bits per pixel for the image (16, 24, 32)
    int channels = 0;                   // The channels of the image (3 = RGA : 4 = RGBA)
    int stride = 0;                     // The stride (channels * width)
    int i = 0;                          // A counter

    // This function loads in a TARGA (.TGA) file and returns its data to be
    // used as a texture or what have you.  This currently loads in a 16, 24
    // and 32-bit targa file, along with RLE compressed files.  Eventually you
    // will want to do more error checking to make it more robust.  This is
    // also a perfect start to go into a modular class for an engine.
    // Basically, how it works is, you read in the header information, then
    // move your file pointer to the pixel data.  Before reading in the pixel
    // data, we check to see the if it's an RLE compressed image.  This is because
    // we will handle it different.  If it isn't compressed, then we need another
    // check to see if we need to convert it from 16-bit to 24 bit.  24-bit and
    // 32-bit textures are very similar, so there's no need to do anything special.
    // We do, however, read in an extra bit for each color.

    // Allocate the structure that will hold our eventual image data (must free it!)
    pImageData = new tImageTGA;
    pImageData->data     = NULL;
    pImageData->channels = 0;
    pImageData->sizeX    = 0;
    pImageData->sizeY    = 0;

    // Read in the length in bytes from the header to the pixel data
    memcpy(&length, buffer, sizeof(GLbyte));
    buffer += sizeof(GLbyte);

    // Jump over one byte
    ++buffer;

    // Read in the imageType (RLE, RGB, etc...)
    memcpy(&imageType, buffer, sizeof(GLbyte));
    buffer += sizeof(GLbyte);

    // Skip past general information we don't care about
    buffer += 9;

    // Read the width, height and bits per pixel (16, 24 or 32)
    memcpy(&width, buffer, sizeof(short));
    buffer += sizeof(short);
    memcpy(&height, buffer, sizeof(short));
    buffer += sizeof(short);
    memcpy(&bits, buffer, sizeof(GLbyte));
    buffer += sizeof(GLbyte);

    // Now we move the file pointer to the pixel data
    buffer += length + 1;

    // Check if the image is RLE compressed or not
    if(imageType != TGA_RLE)
    {
        // Check if the image is a 24 or 32-bit image
        if(bits == 24 || bits == 32)
        {
            // Calculate the channels (3 or 4) - (use bits >> 3 for more speed).
            // Next, we calculate the stride and allocate enough memory for the pixels.
            channels = bits / 8;
            stride = channels * width;
            pImageData->data = new unsigned char[stride * height];

            // Load in all the pixel data line by line
            for(int y = 0; y < height; y++)
            {
                // Store a pointer to the current line of pixels
                unsigned char *pLine = &(pImageData->data[stride * y]);

                // Read in the current line of pixels
                memcpy(pLine, buffer, stride);
                buffer += stride;

                // Go through all of the pixels and swap the B and R values since TGA
                // files are stored as BGR instead of RGB (or use GL_BGR_EXT verses GL_RGB)
                for(i = 0; i < stride; i += channels)
                {
                    unsigned char temp     = pLine[i];
                    pLine[i]     = pLine[i + 2];
                    pLine[i + 2] = temp;
                }
            }
        }
        // Check if the image is a 16 bit image (RGB stored in 1 unsigned short)
        else if(bits == 16)
        {
            unsigned short pixels = 0;
            unsigned char r=0, g=0, b=0;

            // Since we convert 16-bit images to 24 bit, we hardcode the channels to 3.
            // We then calculate the stride and allocate memory for the pixels.
            channels = 3;
            stride = channels * width;
            pImageData->data = new unsigned char[stride * height];

            // Load in all the pixel data pixel by pixel
            for(i = 0; i < width*height; i++)
            {
                // Read in the current pixel
                memcpy(&pixels, buffer, sizeof(unsigned short));
                buffer += sizeof(unsigned short);

                // To convert a 16-bit pixel into an R, G, B, we need to
                // do some masking and such to isolate each color value.
                // 0x1f = 11111 in binary, so since 5 bits are reserved in
                // each unsigned short for the R, G and B, we bit shift and mask
                // to find each value.  We then bit shift up by 3 to get the full color.
                b = static_cast<unsigned char>((pixels & 0x1f) << 3);
                g = static_cast<unsigned char>(((pixels >> 5) & 0x1f) << 3);
                r = static_cast<unsigned char>(((pixels >> 10) & 0x1f) << 3);

                // This essentially assigns the color to our array and swaps the
                // B and R values at the same time.
                pImageData->data[i * 3 + 0] = r;
                pImageData->data[i * 3 + 1] = g;
                pImageData->data[i * 3 + 2] = b;
            }
        }   
        // Else return a NULL for a bad or unsupported pixel format
        else
        {
            delete pImageData;
            return NULL;
        }
    }
    // Else, it must be Run-Length Encoded (RLE)
    else
    {
        // First, let me explain real quickly what RLE is.  
        // For further information, check out Paul Bourke's intro article at: 
        // http://astronomy.swin.edu.au/~pbourke/dataformats/rle/
        // 
        // Anyway, we know that RLE is a basic type compression.  It takes
        // colors that are next to each other and then shrinks that info down
        // into the color and a integer that tells how much of that color is used.
        // For instance:
        // aaaaabbcccccccc would turn into a5b2c8
        // Well, that's fine and dandy and all, but how is it down with RGB colors?
        // Simple, you read in an color count (rleID), and if that number is less than 128,
        // it does NOT have any optimization for those colors, so we just read the next
        // pixels normally.  Say, the color count was 28, we read in 28 colors like normal.
        // If the color count is over 128, that means that the next color is optimized and
        // we want to read in the same pixel color for a count of (colorCount - 127).
        // It's 127 because we add 1 to the color count, as you'll notice in the code.

        // Create some variables to hold the rleID, current colors read, channels, & stride.
        unsigned char rleID = 0;
        int colorsRead = 0;
        channels = bits / 8;
        stride = channels * width;

        // Next we want to allocate the memory for the pixels and create an array,
        // depending on the channel count, to read in for each pixel.
        pImageData->data = new unsigned char[stride * height];
        GLbyte *pColors = new GLbyte [channels];

        // Load in all the pixel data
        while(i < width*height)
        {
            // Read in the current color count + 1
            memcpy(&rleID, buffer, sizeof(unsigned char));
            buffer += sizeof(unsigned char);

            // Check if we don't have an encoded string of colors
            if(rleID < 128)
            {
                // Increase the count by 1
                rleID++;

                // Go through and read all the unique colors found
                while(rleID)
                {
                    // Read in the current color
                    memcpy(pColors, buffer, sizeof(GLbyte) * channels);
                    buffer += sizeof(GLbyte) * channels;

                    // Store the current pixel in our image array
                    pImageData->data[colorsRead + 0] = pColors[2];
                    pImageData->data[colorsRead + 1] = pColors[1];
                    pImageData->data[colorsRead + 2] = pColors[0];

                    // If we have a 4 channel 32-bit image, assign one more for the alpha
                    if(bits == 32)
                        pImageData->data[colorsRead + 3] = pColors[3];

                    // Increase the current pixels read, decrease the amount
                    // of pixels left, and increase the starting index for the next pixel.
                    i++;
                    rleID--;
                    colorsRead += channels;
                }
            }
            // Else, let's read in a string of the same character
            else
            {
                // Minus the 128 ID + 1 (127) to get the color count that needs to be read
                rleID -= 127;

                // Read in the current color, which is the same for a while
                memcpy(pColors, buffer, sizeof(GLbyte) * channels);
                buffer += sizeof(GLbyte) * channels;

                // Go and read as many pixels as are the same
                while(rleID)
                {
                    // Assign the current pixel to the current index in our pixel array
                    pImageData->data[colorsRead + 0] = pColors[2];
                    pImageData->data[colorsRead + 1] = pColors[1];
                    pImageData->data[colorsRead + 2] = pColors[0];

                    // If we have a 4 channel 32-bit image, assign one more for the alpha
                    if(bits == 32)
                        pImageData->data[colorsRead + 3] = pColors[3];

                    // Increase the current pixels read, decrease the amount
                    // of pixels left, and increase the starting index for the next pixel.
                    i++;
                    rleID--;
                    colorsRead += channels;
                }

            }

        }

        // Free up pColors
        delete[] pColors;
    }

    // Fill in our tImageTGA structure to pass back
    pImageData->channels = channels;
    pImageData->sizeX    = width;
    pImageData->sizeY    = height;

    // Return the TGA data (remember, you must free this data after you are done)
    return pImageData;
}

} // namespace yaf3d
