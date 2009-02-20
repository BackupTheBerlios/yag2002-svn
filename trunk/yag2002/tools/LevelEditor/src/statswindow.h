/****************************************************************
 *  YAG2002 (http://yag2002.sourceforge.net)
 *  Copyright (C) 2005-2009, A. Botorabi
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
 # editor's stats window
 #
 #   date of creation:  02/03/2009
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _STATSWINDOW__H_
#define _STATSWINDOW__H_

#include <vrc_main.h>
#include "guibase.h"


#define SYMBOL_STATSWINDOW_STYLE      wxCAPTION|wxDIALOG_NO_PARENT
#define SYMBOL_STATSWINDOW_TITLE      _("Stats")
#define SYMBOL_STATSWINDOW_IDNAME     ID_STATS
#define SYMBOL_STATSWINDOW_SIZE       wxSize(400, 300)
#define SYMBOL_STATSWINDOW_POSITION   wxDefaultPosition


//! Statistics window
class StatsWindow: public wxDialog
{
    DECLARE_DYNAMIC_CLASS( StatsWindow )
    DECLARE_EVENT_TABLE()

    public:

                                                    StatsWindow();

        virtual                                     ~StatsWindow();

        //! Set FPS
        void                                        setFPS( unsigned int fps );

        //! Set camera speed
        void                                        setCameraSpeed( float speed );

        //! Set position
        void                                        setCameraPosition( const osg::Vec3f& pos );

        //! Set rotation
        void                                        setCameraPitchYaw( float pitch, float yaw );

        //! Enable/disable the inspector view
        void                                        enableInspectorDisplay( bool en );

        //! Set the inspector position.
        void                                        setInspectorPosition( const osg::Vec3f& pos );

        //! Set the inspector normal.
        void                                        setInspectorNormal( const osg::Vec3f& norm );

        //! Set the inspector object.
        void                                        setInspectorObject( const std::string& object );

    protected:

        //! Create the window controls.
        void                                        createControls();

        wxStaticText*                               _p_textCamSpeed;

        wxTextCtrl*                                 _p_textCamPosition;

        wxTextCtrl*                                 _p_textCamRotation;

        wxStaticBox*                                _p_sizerInspector;

        wxTextCtrl*                                 _p_textInspPosition;

        wxTextCtrl*                                 _p_textInspNormal;

        wxTextCtrl*                                 _p_textInspObject;

        float                                       _currSpeed;

        //! Control IDs
        enum
        {
            ID_STATS                = 10500,
            ID_PANEL                = 10501,
            ID_TEXT_CAM_SPEED       = 10502,
            ID_TEXTCTRL_CAM_POS     = 10503,
            ID_TEXTCTRL_CAM_ROT     = 10504,
            ID_TEXTCTRL_INSP_POS    = 10505,
            ID_TEXTCTRL_INSP_NORMAL = 10506,
            ID_TEXTCTRL_INSP_OBJECT = 10507
        };
};

#endif // _STATSWINDOW__H_
