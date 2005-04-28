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
 # player animation
 #
 # this class implements the player animation control
 #
 #   date of creation:  03/13/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <ctd_main.h>
#include "ctd_playeranim.h"
#include "ctd_player.h"

using namespace osg;
using namespace osgCal;
using namespace std;


namespace CTD
{

//! Implement and register the player animation entity factory
CTD_IMPL_ENTITYFACTORY_AUTO( PlayerAnimationEntityFactory );

// a small model cache for sharing same animation models
std::map< std::string, osgCal::Model* >    s_modelCache;

EnPlayerAnimation::EnPlayerAnimation() :
_p_player( NULL ),
_scale( 1.0f ),
_IdAnimIdle( -1 ),
_IdAnimWalk( -1 ),
_IdAnimRun( -1 ),
_IdAnimJump( -1 ),
_IdAnimLand( -1 ),
_renderingEnabled( true )
{ 
    // register entity in order to get updated per simulation step
    EntityManager::get()->registerUpdate( this );

    // the deletion must not be controled by entity manager, but by player
    setAutoDelete( false );
    // register attributes
    getAttributeManager().addAttribute( "animconfig"   , _animCfgFile );
    getAttributeManager().addAttribute( "position"     , _position    );
    getAttributeManager().addAttribute( "rotation"     , _rotation    );
    getAttributeManager().addAttribute( "scale"        , _scale       );
}

EnPlayerAnimation::~EnPlayerAnimation()
{
}

void EnPlayerAnimation::initialize()
{
    log << Log::LogLevel( Log::L_INFO ) << "  initializing player animation instance '" << getInstanceName() << "' ..." << endl;

    if ( !_animCfgFile.length() )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "*** missing animation config file parameter" << endl;
        return;
    }

    // look up the model cache first
    std::map< std::string, osgCal::Model* >::iterator p_model;
    p_model = s_modelCache.find( _animCfgFile );
    if ( p_model != s_modelCache.end() )
    {
        log << Log::LogLevel( Log::L_DEBUG ) << "  using cached model '" << _animCfgFile << "' ..." << endl;

        _model = new osgCal::Model( *( p_model->second ) );
        // create a transform node in order to set position and rotation offsets
        _animNode = new PositionAttitudeTransform;
        _animNode->setPosition( _position );
        Quat quat( 
            _rotation.x() * PI / 180.0f, Vec3f( 1, 0, 0 ),
            _rotation.y() * PI / 180.0f, Vec3f( 0, 1, 0 ),
            _rotation.z() * PI / 180.0f, Vec3f( 0, 0, 1 )
            );
        _animNode->setAttitude( quat );
        _animNode->addChild( _model.get() );

        log << Log::LogLevel( Log::L_INFO ) << "  initializing player animation instance completed" << endl;

        return;
    }

    // if no cache hit then setup and create a new model
    string file     = Application::get()->getMediaPath() + _animCfgFile;
    string rootDir  = extractPath( file );
    string configfilename = extractFileName( file );
    // all textures and cal3d files must be in root dir!
    if ( !setupAnimation( rootDir, configfilename ) )
    {
        return;
    }

    if( !_model->setCoreModel( _coreModel.get() ) )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "***  cannot set core model " << endl;
        return;
    }

    // attach all meshes into model
    for( int coreMeshId = 0; coreMeshId < _model->getCalCoreModel()->getCoreMeshCount(); coreMeshId++ )
        _model->getCalModel()->attachMesh( coreMeshId );

    // set the material set of the whole model
    _model->getCalModel()->setMaterialSet( 0 );

    // try to enable hw blending
    _model->setUseVertexProgram( true );
    //------------------------------------------
    if ( !_model->getUseVertexProgram() )
        log << Log::LogLevel( Log::L_WARNING ) << " cannot use hardware (gpu) for blending, using software blending instead." << endl;

    // creating a concrete model using the core template
    if( !_model->create() ) 
    {
        log << Log::LogLevel( Log::L_ERROR ) << "***  cannot create model: " << CalError::getLastErrorDescription() << endl;
        return;
    }

    // set initial animation
    _model->getCalModel()->getMixer()->blendCycle( _IdAnimIdle, 1.0f, 0.0f );

    // scale the model
    _p_calCoreModel->scale( _scale );

    // create a transform node in order to set position and rotation offsets
    _animNode = new PositionAttitudeTransform;
    _animNode->setPosition( _position );
    Quat quat( 
        _rotation.x() * PI / 180.0f, Vec3f( 1, 0, 0 ),
        _rotation.y() * PI / 180.0f, Vec3f( 0, 1, 0 ),
        _rotation.z() * PI / 180.0f, Vec3f( 0, 0, 1 )
        );
    _animNode->setAttitude( quat );
    _animNode->addChild( _model.get() );

    // register model
    s_modelCache.insert( std::make_pair( _animCfgFile, _model.get() ) );

    log << Log::LogLevel( Log::L_INFO ) << "  initializing player animation instance completed" << endl;
}

