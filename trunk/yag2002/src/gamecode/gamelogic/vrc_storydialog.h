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
 # story dialog implementation
 #
 #   date of creation:  08/05/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/


#ifndef _VRC_STORYDIALOG_H_
#define _VRC_STORYDIALOG_H_

#include <vrc_main.h>
#include <vrc_gameutils.h>

namespace vrc
{

//! Story dialog
class StoryDialog
{
    public:

        //! Answer types
        enum AnswerType
        {
            //! The anwer is a choice.
            eChoice   = 0x01,
            //! The answer is a string.
            eString   = 0x02
        };

        //! Callback for notifying the player answer in a dialog. Use this to get the dialog result.
        class DialogCallback
        {
            public:

                explicit                                DialogCallback( StoryDialog* p_dlg ) : _p_dlg( p_dlg ) {}

                virtual                                 ~DialogCallback() {}

                //! Called when the player has entered the dialog answer.
                virtual void                            onResult( StoryDialog* p_dlg ) = 0;

            protected:

                StoryDialog*                            _p_dlg;
        };

    public:

        //! Construct a dialog with given ID and callback. The callback can also be set later via 'setDialogCallback'.
        explicit                                    StoryDialog( unsigned int dialogID, DialogCallback* p_cb = NULL );

        virtual                                     ~StoryDialog();

        //! Add a new answer ( choice ) to dialog. Add all answers before calling 'createDialog'.
        void                                        addAnswer( unsigned int type, std::string answertext );

        //! Set the result callback
        void                                        setDialogCallback( DialogCallback* p_cb );

        //! Create a dialog with given title and text. 'answers' contains possible answers which the player can give. The order in vector is important!
        void                                        createDialog( const std::string& title, const std::string& text );

        //! Close the dialog before the player gives any answer. This is a forced dialog closing. The dialog object is not deleted, the caller must delete it!
        void                                        closeDialog();

        //! Show / hide the dialog.
        void                                        show( bool en );

        //! Get player choice. The number matches the order of answers used in openDialog.
        unsigned int                                getChoice();

        //! Get the num'th answer text. Use this only if there are any answers of type eString in dialog. 'num' must match the index in 'answers' used in openDialog.
        std::string                                 getText( unsigned int num );

        //! Get the dialog ID. The ID is used to identify the dialog on server.
        unsigned int                                getDialogID() { return _dialogID; }

    protected:

        //! Type for a single anwer
        typedef std::pair< unsigned int /* answer type */, std::string /* text */ >  Answer;

        //! List of possible answers
        std::vector< Answer >                       _answers;

        //! Unique ID used für identifying the dialog
        unsigned int                                _dialogID;

        //! Result callback
        DialogCallback*                             _p_cb;

        //! Main GUI window
        CEGUI::Window*                              _p_wnd;
};

} // namespace vrc

#endif // _VRC_STORYSYSTEM_H_
