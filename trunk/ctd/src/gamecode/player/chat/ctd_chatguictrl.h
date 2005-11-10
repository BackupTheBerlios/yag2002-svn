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
 # player's chat gui control
 #
 #   date of creation:  10/16/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _CTD_CHATGUICTRL_H_
#define _CTD_CHATGUICTRL_H_

#include <ctd_main.h>
#include "ctd_chatmgr.h"

namespace CTD
{

class ChatManager;

//! Class for player's chat gui control
class ChatGuiCtrl
{

    public:

                                                    ChatGuiCtrl();

        virtual                                     ~ChatGuiCtrl();

        //! Initialize gui control
        void                                        initialize( ChatManager* p_chatMgr );

        //! Set Edit / Walk mode ( is used to set the mode image )
        void                                        setEditMode( bool en );

        //! Show / hide control elements
        void                                        show( bool en );

    protected:

        CEGUI::Window*                              _p_wnd;

        CEGUI::StaticImage*                         _p_btnMode;

        ChatManager*                                _p_chatMgr;
};

} // namespace CTD

#endif // _CTD_CHATGUICTRL_H_
