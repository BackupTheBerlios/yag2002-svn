/****************************************************************
 *	3D Game	'Capture The Diamond'
 *	Copyright (C) 2004-2006, Ali Botorabi
 *
 *	This program is	free software; you can redistribute	it and/or 
 *	modify it under	the	terms of the GNU General Public	License	
 *	as published by	the	Free Software Foundation; either version 2
 *	of the License,	or (at your	option)	any	later version.
 *
 *	This program is	distributed	in the hope	that it	will be	useful,
 *	but	WITHOUT	ANY	WARRANTY; without even the implied warranty	of
 *	MERCHANTABILITY	or FITNESS FOR A PARTICULAR	PURPOSE.  See the
 *	GNU	General	Public License for more	details.
 *
 *	You	should have	received a copy	of the GNU General Public 
 *	License	along with this	program; if	not, write to the Free 
 *	Software Foundation, Inc., 59 Temple Place,	Suite 330, Boston, 
 *	MA	02111-1307	USA
 * 
 ****************************************************************/

/*###############################################################
 # graphical user interface: message box
 #
 #	 date of creation:	04/06/2005
 #
 #	 author:			ali	botorabi (boto)	
 #		e-mail:			botorabi@gmx.net
 #
 ################################################################*/

#ifndef	_CTD_GUIMSGBOX_H_
#define	_CTD_GUIMSGBOX_H_

#include <CEGUIBase.h>
#include <ctd_guimanager.h>

namespace CTD
{
//! Message box dialog
/** 
*  The message text can contain linefeeds ("\n") for a multi-line output. The size of the dialog is adjusted to the message text.
*  example:
*   
    MessageBoxDialog* p_msgbox = new MessageBoxDialog( "Yes-No message box", "This is a text with\nmultiple lines and\nYes/No buttons.", MessageBoxDialog::YES_NO, true );
    MyClickClb* p_msgClb = new MyClickClb;
    p_msgbox->setClickCallback( p_msgClb );
    p_msgbox->show();
*
* When Yes or No button is clicked then the callback method in p_msgClb is called with clicked button id. Note that in this example the auto-deletion is
*  activated, so after clicking one of the buttons both objects p_msgbox and p_msgClb are deleted automatically.
*/
class MessageBoxDialog
{
    public:

        //! Supported dialog types
        enum DialogType 
        {
            YES_NO = 0x10,
            OK_CANCEL,
            OK
        };

        //! Construct the message box dialog
        /**
        * Set parameter autodelete to true if you want that the object is automatically deleted after clicking a button. Otherwise
        *  the dialog is only hidden after a click. The user has then to delete the dialog object and optionally set callback object ( see below ).
        * The object must not further used after clicking a button!
        * If p_parent is NULL then the dialog will be appended into root window.
        */
                                                MessageBoxDialog( 
                                                                 const std::string& title, 
                                                                 const std::string& text, 
                                                                 unsigned int type, 
                                                                 bool autodelete = true,
                                                                 CEGUI::Window* p_parent = NULL
                                                                );

                                                ~MessageBoxDialog();

        //! Show up the message box
        void                                    show();

        //! Button click callback flags
        enum ClickFlags 
        {
            BTN_OK = 0x1,
            BTN_CANCEL,
            BTN_YES,
            BTN_NO
        };

        //! Class for defining client callback which is used when a button has been clicked.
        class ClickCallback
        {
            public:

                virtual                         ~ClickCallback() {}
 
                virtual void                    onClicked( unsigned int btnId ) = 0;
        };

        //! Set callback which is called when a button has been clicked.
        void                                        setClickCallback( MessageBoxDialog::ClickCallback* p_clb );

    protected:

        bool                                    onClickedOk( const CEGUI::EventArgs& arg );

        bool                                    onClickedCancel( const CEGUI::EventArgs& arg );

        bool                                    onClickedYes( const CEGUI::EventArgs& arg );

        bool                                    onClickedNo( const CEGUI::EventArgs& arg );

        void                                    processClick( unsigned int );

    protected:

        CEGUI::FrameWindow*                     _p_wnd;

        CEGUI::Window*                          _p_parent;

        MessageBoxDialog::ClickCallback*        _p_clb;

        bool                                    _autodel;
};

} // namespace CTD


#endif	// _CTD_GUIMSGBOX_H_
