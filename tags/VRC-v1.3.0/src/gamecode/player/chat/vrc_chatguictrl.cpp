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
 # player's chat gui control
 #
 #   date of creation:  10/16/2005
 #
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_chatguictrl.h"
#include "vrc_chatmgr.h"

namespace vrc
{

ChatGuiCtrl::ChatGuiCtrl() :
_p_wnd( NULL ),
_editMode( false )
{
}

ChatGuiCtrl::~ChatGuiCtrl()
{
}

void ChatGuiCtrl::initialize( ChatManager* p_chatMgr )
{
    _p_chatMgr = p_chatMgr;
    _p_wnd     = gameutils::GuiUtils::get()->getMainGuiWindow();

    gameutils::GuiUtils::get()->showMousePointer( false );
}

void ChatGuiCtrl::setEditMode( bool en )
{
    if ( !en )
    {
        gameutils::GuiUtils::get()->showMousePointer( false );
    }
    else
    {
        gameutils::GuiUtils::get()->showMousePointer( true );
    }

    _editMode = en;
}

void ChatGuiCtrl::show( bool en )
{
    if ( en )
    {
        _p_wnd->show();
        // restore the mode
        setEditMode( _editMode );
    }
    else
    {
        //! TODO: centralize this task! other gui elements are also attached to main window.
        _p_wnd->hide();
    }
}

} // namespace vrc
