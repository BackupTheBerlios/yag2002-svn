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
 # settings dialog control
 #
 #   date of creation:  04/05/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_DIALOGSETTINGS_H_
#define _CTD_DIALOGSETTINGS_H_

#include <ctd_main.h>

namespace vrc
{

class EnMenu;
class EnAmbientSound;
class DialogGameSettings;
class DialogPlayerConfig;

//! Input handler for sensing
class BtnInputHandler : public yaf3d::GenericInputHandler< CEGUI::PushButton >
{
    public:

                                                    BtnInputHandler( CEGUI::PushButton* p_obj, DialogGameSettings* p_dlg ) : 
                                                        yaf3d::GenericInputHandler< CEGUI::PushButton >( p_obj ),
                                                        _p_dlg( p_dlg ),
                                                        _lockInput( false )
                                                    {}

        virtual                                     ~BtnInputHandler() {};
        
        bool                                        handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

    protected:

        //! Updates key bindings considering potential overriding bindings
        void                                        updateBindings( const std::string newkey );

        DialogGameSettings*                         _p_dlg;

        bool                                        _lockInput;
};

//! Class for game settings dialog 
class DialogGameSettings
{
    public:

        explicit                                    DialogGameSettings( EnMenu* p_menuEntity );

        virtual                                     ~DialogGameSettings();

        //! Initialize gui layout, return false when something went wrong.
        bool                                        initialize( const std::string& layoutfile );

        //! Update dialog control
        void                                        update( float deltaTime );

        //! Show/hide the dialog
        void                                        show( bool visible );

        //! Set click sound object
        void                                        setClickSound( EnAmbientSound* p_sound );

    protected:

        // Interface method for player configuration dialog
        //---
        //! This is called by player configuration instance when the player closes player configuration dialog
        void                                        onPlayerConfigDialogClose();
        //---

        //! Is asked on canceling dialog
        bool                                        isDirty();

        //! Setup all controls when the dialog is opening ( show( true ) )
        void                                        setupControls();

        //! Dialog callback for config player button        
        bool                                        onClickedPlayerConfig( const CEGUI::EventArgs& arg );

        //! Dialog callback for Ok button        
        bool                                        onClickedOk( const CEGUI::EventArgs& arg );

        //! Dialog callback for Cancel button        
        bool                                        onClickedCancel( const CEGUI::EventArgs& arg );

        //! Dialog callback for changing the tab      
        bool                                        onTabChanged( const CEGUI::EventArgs& arg );

        //! Dialog callback for mouse sensitivity scrollbar        
        bool                                        onMouseSensitivityChanged( const CEGUI::EventArgs& arg );
       
        //! Dialog callback for keyboarch checkbox: english        
        bool                                        onKeyboardEnglishChanged( const CEGUI::EventArgs& arg );
       
        //! Dialog callback for keyboarch checkbox: german        
        bool                                        onKeyboardGermanChanged( const CEGUI::EventArgs& arg );
       
        //! Dialog callback for move forward button        
        bool                                        onClickedForward( const CEGUI::EventArgs& arg );

        //! Dialog callback for move backward button        
        bool                                        onClickedBackward( const CEGUI::EventArgs& arg );

        //! Dialog callback for move left button        
        bool                                        onClickedLeft( const CEGUI::EventArgs& arg );

        //! Dialog callback for move right button        
        bool                                        onClickedRight( const CEGUI::EventArgs& arg );

        //! Dialog callback for jump button        
        bool                                        onClickedJump( const CEGUI::EventArgs& arg );

        //! Dialog callback for camera mode button        
        bool                                        onClickedCameraMode( const CEGUI::EventArgs& arg );

        //! Dialog callback for chat mode button        
        bool                                        onClickedChatMode( const CEGUI::EventArgs& arg );

        //! Dialog callback for fullscreen checkbox        
        bool                                        onFullscreenChanged( const CEGUI::EventArgs& arg );

        //! This method is used for sensing keybinding by the mean of a little messagebox
        void                                        senseKeybinding( CEGUI::PushButton* p_btn );
        //---------

        //  internal variables

        //! Busy flag ( see method onClickedOk for more details )
        bool                                        _busy;
    
        EnAmbientSound*                             _p_clickSound;

        CEGUI::Window*                              _p_settingsDialog;

        CEGUI::Editbox*                             _p_playerName;

        CEGUI::Editbox*                             _p_serverName;

        CEGUI::Editbox*                             _p_serverIP;

        CEGUI::Editbox*                             _p_serverPort;

        CEGUI::Scrollbar*                           _p_mouseSensivity;

        float                                       _mouseSensitivity;

        CEGUI::PushButton*                          _p_keyMoveForward;

        CEGUI::PushButton*                          _p_keyMoveBackward;

        CEGUI::PushButton*                          _p_keyMoveLeft;

        CEGUI::PushButton*                          _p_keyMoveRight;

        CEGUI::PushButton*                          _p_keyCameraMode;

        CEGUI::PushButton*                          _p_keyJump;

        CEGUI::Checkbox*                            _p_mouseInvert;

        CEGUI::PushButton*                          _p_keyChatMode;

        bool                                        _mouseInverted;

        CEGUI::Checkbox*                            _p_keyKeybEnglish;

        CEGUI::Checkbox*                            _p_keyKeybGerman;

        CEGUI::Combobox*                            _p_resolution;

        CEGUI::RadioButton*                         _p_fullscreen;

        CEGUI::RadioButton*                         _p_wndscreen;

        EnMenu*                                     _p_menuEntity;

        std::auto_ptr< DialogPlayerConfig >         _playerConfigDialog;

        //! Lookup for current key bindings, it is used for detecting an overriding key binding
        std::vector< std::pair< std::string, CEGUI::PushButton* > > _keyBindingLookup;
        typedef std::vector< std::pair< std::string, CEGUI::PushButton* > >     tBindingLookup;

    friend class BtnInputHandler;
    friend class DialogPlayerConfig;
};

}

#endif // _CTD_DIALOGSETTINGS_H_
