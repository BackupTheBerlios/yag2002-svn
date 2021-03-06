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
 # physics system
 #
 #   date of creation:  02/20/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <base.h>
#include "log.h"
#include "utils.h"
#include "physics.h"
#include "levelmanager.h"
#include "application.h"
#include "physics_helpers.h"
#include <osg/Transform>

namespace yaf3d
{

YAF3D_SINGLETON_IMPL( Physics )

// default collision struct for materials
CollisionStruct defaultCollStruct;
// predefined material-material collision structures
CollisionStruct level_woodCollStruct;
CollisionStruct level_metalCollStruct;
CollisionStruct level_grassCollStruct;
CollisionStruct level_stoneCollStruct;
CollisionStruct wood_woodCollStruct;
CollisionStruct wood_metalCollStruct;
CollisionStruct wood_grassCollStruct;
CollisionStruct wood_stoneCollStruct;
CollisionStruct metal_metalCollStruct;
CollisionStruct metal_grassCollStruct;
CollisionStruct metal_stoneCollStruct;
CollisionStruct stone_stoneCollStruct;
CollisionStruct stone_grassCollStruct;
CollisionStruct grass_grassCollStruct;

static CollisionStruct* s_colStruct;

// this callback is used for visualizing colliding faces, for definition see below
void levelCollisionCallback (
                                const NewtonBody*   p_bodyWithTreeCollision,
                                const NewtonBody*   p_body,
                                const float*        p_vertex,
                                int                 vertexstrideInBytes,
                                int                 indexCount,
                                const int*          p_indexArray
                            );


static unsigned int freedBytesSum = 0;
static unsigned int allocBytesSum = 0;

// memory allocation for Newton
void* physicsAlloc( int sizeInBytes )
{
    allocBytesSum += sizeInBytes;
    return new char[ sizeInBytes ];
}

// memory de-allocation for Newton
void physicsFree( void* ptr, int sizeInBytes )
{
    freedBytesSum += sizeInBytes;
    delete[] static_cast< char* >( ptr );
}

// implementation of physics core
//-------------------------------
Physics::Physics() :
_p_world( NULL ),
_p_body( NULL ),
_p_debugGeode( NULL ),
_gravity( -9.8f )
{
}

Physics::~Physics()
{
}

bool Physics::initialize()
{
    // init physics engine
    _p_world = NewtonCreate( physicsAlloc, physicsFree );
    assert( _p_world );

    // set minimum frame rate
    NewtonSetMinimumFrameRate( _p_world, 1.0f / FIX_PHYSICS_UPDATE_PERIOD );

    NewtonSetSolverModel( _p_world, 0 );   // 0: exact, 1 adaptive, n linear. for games linear is ok
    NewtonSetFrictionModel( _p_world, 0 ); // 0: exact, 1 adaptive

    // setup all predefined materials
    setupMaterials();
    return true;
}

bool Physics::reinitialize()
{
    // free up bodies
    NewtonDestroyAllBodies( _p_world );
    // free all materials
    NewtonMaterialDestroyAllGroupID( _p_world );
    // destroy the existing physics world and initialize things again
    NewtonDestroy( _p_world );
    _p_world = NULL;

    // clear the material cache
    _materials.clear();

    return initialize();
}

void Physics::update( float deltaTime )
{
    if ( !_p_world )
        return;

    // Newton must be updated with fixed timesteps
    static float time_elapsed = 0.0f;
    time_elapsed += deltaTime;
    if ( time_elapsed > FIX_PHYSICS_UPDATE_PERIOD )
    {
        do
        {
            NewtonUpdate( _p_world, FIX_PHYSICS_UPDATE_PERIOD );
            time_elapsed -= FIX_PHYSICS_UPDATE_PERIOD;
        }
        while( time_elapsed > FIX_PHYSICS_UPDATE_PERIOD );
    }
}

void Physics::shutdown()
{
    log_info << "Physics: shutting down" << std::endl;

    if ( _p_world )
    {
        // free up bodies
        NewtonDestroyAllBodies( _p_world );
        // free all materials
        NewtonMaterialDestroyAllGroupID( _p_world );
        // destroy the existing physics world and initialize things again
        NewtonDestroy( _p_world );
        _p_world = NULL;
    }

    log_verbose << "Physics: remaining non-freed bytes: " <<  allocBytesSum - freedBytesSum << std::endl;

    // destroy singleton
    destroy();
}

void serializationCallback( void* p_serializeHandle, const void* p_buffer, size_t size )
{
    std::ofstream* p_stream  = static_cast< std::ofstream* >( p_serializeHandle );
    const char*    p_charbuf = static_cast< const char* >( p_buffer );
    for ( size_t cnt = 0; cnt < size; ++cnt )
        p_stream->put( p_charbuf[ cnt ] );
}

void deserializationCallback( void* p_serializeHandle, void* p_buffer, size_t size )
{
    std::ifstream* p_stream  = static_cast< std::ifstream* >( p_serializeHandle );
    char*          p_charbuf = static_cast< char* >( p_buffer );
    for ( size_t cnt = 0; cnt < size; ++cnt )
        p_stream->get( p_charbuf[ cnt ] );
}

bool Physics::serialize( const std::string& meshFile, const std::string& outputFile )
{
    log_debug << "Physics: serializing physics static geometry '" << meshFile << "'" << std::endl;
    log_debug << "Physics: unloading existing level" << std::endl;
    // first unload existing level
    LevelManager::get()->unloadLevel( true, true );

    osg::ref_ptr< osg::Node >  meshnode = LevelManager::get()->loadMesh( meshFile, false );
    if ( !meshnode.valid() )
        return false;

    osg::ref_ptr< osg::Group > root = new osg::Group;
    root->addChild( meshnode.get() );

    // begin build the collision faces
    //--------------------------
    NewtonCollision* p_collision = NewtonCreateTreeCollision( _p_world, levelCollisionCallback );
    NewtonTreeCollisionBeginBuild( p_collision );

    // start timer
    osg::Timer_t start_tick = osg::Timer::instance()->tick();
    //! iterate through all geometries and create their collision faces
    PhysicsVisitor physVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN, p_collision );
    root->accept( physVisitor );
    // stop timer and give out the time messure
    osg::Timer_t end_tick = osg::Timer::instance()->tick();
    log_debug << "Physics: elapsed time for building physics collision faces = "<< osg::Timer::instance()->delta_s( start_tick, end_tick ) << std::endl;
    log_debug << "Physics:  total num of evaluated primitives: " << physVisitor.getNumPrimitives() << std::endl;
    log_debug << "Physics:  total num of vertices: " << physVisitor.getNumVertices() << std::endl;

