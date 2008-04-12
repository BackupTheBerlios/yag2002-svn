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
 # voice chat gui
 #
 #   date of creation:  04/11/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_voicegui.h"
#include "vrc_netvoice.h"


#define VOICELAYOUT_PREFIX       "voicegui_"


namespace vrc
{

VoiceGui::VoiceGui( EnNetworkVoice* p_netvoice ) :
 _p_netVoice( p_netvoice ),
 _p_btnVoice( NULL ),
 _p_wndVoiceMenu( NULL ),
 _p_imageNormal( NULL ),
 _p_imageHoover( NULL ),
 _p_imageDisableNormal( NULL ),
 _p_imageDisableHoover( NULL ),
 _p_imageMuteNormal( NULL ),
 _p_imageMuteHoover( NULL ),
 _p_imageMuteDisableNormal( NULL ),
 _p_imageMuteDisableHoover( NULL ),
 _p_checkboxOnOff( NULL ),
 _p_checkboxMute( NULL ),
 _p_volumeInput( NULL ),
 _p_volumeOutput( NULL )
{
}

VoiceGui::~VoiceGui()
{
    try
    {
        if ( _p_btnVoice )
            CEGUI::WindowManager::getSingleton().destroyWindow( _p_btnVoice );

        if ( _p_wndVoiceMenu )
            CEGUI::WindowManager::getSingleton().destroyWindow( _p_wndVoiceMenu );

        // release button images
        if ( _p_imageNormal )
            delete _p_imageNormal;
        if ( _p_imageHoover )
            delete _p_imageHoover;
        if ( _p_imageDisableNormal )
            delete _p_imageDisableNormal;
        if ( _p_imageDisableHoover )
            delete _p_imageDisableHoover;
        if ( _p_imageMuteNormal )
            delete _p_imageMuteNormal;
        if ( _p_imageMuteHoover )
            delete _p_imageMuteHoover;
        if ( _p_imageMuteDisableNormal )
            delete _p_imageMuteDisableNormal;
        if ( _p_imageMuteDisableHoover )
            delete _p_imageMuteDisableHoover;
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "VoiceGui: problem cleaning up gui resources" << std::endl;
        log_error << "   reason: " << e.getMessage() << std::endl;
    }
}

void VoiceGui::initialize()
{
    CEGUI::Window* p_wnd = gameutils::GuiUtils::get()->getMainGuiWindow();

    _p_btnVoice = createVoiceButton();
    if ( _p_btnVoice )
        p_wnd->addChildWindow( _p_btnVoice );

    _p_wndVoiceMenu = createVoiceMenu();
    if ( _p_wndVoiceMenu )
    {
        p_wnd->addChildWindow( _p_wndVoiceMenu );
        _p_wndVoiceMenu->hide();
    }
}

void VoiceGui::updateVoiceConfiguration()
{
    // get current settings
    bool  voicenable = true;
    bool  inputmute  = false;
    float outputgain = 1.0f;
    float inputgain  = 1.0f;

    yaf3d::Configuration::get()->getSettingValue( VRC_GS_VOICE_INPUT_MUTE, inputmute );
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_VOICECHAT_ENABLE, voicenable );
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_VOICE_OUTPUT_GAIN, outputgain );
    yaf3d::Configuration::get()->getSettingValue( VRC_GS_VOICE_INPUT_GAIN, inputgain );

    _p_volumeInput->setScrollPosition( inputgain );
    _p_volumeOutput->setScrollPosition( outputgain );
    _p_checkboxMute->setSelected( inputmute );

    if ( voicenable )
    {
        _p_volumeInput->enable();
        _p_volumeOutput->enable();
        _p_checkboxOnOff->setSelected( false );
    }
    else
    {
        _p_volumeInput->disable();
        _p_volumeOutput->disable();
        _p_checkboxOnOff->setSelected( true );
    }

    updateButtonImages( voicenable, inputmute );
}

