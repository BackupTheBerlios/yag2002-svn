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
 # connection dialog
 #
 #   date of creation:  11/10/2005
 #
 #   author:            ali botorabi (boto) 
 #      e-mail:         botorabi@gmx.net
 #
 ################################################################*/

#ifndef _VRC_CHATGUICONDIAG_H_
#define _VRC_CHATGUICONDIAG_H_

#include <vrc_main.h>
#include <vrc_gameutils.h>
#include "vrc_chatmgr.h"

namespace vrc
{

//! Connection dialog
/**
* TypeT must have following two callback methods:
*
* void onConnectionDialogClickedConnect( const ChatConnectionConfig& conf );
* void onConnectionDialogClickedCancel();
*
*/
template< class TypeT >
class ConnectionDialog
{
    public:

        explicit                                    ConnectionDialog( TypeT* p_parent );

        virtual                                     ~ConnectionDialog();

        //! Set pane title
        void                                        setTitle( const std::string& title )
                                                    {
                                                        _p_frame->setText( title );
                                                    }

        //! Update the configuration settings
        void                                        setConfiguration( const ChatConnectionConfig& cfg )
                                                    {
                                                        _cfg = cfg;
                                                    }

        ChatConnectionConfig&                       getConfiguration()
                                                    {
                                                        return _cfg;
                                                    }

        //! Show / hide dialog
        void                                        show( bool en );

    protected:

        //! Callback for clicking 'connect' button
        bool                                        onClickedConnect( const CEGUI::EventArgs& arg );

        //! Callback for key down, checking for "Return" key
        bool                                        onKeyDown( const CEGUI::EventArgs& arg );

        //! Callback for clicking 'cancel' button
        bool                                        onClickedCancel( const CEGUI::EventArgs& arg );

        TypeT*                                      _p_parent;

        CEGUI::FrameWindow*                         _p_frame;

        CEGUI::Editbox*                             _p_editProtocol;

        CEGUI::Editbox*                             _p_editServerUrl;

        CEGUI::Editbox*                             _p_editChannel;

        CEGUI::Editbox*                             _p_editNickName;

        CEGUI::Editbox*                             _p_editUserName;

        CEGUI::Editbox*                             _p_editRealName;

        CEGUI::Editbox*                             _p_editPassword;

        CEGUI::Editbox*                             _p_editPort;

        ChatConnectionConfig                        _cfg;
};

// include the inline file
#include "vrc_chatguicondiag.inl"

} // namespace vrc

#endif // _VRC_CHATGUICONDIAG_H_
