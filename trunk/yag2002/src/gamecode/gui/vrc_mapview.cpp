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
_p_imgPlayerMarker( NULL ),
_levelDimensions( osg::Vec2f( 175.0f, 175.0f ) ),
_stretch( osg::Vec2f( 1.0f, -1.0f ) ),
_offset( osg::Vec2f( 100.0f, 100.0f ) ),
_screenSize( osg::Vec2f( 800.0f, 600.0f ) )
{
    // register entity attributes
    getAttributeManager().addAttribute( "minMapFile",       _minMapFile      );
    getAttributeManager().addAttribute( "align",            _align           );
    getAttributeManager().addAttribute( "size",             _size            );
    getAttributeManager().addAttribute( "transparency",     _alpha           );
    getAttributeManager().addAttribute( "levelDimensions",  _levelDimensions );
    getAttributeManager().addAttribute( "stretch",          _stretch         );
    getAttributeManager().addAttribute( "offset",           _offset          );
}

EnMapView::~EnMapView()
{
    destroyMapView();
    // register to get notified whenever the player list changes
    vrc::gameutils::PlayerUtils::get()->registerNotificationPlayerListChanged( this, false );
}

void EnMapView::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle notifications
    switch( notification.getId() )
    {
        case VRC_NOTIFY_PLAYERLIST_CHANGED:
            // on player list changes we have to update the map information
            updatePlayerList();
            break;

        case YAF3D_NOTIFY_ENTITY_ATTRIBUTE_CHANGED:
            // recreate the map view
            destroyMapView();
            setupMapView();
            break;

        case YAF3D_NOTIFY_MENU_ENTER:
            break;

        case YAF3D_NOTIFY_MENU_LEAVE:
            break;

        case YAF3D_NOTIFY_SHUTDOWN:
            break;

        default:
            ;
    }
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

void EnMapView::initialize()
{
    // set up the map view
    setupMapView();
    // register for getting periodic updates
    yaf3d::EntityManager::get()->registerUpdate( this, true );
    // register to get notified whenever the player list changes
    vrc::gameutils::PlayerUtils::get()->registerNotificationPlayerListChanged( this, true );
}

void EnMapView::setupMapView()
{
    // get the screen size
    unsigned int windowwidth  = 600.0f;
    unsigned int windowheight = 800.0f;
    yaf3d::Application::get()->getScreenSize( windowwidth, windowheight );
    _screenSize = osg::Vec2f( static_cast< float >( windowwidth ), static_cast< float >( windowheight ) );

    // calculate internal variables
    _factor   = osg::Vec2f( ( 1.0f / _levelDimensions.x() ) * _stretch.x(), ( 1.0f / _levelDimensions.y() ) * _stretch.y() );
    _halfSize = osg::Vec2f( _size * 0.5f );

    try
    {
        // calculate position of view map
        osg::Vec2 position = calcPosition( _align, _size, _screenSize );

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

        // image for marking a player on map
        _p_imgPlayerMarker = vrc::gameutils::GuiUtils::get()->getCustomImage( IMAGE_NAME_CROSSHAIR );

        vrc::gameutils::GuiUtils::get()->getMainGuiWindow()->addChildWindow( _p_wnd );

        // update the map
        updatePlayerList();
        updateMap();
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "EnMapView: problem creating gui" << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
    }
}

void EnMapView::updateEntity( float deltaTime )
{
    updateMap();
}

