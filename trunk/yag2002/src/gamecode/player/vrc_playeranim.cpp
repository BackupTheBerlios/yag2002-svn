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

#include <vrc_main.h>
#include "vrc_playeranim.h"
#include "vrc_playerimpl.h"

namespace vrc
{

//! Implement and register the player animation entity factory
YAF3D_IMPL_ENTITYFACTORY( PlayerAnimationEntityFactory );

EnPlayerAnimation::EnPlayerAnimation() :
_anim( eIdle ),
_p_player( NULL ),
_renderingEnabled( true ),
_scale( 1.0f ),
_IdAnimIdle( -1 ),
_IdAnimWalk( -1 ),
_IdAnimRun( -1 ),
_IdAnimJump( -1 ),
_IdAnimLand( -1 ),
_IdAnimTurn( -1 )
{ 
    // register attributes
    getAttributeManager().addAttribute( "animconfig"   , _animCfgFile );
    getAttributeManager().addAttribute( "position"     , _position    );
    getAttributeManager().addAttribute( "rotation"     , _rotation    );
    getAttributeManager().addAttribute( "scale"        , _scale       );
}

EnPlayerAnimation::~EnPlayerAnimation()
{
    if ( _animNode.get() )
        _animNode = NULL; // delete the anim node
}

void EnPlayerAnimation::initialize()
{
    yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_INFO ) << "  initializing player animation instance '" << getInstanceName() << "' ..." << std::endl;

    if ( !_animCfgFile.length() )
    {
        yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "*** missing animation config file parameter" << std::endl;
        return;
    }
    
    // setup and create a new model
    std::string file     = yaf3d::Application::get()->getMediaPath() + _animCfgFile;
    std::string rootDir  = yaf3d::extractPath( file );
    std::string configfilename = yaf3d::extractFileName( file );
    // all textures and cal3d files must be in root dir!
    if ( !setupAnimation( rootDir, configfilename ) )
        return;

    // set initial animation
    _model->startLoop( _IdAnimIdle, 1.0f, 0.0f );
    _anim = eIdle;

    // scale the model
    _coreModel->get()->scale( _scale );

    // create a transform node in order to set position and rotation offsets
    _animNode = new osg::PositionAttitudeTransform;
    _animNode->setPosition( _position );
    osg::Quat quat( 
        _rotation.x() * osg::PI / 180.0f, osg::Vec3f( 1, 0, 0 ),
        _rotation.y() * osg::PI / 180.0f, osg::Vec3f( 0, 1, 0 ),
        _rotation.z() * osg::PI / 180.0f, osg::Vec3f( 0, 0, 1 )
        );
    _animNode->setAttitude( quat );
    _animNode->addChild( _model.get() );

    yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_INFO ) << "  initializing player animation instance completed" << std::endl;

    // register entity in order to get updated per simulation step
    yaf3d::EntityManager::get()->registerUpdate( this, true );
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
        _p_player->getPlayerEntity()->removeTransformationNode( _animNode.get() );
    else
        _p_player->getPlayerEntity()->appendTransformationNode( _animNode.get() );

    _renderingEnabled = render;
}

void EnPlayerAnimation::setPlayer( BasePlayerImplementation* p_player )
{
    _p_player = p_player;
    // add the new mesh into player's transformable scene group
    _p_player->getPlayerEntity()->appendTransformationNode( _animNode.get() );
}