void VoiceGui::updateButtonImages( bool voicenable, bool inputmute )
{
    if ( voicenable )
    {
        if ( inputmute )
        {
            _p_btnVoice->setPushedImage( _p_imageMuteNormal );
            _p_btnVoice->setNormalImage( _p_imageMuteNormal );
            _p_btnVoice->setHoverImage( _p_imageMuteHoover );
        }
        else
        {
            _p_btnVoice->setPushedImage( _p_imageNormal );
            _p_btnVoice->setNormalImage( _p_imageNormal );
            _p_btnVoice->setHoverImage( _p_imageHoover );
        }
    }
    else
    {
        if ( inputmute )
        {
            _p_btnVoice->setPushedImage( _p_imageMuteDisableNormal );
            _p_btnVoice->setNormalImage( _p_imageMuteDisableNormal );
            _p_btnVoice->setHoverImage( _p_imageMuteDisableHoover );
        }
        else
        {
            _p_btnVoice->setPushedImage( _p_imageDisableNormal );
            _p_btnVoice->setNormalImage( _p_imageDisableNormal );
            _p_btnVoice->setHoverImage( _p_imageDisableHoover );
        }
    }

    if ( inputmute )
        _p_volumeInput->disable();
    else
        _p_volumeInput->enable();
}

CEGUI::PushButton* VoiceGui::createVoiceButton()
{
    CEGUI::PushButton* p_btn = NULL;
    try
    {
        // get the imageset with the images; thanks to CrazyEddie for helping out here
        CEGUI::Imageset* p_iset = vrc::gameutils::GuiUtils::get()->getCustomImageSet();
        // get the scaled image dimensions.
        float pixwidth  = p_iset->getImageWidth( IMAGE_NAME_CHAT_NORMAL );
        float pixheight = p_iset->getImageHeight( IMAGE_NAME_CHAT_NORMAL );

        // setup the voice button
        p_btn = static_cast< CEGUI::PushButton* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Button", VOICELAYOUT_PREFIX "_btn_voice_" ) );
        p_btn->subscribeEvent( CEGUI::PushButton::EventClicked, CEGUI::Event::Subscriber( &vrc::VoiceGui::onClickedOpen, this ) );
        p_btn->subscribeEvent( CEGUI::PushButton::EventMouseEnters, CEGUI::Event::Subscriber( &vrc::VoiceGui::onHoverOpen, this ) );
        p_btn->setStandardImageryEnabled( false );
        p_btn->setPosition( CEGUI::Point( 0.005f, 0.8f ) );
        // set button size according to the image dimensions
        p_btn->setSize( CEGUI::Absolute, CEGUI::Size( pixwidth, pixheight ) );

        // set open button images
        const CEGUI::Image* p_image = NULL;

        p_image = vrc::gameutils::GuiUtils::get()->getCustomImage( IMAGE_NAME_VOICE_NORMAL );
        _p_imageNormal = new CEGUI::RenderableImage;
        _p_imageNormal->setImage( p_image );
        p_btn->setPushedImage( _p_imageNormal );
        p_btn->setNormalImage( _p_imageNormal );
        p_image = vrc::gameutils::GuiUtils::get()->getCustomImage( IMAGE_NAME_VOICE_HOOVER );
        _p_imageHoover = new CEGUI::RenderableImage;
        _p_imageHoover->setImage( p_image );
        p_btn->setHoverImage( _p_imageHoover );

        p_image = vrc::gameutils::GuiUtils::get()->getCustomImage( IMAGE_NAME_VOICE_DISABLE_NORMAL );
        _p_imageDisableNormal = new CEGUI::RenderableImage;
        _p_imageDisableNormal->setImage( p_image );
        p_image = vrc::gameutils::GuiUtils::get()->getCustomImage( IMAGE_NAME_VOICE_DISABLE_HOOVER );
        _p_imageDisableHoover = new CEGUI::RenderableImage;
        _p_imageDisableHoover->setImage( p_image );

        p_image = vrc::gameutils::GuiUtils::get()->getCustomImage( IMAGE_NAME_VOICE_MUTE_NORMAL );
        _p_imageMuteNormal = new CEGUI::RenderableImage;
        _p_imageMuteNormal->setImage( p_image );
        p_image = vrc::gameutils::GuiUtils::get()->getCustomImage( IMAGE_NAME_VOICE_MUTE_HOOVER );
        _p_imageMuteHoover = new CEGUI::RenderableImage;
        _p_imageMuteHoover->setImage( p_image );

        p_image = vrc::gameutils::GuiUtils::get()->getCustomImage( IMAGE_NAME_VOICE_MUTEDIS_NORMAL );
        _p_imageMuteDisableNormal = new CEGUI::RenderableImage;
        _p_imageMuteDisableNormal->setImage( p_image );
        p_image = vrc::gameutils::GuiUtils::get()->getCustomImage( IMAGE_NAME_VOICE_MUTEDIS_HOOVER );
        _p_imageMuteDisableHoover = new CEGUI::RenderableImage;
        _p_imageMuteDisableHoover->setImage( p_image );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "VoiceGui: problem setting up voice button" << std::endl;
        log_error << "   reason: " << e.getMessage() << std::endl;
        return NULL;
    }

    return p_btn;
}

