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
 # entity for displaying fps
 #
 #   date of creation:  05/23/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_FPSDISPLAY_H_
#define _VRC_FPSDISPLAY_H_

#include <vrc_main.h>

namespace vrc
{

#define ENTITY_NAME_FPSDISPLAY    "FPSDisplay"

//! FPS display entity
class EnFPSDisplay :  public yaf3d::BaseEntity
{
    public:
                                                    EnFPSDisplay();

        virtual                                     ~EnFPSDisplay();

        //! Initialize entity
        void                                        initialize();

        //! Override this method and return true to get a persisten entity
        const bool                                  isPersistent() const { return true; }

        //! This entity needs no transformation
        const bool                                  isTransformable() const { return false; }

        //! Enable / disable statistics rendering
        void                                        enable( bool en );

    protected:

        osg::Vec3f                                  _position;


    protected:

        //! Update entity
        void                                        updateEntity( float deltaTime );

        //! Override notification callback
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        float                                       _fpsTimer;

        unsigned int                                _fps;

        bool                                        _enable;

        // Statistics window
        CEGUI::Window*                              _p_wnd;

        CEGUI::StaticText*                          _p_outputText;
};

//! Entity type definition used for type registry
class FPSDisplayEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    FPSDisplayEntityFactory() : 
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_FPSDISPLAY, yaf3d::BaseEntityFactory::Standalone | yaf3d::BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~FPSDisplayEntityFactory() {}

        Macro_CreateEntity( EnFPSDisplay );
};

} // namespace vrc

#endif // _VRC_FPSDISPLAY_H_
