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
 # a story builds an interaction task with the player.
 #  it can be e.g. a quest.
 #
 #   date of creation:  05/12/2007
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 ################################################################*/

#ifndef _VRC_STORY_H_
#define _VRC_STORY_H_

#include <vrc_main.h>
#include "vrc_storyevent.h"
#include <scripting/vrc_script.h>

namespace vrc
{

class StoryEngine;
class StoryBookLoader;

//! Class used for creating a story dialog.
class StoryDialogParams
{
    public:

                                                    StoryDialogParams() :
                                                     _destNetworkID( 0 ),
                                                     _sourceID( 0 ),
                                                     _id( 0 )
                                                    {}

        virtual                                    ~StoryDialogParams() {}

        //! Network session ID used for identifying the client where to open the dialog
        int                                         _destNetworkID;

        //! ID identifying the dialog trigger, e.g. an actor ID
        unsigned int                                _sourceID;

        //! Unique dialog ID
        unsigned int                                _id;

        //! Dialog title
        std::string                                 _title;

        //! Dialog Text
        std::string                                 _text;

        //! Type for choice input fields
        typedef std::pair< std::string /*choice text*/, bool /*selected*/ >    ChoiceInput;

        //! List of provided choices in a dialog
        std::vector< ChoiceInput >                  _choices;

        //! Type for text input fields
        typedef std::pair< std::string /*input text*/, std::string /*default value*/ >    TextInput;

        //! Text input fields
        std::vector< TextInput >                    _textFields;
};

//! Class used for delivering the dialog results
class StoryDialogResults
{
    public:

                                                    StoryDialogResults() :
                                                     _destNetworkID( 0 ),
                                                     _sourceID( 0 ),
                                                     _id( 0 ),
                                                     _dialogAbort( false ),
                                                     _choice( 0xff )
                                                    {}

        virtual                                    ~StoryDialogResults() {}

        //! Network session ID used for identifying the client sending the dialog results
        int                                         _destNetworkID;

        //! ID identifying the dialog trigger, e.g. an actor ID
        unsigned int                                _sourceID;

        //! Unique dialog ID
        unsigned int                                _id;

        //! Has the dialog been aborted?
        bool                                        _dialogAbort;

        //! Index of selected choice
        unsigned int                                _choice;

        //! Input fields' values
        std::vector< std::string >                  _textFields;
};


//! Story class responsible to process player events and progress the story state
class Story : public BaseScript< Story >, public yaf3d::RefCount< Story >
{
    public:

        //! Get the owner ID.
        unsigned int                                getOwnerID() const { return _ownerID; }

        //! Get story type
        const std::string&                          getType() const { return _type; }

        //! Get story name
        const std::string&                          getName() const { return _name; }

    protected:

        //! Construct a story with given type and name, this story will be a stock element. Concrete stories are
        //   created by cloning.
                                                    Story( const std::string type, const std::string name );

        virtual                                     ~Story();

        //! Setup the story given its script file
        bool                                        setup( const std::string& scriptfile );

        //! Create the dialog function bindings.
        void                                        createDialogFuntions();

        //! Create the inventory function bindings.
        void                                        createInventoryFunctions();

        //! Create a concrete story as clone with given owner ID and name
        yaf3d::SmartPtr< Story >                    clone( unsigned int ownerID, const std::string& name );

        //! Process the event in story by calling script function.
        void                                        processEvent( unsigned int storyTime, const StoryEvent& event );

        //! Process incoming dialog results.
        void                                        processDialogResutls( const StoryDialogResults& results );

        //! Update the story
        void                                        update( float deltaTime );

        // Exposed methods to scripting
        // ############################

        //! Method for outputting to log system
        void                                        llog( const Params& arguments, Params& /*returnvalues*/ );

        //! Begin a new story.
        void                                        lbeginStory( const Params& arguments, Params& /*returnvalues*/ );

        //! End an existing story.
        void                                        lcloseStory( const Params& arguments, Params& /*returnvalues*/ );

        //! Send an event to another story or actor.
        void                                        lsendEvent( const Params& arguments, Params& /*returnvalues*/ );

        /*! Open a dialog, it returns a unique dialog ID which is used in subsequent fucntions. The result of dialog is retrieved via the event funtion with identifying dialog ID.
            If a dialog is created, then it can be re-used several times by calling dialogOpen. If it is no longer needed then it should be destroyed via dialogDestroy.
        */
        void                                        ldialogCreate( const Params& /*arguments*/, Params& returnvalues );

        //! Destroy dialog freeing up resources. Returns false to script if the dialog handle does not exist.
        void                                        ldialogDestoy( const Params& arguments, Params& returnvalues );

        //! Set the dialog title.
        void                                        ldialogSetTitle( const Params& arguments, Params& /*returnvalues*/ );

        //! Set the dialog text.
        void                                        ldialogSetText( const Params& arguments, Params& /*returnvalues*/ );

        //! Add a choice field to dialog.
        void                                        ldialogAddChoice( const Params& arguments, Params& /*returnvalues*/ );

        //! Add a string input field to dialog.
        void                                        ldialogAddStringIput( const Params& arguments, Params& /*returnvalues*/ );

        //! Open the dialog. Returns false if the handle is invalid.
        void                                        ldialogOpen( const Params& arguments, Params& returnvalues );

        //! Given a dialog handle return true if the player aborted the dialog, otherwise return false. This script function can only be used in processDialogResults.
        void                                        ldialogGetAborted( const Params& arguments, Params& returnvalues );

        //! Given a dialog handle get the selected choice. This script function can only be used in processDialogResults.
        void                                        ldialogGetChoice( const Params& arguments, Params& returnvalues );

        //! Given a dialog handle and input field index return field's string value. This script function can only be used in processDialogResults.
        void                                        ldialogGetStringInput( const Params& arguments, Params& returnvalues );

        //! Does an inventory item exist?
        void                                        linvGetItemExist( const Params& arguments, Params& returnvalues );

        //! Get the value of a item property.
        void                                        linvGetItemProperty( const Params& arguments, Params& returnvalues );

        //! Set the value of a item property.
        void                                        linvSetItemProperty( const Params& arguments, Params& returnvalues );

        // ############################

        //! Story owner ID
        unsigned int                                _ownerID;

        //! Script file
        std::string                                 _scriptFile;

        //! Story type
        std::string                                 _type;

        //! Story name
        std::string                                 _name;

        //! The story engine
        StoryEngine*                                _p_storyEngine;

        //! Log object for story system output
        yaf3d::Log*                                 _p_log;

        //! This will be set if script errors detected, then event processing and updates are no longer done on the story scripts.
        bool                                        _freeze;

        //! Internal cache for dialogs
        std::map< int /* handle */, StoryDialogParams* > _dialogCache;

        //! Unique dialog handle used for creating dialogs.
        static unsigned int                         _dialogHandle;

        //! Count of created dialogs in all stories
        static unsigned int                         _dialogCount;

        //! Temporary pointer on dialog results, valid only during calling the script function processDialogResults
        const StoryDialogResults*                   _p_dialogResults;

   friend class StoryEngine;
   friend class StoryBookLoader;
   friend class yaf3d::SmartPtr< Story >;
};

//! Type for story smart pointer
typedef yaf3d::SmartPtr< Story >    StoryPtr;

}

#endif // _VRC_STORY_H_
