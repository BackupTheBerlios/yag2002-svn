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
 #   author:            boto (botorabi at users.sourceforge.net) 
 #
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

        //! Dialog callback for create account button
        bool                                        onClickedRegistration( const CEGUI::EventArgs& arg );

        //! Dialog callback for cancel button
        bool                                        onClickedCancel( const CEGUI::EventArgs& arg );

        //! Dialog callback for cancel registration button
        bool                                        onClickedRegistrationCancel( const CEGUI::EventArgs& arg );

        //! Dialog callback for create registration button
        bool                                        onClickedRegistrationCreate( const CEGUI::EventArgs& arg );

        //  internal variables

        CEGUI::Window*                              _p_loginDialog;

        CEGUI::Window*                              _p_registrationDialog;

        CEGUI::Editbox*                             _p_login;

        CEGUI::Editbox*                             _p_passwd;

        CEGUI::Editbox*                             _p_regNickName;

        CEGUI::Editbox*                             _p_regRealName;
        
        CEGUI::Editbox*                             _p_regPasswd;
        
        CEGUI::Editbox*                             _p_regEmail;

        EnMenu*                                     _p_menuEntity;

        bool                                        _enable;
};

}

#endif // _VRC_DIALOGLOGIN_H_