    //--------------------------
    // finalize tree building
    NewtonTreeCollisionEndBuild( p_collision, 0 );

    // write out the serialization data
    std::string file( yaf3d::Application::get()->getMediaPath() + outputFile + YAF3DPHYSICS_SERIALIZE_POSTFIX );
    log_debug << "Physics: write to serialization file '" << file << "'" << std::endl;
    std::ofstream serializationoutput;
    serializationoutput.open( file.c_str(), std::ios_base::binary | std::ios_base::out );
    if ( !serializationoutput )
    {
        log_error << "Physics: cannot write to serialization file '" << file << "'" << std::endl;
        serializationoutput.close();
        NewtonReleaseCollision( _p_world, p_collision );
        return false;
    }
    NewtonTreeCollisionSerialize( p_collision, serializationCallback, &serializationoutput );
    serializationoutput.close();
    NewtonReleaseCollision( _p_world, p_collision );
    
    return true;
}

bool Physics::buildStaticGeometry( osg::Group* p_root, const std::string& levelFile )
{
    NewtonCollision* p_collision = NULL;

    // check if a serialization file exists, if so then load it. otherwise build the static geometry on the fly.
    assert( levelFile.length() && "internal error: missing levelFile name!" );
    std::string file = cleanPath( levelFile );
    std::vector< std::string > path;
    explode( file, "/", &path );
    file = yaf3d::Application::get()->getMediaPath() + YAF3DPHYSICS_MEDIA_FOLDER + path[ path.size() - 1 ] + YAF3DPHYSICS_SERIALIZE_POSTFIX;
    std::ifstream serializationfile;
    serializationfile.open( file.c_str(), std::ios_base::binary | std::ios_base::in );
    if ( !serializationfile )
    {
        log_warning << "Physics: no serialization file for physics static geometry exists, building on-the-fly ..." << std::endl;

        p_collision = NewtonCreateTreeCollision( _p_world, levelCollisionCallback );
        NewtonTreeCollisionBeginBuild( p_collision );

        // build the collision faces
        //--------------------------
        // start timer
        osg::Timer_t start_tick = osg::Timer::instance()->tick();
        //! iterate through all geometries and create their collision faces
        PhysicsVisitor physVisitor( osg::NodeVisitor::TRAVERSE_ALL_CHILDREN, p_collision );
        p_root->accept( physVisitor );
        // stop timer and give out the time messure
        osg::Timer_t end_tick = osg::Timer::instance()->tick();
        log_debug << "Physics: elapsed time for building physics collision faces = "<< osg::Timer::instance()->delta_s( start_tick, end_tick ) << std::endl;
        log_debug << "Physics:  total num of evaluated primitives: " << physVisitor.getNumPrimitives() << std::endl;
        log_debug << "Physics:  total num of vertices: " << physVisitor.getNumVertices() << std::endl;

        //--------------------------
        // finalize tree building with optimization off ( because the meshes are already optimized by
        //  osg _and_ Newton has currently problems with optimization )
        NewtonTreeCollisionEndBuild( p_collision, 0 /* 1 */);
    }
    else
    {
        log_debug << "Physics: loading serialization file for physics static geometry: '" << file << "' ..." << std::endl;

        // start timer
        osg::Timer_t start_tick = osg::Timer::instance()->tick();

        p_collision = NewtonCreateTreeCollisionFromSerialization( _p_world, NULL, deserializationCallback, &serializationfile );
        assert( p_collision && "internal error, something went wrong during physics deserialization!" );

        // stop timer and give out the time messure
        osg::Timer_t end_tick = osg::Timer::instance()->tick();
        log_debug << "Physics: elapsed time for deserializing physics collision faces = "<< osg::Timer::instance()->delta_s( start_tick, end_tick ) << std::endl;

        serializationfile.close();
    }

    _p_body = NewtonCreateBody( _p_world, p_collision );

    // release collision object
    NewtonReleaseCollision( _p_world, p_collision );

    // set Material Id for this object
    NewtonBodySetMaterialGroupID( _p_body, getMaterialId( "level" ) );

    osg::Matrixf mat;
    mat.identity();
    NewtonBodySetMatrix( _p_body, mat.ptr() );

    // calculate the world bbox and world size
    float  bmin[ 4 ], bmax[ 4 ];
    NewtonCollisionCalculateAABB( p_collision, mat.ptr(), bmin, bmax );
    bmin[ 0 ] -= 10.0f;
    bmin[ 1 ] -= 10.0f;
    bmin[ 2 ] -= 10.0f;
    bmin[ 3 ] = 1.0f;
    bmax[ 0 ] += 10.0f;
    bmax[ 1 ] += 10.0f;
    bmax[ 2 ] += 10.0f;
    bmax[ 3 ] = 1.0f;
    NewtonSetWorldSize( _p_world, bmin, bmax );

    return true;
}

