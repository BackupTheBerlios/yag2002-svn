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
#include <vrc_gameutils.h>
#include "vrc_playeranim.h"
#include "vrc_playerimpl.h"

namespace vrc
{

//! "Implement and register the player animation entity factory
YAF3D_IMPL_ENTITYFACTORY( PlayerAnimationEntityFactory )

static const char glsl_vp[] =
    "/*                                                                             \n"
    "* Vertex shader for rendering the player                                       \n"
    "* http://yag2002.sf.net                                                        \n"
    "* 11/28/2005                                                                   \n"
    "*/                                                                             \n"
    "varying vec4 diffuse,ambient;                                                  \n"
    "varying vec3 normal,lightDir,halfVector;                                       \n"
    "varying vec2 texCoords;                                                        \n"
    "                                                                               \n"
    "void main()                                                                    \n"
    "{                                                                              \n"
    "   normal      = normalize(gl_NormalMatrix * gl_Normal);                       \n"
    "   lightDir    = normalize(vec3(gl_LightSource[0].position));                  \n"
    "   halfVector  = normalize(gl_LightSource[0].halfVector.xyz);                  \n"
    "   diffuse     = gl_FrontMaterial.diffuse * gl_LightSource[0].diffuse;         \n"
    "   ambient     = gl_FrontMaterial.ambient * gl_LightSource[0].ambient;         \n"
    "   ambient     += gl_LightModel.ambient * gl_FrontMaterial.ambient;            \n"
    "   gl_Position = ftransform();                                                 \n"
    "   texCoords   = gl_MultiTexCoord0.st;                                         \n"
    "}                                                                              \n"
;
static const char glsl_fp[] =
    "/*                                                                             \n"
    "* Fragment shader for rendering the player                                     \n"
    "* http://yag2002.sf.net                                                        \n"
    "* 11/28/2005                                                                   \n"
    "*/                                                                             \n"
    "varying vec4 diffuse,ambient;                                                  \n"
    "varying vec3 normal,lightDir,halfVector;                                       \n"
    "varying vec2 texCoords;                                                        \n"
    "uniform sampler2D tex;                                                         \n"
    "                                                                               \n"
    "void main()                                                                    \n"
    "{                                                                              \n"
    "   vec3 n,halfV;                                                               \n"
    "   float NdotL,NdotHV;                                                         \n"
    "                                                                               \n"
    "   vec4 color = ambient;                                                       \n"
    "   n = normalize(normal);                                                      \n"
    "   NdotL = max(dot(n,lightDir),0.0);                                           \n"
    "    if (NdotL > 0.0) {                                                         \n"
    "       color += diffuse * NdotL;                                               \n"
    "       halfV = normalize(halfVector);                                          \n"
    "       NdotHV = max(dot(n,halfV),0.0);                                         \n"
    "       color += gl_FrontMaterial.specular *                                    \n"
    "               gl_LightSource[0].specular *                                    \n"
    "               pow(NdotHV, gl_FrontMaterial.shininess);                        \n"
    "   }                                                                           \n"
    "                                                                               \n"
    "   vec4 texcolor = texture2D(tex,texCoords);                                   \n"
    "   gl_FragColor = color * texcolor;                                            \n"
    "}                                                                              \n"
;

static osg::ref_ptr< osg::Program > s_program;

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
    log_info << "  initializing player animation instance '" << getInstanceName() << "' ..." << std::endl;

    if ( !_animCfgFile.length() )
    {
        log_error << "*** missing animation config file parameter" << std::endl;
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
    _animNode->setName( "_playerAnimationNode_" );
    _animNode->setPosition( _position );
    osg::Quat quat(
        _rotation.x() * osg::PI / 180.0f, osg::Vec3f( 1, 0, 0 ),
        _rotation.y() * osg::PI / 180.0f, osg::Vec3f( 0, 1, 0 ),
        _rotation.z() * osg::PI / 180.0f, osg::Vec3f( 0, 0, 1 )
        );
    _animNode->setAttitude( quat );
    _animNode->addChild( _model.get() );

   // register entity in order to get notifications (e.g. from menu entity)
    yaf3d::EntityManager::get()->registerNotification( this, true );

    log << "  initializing player animation instance completed" << std::endl;
}

void EnPlayerAnimation::handleNotification( const yaf3d::EntityNotification& notification )
{
    // handle some notifications
    switch( notification.getId() )
    {
        case YAF3D_NOTIFY_MENU_LEAVE:
        {
            // if shadow has been enabled then remove the player shader from state set
            bool shadow;
            yaf3d::Configuration::get()->getSettingValue( VRC_GS_SHADOW_ENABLE, shadow );
            osg::StateSet* p_stateSet = _animNode->getStateSet();
            if ( shadow )
            {
                if ( p_stateSet )
                {
                    p_stateSet->clear();
                    _animNode->setStateSet( NULL );
                }
            }
            else
            {
                // setup the shader if glsl is supported
                const osg::GL2Extensions* p_extensions = osg::GL2Extensions::Get( 0, true );
                if ( p_extensions->isGlslSupported() )
                {
                    if ( !s_program.valid() )
                    {
                        s_program = new osg::Program;
                        s_program->setName( "_playerAnim_" );
                        s_program->addShader( new osg::Shader( osg::Shader::VERTEX, glsl_vp ) );
                        s_program->addShader( new osg::Shader( osg::Shader::FRAGMENT, glsl_fp ) );
                    }
                    // add shader if it does not already exist
                    if ( !p_stateSet )
                    {
                        p_stateSet = _animNode->getOrCreateStateSet();
                        p_stateSet->setAttributeAndModes( s_program.get(), osg::StateAttribute::ON );
                        _animNode->setStateSet( p_stateSet );
                    }
                }
            }
        }
        break;

        default:
            ;
    }
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
        log_error << "***  failed to open model configuration file: " << file << std::endl;
        return false;
    }


