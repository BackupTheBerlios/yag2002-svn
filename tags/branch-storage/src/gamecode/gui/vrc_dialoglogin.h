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
 # dialog constrol for selecting a level
 #
 #   date of creation:  09/29/2007
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_DIALOGLOGIN_H_
#define _VRC_DIALOGLOGIN_H_

#include <vrc_main.h>

namespace vrc
{

class EnMenu;


class DialogLogin
{
    public:

        explicit                                    DialogLogin( EnMenu* p_menuEntity );

        virtual                                     ~DialogLogin();

        //! Initialize gui layout, return false if something went wrong.
        bool                                        initialize( const std::string& layoutfile );

        //! Enable / disable dialog.
        void                                        enable( bool en );

        //! Retrieve the login details. Calling the method erazes the login senstitive details for security reasons.
        void                                        getAndErazeDetails( std::string& login, std::string& passwd );

    protected:

        //! Dialog callback for Start button        
        bool                                        onClickedLogin( const CEGUI::EventArgs& arg );

        //! Dialog callback for Return button        
        bool                                        onClickedCancel( const CEGUI::EventArgs& arg );
    
        //  internal variables

        CEGUI::Window*                              _p_loginDialog;

        CEGUI::Editbox*                             _p_login;

        CEGUI::Editbox*                             _p_passwd;

        EnMenu*                                     _p_menuEntity;

        bool                                        _enable;
};

}

#endif // _VRC_DIALOGLOGIN_H_