CEGUI::Window* VoiceGui::createVoiceMenu()
{
    CEGUI::Window* p_wnd = NULL;

    try
    {
        // setup the voice button
        p_wnd = static_cast< CEGUI::Window* >( CEGUI::WindowManager::getSingleton().createWindow( "DefaultWindow", VOICELAYOUT_PREFIX "_wnd_voicemenu_" ) );
        p_wnd->setPosition( CEGUI::Point( 0.05f, 0.785f ) );
        p_wnd->setSize( CEGUI::Size( 0.2f, 0.085f ) );

        // setup the background image
        CEGUI::StaticImage* p_wndbackgrnd = static_cast< CEGUI::StaticImage* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/StaticImage", VOICELAYOUT_PREFIX "_wnd_voicemenu_backgrnd_" ) );
        p_wndbackgrnd->setPosition( CEGUI::Point( 0.0f, 0.0f ) );
        p_wndbackgrnd->setSize( CEGUI::Size( 1.0f, 1.0f ) );
        p_wndbackgrnd->setBackgroundEnabled( false );
        p_wndbackgrnd->setFrameEnabled( false );
        p_wndbackgrnd->setEnabled( false );
        const CEGUI::Image* p_image = vrc::gameutils::GuiUtils::get()->getCustomImage( IMAGE_NAME_VOICE_MENU );
        p_wndbackgrnd->setImage( p_image );
        p_wnd->addChildWindow( p_wndbackgrnd );

        _p_volumeInput = static_cast< CEGUI::Scrollbar* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/HorizontalScrollbar", VOICELAYOUT_PREFIX "_wnd_voicemenu_volin_" ) );
        _p_volumeInput->subscribeEvent( CEGUI::Scrollbar::EventThumbTrackEnded, CEGUI::Event::Subscriber( &vrc::VoiceGui::onInputVolumeChanged, this ) );
        _p_volumeInput->setPosition( CEGUI::Point( 0.036f, 0.1f ) );
        _p_volumeInput->setSize( CEGUI::Size( 0.62f, 0.18f ) );
        p_wnd->addChildWindow( _p_volumeInput );

        _p_volumeOutput = static_cast< CEGUI::Scrollbar* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/HorizontalScrollbar", VOICELAYOUT_PREFIX "_wnd_voicemenu_volout_" ) );
        _p_volumeOutput->subscribeEvent( CEGUI::Scrollbar::EventThumbTrackEnded, CEGUI::Event::Subscriber( &vrc::VoiceGui::onOutputVolumeChanged, this ) );
        _p_volumeOutput->setPosition( CEGUI::Point( 0.036f, 0.71f ) );
        _p_volumeOutput->setSize( CEGUI::Size( 0.62f, 0.18f ) );
        p_wnd->addChildWindow( _p_volumeOutput );

        _p_checkboxMute = static_cast< CEGUI::Checkbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Checkbox", VOICELAYOUT_PREFIX "_wnd_voicemenu_mute_" ) );
        _p_checkboxMute->subscribeEvent( CEGUI::Checkbox::EventCheckStateChanged, CEGUI::Event::Subscriber( &vrc::VoiceGui::onMuteChanged, this ) );
        _p_checkboxMute->setSize( CEGUI::Size( 0.2f, 0.25f ) );
        _p_checkboxMute->setPosition( CEGUI::Point( 0.72f, 0.17f ) );
        _p_checkboxMute->setSelected( false );
        p_wnd->addChildWindow( _p_checkboxMute );

        _p_checkboxOnOff = static_cast< CEGUI::Checkbox* >( CEGUI::WindowManager::getSingleton().createWindow( "TaharezLook/Checkbox", VOICELAYOUT_PREFIX "_wnd_voicemenu_off_" ) );
        _p_checkboxOnOff->subscribeEvent( CEGUI::Checkbox::EventCheckStateChanged, CEGUI::Event::Subscriber( &vrc::VoiceGui::onOnOffChanged, this ) );
        _p_checkboxOnOff->setSize( CEGUI::Size( 0.2f, 0.25f ) );
        _p_checkboxOnOff->setPosition( CEGUI::Point( 0.72f, 0.57f ) );
        _p_checkboxOnOff->setSelected( false );
        p_wnd->addChildWindow( _p_checkboxOnOff );
    }
    catch ( const CEGUI::Exception& e )
    {
        log_error << "VoiceGui: problem setting up voice menu" << std::endl;
        log_error << "   reason: " << e.getMessage() << std::endl;
        return NULL;
    }

    return p_wnd;
}