bool EnPlayerAnimation::setupAnimation( const std::string& rootDir, const std::string& configfilename )
{ // this code is basing on cal3d's miniviewer

    std::string destFileName, cfgFileName;
    cfgFileName = rootDir;
    cfgFileName += "/";
    cfgFileName += configfilename;

    std::ifstream file;

    // open the model configuration file
    file.open( cfgFileName.c_str(), std::ios::in | std::ios::binary );
    if( !file )
    {
        yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "***  failed to open model configuration file: " << file << std::endl;
        return false;
    }


    // set cal3d's texture coord loading mode
    CalLoader::setLoadingMode( LOADER_INVERT_V_COORD | LOADER_ROTATE_X_AXIS );

    // create a core model instance
    std::string modelname = "character_" + configfilename;
    _coreModel = new osgCal::CoreModel( modelname );
    CalCoreModel* p_calCoreModel = _coreModel->get();

    // parse all lines from the model configuration file
    int line;
    for( line = 1; ; line++ )
    {
        // read the next model configuration line
        std::string textBuffer;
        getline( file, textBuffer );

        // stop if we reached the end of file
        if( file.eof() ) break;

        // check if an error happend while reading from the file
        if( !file )
        {
            yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "***  error reading config file: " << configfilename << std::endl;
            return false;
        }

        // find the first non-whitespace character
        std::string::size_type pos;
        pos = textBuffer.find_first_not_of( " \t" );

        // check for empty lines
        if( ( pos == std::string::npos) || ( textBuffer[ pos ] == '\n' ) || ( textBuffer[ pos ] == '\r' ) || ( textBuffer[ pos ] == 0 ) ) continue;

        // check for comment lines
        if( textBuffer[ pos ] == '#' ) 
        {
            continue;
        }

        // get the key
        std::string strKey;
        strKey = textBuffer.substr( pos, textBuffer.find_first_of( " =\t\n\r", pos ) - pos );
        pos += strKey.size();

        // get the '=' character
        pos = textBuffer.find_first_not_of( " \t", pos );
        if( (pos == std::string::npos ) || ( textBuffer[ pos ] != '=' ) )
        {
            yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "***  invalid syntax in config file: '" << configfilename;
            yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "' at line" << line << std::endl;
            return false;
        }

        // find the first non-whitespace character after the '=' character
        pos = textBuffer.find_first_not_of( " \t", pos + 1 );

        // get the data
        std::string strData;
        strData = textBuffer.substr( pos, textBuffer.find_first_of( "\n\r", pos ) - pos );
        destFileName = rootDir + "/" + strData;

        // handle the model creation
        if( strKey == "skeleton" )
        {
            if ( !p_calCoreModel->loadCoreSkeleton( destFileName ) )
                yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "***  line " << line << ", problem loading skeleton: " << destFileName << std::endl;                      
        }
        else if( strKey == "mesh" )
        {
            if ( p_calCoreModel->loadCoreMesh( destFileName ) < 0 )
                yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "***  line " << line << ", problem loading mesh: " << destFileName << std::endl;                      
        }
        else if( strKey == "material" )
        {
            int materialId = p_calCoreModel->loadCoreMaterial( destFileName );
            if( materialId < 0 ) 
            {                
                yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "***  line " << line << ", problem loading material: " << destFileName;                      
                yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "   reason: " << CalError::getLastErrorDescription() << std::endl;
            } 
            else 
            {
                p_calCoreModel->createCoreMaterialThread( materialId );
                p_calCoreModel->setCoreMaterialId( materialId, 0, materialId );

                CalCoreMaterial* p_material = p_calCoreModel->getCoreMaterial( materialId );
                // the texture file path is relative to the CRF data directory
                for( std::vector< CalCoreMaterial::Map >::iterator beg = p_material->getVectorMap().begin(), 
                    end = p_material->getVectorMap().end(); beg != end; beg++ )
                {
                    beg->strFilename = beg->strFilename;
                }

            }

        }

        // read animation ids
        else if( strKey == "animation_idle" )
        {
            _IdAnimIdle = p_calCoreModel->loadCoreAnimation( destFileName );
            if ( _IdAnimIdle < 0 )
                yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "***  line " << line << ", problem loading animation: " << destFileName << std::endl; 
        }
        else if( strKey == "animation_walk" )
        {
            _IdAnimWalk = p_calCoreModel->loadCoreAnimation( destFileName );
            if ( _IdAnimWalk < 0 )
                yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "***  line " << line << ", problem loading animation: " << destFileName << std::endl; 
        }
        else if( strKey == "animation_run" )
        {
            _IdAnimRun = p_calCoreModel->loadCoreAnimation( destFileName );
            if ( _IdAnimRun < 0 )
                yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "***  line " << line << ", problem loading animation: " << destFileName << std::endl; 
        }
        else if( strKey == "animation_turn" )
        {
            _IdAnimTurn = p_calCoreModel->loadCoreAnimation( destFileName );
            if ( _IdAnimTurn < 0 )
                yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "***  line"  << line << ", problem loading animation: " << destFileName << std::endl; 
        }
        else if( strKey == "animation_jump" )
        {
            _IdAnimJump = p_calCoreModel->loadCoreAnimation( destFileName );
            if ( _IdAnimJump < 0 )
                yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "***  line"  << line << ", problem loading animation: " << destFileName << std::endl; 
        }        else if( strKey == "animation_landing" )
        {
            _IdAnimLand = p_calCoreModel->loadCoreAnimation( destFileName );
            if ( _IdAnimLand < 0 )
                yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "***  line " << line << ", problem loading animation: " << destFileName << std::endl; 
        }
        else if( strKey == "animation" )
        {
            yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "***  token 'animation' is not valid, line: " << line << std::endl;
            yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "***  use one of following animation tokens instead: " << std::endl;;
            yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "***    animation_idle" << std::endl;;
            yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "***    animation_walk" << std::endl;;
            yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "***    animation_run" << std::endl;;
            yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "***    animation_jump" << std::endl;;
            yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "***    animation_land" << std::endl;;
        }
        else
        {
            // everything else triggers an error message
            yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "***  invalid syntax in config file: '" << configfilename;
            yaf3d::log << yaf3d::Log::LogLevel( yaf3d::Log::L_ERROR ) << "' at line: " << line << std::endl;
            return false;
        }
    }
    file.close();

    // now load all textures, before creating the model below
    _coreModel->loadAllTextures( rootDir );

    // create the model
    _model = new osgCal::Model();
    _model->create( _coreModel.get() );
    
    return true;
}

unsigned char EnPlayerAnimation::getAnimationFlags()
{
    return static_cast< unsigned char >( _anim );
}

void EnPlayerAnimation::setAnimation( unsigned char flags )
{
    switch( flags )
    {
        case eIdle:
            
            animIdle();
            break;

        case eWalk:
            
            animWalk();
            break;

        case eJump:
            
            animJump();
            break;

        case eTurn:
            
            animTurn();
            break;
    }    
}

void EnPlayerAnimation::animIdle()
{
    if ( _anim == eIdle )
        return;

    _model->startLoop( _IdAnimIdle, 1.0f, 0.5f );
    _model->stopLoop( _IdAnimWalk, 0.4f );
    _model->stopLoop( _IdAnimTurn, 0.4f );
    _anim = eIdle;
}

void EnPlayerAnimation::animWalk()
{
    if ( _anim == eWalk )
        return;

    _model->startLoop( _IdAnimWalk, 1.0f, 0.5f );
    _model->stopLoop( _IdAnimIdle, 0.5f );
    _model->stopLoop( _IdAnimTurn, 0.0f );
    _anim = eWalk;
}

void EnPlayerAnimation::animJump()
{
    if ( _anim == eJump )
        return;

    _model->startAction( _IdAnimJump, 0.0f, 0.9f );
    _anim = eJump;
}

void EnPlayerAnimation::animTurn()
{
    if ( _anim == eTurn )
        return;

    _model->startLoop( _IdAnimTurn, 1.0f, 0.0f );
    _anim = eTurn;
}

} // namespace vrc
