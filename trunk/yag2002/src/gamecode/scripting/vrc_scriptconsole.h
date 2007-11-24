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
 # Small build-in console for script debugging
 #
 #   date of creation:  04/11/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/


#ifndef _VRC_SCRIPTCONSOLE_H_
#define _VRC_SCRIPTCONSOLE_H_

#include <vrc_main.h>

namespace vrc
{

//! Build-in script editor which can be derived from in your script classes
class ScriptConsole
{
    public:

                                                    ScriptConsole();

        virtual                                     ~ScriptConsole();

        //! Initialize the editor
        bool                                        scInitialize();

        //! Show/hide the editor
        void                                        scShow( bool en );

        //! Add message to output
        void                                        scAddOutput( const std::string& msg, bool timestamp = false );

    protected:

        //! Callback for closing the frame
        bool                                        onClickedClose( const CEGUI::EventArgs& arg );

        //! Callback for getting cmd line
        bool                                        scOnInputTextChanged( const CEGUI::EventArgs& arg );

        //! Override this process input command method
        virtual void                                scProcessCmd( const std::string& cmd ) = 0;

        //! Script gui object
        CEGUI::FrameWindow*                         _p_scScriptWnd;

        //! Script editor's output
        CEGUI::Editbox*                             _p_scInputWindow;

        //! Script editor's output
        CEGUI::MultiLineEditbox*                    _p_scOutputWindow;
};

}

#endif // _VRC_SCRIPTCONSOLE_H_
