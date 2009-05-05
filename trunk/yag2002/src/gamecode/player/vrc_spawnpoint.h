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
 # entity spawn point used by player when it enters the world
 #  the player entity can use this entity to get next possible
 #  spawn point considering other players which may occupy some
 #  spawn points ( e.g. because they just respawned too ).
 #
 #   date of creation:  06/21/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _VRC_SPAWNPOINT_H_
#define _VRC_SPAWNPOINT_H_

#include <vrc_main.h>

namespace vrc
{
//! Minimum radius of free area around a spawn point which is evaluated for selecting a spawn point
#define SPAWN_MIN_FREE_RADIUS      5.0f


#define ENTITY_NAME_SPAWNPOINT    "SpawnPoint"

class EnSpawnPoint :  public yaf3d::BaseEntity
{
    public:
                                                    EnSpawnPoint();

        virtual                                     ~EnSpawnPoint();
        
        //! Initializing function, this is called after all engine modules are initialized and a map is loaded.
        void                                        initialize();

        //! Get next possible spawn point, if all are occupied then return false.
        static bool                                 getNextSpawnPoint( osg::Vec3f& pos, osg::Quat& rot );

        const osg::Vec3f&                           getSpawnPosition() const { return _position; }

        const osg::Quat&                            getSpawnRotation() const { return _quat; }

        //! This entity needs not transformation
        bool                                        isTransformable() const { return false; }

    protected:

        //! Handle level changing etc.
        void                                        handleNotification( const yaf3d::EntityNotification& notification );

        osg::Vec3f                                  _position;

        osg::Vec3f                                  _rotation;

        osg::Quat                                   _quat;

        static std::vector< EnSpawnPoint* >         _allSpawnPoints;
};

//! Entity type definition used for type registry
class SpawnPointEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    SpawnPointEntityFactory() : 
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_SPAWNPOINT, yaf3d::BaseEntityFactory::Standalone | yaf3d::BaseEntityFactory::Server )
                                                    {}

        virtual                                     ~SpawnPointEntityFactory() {}

        Macro_CreateEntity( EnSpawnPoint );
};

}

#endif // _VRC_SPAWNPOINT_H_
