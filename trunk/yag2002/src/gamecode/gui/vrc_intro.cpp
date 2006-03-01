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
 # intro played at startup or triggered by main menu
 #
 #   date of creation:  04/27/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_intro.h"
#include "../sound/vrc_ambientsound.h"

namespace vrc
{

// prefix for intro layout resources
#define INTRO_PREFIX            "intro_"
// fade-in time
#define INTRO_FADEIN_TIME       3.0f

IntroControl::IntroControl() :
_introState( None ),
_p_introSound( NULL ),
_p_wndIntro( NULL ),
_p_introImage( NULL ),
_p_imageSet( NULL ),
_introTimer( 0 )
{
}

IntroControl::~IntroControl()
{
}

bool IntroControl::initialize( const std::string& intoImage )
{    
    try
    {
        // setup intro stuff
        _p_wndIntro   = static_cast< CEGUI::Window* >( CEGUI::WindowManager::getSingleton().createWindow( "DefaultWindow", INTRO_PREFIX "into_wnd" ) );
        _p_introImage = static_cast< CEGUI::StaticImage* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticImage", INTRO_PREFIX "img_intro" ) );
        _p_wndIntro->addChildWindow( _p_introImage );
        _p_wndIntro->setPosition( CEGUI::Point( 0.5f, 0.5f ) ); // centeric zoom

        // create a new imageset for intro
        std::string imagename = "intro";
        CEGUI::Texture*  p_texture   = yaf3d::GuiManager::get()->getGuiRenderer()->createTexture( intoImage, "MenuResourcesIntro" );
        _p_imageSet = CEGUI::ImagesetManager::getSingleton().createImageset( imagename, p_texture );
        if ( !_p_imageSet->isImageDefined( intoImage ) )
        {
            _p_imageSet->defineImage( imagename, CEGUI::Point( 0.0f, 0.0f ), CEGUI::Size( p_texture->getWidth(), p_texture->getHeight() ), CEGUI::Point( 0.0f,0.0f ) );
        }
        CEGUI::Image* p_image = &const_cast< CEGUI::Image& >( _p_imageSet->getImage( imagename ) );
        _p_introImage->setImage( p_image );
        _p_introImage->setSize( CEGUI::Size( 1.0f, 1.0f ) );
        _p_introImage->setPosition( CEGUI::Point( 0, 0 ) );
        _p_introImage->setBackgroundEnabled( false );
        _p_introImage->setFrameEnabled( false );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "*** Intro: cannot setup layout!" << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
        return false;
    }

    return true;
}

void IntroControl::setIntroSound( EnAmbientSound* p_sound )
{
    _p_introSound = p_sound;
}

void IntroControl::update( float deltaTime )
{
    if ( _introState == Stopped )
        return;

    _introTimer += deltaTime;
    if ( _introTimer > INTRO_FADEIN_TIME )
    {
        _intoImageSize = CEGUI::Size( 1.0f, 1.0f );
        _p_wndIntro->setSize( _intoImageSize );
        _p_wndIntro->setAlpha( 1.0f );
        return;
    }

    // fade-in the intro image
    float imgSize = _introTimer / float( INTRO_FADEIN_TIME );
    CEGUI::Point pos = CEGUI::Point( 0.5f - imgSize * 0.5f, 0.5f - imgSize * 0.5f );
    _p_wndIntro->setPosition( pos );
    _intoImageSize = CEGUI::Size( imgSize, imgSize );
    _p_wndIntro->setSize( _intoImageSize );
    _p_wndIntro->setAlpha( std::max( 0.2f, imgSize ) );
}

void IntroControl::start()
{
    if ( _introState == Running )
        return;

    if ( _p_introSound )
        _p_introSound->startPlaying();

    _p_wndIntro->setAlpha( 0.2f );

    // append the intro window to root
    yaf3d::GuiManager::get()->getRootWindow()->addChildWindow( _p_wndIntro );
    _introState = Running;
    _introTimer = 0;

    gameutils::GuiUtils::get()->showMousePointer( false ); // let the mouse disappear 
}

void IntroControl::stop()
{
    if ( _introState == Stopped )
        return;

    if ( _p_introSound )
        _p_introSound->stopPlaying();

    // cleanup resources
    try
    {
        if ( _p_wndIntro )
            CEGUI::WindowManager::getSingleton().destroyWindow( _p_wndIntro );

        if ( _p_imageSet )
            CEGUI::ImagesetManager::getSingleton().destroyImageset( _p_imageSet );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "IntroControl: problem cleaning up gui resources." << std::endl;
        log << "      reason: " << e.getMessage().c_str() << std::endl;
    }

    _introState = Stopped;
}

} // namespace vrc
