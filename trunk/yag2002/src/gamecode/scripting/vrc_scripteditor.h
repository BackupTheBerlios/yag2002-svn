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
 # Small build-in utility for script debugging
 #
 #   date of creation:  04/11/2007
 #
 #   author:            ali botorabi (boto)
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/


#ifndef _VRC_SCRIPTEDITOR_H_
#define _VRC_SCRIPTEDITOR_H_

#include <vrc_main.h>

namespace vrc
{

//! Build-in script editor which can be derived from in your script classes
class ScriptEditor
{
    public:

                                                    ScriptEditor();

        virtual                                     ~ScriptEditor();

        //! Initialize the editor
        bool                                        seInitialize();

        //! Show/hide the editor
        void                                        seShow( bool en );

        //! Add message to output
        void                                        seAddOutput( const std::string& msg );

    protected:

        //! Callback for getting cmd line
        bool                                        seOnInputTextChanged( const CEGUI::EventArgs& arg );

        //! Override this process input command method
        virtual void                                seProcessCmd( const std::string& cmd ) = 0;

        //! Script gui object
        CEGUI::FrameWindow*                         _p_seScriptWnd;

        //! Script editor's output
        CEGUI::Editbox*                             _p_seInputWindow;

        //! Script editor's output
        CEGUI::MultiLineEditbox*                    _p_seOutputWindow;
};

}

#endif // _VRC_SCRIPTEDITOR_H_