osg::Vec2 EnMapView::calcPosition( const std::string& align, const osg::Vec2& size, const osg::Vec2& screenSize )
{
    osg::Vec2 pos;

    if ( align == ALIGN_TOPLEFT )
    {
        pos._v[ 0 ] = 0.0f;
        pos._v[ 1 ] = 0.0f;
    }
    else if ( align == ALIGN_TOPMIDDLE )
    {
        pos._v[ 0 ] = screenSize.x() / 2.0f - size.x() / 2.0f;
        pos._v[ 1 ] = 0.0f;
    }
    else if ( align == ALIGN_TOPRIGHT )
    {
        pos._v[ 0 ] = screenSize.x() - size.x();
        pos._v[ 1 ] = 0.0f;
    }
    else if ( align == ALIGN_BOTTOMLEFT )
    {
        pos._v[ 0 ] = 0.0f;
        pos._v[ 1 ] = screenSize.y() - size.y();
    }
    else if ( align == ALIGN_BOTTOMMIDDLE )
    {
        pos._v[ 0 ] = screenSize.x() / 2.0f - size.x() / 2.0f;
        pos._v[ 1 ] = screenSize.y() - size.y();
    }
    else if ( align == ALIGN_BOTTOMRIGHT )
    {
        pos._v[ 0 ] = screenSize.x() - size.x();
        pos._v[ 1 ] = screenSize.y() - size.y();
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

CEGUI::StaticImage* EnMapView::createMarkerElement( const CEGUI::Image* p_img )
{
    CEGUI::StaticImage* p_staticimg = NULL;

    // the gui elements must have non-existing names, hence we use a postfix
    static unsigned int postfixcnt = 0;
    std::stringstream postfix;
    postfix << postfixcnt++;

    try
    {
        p_staticimg = static_cast< CEGUI::StaticImage* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticImage", std::string( MAPVIEW_WND "playermarker" ) + postfix.str() ) );
        p_staticimg->setMetricsMode( CEGUI::Absolute );
        p_staticimg->setSize( CEGUI::Size( 10.0f, 10.0f ) );
        p_staticimg->setPosition( CEGUI::Point( 0.0f, 0.0f ) );
        p_staticimg->setAlpha( 1.0f );
        p_staticimg->setImage( p_img );
        p_staticimg->setBackgroundEnabled( false );
        p_staticimg->setFrameEnabled( false );
        p_staticimg->setAlwaysOnTop( true );

    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "EnMapView: problem creating marker element" << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
    }

    return p_staticimg;
}

void EnMapView::updatePlayerList()
{
    // first remove the current player markers
    {
        std::map< yaf3d::BaseEntity*, CEGUI::StaticImage* >::iterator p_beg = _playerMarkers.begin(), p_end = _playerMarkers.end();
        for ( ; p_beg != p_end; ++p_beg )
            _p_wnd->removeChildWindow( p_beg->second );

        _playerMarkers.clear();
    }
    // create the player markers for new remote player list
    {
        std::vector< yaf3d::BaseEntity* >& remoteplayers = vrc::gameutils::PlayerUtils::get()->getRemotePlayers();
        std::vector< yaf3d::BaseEntity* >::iterator p_beg = remoteplayers.begin(), p_end = remoteplayers.end();
        for ( ; p_beg != p_end; ++p_beg )
        {
            CEGUI::StaticImage* p_staticimg = createMarkerElement( _p_imgPlayerMarker );

            assert( p_staticimg && "EnMapView::updatePlayerList: internal error" );
            
            _p_wnd->addChildWindow( p_staticimg );

            // store the new marker in list
            _playerMarkers[ *p_beg ] = p_staticimg;
        }
    }
    // create marker local player
    {
        yaf3d::BaseEntity* p_localplayer = vrc::gameutils::PlayerUtils::get()->getLocalPlayer();
        if ( p_localplayer )
        {
            CEGUI::StaticImage* p_staticimg = createMarkerElement( _p_imgPlayerMarker );

            assert( p_staticimg && "EnMapView::updatePlayerList: internal error" );
            
            _p_wnd->addChildWindow( p_staticimg );

            // store the new marker in list
            _playerMarkers[ p_localplayer ] = p_staticimg;
        }
    }
}

void EnMapView::updateMap()
{
    // update marker positions
    {
        std::map< yaf3d::BaseEntity*, CEGUI::StaticImage* >::iterator p_beg = _playerMarkers.begin(), p_end = _playerMarkers.end();
        for ( ; p_beg != p_end; ++p_beg )
        {
            CEGUI::Point markerpos;
            osg::Vec3f playerpos = p_beg->first->getPosition();
            markerpos.d_x = ( ( playerpos.x() + _offset.x() ) * _factor.x() + 1.0f ) * _halfSize.x();
            markerpos.d_y = ( ( playerpos.y() + _offset.y() ) * _factor.y() + 1.0f ) * _halfSize.y();            
            p_beg->second->setPosition( markerpos );
        }
    }
}

} // namespace vrc
