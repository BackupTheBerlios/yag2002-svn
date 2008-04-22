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
 # wxWidget's main app
 #
 #   date of creation:  07/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _EDITOR_H_
#define _EDITOR_H_

#include <main.h>

namespace beditor
{

class EditorApp: public wxApp
{    
        DECLARE_CLASS( EditorApp )
        DECLARE_EVENT_TABLE()

    public:

                                EditorApp();

    protected:

        void                    initialize();

        //! Initialize the application
        virtual bool            OnInit();

        //! Called on exit
        virtual int             OnExit();
};

} // namespace beditor

DECLARE_APP( beditor::EditorApp )

#endif // _EDITOR_H_