void Physics::setupMaterials()
{
    assert( _p_world );

    // get the default material ID
    int defaultID = NewtonMaterialGetDefaultGroupID( _p_world );
    // set default material properties
    NewtonMaterialSetDefaultSoftness( _p_world, defaultID, defaultID, 0.05f );
    NewtonMaterialSetDefaultElasticity( _p_world, defaultID, defaultID, 0.4f );
    NewtonMaterialSetDefaultCollidable( _p_world, defaultID, defaultID, 1 );
    NewtonMaterialSetDefaultFriction( _p_world, defaultID, defaultID, 1.0f, 0.5f );
    NewtonMaterialSetCollisionCallback( _p_world, defaultID, defaultID, &defaultCollStruct, genericContactBegin, genericContactProcess, genericContactEnd );

    // create all predefined materials IDs
    int nocolID = NewtonMaterialCreateGroupID( _p_world );
    int wallID  = NewtonMaterialCreateGroupID( _p_world );
    int levelID = NewtonMaterialCreateGroupID( _p_world );
    int woodID  = NewtonMaterialCreateGroupID( _p_world );
    int metalID = NewtonMaterialCreateGroupID( _p_world );
    int stoneID = NewtonMaterialCreateGroupID( _p_world );
    int grassID = NewtonMaterialCreateGroupID( _p_world );

    _materials.insert( std::make_pair( "default", defaultID ) );
    _materials.insert( std::make_pair( "wall"   , wallID    ) );
    _materials.insert( std::make_pair( "nocol"  , nocolID   ) );
    _materials.insert( std::make_pair( "level"  , levelID   ) );
    _materials.insert( std::make_pair( "wood"   , woodID    ) );
    _materials.insert( std::make_pair( "metal"  , metalID   ) );
    _materials.insert( std::make_pair( "stone"  , stoneID   ) );
    _materials.insert( std::make_pair( "grass"  , grassID   ) );

    // set non-colliding pairs
    NewtonMaterialSetDefaultCollidable( _p_world, nocolID, defaultID, 0 );
    NewtonMaterialSetDefaultCollidable( _p_world, nocolID, wallID,    0 );
    NewtonMaterialSetDefaultCollidable( _p_world, nocolID, levelID,   0 );
    NewtonMaterialSetDefaultCollidable( _p_world, nocolID, woodID,    0 );
    NewtonMaterialSetDefaultCollidable( _p_world, nocolID, metalID,   0 );
    NewtonMaterialSetDefaultCollidable( _p_world, nocolID, grassID,   0 );
    NewtonMaterialSetDefaultCollidable( _p_world, nocolID, stoneID,   0 );

    // set the material properties for level on wood
    NewtonMaterialSetDefaultElasticity( _p_world, levelID, woodID, 0.5f );
    NewtonMaterialSetDefaultFriction  ( _p_world, levelID, woodID, 0.7f, 0.6f);
    NewtonMaterialSetCollisionCallback( _p_world, levelID, woodID, &level_woodCollStruct, genericContactBegin, levelContactProcess, genericContactEnd );

    // set the material properties for level on metal
    NewtonMaterialSetDefaultElasticity( _p_world, levelID, metalID, 0.5f );
    NewtonMaterialSetDefaultFriction  ( _p_world, levelID, metalID, 0.8f, 0.6f );
    NewtonMaterialSetCollisionCallback( _p_world, levelID, metalID, &level_metalCollStruct, genericContactBegin, levelContactProcess, genericContactEnd );

    // set the material properties for level on grass
    NewtonMaterialSetDefaultElasticity( _p_world, levelID, grassID, 0.3f );
    NewtonMaterialSetDefaultFriction  ( _p_world, levelID, grassID, 0.9f, 0.7f );
    NewtonMaterialSetCollisionCallback( _p_world, levelID, grassID, &level_grassCollStruct, genericContactBegin, levelContactProcess, genericContactEnd );

    // set the material properties for level on stone
    NewtonMaterialSetDefaultElasticity( _p_world, levelID, stoneID, 0.45f );
    NewtonMaterialSetDefaultFriction  ( _p_world, levelID, stoneID, 0.8f, 0.7f );
    NewtonMaterialSetCollisionCallback( _p_world, levelID, stoneID, &level_stoneCollStruct, genericContactBegin, levelContactProcess, genericContactEnd );

    //------

    // set the material properties for wood on wood
    NewtonMaterialSetDefaultElasticity( _p_world, woodID, woodID, 0.3f );
    NewtonMaterialSetDefaultFriction  ( _p_world, woodID, woodID, 1.1f, 0.7f);
    NewtonMaterialSetCollisionCallback( _p_world, woodID, woodID, &wood_woodCollStruct, genericContactBegin, genericContactProcess, genericContactEnd );

    // set the material properties for wood on metal
    NewtonMaterialSetDefaultElasticity( _p_world, woodID, metalID, 0.5f );
    NewtonMaterialSetDefaultFriction  ( _p_world, woodID, metalID, 0.8f, 0.6f );
    NewtonMaterialSetCollisionCallback( _p_world, woodID, metalID, &wood_metalCollStruct, genericContactBegin, genericContactProcess, genericContactEnd );

    // set the material properties for wood on grass
    NewtonMaterialSetDefaultElasticity( _p_world, woodID, grassID, 0.2f );
    NewtonMaterialSetDefaultFriction  ( _p_world, woodID, grassID, 0.9f, 0.7f );
    NewtonMaterialSetCollisionCallback( _p_world, woodID, grassID, &wood_grassCollStruct, genericContactBegin, genericContactProcess, genericContactEnd );

    // set the material properties for wood on stone
    NewtonMaterialSetDefaultElasticity( _p_world, woodID, stoneID, 0.45f );
    NewtonMaterialSetDefaultFriction  ( _p_world, woodID, stoneID, 0.7f, 0.6f );
    NewtonMaterialSetCollisionCallback( _p_world, woodID, stoneID, &wood_stoneCollStruct, genericContactBegin, genericContactProcess, genericContactEnd );

    //------

    // set the material properties for metal on metal
    NewtonMaterialSetDefaultElasticity( _p_world, metalID, metalID, 0.7f );
    NewtonMaterialSetDefaultFriction  ( _p_world, metalID, metalID, 0.5f, 0.2f );
    NewtonMaterialSetCollisionCallback( _p_world, metalID, metalID, &metal_metalCollStruct, genericContactBegin, genericContactProcess, genericContactEnd );

    // set the material properties for metal on grass
    NewtonMaterialSetDefaultElasticity( _p_world, metalID, grassID, 0.2f );
    NewtonMaterialSetDefaultFriction  ( _p_world, metalID, grassID, 0.6f, 0.5f );
    NewtonMaterialSetCollisionCallback( _p_world, metalID, grassID, &metal_grassCollStruct, genericContactBegin, genericContactProcess, genericContactEnd );

    // set the material properties for metal on stone
    NewtonMaterialSetDefaultElasticity( _p_world, metalID, stoneID, 0.6f );
    NewtonMaterialSetDefaultFriction  ( _p_world, metalID, stoneID, 0.5f, 0.4f );
    NewtonMaterialSetCollisionCallback( _p_world, metalID, stoneID, &metal_stoneCollStruct, genericContactBegin, genericContactProcess, genericContactEnd );

    //------

    // set the material properties for stone on stone
    NewtonMaterialSetDefaultElasticity( _p_world, stoneID, stoneID, 0.8f );
    NewtonMaterialSetDefaultFriction  ( _p_world, stoneID, stoneID, 0.5f, 0.2f );
    NewtonMaterialSetCollisionCallback( _p_world, stoneID, stoneID, &stone_stoneCollStruct, genericContactBegin, genericContactProcess, genericContactEnd );

    // set the material properties for stone on grass
    NewtonMaterialSetDefaultElasticity( _p_world, stoneID, grassID, 0.2f );
    NewtonMaterialSetDefaultFriction  ( _p_world, stoneID, grassID, 0.7f, 0.6f );
    NewtonMaterialSetCollisionCallback( _p_world, stoneID, grassID, &stone_grassCollStruct, genericContactBegin, genericContactProcess, genericContactEnd );

    //------

    // set the material properties for grass on grass ( may we need this!? )
    NewtonMaterialSetDefaultElasticity( _p_world, grassID, grassID, 0.1f );
    NewtonMaterialSetDefaultFriction  ( _p_world, grassID, grassID, 0.9f, 0.8f );
    NewtonMaterialSetCollisionCallback( _p_world, grassID, grassID, &grass_grassCollStruct, genericContactBegin, genericContactProcess, genericContactEnd );
}

