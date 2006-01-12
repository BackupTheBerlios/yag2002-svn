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

#ifndef _VRC_MAPVIEW_H_
#define _VRC_MAPVIEW_H_

#include <vrc_main.h>

namespace vrc
{

#define ENTITY_NAME_MAPVIEW    "MapView"

//! Map view entity
class EnMapView :  public yaf3d::BaseEntity
{
    public:
                                                    EnMapView();

        virtual                                     ~EnMapView();

        //! Initializing function, this is called after all engine modules are initialized and a level is loaded.
        void                                        initialize();

        //! Minimize mini map
        void                                        minimize();

        //! Maximize mini map
        void                                        maximize();

        //! Set the map mode, mini map / full-window
        void                                        setMode( bool minimap );

    protected:

        //! Periodic update
        void                                        updateEntity( float deltaTime );

        //! This entity is persistent so it has to trigger its destruction on shutdown ifself.
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        //! Set up the map view
        void                                        setupMapView();

        //! Update the map information
        void                                        updateMap();

        //! Update the player list and related gui elements
        void                                        updatePlayerList();

        //! Destory the map view
        void                                        destroyMapView();

        //! Calculate absolute position giving an aligning rule and the view and screen size
        osg::Vec2                                   calcPosition( const std::string& align, const osg::Vec2& mapsize, const osg::Vec2& screenSize );

        //! Helper method for creating a staticimage element given an image
        CEGUI::StaticImage*                         createMarkerElement( const CEGUI::Image* p_img );

        //! Consider snapping for given position, return true if snapping happened ( used for dragging the map view )
        bool                                        snapMapView( CEGUI::Point& pos );

        //! Calculate 6 snap points on screen given map view and screen size
        std::vector< CEGUI::Point >                 calcSnapPoints( const osg::Vec2& mapsize, const osg::Vec2& screenSize );

        //! Callback for mouse down ( used for dragging )
        bool                                        onMouseDown( const CEGUI::EventArgs& arg );

        //! Callback for mouse up ( used for dragging )
        bool                                        onMouseUp( const CEGUI::EventArgs& arg );

        //! Callback for mouse move ( used for dragging )
        bool                                        onMouseMove( const CEGUI::EventArgs& arg );

        //! File name of mini map ( currently it can be only in tga format )
        std::string                                 _minMapFile;

        //! Align the map window, valid values: topleft, topmiddle, topright, bottomleft, bottommiddle, bottomright
        std::string                                 _align;

        //! Size of mini map ( screen absolute values )
        osg::Vec2f                                  _size;

        //! Transparency of map view
        float                                       _alpha;

        osg::Vec2f                                  _levelDimensions;

        osg::Vec2f                                  _offset;

        osg::Vec2f                                  _stretch;

        //! GUI stuff
        CEGUI::Window*                              _p_wnd;

        //! Map marker for players
        const CEGUI::Image*                         _p_imgPlayerMarker;

        //! Screen width
        osg::Vec2f                                  _screenSize;

        //! For internal use
        osg::Vec2f                                  _factor;

        //! For internal use
        osg::Vec2f                                  _halfSize;

        //! Flag showing dragging the map view
        bool                                        _dragging;

        //! Mouse position on dragging the map view
        CEGUI::Point                                _dragPosition;

        //! 6 snap points used for dragging the map view
        std::vector< CEGUI::Point >                 _snapPoints;

        //! Map of players and associated markers
        std::map< yaf3d::BaseEntity*, CEGUI::StaticImage* >  _playerMarkers;
};

//! Entity type definition used for type registry
class MapViewEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    MapViewEntityFactory() :
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_MAPVIEW, yaf3d::BaseEntityFactory::Standalone | yaf3d::BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~MapViewEntityFactory() {}

        Macro_CreateEntity( EnMapView );
};

} // namespace vrc

#endif // _VRC_MAPVIEW_H_
