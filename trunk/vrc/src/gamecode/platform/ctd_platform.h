/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2004-2006, Ali Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU General Public License 
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public 
 *  License along with this program; if not, write to the Free 
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 * 
 ****************************************************************/

/*###############################################################
 # entity Platform
 #
 #   date of creation:  02/18/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_PLATFORM_H_
#define _CTD_PLATFORM_H_

#include <ctd_main.h>

namespace CTD
{

#define ENTITY_NAME_PLATFORM    "Platform"

//! Platform Entity
class EnPlatform : public BaseEntity
{
    public:
                                                    EnPlatform();

        virtual                                     ~EnPlatform();

        //! Initialize 
        void                                        initialize();

        //! This entity needs updating
        void                                        updateEntity( float deltaTime );

    protected:

        std::string                                 _meshFile;

        float                                       _speed;

        bool                                        _loop;

        osg::Vec3f                                  _position;

};

//! Entity type definition used for type registry
class PlatformEntityFactory : public BaseEntityFactory
{
    public:
                                                    PlatformEntityFactory() : BaseEntityFactory(ENTITY_NAME_PLATFORM) {}

        virtual                                     ~PlatformEntityFactory() {}

        Macro_CreateEntity( EnPlatform );
};

}

#endif // _CTD_PLATFORM_H_
