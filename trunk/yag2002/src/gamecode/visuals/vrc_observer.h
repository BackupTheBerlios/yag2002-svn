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
 # entity observer
 #  this entity can be used for freely flying through a level
 #
 #   date of creation:  07/20/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_OBSERVER_H_
#define _VRC_OBSERVER_H_

#include <vrc_main.h>

namespace vrc
{
//! Entity type name
#define ENTITY_NAME_OBSERVER    "Observer"

class EnCamera;
class ObserverIH;

//! Observer entity
class EnObserver :  public yaf3d::BaseEntity
{
    public:
                                                    EnObserver();

        virtual                                     ~EnObserver();

        //! Initializing function
        void                                        initialize();

        //! Post-initialize the entity
        void                                        postInitialize();

        //! This entity does not need a transformation node.
        const bool                                  isTransformable() const { return false; }

        //! Set the persistence flag. 
        //! Note: this flag is checked by framework on destruction of a level.
        void                                        setPersistent( bool persistence ) { _isPersistent = persistence; }

        //! This entity can be either persistent or not!
        const bool                                  isPersistent() const { return _isPersistent; }

    protected:

        //! Update entity
        void                                        updateEntity( float deltaTime );

        //! This entity is persistent so we have to handle entity's update registration on every level loading and
        //  destruction ourselves.
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        //! Enable / disable info window rendering
        void                                        enableInfoWindow( bool en );

        //! Gui callback for speed scrollbar        
        bool                                        onSpeedChanged( const CEGUI::EventArgs& arg );

        //! Gui callback for movement locking checkbox
        bool                                        onLockChanged( const CEGUI::EventArgs& arg );

        //! Gui callback for closing info window
        bool                                        onClickedClose( const CEGUI::EventArgs& arg );

        // Entity parameters

        //! Initial position
        osg::Vec3f                                  _position;

        //! Initial rotation in degree
        osg::Vec3f                                  _rotation;

        //! Maximal movement speed
        float                                       _maxSpeed;

        //! Time passed since last update
        float                                       _deltaTime;

        //! Persistence flag
        bool                                        _isPersistent;

        //! Dirty flag
        bool                                        _needUpdate;

        //! Observer's camera
        EnCamera*                                   _p_cameraEntity;

        //! Input handler
        osg::ref_ptr< ObserverIH >                  _inputHandler;

        //! GUI window
        CEGUI::FrameWindow*                         _p_wnd;

        CEGUI::StaticText*                          _p_outputText;

        CEGUI::Scrollbar*                           _p_speedBar;

        CEGUI::Checkbox*                            _p_lockCheckbox;

        //! Current movement speed
        float                                       _speed;

        //! Flag showing if the info window is enabled
        bool                                        _infoWindowEnable;

        float                                       _fpsTimer;

        unsigned int                                _fpsCounter;

        unsigned int                                _fps;

    friend class ObserverIH;
};

//! Entity type definition used for type registry
class ObserverEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    ObserverEntityFactory() : 
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_OBSERVER, yaf3d::BaseEntityFactory::Standalone | yaf3d::BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~ObserverEntityFactory() {}

        Macro_CreateEntity( EnObserver );
};

}

#endif // _VRC_OBSERVER_H_
