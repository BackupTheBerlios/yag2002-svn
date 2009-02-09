/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2009, A. Botorabi
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
 # common utils
 #
 #   date of creation:  02/02/2009
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/


#include <vrc_main.h>
#include "guibase.h"
#include "editorutils.h"

//! Include the pix
#include "pix/filesave.xpm"
#include "pix/fileopen.xpm"
#include "pix/mainframe.xpm"

//! Bitmap resources
#define BITMAP_FILE_SAVE            filesave_xpm
#define BITMAP_FILE_OPEN            fileopen_xpm
#define BITMAP_ICON_MAINFRAME       mainframe_xpm


YAF3D_SINGLETON_IMPL( BitmapResource )


std::string Conversion::floatToString( float value, std::streamsize prec )
{
    std::stringstream v;
    v.precision( prec );
    v.setf( std::ios_base::fixed, std::ios::floatfield );
    v << value;
    return v.str();
}

float Conversion::stringToFloat( const std::string& value )
{
    float v = 0;
    std::stringstream str;
    str << value;
    str >> v;
    return v;
}

std::string Conversion::vec3ToString( const osg::Vec3f& value, std::streamsize prec )
{
    std::stringstream v;
    v.precision( prec );
    v.setf( std::ios_base::fixed, std::ios::floatfield );
    v << value._v[ 0 ] << "  " << value._v[ 1 ] << "  " << value._v[ 2 ];
    return v.str();
}

std::string Conversion::vec2ToString( const osg::Vec2f& value, std::streamsize prec )
{
    std::stringstream v;
    v.precision( prec );
    v.setf( std::ios_base::fixed, std::ios::floatfield );
    v << value._v[ 0 ] << "  " << value._v[ 1 ];
    return v.str();
}

osg::Vec3f Conversion::stringToVec3( const std::string& value )
{
    osg::Vec3f v;
    std::stringstream str;
    str << value;
    str >> v._v[ 0 ] >> v._v[ 1 ] >> v._v[ 2 ];
    return v;
}

osg::Vec2f Conversion::stringToVec2( const std::string& value )
{
    osg::Vec2f v;
    std::stringstream str;
    str << value;
    str >> v._v[ 0 ] >> v._v[ 1 ];
    return v;
}

unsigned int Conversion::stringToUint( const std::string& value )
{
    unsigned int v = 0;
    std::stringstream str;
    str << value;
    str >> v;
    return v;
}

std::string Conversion::uintToString( unsigned int value )
{
    std::stringstream str;
    str << value;
    return str.str();
}

osg::Vec3f Conversion::colorToVec( const wxColour& value )
{
    float r = static_cast< float >( value.Red() );
    float g = static_cast< float >( value.Green() );
    float b = static_cast< float >( value.Blue() );
    osg::Vec3f v( r, g, b );
    return v;
}

wxColour Conversion::vecToColour( const osg::Vec3f& value )
{
    unsigned char r = static_cast< unsigned char>( value._v[ 0 ] );
    unsigned char g = static_cast< unsigned char>( value._v[ 1 ] );
    unsigned char b = static_cast< unsigned char>( value._v[ 2 ] );
    unsigned char a = 0;
    wxColour v( r, g, b, a );
    return v;
}

void Conversion::getEulerRotation( const osg::Quat& q, osg::Vec3f& rotation )
{
    //! NOTE: This code is basing on quat to euler transformation code
    //        of project: http://opensimon.berlios.de/, thanks go to authors of opensimon.
    static osg::Matrixf m;
    q.get( m );
    float cosY = sqrt( m( 0, 0 ) * m( 0, 0 )  + m( 1, 0 ) * m( 1, 0 ) );
    if ( cosY > 16 * FLT_EPSILON )
    {
        rotation._v[ 0 ] = atan2( 1.0f*m( 2, 1 ), m( 2, 2 ) );
        rotation._v[ 1 ] = atan2(-1.0f*m( 2, 0 ), cosY);
        rotation._v[ 2 ] = atan2( 1.0f*m( 1, 0 ), m( 0, 0 ) );
    }
    else
    {
        rotation._v[ 0 ] = atan2( -1.0f*m( 1, 2 ), m( 1, 1 ) );
        rotation._v[ 1 ] = atan2( -1.0f*m( 2, 0 ), cosY );
        rotation._v[ 2 ] = 0.0;
    }

    rotation *= 180.0f / M_PI;
}

BitmapResource::BitmapResource()
{
}

BitmapResource::~BitmapResource()
{
}

void BitmapResource::initialize()
{
    // init the xpm handler
    wxImage::AddHandler( new wxXPMHandler );

    _icons[ EDITOR_RESID_ICON_MAINFRAME ] = wxIcon( BITMAP_ICON_MAINFRAME );
    _icons[ EDITOR_RESID_ICON_STATSWND ]  = wxIcon( BITMAP_ICON_MAINFRAME );
    _icons[ EDITOR_RESID_ICON_LOGWND ]    = wxIcon( BITMAP_ICON_MAINFRAME );

    _bitmaps[ EDITOR_RESID_BMP_FILEOPEN ] = wxBitmap( BITMAP_FILE_OPEN );
    _bitmaps[ EDITOR_RESID_BMP_FILESAVE ] = wxBitmap( BITMAP_FILE_SAVE );
}

void BitmapResource::shutdown()
{
}

wxBitmap& BitmapResource::getBitmap( unsigned int resourceid )
{
    static wxBitmap emptybitmap;
    if ( _bitmaps.find( resourceid ) == _bitmaps.end() )
        return emptybitmap;

    return _bitmaps[ resourceid ];
}

wxIcon& BitmapResource::getIcon( unsigned int resourceid )
{
    static wxIcon emptyicon;
    if ( _icons.find( resourceid ) == _icons.end() )
        return emptyicon;

    return _icons[ resourceid ];
}
