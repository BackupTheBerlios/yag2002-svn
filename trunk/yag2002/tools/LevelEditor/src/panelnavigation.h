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
 # navigation panel
 #
 #   date of creation:  02/02/2009
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/


#ifndef _PANELNAVIGATION_H_
#define _PANELNAVIGATION_H_


#include "guibase.h"


#define SYMBOL_PANELNAVIGATION_STYLE    wxRAISED_BORDER|wxTAB_TRAVERSAL
#define SYMBOL_PANELNAVIGATION_IDNAME   ID_PANEL_NAVIGATION
#define SYMBOL_PANELNAVIGATION_SIZE     wxDefaultSize
#define SYMBOL_PANELNAVIGATION_POSITION wxDefaultPosition


class PanelNavigation: public wxPanel
{
    DECLARE_DYNAMIC_CLASS( PanelNavigation )
    DECLARE_EVENT_TABLE()

    public:

        explicit                                    PanelNavigation( wxWindow* parent );

                                                    ~PanelNavigation();

        //! Load the navigation settings.
        void                                        loadSettings();

    protected:

        //! Make wx happy
                                                    PanelNavigation() {}

        //! Create the controls and sizers.
        void                                        createControls();

       //! wxEVT_COMMAND_CHECKBOX_CLICKED event handler for ID_CHECKBOX_NAV_ENABLE
        void                                        onCheckboxNavEnableClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_NAV_DEFAULTS
        void                                        onButtonNavDefaultsClick( wxCommandEvent& event );

        //! wxEVT_COMMAND_BUTTON_CLICKED event handler for ID_BUTTON_NAV_APPLY
        void                                        onButtonNavApplyClick( wxCommandEvent& event );

        wxCheckBox*                                 _p_checkboxNavEnable;

        wxChoice*                                   _p_choiceNavMode;

        wxTextCtrl*                                 _p_textNavSpeed;

        wxTextCtrl*                                 _p_textNavPosition;

        wxTextCtrl*                                 _p_textNavRotation;

        wxTextCtrl*                                 _p_textNavFOV;

        wxTextCtrl*                                 _p_textNavNearClip;

        wxTextCtrl*                                 _p_textNavFarClip;

        wxColourPickerCtrl*                         _p_textNavBkgColor;

        //! Control IDs
        enum
        {
            ID_PANEL_NAVIGATION         = 10600,
            ID_CHECKBOX_NAV_ENABLE      = 10601,
            ID_CHOICE_NAV_MODE          = 10602,
            ID_TEXTCTRL_NAV_SPEED       = 10603,
            ID_TEXTCTRL_NAV_POS         = 10604,
            ID_TEXTCTRL_NAV_ROTATION    = 10605,
            ID_TEXTCTRL_NAV_FOV         = 10606,
            ID_TEXTCTRL_NAV_NEARCLIP    = 10607,
            ID_TEXTCTRL_NAV_FARCLIP     = 10608,
            ID_COLPICKERCTRL            = 10609,
            ID_BUTTON_NAV_DEFAULTS      = 10610,
            ID_BUTTON_NAV_APPLY         = 10611
        };
};

#endif // _PANELNAVIGATION_H_