int Physics::createMaterialID( const std::string& materialType )
{
    std::map< std::string, int >::iterator id = _materials.find( materialType );
    assert( _p_world && "physics world has not been created, first initialize the physics system" );
    assert( id == _materials.end() && "material already exists!" );

    int matID = NewtonMaterialCreateGroupID( _p_world );
    _materials.insert( make_pair( materialType, matID ) );
    return matID;
}

int Physics::getMaterialId( const std::string& materialType )
{
    std::map< std::string, int >::iterator id = _materials.find( materialType );
    assert( id != _materials.end() && "requesting for an invalid material id!" );
    return id->second;
}

void Physics::enableDebugRender()
{
    if ( _p_debugGeode )
        return;

    // create and attach the debug node into scene
    _p_debugGeode = new osg::Geode;
    PhysicsDebugDrawable* p_debugdraw = new PhysicsDebugDrawable;
    _p_debugGeode->addDrawable( p_debugdraw );
    static_cast< osg::Group* >( Application::get()->getSceneRootNode() )->addChild( _p_debugGeode );
}

void Physics::disableDebugRender()
{
   static_cast< osg::Group* >( Application::get()->getSceneRootNode() )->removeChild( _p_debugGeode );
   _p_debugGeode = NULL;
}


// collision callback and some debug functions
//--------------------------------------------