bool VoiceGui::onHoverOpen( const CEGUI::EventArgs& /*arg*/ )
{
    // play mouse over sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_HOVER );

    return true;
}

bool VoiceGui::onClickedOpen( const CEGUI::EventArgs& /*arg*/ )
{
    // play click sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_CLICK );

    // first update the values, they may have been changed in main options menu
    updateVoiceConfiguration();

    if ( _p_wndVoiceMenu )
    {
        if ( _p_wndVoiceMenu->isVisible() )
        {
            _p_wndVoiceMenu->hide();
        }
        else
        {
            _p_wndVoiceMenu->show();
            _p_wndVoiceMenu->moveToFront();
        }
    }

    return true;
}

bool VoiceGui::onOnOffChanged( const CEGUI::EventArgs& /*arg*/ )
{
    // play mouse over sound
    gameutils::GuiUtils::get()->playSound( GUI_SND_NAME_HOVER );

    // set the state in configuration
    bool  voicenable = !_p_checkboxOnOff->isSelected();
    yaf3d::Configuration::get()->setSettingValue( VRC_GS_VOICECHAT_ENABLE, voicenable );

    if ( _p_btnVoice )
    {
        if ( _p_checkboxOnOff->isSelected() )
        {
            _p_checkboxMute->disable();
            _p_volumeInput->disable();
            _p_volumeOutput->disable();
        }
        else
        {
            _p_checkboxMute->enable();
            _p_volumeInput->enable();
            _p_volumeOutput->enable();
        }
    }

    // update the button images
    updateButtonImages( voicenable, _p_checkboxMute->isSelected() );

    // update the net voice instance so the changes can take place
    _p_netVoice->updateConfiguration();

    return true;
}

bool VoiceGui::onMuteChanged( const CEGUI::EventArgs& /*arg*/ )
{
    bool inputmute = _p_checkboxMute->isSelected();
    yaf3d::Configuration::get()->setSettingValue( VRC_GS_VOICE_INPUT_MUTE, inputmute );

    // update the button images
    updateButtonImages( !_p_checkboxOnOff->isSelected(), inputmute );

    // update the net voice instance so the changes can take place
    _p_netVoice->updateConfiguration();

    return true;
}

bool VoiceGui::onInputVolumeChanged( const CEGUI::EventArgs& /*arg*/ )
{
    // update configuration
    float inputgain  = _p_volumeInput->getScrollPosition();
    yaf3d::Configuration::get()->setSettingValue( VRC_GS_VOICE_INPUT_GAIN, inputgain );

    // update the net voice instance so the changes can take place
    _p_netVoice->updateConfiguration();

    return true;
}

bool VoiceGui::onOutputVolumeChanged( const CEGUI::EventArgs& /*arg*/ )
{
    // update configuration
    float outputgain = _p_volumeOutput->getScrollPosition();
    yaf3d::Configuration::get()->setSettingValue( VRC_GS_VOICE_OUTPUT_GAIN, outputgain );

    // update the net voice instance so the changes can take place
    _p_netVoice->updateConfiguration();

    return true;
}

} // namespace vrc
