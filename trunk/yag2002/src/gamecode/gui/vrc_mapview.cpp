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
 # entity for map view, contains an overview and a full-window map view
 #
 #   date of creation:  01/07/2006
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_mapview.h"


namespace vrc
{
// window element name for mapview
#define MAPVIEW_WND    "_mapview_"
// aligning 
#define ALIGN_TOPLEFT       "topleft"
#define ALIGN_TOPMIDDLE     "topmiddle"
#define ALIGN_TOPRIGHT      "topright"
#define ALIGN_BOTTOMLEFT    "bottomleft"
#define ALIGN_BOTTOMMIDDLE  "bottommiddle"
#define ALIGN_BOTTOMRIGHT   "bottomright"

//! Implement and register the map view entity factory
YAF3D_IMPL_ENTITYFACTORY( MapViewEntityFactory );

EnMapView::EnMapView() :
_align( "topright" ),
_size( 0.2f, 0.2f ),
_alpha( 0.8f ),
_p_wnd( NULL ),
_p_imgPlayerMarker( NULL )
{
    // register entity attributes
    getAttributeManager().addAttribute( "minMapFile",   _minMapFile );
    getAttributeManager().addAttribute( "align",        _align      );
    getAttributeManager().addAttribute( "size",         _size       );
    getAttributeManager().addAttribute( "transparency", _alpha      );
}

EnMapView::~EnMapView()
{
    destroyMapView();
}

void EnMapView::destroyMapView()
{
    try
    {
        // destroy imageset
        CEGUI::ImagesetManager::getSingleton().destroyImageset( std::string( MAPVIEW_WND ) + _minMapFile );
        vrc::gameutils::GuiUtils::get()->getMainGuiWindow()->removeChildWindow( _p_wnd );
        CEGUI::WindowManager::getSingleton().destroyWindow( _p_wnd );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "EnMapView: problem cleaning up entity." << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

void EnMapView::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle notifications
    switch( notification.getId() )
    {

        case YAF3D_NOTIFY_MENU_ENTER:

            break;

        case YAF3D_NOTIFY_MENU_LEAVE:

            break;

        case YAF3D_NOTIFY_ENTITY_ATTRIBUTE_CHANGED:
            // recreate the map view
            destroyMapView();
            setupMapView();
            break;

        case YAF3D_NOTIFY_SHUTDOWN:

            break;

        default:
            ;
    }
}

void EnMapView::initialize()
{
    // set up the map view
    setupMapView();
}

void EnMapView::setupMapView()
{
    try
    {
        osg::Vec2 position = calcPosition( _align, _size );

        _p_wnd = CEGUI::WindowManager::getSingleton().createWindow( "DefaultWindow", MAPVIEW_WND "mainWnd" );
        _p_wnd->setMetricsMode( CEGUI::Absolute );
        _p_wnd->setSize( CEGUI::Size( _size.x(), _size.y() ) );
        _p_wnd->setPosition( CEGUI::Point( position.x(), position.y() ) );
        _p_wnd->setAlpha( _alpha );
        _p_wnd->setAlwaysOnTop( true );

        // put the map picture into a static image element
        CEGUI::StaticImage* p_staticimg = static_cast< CEGUI::StaticImage* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticImage", MAPVIEW_WND "image" ) );
        p_staticimg->setSize( CEGUI::Size( 1.0f, 1.0f ) );
        p_staticimg->setPosition( CEGUI::Point( 0.0f, 0.0f ) );
        p_staticimg->setAlpha( _alpha );

        // create a new imageset for map picture
        {
            CEGUI::Texture*  p_texture = yaf3d::GuiManager::get()->getGuiRenderer()->createTexture( _minMapFile, std::string( MAPVIEW_WND ) + _minMapFile );
            CEGUI::Imageset* p_imageSet = CEGUI::ImagesetManager::getSingleton().createImageset( std::string( MAPVIEW_WND ) + _minMapFile, p_texture );
            if ( !p_imageSet->isImageDefined( _minMapFile ) )
                p_imageSet->defineImage( std::string( MAPVIEW_WND ) + _minMapFile, CEGUI::Point( 0.0f, 0.0f ), CEGUI::Size( p_texture->getWidth(), p_texture->getHeight() ), CEGUI::Point( 0.0f,0.0f ) );

            CEGUI::Image* p_image = &const_cast< CEGUI::Image& >( p_imageSet->getImage( std::string( MAPVIEW_WND ) + _minMapFile ) );
            p_staticimg->setImage( p_image );
            _p_wnd->addChildWindow( p_staticimg );
        }

        _p_imgPlayerMarker = vrc::gameutils::GuiUtils::get()->getCustomImage( IMAGE_NAME_CROSSHAIR );


        vrc::gameutils::GuiUtils::get()->getMainGuiWindow()->addChildWindow( _p_wnd );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "EnMapView: problem creating gui" << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
    }

}

osg::Vec2 EnMapView::calcPosition( const std::string& align, const osg::Vec2& size )
{
    osg::Vec2 pos;
    unsigned int windowwidth  = 0.0f;
    unsigned int windowheight = 0.0f;
    yaf3d::Application::get()->getScreenSize( windowwidth, windowheight );

    if ( align == ALIGN_TOPLEFT )
    {
        pos._v[ 0 ] = 0.0f;
        pos._v[ 1 ] = 0.0f;
    }
    else if ( align == ALIGN_TOPMIDDLE )
    {
        pos._v[ 0 ] = static_cast< float >( windowwidth ) / 2.0f - size.x() / 2.0f;
        pos._v[ 1 ] = 0.0f;
    }
    else if ( align == ALIGN_TOPRIGHT )
    {
        pos._v[ 0 ] = static_cast< float >( windowwidth ) - size.x();
        pos._v[ 1 ] = 0.0f;
    }
    else if ( align == ALIGN_BOTTOMLEFT )
    {
        pos._v[ 0 ] = 0.0f;
        pos._v[ 1 ] = static_cast< float >( windowheight ) - size.y();
    }
    else if ( align == ALIGN_BOTTOMMIDDLE )
    {
        pos._v[ 0 ] = static_cast< float >( windowwidth ) / 2.0f - size.x() / 2.0f;
        pos._v[ 1 ] = static_cast< float >( windowheight ) - size.y();
    }
    else if ( align == ALIGN_BOTTOMRIGHT )
    {
        pos._v[ 0 ] = static_cast< float >( windowwidth ) - size.x();
        pos._v[ 1 ] = static_cast< float >( windowheight ) - size.y();
    }
    else
    {
        log_error << "EnMapView: invalid aligning '" << align << "'" << std::endl;    
 
        log       << "           valid values are: " << 
                        ALIGN_TOPLEFT       << ", "  <<
                        ALIGN_TOPMIDDLE     << ", "  <<
                        ALIGN_TOPRIGHT      << ", "  <<
                        ALIGN_BOTTOMLEFT    << ", "  <<
                        ALIGN_BOTTOMMIDDLE  << ", "  <<
                        ALIGN_BOTTOMRIGHT   << std::endl;    

        log       << "           set to default '" << ALIGN_TOPLEFT << "'" << std::endl;
    }

    return pos;
}

} // namespace vrc