// this callback is used for visualizing colliding faces
void levelCollisionCallback (
                                const NewtonBody*   /*p_bodyWithTreeCollision*/,
                                const NewtonBody*   /*p_body*/,
                                const float*        /*p_vertex*/,
                                int                 /*vertexstrideInBytes*/,
                                int                 /*indexCount*/,
                                const int*          /*p_indexArray*/
                            )
{
    //! currently we do not need this funtionality!

    //int i;
    //int j;
    //int stride = vertexstrideInBytes / 4;

    //if ( debugLinesMode )
    //{
    //    if ( debugCount < 1000 )
    //    {
    //        j = p_indexArray[ indexCount - 1 ];
    //        Vec3f p0( p_vertex[ j * stride + 0 ], p_vertex[ j * stride + 1 ] , p_vertex[ j * stride + 2 ] );
    //        for ( i = 0; i < indexCount; ++i )
    //        {
    //            j = p_indexArray[i];
    //            Vec3f p1( p_vertex[ j * stride + 0 ], p_vertex[ j * stride + 1 ] , p_vertex[ j * stride + 2 ] );
    //            debugFace[ debugCount ][ 0 ] = p0;
    //            debugFace[ debugCount ][ 1 ] = p1;
    //            ++debugCount;
    //            p0 = p1;
    //        }
    //    }
    //}
}