void EnPlayerAnimation::updateEntity( float deltaTime )
{
    // calculate lod factor depending on camera distance
    //_model->getCalModel()->setLodLevel( lod );
}

void EnPlayerAnimation::enableRendering( bool render )
{
    if ( ( _renderingEnabled && render ) || ( !_renderingEnabled && !render ) )
        return;

    if ( !render )
        _p_player->removeFromTransformationNode( _animNode.get() );
    else
        _p_player->addToTransformationNode( _animNode.get() );

    _renderingEnabled = render;
}

void EnPlayerAnimation::setPlayer( EnPlayer* p_player )
{
    _p_player = p_player;
    // add the new mesh into player's transformable scene group
    _p_player->addToTransformationNode( _animNode.get() );
}

void EnPlayerAnimation::destroy()
{
    // note: this method is called by player when it is destroying, e.g. when player disconnects from network
    _p_player->removeFromTransformationNode( _animNode.get() );
    EntityManager::get()->deleteEntity( this );
}

bool EnPlayerAnimation::setupAnimation( const string& rootDir, const string& configfilename )
{ // this code is basing on cal3d's miniviewer

    string destFileName, cfgFileName;
    cfgFileName = rootDir;
    cfgFileName += "/";
    cfgFileName += configfilename;

    ifstream file;

    // open the model configuration file
    file.open( cfgFileName.c_str(), std::ios::in | std::ios::binary );
    if( !file )
    {
        log << Log::LogLevel( Log::L_ERROR ) << "***  failed to open model configuration file: " << file << endl;
        return false;
    }

    // create a core model instance
    string modelname = "character_" + configfilename;
    _model = new Model();
    _coreModel = new CoreModel();
    _p_calCoreModel = _coreModel->getCalCoreModel();
    // parse all lines from the model configuration file
    int line;
    for( line = 1; ; line++ )
    {
        // read the next model configuration line
        string textBuffer;
        getline( file, textBuffer );

        // stop if we reached the end of file
        if( file.eof() ) break;

        // check if an error happend while reading from the file
        if( !file )
        {
            log << Log::LogLevel( Log::L_ERROR ) << "***  error reading config file: " << configfilename << endl;
            return false;
        }

        // find the first non-whitespace character
        string::size_type pos;
        pos = textBuffer.find_first_not_of( " \t" );

        // check for empty lines
        if( ( pos == string::npos) || ( textBuffer[ pos ] == '\n' ) || ( textBuffer[ pos ] == '\r' ) || ( textBuffer[ pos ] == 0 ) ) continue;

        // check for comment lines
        if( textBuffer[ pos ] == '#' ) 
        {
            continue;
        }

        // get the key
        string strKey;
        strKey = textBuffer.substr( pos, textBuffer.find_first_of( " =\t\n\r", pos ) - pos );
        pos += strKey.size();

        // get the '=' character
        pos = textBuffer.find_first_not_of( " \t", pos );
        if( (pos == string::npos ) || ( textBuffer[ pos ] != '=' ) )
        {
            log << Log::LogLevel( Log::L_ERROR ) << "***  invalid syntax in config file: '" << configfilename;
            log << Log::LogLevel( Log::L_ERROR ) << "' at line" << line << endl;
            return false;
        }

        // find the first non-whitespace character after the '=' character
        pos = textBuffer.find_first_not_of( " \t", pos + 1 );

        // get the data
        string strData;
        strData = textBuffer.substr( pos, textBuffer.find_first_of( "\n\r", pos ) - pos );
        destFileName = rootDir + "/" + strData;

        // handle the model creation
        if( strKey == "skeleton" )
        {
            if ( !_p_calCoreModel->loadCoreSkeleton( destFileName ) )
                log << Log::LogLevel( Log::L_ERROR ) << "***  line " << line << ", problem loading skeleton: " << destFileName << endl;                      
        }
        else if( strKey == "mesh" )
        {
            if ( _p_calCoreModel->loadCoreMesh( destFileName ) < 0 )
                log << Log::LogLevel( Log::L_ERROR ) << "***  line " << line << ", problem loading mesh: " << destFileName << endl;                      
        }
        else if( strKey == "material" )
        {
            int materialId = _p_calCoreModel->loadCoreMaterial( destFileName );
            if( materialId < 0 ) 
            {                
                log << Log::LogLevel( Log::L_ERROR ) << "***  line " << line << ", problem loading material: " << destFileName;                      
                log << Log::LogLevel( Log::L_ERROR ) << "   reason: " << CalError::getLastErrorDescription() << endl;
            } 
            else 
            {
                _p_calCoreModel->createCoreMaterialThread( materialId ); 
                _p_calCoreModel->setCoreMaterialId(materialId, 0, materialId);

                CalCoreMaterial* p_material = _p_calCoreModel->getCoreMaterial( materialId );
                // the texture file path is relative to the CRF data directory
                for( vector<CalCoreMaterial::Map>::iterator beg = p_material->getVectorMap().begin(), 
                    end = p_material->getVectorMap().end(); beg != end; beg++ )
                {
                    beg->strFilename = rootDir + "/" + beg->strFilename;
                }

            }

        }

        // read animation ids
        else if( strKey == "animation_idle" )
        {
            _IdAnimIdle = _p_calCoreModel->loadCoreAnimation( destFileName );
            if ( _IdAnimIdle < 0 )
                log << Log::LogLevel( Log::L_ERROR ) << "***  line " << line << ", problem loading animation: " << destFileName << endl; 
        }
        else if( strKey == "animation_walk" )
        {
            _IdAnimWalk = _p_calCoreModel->loadCoreAnimation( destFileName );
            if ( _IdAnimWalk < 0 )
                log << Log::LogLevel( Log::L_ERROR ) << "***  line " << line << ", problem loading animation: " << destFileName << endl; 
        }
        else if( strKey == "animation_run" )
        {
            _IdAnimRun = _p_calCoreModel->loadCoreAnimation( destFileName );
            if ( _IdAnimRun < 0 )
                log << Log::LogLevel( Log::L_ERROR ) << "***  line " << line << ", problem loading animation: " << destFileName << endl; 
        }
        else if( strKey == "animation_turn" )
        {
            _IdAnimTrun = _p_calCoreModel->loadCoreAnimation( destFileName );
            if ( _IdAnimTrun < 0 )
                log << Log::LogLevel( Log::L_ERROR ) << "***  line"  << line << ", problem loading animation: " << destFileName << endl; 
        }
        else if( strKey == "animation_jump" )
        {
            _IdAnimJump = _p_calCoreModel->loadCoreAnimation( destFileName );
            if ( _IdAnimJump < 0 )
                log << Log::LogLevel( Log::L_ERROR ) << "***  line"  << line << ", problem loading animation: " << destFileName << endl; 
        }        else if( strKey == "animation_landing" )
        {
            _IdAnimLand = _p_calCoreModel->loadCoreAnimation( destFileName );
            if ( _IdAnimLand < 0 )
                log << Log::LogLevel( Log::L_ERROR ) << "***  line " << line << ", problem loading animation: " << destFileName << endl; 
        }
        else if( strKey == "animation" )
        {
            log << Log::LogLevel( Log::L_ERROR ) << "***  token 'animation' is not valid, line: " << line << endl;
            log << Log::LogLevel( Log::L_ERROR ) << "***  use one of following animation tokens instead: " << endl;;
            log << Log::LogLevel( Log::L_ERROR ) << "***    animation_idle" << endl;;
            log << Log::LogLevel( Log::L_ERROR ) << "***    animation_walk" << endl;;
            log << Log::LogLevel( Log::L_ERROR ) << "***    animation_run" << endl;;
            log << Log::LogLevel( Log::L_ERROR ) << "***    animation_jump" << endl;;
            log << Log::LogLevel( Log::L_ERROR ) << "***    animation_land" << endl;;
        }
        else
        {
            // everything else triggers an error message
            log << Log::LogLevel( Log::L_ERROR ) << "***  invalid syntax in config file: '" << configfilename;
            log << Log::LogLevel( Log::L_ERROR ) << "' at line: " << line << endl;
            return false;
        }
    }
    file.close();
    return true;
}

