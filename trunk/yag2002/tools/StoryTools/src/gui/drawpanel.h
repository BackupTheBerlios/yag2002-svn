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
 # draw panel where the display and editing of the blocks happens
 #
 #   date of creation:  07/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _DRAWPANEL_H_
#define _DRAWPANEL_H_

#include <main.h>
#include <core/rendermanager.h>

namespace beditor
{

/*! Draw panel class */
class DrawPanel: public wxPanel
{    
    //DECLARE_CLASS( DrawPanel )
    DECLARE_EVENT_TABLE()

    public:

        // Construct the draw panel.
        explicit                            DrawPanel( wxWindow* p_parent );

        virtual                             ~DrawPanel();

        //! Reset the zoom
        void                                resetZoom();

        //! Forces the draw panel to re-render the gl canvas area.
        //! NOTE: using wxPanel's Refresh method causes a flicker!
        void                                forceRedraw();

    protected:

        //! Create the controls.
        void                                createControls();

        //! wxEVT_SIZE event handler for ID_PANEL_DRAW
        void                                onSize( wxSizeEvent& event );

        //! wxEVT_KEY_DOWN and wxEVT_KEY_UP event handler for ID_PANEL_DRAW
        void                                onKey( wxKeyEvent& event );

        //! wxEVT_PAINT event handler for ID_PANEL_DRAW
        void                                onPaint( wxPaintEvent& event );

        //! All mouse events event handler for ID_PANEL_DRAW
        void                                onMouse( wxMouseEvent& event );

        //! wxEVT_SET_FOCUS event handler for ID_PANEL_DRAW
        void                                onSetFocus( wxFocusEvent& event );

        //! wxEVT_KILL_FOCUS event handler for ID_PANEL_DRAW
        void                                onKillFocus( wxFocusEvent& event );

        //! Panel ID
        enum
        {
            ID_DRAW_PANEL  = 19000,
            ID_DRAW_CANVAS = 19001
        };

        //! Current pan
        wxPoint                             _pan;

        //! Last mouse position
        wxPoint                             _lastMousePosition;

        //! Zoom factor
        float                               _zoom;

        //! Combination of edit states
        enum EditState
        {
            eStateNode      = 0x0000,
            eStatePanning   = 0x0001,
            eStateMultiSel  = 0x0002
        };

        //! Edit state
        unsigned int                        _editState;

        //! Currently selected nodes
        std::vector< BaseNodePtr >          _selNodes;

        //! The opengl canvas we will draw to
        wxGLCanvas*                         _p_canvas;
};

} // namespace beditor

#endif _DRAWPANEL_H_
