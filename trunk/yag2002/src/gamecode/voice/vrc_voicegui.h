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


#ifndef _VRC_VOICECHATGUI_H_
#define _VRC_VOICECHATGUI_H_


namespace vrc
{

class EnNetworkVoice;


//! Class for controling the voice related gui elements
class VoiceGui
{
    public:

        explicit                                    VoiceGui( EnNetworkVoice* p_netvoice );

                                                    ~VoiceGui();

        //! Initialize the gui
        void                                        initialize();

        //! Update the gui, this is used for animations
        void                                        update( float deltaTime );

        //! Update the gui values regarding to game settings which may change using the main options menu
        void                                        updateVoiceConfiguration();

        //! Let the connection icon display.
        void                                        showConnectingIcon( bool con );

    protected:

        //! Create the voice button
        CEGUI::PushButton*                          createVoiceButton();

        //! Create the voice menu
        CEGUI::Window*                              createVoiceMenu();

        //! Create connection button
        CEGUI::PushButton*                          createConnectionButton();

        //! Update the voice button images given the voice and mute states
        void                                        updateButtonImages( bool voicenable, bool inputmute );

        //! Callback for button 'open settings'
        bool                                        onClickedOpen( const CEGUI::EventArgs& arg );

        //! Callback for mouse over button 'open settings'
        bool                                        onHoverOpen( const CEGUI::EventArgs& arg );

        //! Callback for changed checkbox 'On/Off'
        bool                                        onOnOffChanged( const CEGUI::EventArgs& arg );

        //! Callback for changed checkbox 'Mute'
        bool                                        onMuteChanged( const CEGUI::EventArgs& arg );

        //! Callback for changed input volume scrollbar
        bool                                        onInputVolumeChanged( const CEGUI::EventArgs& arg );

        //! Callback for input volume scrollbar changing
        bool                                        onInputVolumeChanging( const CEGUI::EventArgs& arg );

        //! Callback for changed output volume scrollbar
        bool                                        onOutputVolumeChanged( const CEGUI::EventArgs& arg );

        //! Callback for output volume scrollbar changing
        bool                                        onOutputVolumeChanging( const CEGUI::EventArgs& arg );

        //! The net voice entity object
        EnNetworkVoice*                             _p_netVoice;

        //! Voice button
        CEGUI::PushButton*                          _p_btnVoice;

        //! Voice connection button
        CEGUI::PushButton*                          _p_btnConnection;

        //! Voice menu
        CEGUI::Window*                              _p_wndVoiceMenu;

        //! Button images for normal state
        CEGUI::RenderableImage*                     _p_imageNormal;
        CEGUI::RenderableImage*                     _p_imageHoover;

        //! Button images for disable state
        CEGUI::RenderableImage*                     _p_imageDisableNormal;
        CEGUI::RenderableImage*                     _p_imageDisableHoover;

        //! Button images for mute state
        CEGUI::RenderableImage*                     _p_imageMuteNormal;
        CEGUI::RenderableImage*                     _p_imageMuteHoover;

        //! Button images for mute and disabled state
        CEGUI::RenderableImage*                     _p_imageMuteDisableNormal;
        CEGUI::RenderableImage*                     _p_imageMuteDisableHoover;

        //! On/Off checkbox
        CEGUI::Checkbox*                            _p_checkboxOnOff;

        //! Mute checkbox
        CEGUI::Checkbox*                            _p_checkboxMute;

        //! Input volume
        CEGUI::Scrollbar*                           _p_volumeInput;

        //! Output volume
        CEGUI::Scrollbar*                           _p_volumeOutput;

        // current connection state
        enum
        {
            eIdle,
            eConnectionIconDisplay
        }                                           _connectionIconState;

        //! Start showing the connection icon
        bool                                        _connectionIconShow;

        //! Timer for connection icon
        float                                       _connectionIconTimer;
};

} // namespace vrc

#endif /* _VRC_VOICECHATGUI_H_ */