//! TODO: animation control must be implemented
void EnPlayerAnimation::animIdle()
{
    if ( _anim == eIdle )
        return;

    _model->getCalModel()->getMixer()->blendCycle( _IdAnimIdle, 1.0f, 0.5f );
    _model->getCalModel()->getMixer()->clearCycle( _IdAnimWalk, 0.5f );
    _model->getCalModel()->getMixer()->clearCycle( _IdAnimTrun, 0.5f );
    _anim = eIdle;
}

void EnPlayerAnimation::animWalk()
{
    if ( _anim == eWalk )
        return;

    _model->getCalModel()->getMixer()->blendCycle( _IdAnimWalk, 1.0f, 0.5f );
    _model->getCalModel()->getMixer()->clearCycle( _IdAnimIdle, 0.5f );
    _model->getCalModel()->getMixer()->clearCycle( _IdAnimTrun, 0.0f );
    _anim = eWalk;
}

void EnPlayerAnimation::animJump()
{
    if ( _anim == eJump )
        return;

    _model->getCalModel()->getMixer()->executeAction( _IdAnimJump, 0.0f, 0.0f, 0.4f );
    _anim = eJump;
}

void EnPlayerAnimation::animTurn()
{
    if ( _anim == eTurn )
        return;

    _model->getCalModel()->getMixer()->blendCycle( _IdAnimTrun, 1.0f, 0.0f );
    _anim = eTurn;
}
} // namespace CTD