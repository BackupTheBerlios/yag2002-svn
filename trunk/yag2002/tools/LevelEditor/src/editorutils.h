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


#ifndef _EDITORUTILS_H_
#define _EDITORUTILS_H_


#include "guibase.h"

class EditorApp;

//! Helper class for various type conversions
class Conversion
{
    public:

                                                    Conversion() {}

                                                    ~Conversion() {}

        static std::string                          floatToString( float value, std::streamsize prec = 2 );

        static float                                stringToFloat( const std::string& value );

        static std::string                          vec3ToString( const osg::Vec3f& value, std::streamsize prec = 2 );

        static std::string                          vec2ToString( const osg::Vec2f& value, std::streamsize prec = 2 );

        static osg::Vec3f                           stringToVec3( const std::string& value );

        static osg::Vec2f                           stringToVec2( const std::string& value );

        static std::string                          uintToString( unsigned int value );

        static unsigned int                         stringToUint( const std::string& value );

        static wxColour                             vecToColour( const osg::Vec3f& value );

        static osg::Vec3f                           colorToVec( const wxColour& value );

        static void                                 getEulerRotation( const osg::Quat& q, osg::Vec3f& rotation );

};

//! Scene graph node data used by editor for picking functionality
class EditorSGData: public osg::Referenced
{
    public:

                                                    EditorSGData( yaf3d::BaseEntity* p_entity ) :
                                                     _p_entity( p_entity )
                                                    {
                                                    }

         virtual                                    ~EditorSGData() {}

         yaf3d::BaseEntity*                          getEntity()
                                                     {
                                                         return _p_entity;
                                                     }

    protected:

        yaf3d::BaseEntity*                          _p_entity;
};


//! Available bitmap resource IDs
#define EDITOR_RESID_ICON_MAINFRAME     1
#define EDITOR_RESID_ICON_STATSWND      2
#define EDITOR_RESID_ICON_LOGWND        3
#define EDITOR_RESID_BMP_FILEOPEN       20
#define EDITOR_RESID_BMP_FILESAVE       21
#define EDITOR_RESID_BMP_ENT_SEL        22
#define EDITOR_RESID_BMP_ENT_MOVE       23
#define EDITOR_RESID_BMP_ENT_AUTOPLACE  24
#define EDITOR_RESID_BMP_ENT_ROTATE     25

//! Available bitmap resources are managed by this singleton
class BitmapResource: public yaf3d::Singleton< BitmapResource >
{
    public:

        //! Given a resource ID, return its bitmap
        wxBitmap&                                       getBitmap( unsigned int resourceid );

        //! Given a resource ID return its icon. Note: the bitmap can be 16x16 or 32x32 pix.
        wxIcon&                                         getIcon( unsigned int resourceid );

    protected:

                                                        BitmapResource();

        virtual                                         ~BitmapResource();

        //! Initialize the resources
        void                                            initialize();

        //! Shutdown
        void                                            shutdown();

        //! Bitmap cache
        std::map< unsigned int, wxBitmap >              _bitmaps;

        //! Icon cache
        std::map< unsigned int, wxIcon >                _icons;

    friend class EditorApp;
    friend class yaf3d::Singleton< BitmapResource >;
};

#endif // _EDITORUTILS_H_
