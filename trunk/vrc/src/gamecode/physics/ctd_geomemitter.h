/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2002-2004, Ali Botorabi
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
 # entity GeomEmitter, it emitts box, sphere, cube, etc, into world
 #  it is just a kind of physics demonstrator
 #
 #   date of creation:  02/25/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_GEOMEMITTER_H_
#define _CTD_GEOMEMITTER_H_

#include <ctd_base.h>
#include <ctd_baseentity.h>
#include <ctd_entitymanager.h>

namespace CTD
{

#define ENTITY_NAME_GEOMEMITTER    "GeomEmitter"

//! GeoEmitter Entity
class EnGeomEmitter : public BaseEntity
{
    public:
                                                    EnGeomEmitter();

        virtual                                     ~EnGeomEmitter();

        //! Post-Initialize 
        void                                        postInitialize();

        //! This entity needs updating
        void                                        updateEntity( float deltaTime );

    protected:

        //! Supported geometry types -- spcace-sparated -- in emitter container ( PhysicsBox, PhysicsSphere, etc. )       
        std::string                                 _geomTypes;

        osg::Vec3f                                  _position;

        //! List of instantiated entities which are cloned during emitting
        std::vector< BaseEntity* >                  _geomStock;

        //! List of active geoms
        std::vector< std::pair< BaseEntity*, float > > _geoms;

        float                                       _time;

        int                                         _geomCount;

};

//! Entity type definition used for type registry
class GeomEmitteEntityFactory : public BaseEntityFactory
{
    public:
                                                    GeomEmitteEntityFactory() : BaseEntityFactory(ENTITY_NAME_GEOMEMITTER) {}

        virtual                                     ~GeomEmitteEntityFactory() {}

        Macro_CreateEntity( EnGeomEmitter );
};

}

#endif // _CTD_GEOEMITTER_H_