    // set cal3d's texture coord loading mode
    CalLoader::setLoadingMode( LOADER_INVERT_V_COORD );

    // create a core model instance
    std::string modelname = "character_" + configfilename;
    _coreModel = new osgCal::CoreModel( modelname );
    CalCoreModel* p_calCoreModel = _coreModel->get();

    // parse all lines from the model configuration file
    int line;
    for( line = 1; ; ++line )
    {
        // read the next model configuration line
        std::string textBuffer;
        getline( file, textBuffer );

        // stop if we reached the end of file
        if( file.eof() ) break;

        // check if an error happend while reading from the file
        if( !file )
        {
            log_error << "***  error reading config file: " << configfilename << std::endl;
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
            log_error << "***  invalid syntax in config file: '" << configfilename;
            log_error << "' at line" << line << std::endl;
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
                log_error << "***  line " << line << ", problem loading skeleton: " << destFileName << std::endl;
        }
        else if( strKey == "mesh" )
        {
            if ( p_calCoreModel->loadCoreMesh( destFileName ) < 0 )
                log_error << "***  line " << line << ", problem loading mesh: " << destFileName << std::endl;
        }
        else if( strKey == "material" )
        {
            int materialId = p_calCoreModel->loadCoreMaterial( destFileName );
            if( materialId < 0 )
            {
                log_error << "***  line " << line << ", problem loading material: " << destFileName;
                log_error << "   reason: " << CalError::getLastErrorDescription() << std::endl;
            }
            else
            {
                p_calCoreModel->createCoreMaterialThread( materialId );
                p_calCoreModel->setCoreMaterialId( materialId, 0, materialId );

                CalCoreMaterial* p_material = p_calCoreModel->getCoreMaterial( materialId );
                // the texture file path is relative to the CRF data directory
                std::vector< CalCoreMaterial::Map >::iterator p_beg = p_material->getVectorMap().begin(), p_end = p_material->getVectorMap().end();
                for( ; p_beg != p_end; ++p_beg )
                    p_material->setFilename( p_beg->strFilename );

            }

        }

        // read animation ids
        else if( strKey == "animation_idle" )
        {
            _IdAnimIdle = p_calCoreModel->loadCoreAnimation( destFileName );
            if ( _IdAnimIdle < 0 )
                log_error << "***  line " << line << ", problem loading animation: " << destFileName << std::endl;
        }
        else if( strKey == "animation_walk" )
        {
            _IdAnimWalk = p_calCoreModel->loadCoreAnimation( destFileName );
            if ( _IdAnimWalk < 0 )
                log_error << "***  line " << line << ", problem loading animation: " << destFileName << std::endl;
        }
        else if( strKey == "animation_run" )
        {
            _IdAnimRun = p_calCoreModel->loadCoreAnimation( destFileName );
            if ( _IdAnimRun < 0 )
                log_error << "***  line " << line << ", problem loading animation: " << destFileName << std::endl;
        }
        else if( strKey == "animation_turn" )
        {
            _IdAnimTurn = p_calCoreModel->loadCoreAnimation( destFileName );
            if ( _IdAnimTurn < 0 )
                log_error << "***  line"  << line << ", problem loading animation: " << destFileName << std::endl;
        }
        else if( strKey == "animation_jump" )
        {
            _IdAnimJump = p_calCoreModel->loadCoreAnimation( destFileName );
            if ( _IdAnimJump < 0 )
                log_error << "***  line"  << line << ", problem loading animation: " << destFileName << std::endl;
        }        else if( strKey == "animation_landing" )
        {
            _IdAnimLand = p_calCoreModel->loadCoreAnimation( destFileName );
            if ( _IdAnimLand < 0 )
                log_error << "***  line " << line << ", problem loading animation: " << destFileName << std::endl;
        }
        else if( strKey == "animation" )
        {
            log_error << "***  token 'animation' is not valid, line: " << line << std::endl;
            log_error << "***  use one of following animation tokens instead: " << std::endl;;
            log_error << "***    animation_idle" << std::endl;;
            log_error << "***    animation_walk" << std::endl;;
            log_error << "***    animation_run" << std::endl;;
            log_error << "***    animation_jump" << std::endl;;
            log_error << "***    animation_land" << std::endl;;
        }
        else
        {
            // everything else triggers an error message
            log_error << "***  invalid syntax in config file: '" << configfilename;
            log_error << "' at line: " << line << std::endl;
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
