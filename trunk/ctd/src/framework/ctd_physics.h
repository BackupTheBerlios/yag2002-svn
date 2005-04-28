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
 # physics system 
 #
 #   date of creation:  02/20/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_PHYSICS_H_
#define _CTD_PHYSICS_H_

#include <ctd_base.h>
#include "ctd_singleton.h"

namespace CTD
{

class Application;
class LevelManager;
class PhysicsDebugDrawable;
struct CollisionStruct;

//! Physics core
class Physics : public Singleton< Physics >
{
    public:

        //! Predefined material ids which are stored in NodeMask
        enum {
                NO_BUILD     = 0xFF, // this means build no static collision geometry for this node
                MAT_DEFAULT  = 0x01, // "default", no specific material
                MAT_WOOD     = 0x02, // "wood"
                MAT_STONE    = 0x03, // "stone"
                MAT_METALL   = 0x04, // "metal"
                MAT_GRASS    = 0x05, // "grass"
                MAT_NOCOL    = 0x0F, // "nocol", no collision, use this for passable objects which should be detected though (e.g. for playing a sound)
                //----------------
                MAT_eof
        } MaterialID;

        //! Update physics
        inline void                                 update( float deltaTime );

        //! Return the physics world instance
        inline NewtonWorld*                         getWorld();

        //! Set world gravity
        inline void                                 setWorldGravity( float gravity );

        //! Get world gravity
        inline const float&                         getWorldGravity();

        //! Enables rendering of debug information such as collision faces
        void                                        enableDebugRender();

        //! Disable debug rendering
        void                                        disableDebugRender();

        //! Given a meterial type name retrieve its physics id. 
        /**
        * Follwing materials are predefined: default, wood, stone, metall, grass
        */
        int                                         getMaterialId( const std::string& materialType );

        //! Create a new material id for given name        
        int                                         createMaterialID( const std::string& materialType );

        //! Generic collision callback function for contact begin ( this can be used in entity material definitions )
        static int                                  genericContactBegin( const NewtonMaterial* p_material, const NewtonBody* p_body0, const NewtonBody* p_body1 );

        //! Generic collision callback function for contact process ( this can be used in entity material definitions )
        static int                                  genericContactProcess( const NewtonMaterial* p_material, const NewtonContact* p_contact );
        
        //! Generic collision callback function for contact end ( this can be used in entity material definitions )
        static void                                 genericContactEnd( const NewtonMaterial* p_material );
        
        //! Returns the collision struct which is modified in contact process funtions above.
        //!  Use this struct to get tangent and normal speed of collision after the genericContactEnd function.
        static CollisionStruct*                     getCollisionStruct();

        //! Set the collision struct.
        //!  This method can be used to mix entity-specific physics callbacks and Physics' callbacks
        static void                                 setCollisionStruct( CollisionStruct* p_colStruct );

        //! Collision callback function for level contact processing ( this can be used in entity material definitions )
        static int                                  levelContactProcess( const NewtonMaterial* p_material, const NewtonContact* p_contact );

    protected:

                                                    Physics();

                                                    Physics( const Physics& );

        virtual                                     ~Physics();

        Physics&                                    operator = ( const Physics& );

        //! Initialize physics system
        bool                                        initialize();

        //! Re-Initialize physics system, used on subsequent level loadings
        /*!
        * This call destroys the physics world and initialize it again to allow building new static world geometry.
        */
        bool                                        reinitialize();

        //! Shutdown physics system
        void                                        shutdown();

        //! Build physics world for static geometry given a node.
        /**
        * Note: this process is not done in 'initialize' in order to give entities the
        * opportunity for defining own physics materials in their constructors.
        * The static level geometry is added to given root node ( in normal case is this the top node ).
        */
        bool                                        buildStaticGeometry( osg::Node* p_root );

        //! Create the material graph
        void                                        setupMaterials();

        NewtonWorld*                                _p_world;

        NewtonCollision*                            _p_collision;

        NewtonBody*                                 _p_body;

        osg::Geode*                                 _p_debugGeode;

        float                                       _gravity;

        std::map< std::string, int >                _materials;

    friend class Singleton< Physics >;
    friend class PhysicsDebugDrawable;
    friend class LevelManager;
    friend class Application;
};

// inlines
inline NewtonWorld* Physics::getWorld()
{
    return _p_world;
}

inline void Physics::setWorldGravity( float gravity )
{
    _gravity = gravity;
}

inline const float& Physics::getWorldGravity()
{
    return _gravity;
}

inline void Physics::update( float deltaTime )
{ 
    NewtonUpdate( _p_world, deltaTime );
}

//-------------------------------------//

// default collision struct for materials
struct  CollisionStruct
{
    NewtonBody*      _p_body0;

    NewtonBody*      _p_body1;
    
    osg::Vec3f       _position;
    
    float            _contactMaxNormalSpeed;
    
    float            _contactMaxTangentSpeed;
};

// predefined material-material collision structures
extern CollisionStruct level_woodCollStruct;
extern CollisionStruct level_metalCollStruct;
extern CollisionStruct level_grassCollStruct;
extern CollisionStruct level_stoneCollStruct;
extern CollisionStruct wood_woodCollStruct;
extern CollisionStruct wood_metalCollStruct;
extern CollisionStruct wood_grassCollStruct;
extern CollisionStruct wood_stoneCollStruct;
extern CollisionStruct metal_metalCollStruct;
extern CollisionStruct metal_grassCollStruct;
extern CollisionStruct metal_stoneCollStruct;
extern CollisionStruct stone_stoneCollStruct;
extern CollisionStruct stone_grassCollStruct;
extern CollisionStruct grass_grassCollStruct;

}

#endif // _CTD_PHYSICS_H_
