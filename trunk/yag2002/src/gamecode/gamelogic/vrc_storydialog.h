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

class StoryDialogParams;
class StoryDialogResults;

//! Class used on client for handling player's dialog-based interaction with story system
class StoryDialog
{
    public:

        //! Answer types
        enum AnswerType
        {
            //! The input is a choice.
            eChoice   = 0x01,
            //! The input is a string.
            eString   = 0x02
        };

        //! Callback class for notifying about dialog results
        class DialogCallback
        {
            public:

                                                        DialogCallback() {}

                virtual                                 ~DialogCallback() {}

                //! Called when the player has closed the dialog.
                virtual void                            onDialogResult( const StoryDialogResults& results ) = 0;
        };

    public:

        //! Construct a dialog with given ID and callback. The callback can also be set later via 'setDialogCallback'.
        explicit                                    StoryDialog( DialogCallback* p_cb = NULL );

        //! Create a dialog out of given dialog parameters and shows it up.
        explicit                                    StoryDialog( const StoryDialogParams& params, DialogCallback* p_cb = NULL );

        virtual                                     ~StoryDialog();

        //! Add a new input field of type choice to dialog. Add all fields before calling 'createDialog'.
        void                                        addChoice( std::string fieldname, bool defaultselection = false );

        //! Add a new input field of type input text to dialog. Add all fields before calling 'createDialog'.
        void                                        addInputText( std::string fieldname, const std::string& defaultvalue );

        //! Set the network ID identifying the client.
        void                                        setNetworkID( unsigned int networkID ) { _networkID = networkID; }

        //! Get the network ID.
        unsigned int                                getNetworkID() const { return _networkID; }

        //! Set the source ID identifying the source of dialog trigger, e.g. an actor.
        void                                        setSourceID( unsigned int sourceID ) { _sourceID = sourceID; }

        //! Get the source ID.
        unsigned int                                getSourceID() const { return _sourceID; }

        //! Set the result callback
        void                                        setDialogCallback( DialogCallback* p_cb );

        //! Create a dialog with given title and text. Return false if something goes wrong.
        bool                                        createDialog( unsigned int dialogID, const std::string& title, const std::string& text );

        //! Close the dialog before the player gives any answer. This is a forced dialog closing. The dialog object is not deleted, the caller must delete it!
        void                                        closeDialog();

        //! Remove all input field entries and close dialog. After this call a new dialog content can be created by adding input fields and 'createDialog'.
        void                                        clearDialog();

        //! Show / hide the dialog.
        void                                        show( bool en );

        //! Get the dialog ID. The ID is used to identify the dialog on server.
        unsigned int                                getDialogID() const { return _dialogID; }

        //! NOTE: Following methods can only be used after creation of dialog via method 'createDialog'!
        // ###########

        //! Get 1-based player choice. The number matches the order of answers used in openDialog. A zero means no selection done.
        unsigned int                                getChoice();

        //! Set the choice selection. 'index' begins at 1 and is maximal the count of added choices of type eChoice.
        void                                        setChoice( unsigned int index );

        //! Get the num'th input text. Use this only if there are any input fields of type eString in dialog. 'index' must begins with 1.
        std::string                                 getText( unsigned int index );

        //! Set the value of input text text field with given index. The index begins at 1 and and is maximal the count of added choices of type eString.
        void                                        setText( unsigned int index, const std::string& text );

        //! Get count of text input fields.
        unsigned int                                getCountTextInput() const;

        // ###########

        //! Has the abort button been clicked?
        bool                                        clickedAbort() const;

    protected:

        //! Callback used when clicking on Ok button
        bool                                        onClickedOk( const CEGUI::EventArgs& arg );

        //! Callback used when clicking on Abort button
        bool                                        onClickedAbort( const CEGUI::EventArgs& arg );

        //! Type for a single input field
        typedef struct InputField
        {
            unsigned int  _fieldType; // choice or text input
            std::string   _fieldName;
            std::string   _defaultValue;
            bool          _defaultSelection;

        } InputField;

        //! Unique ID used für identifying the dialog
        unsigned int                                _dialogID;

        //! Network ID
        unsigned int                                _networkID;

        //! Source ID of triggering actor
        unsigned int                                _sourceID;

        //! List of possible input fields
        std::vector< InputField >                   _inputs;

        //! Result callback
        DialogCallback*                             _p_cb;

        //! Main GUI window
        CEGUI::Window*                              _p_wnd;

        //! Indicated that player clicked the abort button
        bool                                        _abortClicked;

        //! Radio buttons for choice selection
        std::vector< CEGUI::RadioButton* >          _choices;

        //! Input text fields
        std::vector< CEGUI::Editbox* >              _textFields;
};

} // namespace vrc

#endif // _VRC_STORYSYSTEM_H_
