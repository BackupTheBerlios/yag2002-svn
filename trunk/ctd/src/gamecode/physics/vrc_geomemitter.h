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
 # entity GeomEmitter, it emitts box, sphere, cube, etc, into world
 #  it is just a kind of physics demonstrator
 #
 #   date of creation:  02/25/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_GEOMEMITTER_H_
#define _VRC_GEOMEMITTER_H_

#include <vrc_main.h>

namespace vrc
{

#define ENTITY_NAME_GEOMEMITTER    "GeomEmitter"

//! GeoEmitter Entity
class EnGeomEmitter : public yaf3d::BaseEntity
{
    public:
                                                    EnGeomEmitter();

        virtual                                     ~EnGeomEmitter();

        //! Post-Initialize 
        void                                        postInitialize();

    protected:

        //! This entity needs updating
        void                                        updateEntity( float deltaTime );

        //! Supported geometry types -- spcace-sparated -- in emitter container ( PhysicsBox, PhysicsSphere, etc. )       
        std::string                                 _geomTypes;

        //! Position of emitter
        osg::Vec3f                                  _position;

        //! Dimensions of emitter describing a cube, in this cube the pyhsics bodies are created
        osg::Vec3f                                  _dimensions;

        //! Time which has to elapsed before creating a new physics body
        float                                       _period;

        //! Avarage life time of bodies
        float                                       _life;

        //! List of instantiated entities which are cloned during emitting
        std::vector< yaf3d::BaseEntity* >                  _geomStock;

        //! List of active geoms
        std::vector< std::pair< yaf3d::BaseEntity*, float > > _geoms;

        float                                       _time;

        int                                         _geomCount;

};

//! Entity type definition used for type registry
class GeomEmitteEntityFactory : public yaf3d::BaseEntityFactory
{
    public:
                                                    GeomEmitteEntityFactory() : 
                                                     yaf3d::BaseEntityFactory( ENTITY_NAME_GEOMEMITTER, yaf3d::BaseEntityFactory::Standalone | yaf3d::BaseEntityFactory::Client )
                                                    {}

        virtual                                     ~GeomEmitteEntityFactory() {}

        Macro_CreateEntity( EnGeomEmitter );
};

}

#endif // _VRC_GEOEMITTER_H_
