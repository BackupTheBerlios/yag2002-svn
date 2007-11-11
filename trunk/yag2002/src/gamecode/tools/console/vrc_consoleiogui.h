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
 # gui-based io handler for an in-game console
 #
 #   date of creation:  09/09/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#ifndef _VRC_CONSOLE_IO_GUI_H_
#define _VRC_CONSOLE_IO_GUI_H_

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_consoleiobase.h"

namespace vrc
{

class EnConsole;

// Input handler for gui based console
class ConsoleIOGui : public vrc::gameutils::GenericInputHandler< EnConsole >, public ConsoleIOBase
{
    public:

        explicit                                    ConsoleIOGui( EnConsole* p_console );

        virtual                                     ~ConsoleIOGui();

        //! Handle input events.
        bool                                        handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa );

        //! Enable / disable io
        void                                        enable( bool en );

        //! Set cmd line text
        void                                        setCmdLine( const std::string& text );

        //! Output a text
        void                                        output( const std::string& text );

        //! Shutdown this handler
        void                                        shutdown();

    protected:

        //! Callback for chat frame window close button
        bool                                        onCloseFrame( const CEGUI::EventArgs& arg );

        void                                        resetToggle( bool en );

        int                                         _retCode;

        int                                         _autoCompleteCode;

        bool                                        _toggleEnable;

        // Console window
        CEGUI::Window*                              _p_wnd;

        CEGUI::Editbox*                             _p_inputWindow;

        CEGUI::MultiLineEditbox*                    _p_outputWindow;

        bool                                        _enable;
};

} // namespace vrc

#endif // _VRC_CONSOLE_IO_GUI_H_