// this callback is called when the two aabb boxes of the colliding objects overlap
int Physics::genericContactBegin( const NewtonMaterial* p_material, const NewtonBody* p_body0, const NewtonBody* p_body1 )
{
    // get the pointer to collision struture
    s_colStruct = ( CollisionStruct* )NewtonMaterialGetMaterialPairUserData( p_material );
    // save the colliding bodies
    s_colStruct->_p_body0 = const_cast< NewtonBody* >( p_body0 );
    s_colStruct->_p_body1 = const_cast< NewtonBody* >( p_body1 );
    // clear the contact normal speed
    s_colStruct->_contactMaxNormalSpeed  = 0.0f;
    // clear the contact sliding speed
    s_colStruct->_contactMaxTangentSpeed = 0.0f;

    // return one to tell Newton the application wants to proccess this contact
    return 1;
}

// this callback is called for every contact between the two bodies
int Physics::genericContactProcess( const NewtonMaterial* p_material, const NewtonContact* p_contact )
{
    float speed0;
    float speed1;
    osg::Vec3f normal;

    // get the maximun normal speed of this impact.
    speed0 = NewtonMaterialGetContactNormalSpeed( p_material, p_contact );
    if ( speed0 > s_colStruct->_contactMaxNormalSpeed ) {
        // save the position of the contact ( e.g. for 3d sound for particle effects )
        s_colStruct->_contactMaxNormalSpeed = speed0;
        NewtonMaterialGetContactPositionAndNormal( p_material, &s_colStruct->_position._v[ 0 ], &normal._v[ 0 ] );
    }

    // get the maximun of the two sliding contact speeds
    speed0 = NewtonMaterialGetContactTangentSpeed( p_material, p_contact, 0 );
    speed1 = NewtonMaterialGetContactTangentSpeed( p_material, p_contact, 1 );
    if ( speed1 > speed0 )
        speed0 = speed1;

    // get the maximun tangent speed of this contact. this can be used e.g. for particles(sparks) or playing scratch sounds
    if ( speed0 > s_colStruct->_contactMaxTangentSpeed ) {
        // save the position of the contact (for 3d sound or particle effects)
        s_colStruct->_contactMaxTangentSpeed = speed0;
        NewtonMaterialGetContactPositionAndNormal( p_material, &s_colStruct->_position._v[ 0 ], &normal._v[ 0 ] );
    }

    // return one to tell Newton we want to accept this contact
    return 1;
}

