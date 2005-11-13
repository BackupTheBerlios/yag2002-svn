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
 # entity candle including fire rendering and light animation
 #
 #   date of creation:  07/24/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_CANDLE_H_
#define _CTD_CANDLE_H_

#include <ctd_main.h>

namespace CTD
{

#define ENTITY_NAME_CANDLE    "Candle"

class EnPointLight;

//! This entity implements a candle fire
class EnCandle :  public BaseEntity
{
    public:
                                                    EnCandle();

        virtual                                     ~EnCandle();

        
        //! Post-initialize
        void                                        postInitialize();

    protected:

        // Entity attribbutes
        //--------------------------------------------------------//

        std::string                                 _lightEntity;

        osg::Vec3f                                  _position;

        float                                       _sizeX;

        float                                       _sizeY;

    protected:

        //! Update entity
        void                                        updateEntity( float deltaTime );

        // internals
        //--------------------------------------------------------//

        //! Notification call-back
        void                                        handleNotification( const EntityNotification& notification );

        EnPointLight*                               _p_pointLightEntity;
};

//! Entity type definition used for type registry
class CandleEntityFactory : public BaseEntityFactory
{
    public:
                                                    CandleEntityFactory() : 
                                                     BaseEntityFactory( ENTITY_NAME_CANDLE, BaseEntityFactory::Standalone | BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~CandleEntityFactory() {}

        Macro_CreateEntity( EnCandle );
};

}

#endif // _CTD_CANDLE_H_