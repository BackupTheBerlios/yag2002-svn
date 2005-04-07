/****************************************************************
 *  3D Game 'Capture The Diamond'
 *  Copyright (C) 2004-2006, Ali Botorabi
 *
 *  This program is free software; you can redistribute it and/or 
 *  modify it under the terms of the GNU General Public License 
 *  as published by the Free Software Foundation; either version 2
 *  of the License, or (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public 
 *  License along with this program; if not, write to the Free 
 *  Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, 
 *  MA  02111-1307  USA
 * 
 ****************************************************************/

/*###############################################################
 # settings dialog control
 #
 #   date of creation:  06/05/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_DIALOGSETTINGS_H_
#define _CTD_DIALOGSETTINGS_H_

#include <ctd_base.h>
#include <ctd_guimanager.h>

namespace CTD
{

class DialogGameSettings
{
    public:
                                                    DialogGameSettings();

        virtual                                     ~DialogGameSettings();

        //! Initialize gui layout, return false when something went wrong. 
        bool                                        initialize( const std::string& layoutfile );

        //! Update dialog control
        void                                        update( float deltaTime );

        //! Show/hide the dialog
        void                                        show( bool visible );

    protected:

        //! Is asked on canceling dialog
        bool                                        isDirty();

        //! Setup all controls when the dialog is opening ( show( true ) )
        void                                        setupControls();

        //! Dialog callback for Ok button        
        bool                                        onClickedOk( const CEGUI::EventArgs& arg );

        //! Dialog callback for Cancel button        
        bool                                        onClickedCancel( const CEGUI::EventArgs& arg );

        //! Dialog callback for mouse sensitivity scrollbar        
        bool                                        onMouseSensitivityChanged( const CEGUI::EventArgs& arg );
       
        //! Busy flag ( see method onClickedOk for more details )
        bool                                        _busy;

        //  internal variables

        CEGUI::Window*                              _p_settingsDialog;

        CEGUI::Editbox*                             _p_playername;

        CEGUI::Editbox*                             _p_serverName;

        CEGUI::Editbox*                             _p_serverIP;

        CEGUI::Editbox*                             _p_serverPort;

        CEGUI::Scrollbar*                           _p_mouseSensivity;

        float                                       _mouseSensitivity;

        CEGUI::Checkbox*                            _p_mouseInvert;

        bool                                        _mouseInverted;
};

}

#endif // _CTD_DIALOGSETTINGS_H_