// this function is called affter all collision contacts are proccesed
void Physics::genericContactEnd( const NewtonMaterial* /*p_material*/ )
{
}

CollisionStruct* Physics::getCollisionStruct()
{
    return s_colStruct;
}

void Physics::setCollisionStruct( CollisionStruct* p_colStruct )
{
    s_colStruct = p_colStruct;
}

int Physics::levelContactProcess( const NewtonMaterial* p_material, const NewtonContact* /*p_contact*/ )
{
    // handle level submaterials
    unsigned int materialType = static_cast< unsigned int >( NewtonMaterialGetContactFaceAttribute( p_material ) );
    switch ( materialType )
    {
        case Physics::MAT_DEFAULT:
            NewtonMaterialSetContactElasticity( p_material, 0.3f );
            NewtonMaterialSetContactStaticFrictionCoef( p_material, 0.6f, 0 );
            NewtonMaterialSetContactKineticFrictionCoef( p_material, 0.3f, 0 );
            break;

        case Physics::MAT_WOOD:

            NewtonMaterialSetContactElasticity( p_material, 0.5f );
            NewtonMaterialSetContactStaticFrictionCoef( p_material, 0.6f, 0 );
            NewtonMaterialSetContactKineticFrictionCoef( p_material, 0.4f, 0 );
            break;

        case Physics::MAT_STONE:

            NewtonMaterialSetContactElasticity( p_material, 0.4f );
            NewtonMaterialSetContactStaticFrictionCoef( p_material, 0.55f, 0 );
            NewtonMaterialSetContactKineticFrictionCoef( p_material, 0.35f, 0 );
            break;

        case Physics::MAT_METALL:

            NewtonMaterialSetContactElasticity( p_material, 0.6f );
            NewtonMaterialSetContactStaticFrictionCoef( p_material, 0.5f, 0 );
            NewtonMaterialSetContactKineticFrictionCoef( p_material, 0.4f, 0 );
            break;

        case Physics::MAT_GRASS:

            NewtonMaterialSetContactElasticity( p_material, 0.4f );
            NewtonMaterialSetContactStaticFrictionCoef( p_material, 0.8f, 0 );
            NewtonMaterialSetContactKineticFrictionCoef( p_material, 0.6f, 0 );
            break;

        case Physics::MAT_WALL:

            NewtonMaterialSetContactElasticity( p_material, 0.8f );
            NewtonMaterialSetContactStaticFrictionCoef( p_material, 0.1f, 0 );
            NewtonMaterialSetContactKineticFrictionCoef( p_material, 0.1f, 0 );
            break;

        // simply return 0 for non-colliding pairs
        case Physics::MAT_NOCOL:
            return 0;
            break;

        default:
            ;
    }

    return 1;
}

} // namespace yaf3d
