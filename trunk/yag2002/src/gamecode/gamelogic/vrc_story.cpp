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

#include <vrc_main.h>
#include "vrc_story.h"
#include "vrc_storysystem.h"
#include "vrc_storyengine.h"


#define SCRIPT_INTERFACE_NAME               "story"
#define FCN_INITIALIZE                      "initialize"
#define FCN_PROC_EVENT                      "processEvent"
#define FCN_PROC_DIALOG_RESULTS             "processDialogResults"
#define FCN_UPDATE                          "update"
#define EXPOSED_METHOD_LOG                  "log"
#define EXPOSED_METHOD_BEGIN_STORY          "begin"
#define EXPOSED_METHOD_CLOSE_STORY          "close"
#define EXPOSED_METHOD_SEND_EVENT           "sendEvent"

#define EXPOSED_METHOD_DLG_CREATE           "dialogCreate"
#define EXPOSED_METHOD_DLG_DESTROY          "dialogDestroy"
#define EXPOSED_METHOD_DLG_SET_TITLE        "dialogSetTitle"
#define EXPOSED_METHOD_DLG_SET_TEXT         "dialogSetText"
#define EXPOSED_METHOD_DLG_ADD_CHOICE       "dialogAddChoice"
#define EXPOSED_METHOD_DLG_ADD_STRING_INPUT "dialogAddStringInput"
#define EXPOSED_METHOD_DLG_OPEN             "dialogOpen"
#define EXPOSED_METHOD_DLG_GET_ABORTED      "dialogGetAborted"
#define EXPOSED_METHOD_DLG_GET_CHOICE       "dialogGetChoice"
#define EXPOSED_METHOD_DLG_GET_STR_INPUT    "dialogGetStringInput"


namespace vrc
{

//! Initialize story's static variables
unsigned int Story::_dialogHandle = 0;
unsigned int Story::_dialogCount  = 0;


Story::Story( const std::string type, const std::string name ) :
 _ownerID( 0 ),
 _type( type ),
 _name( name ),
 _p_storyEngine( NULL ),
 _p_log( NULL ),
 _freeze( false ),
 _p_dialogResults( NULL )
{
    _p_log = StorySystem::get()->getStoryLog();
    _p_storyEngine = StorySystem::get()->getStoryEngine();
}

Story::~Story()
{
    // clean up all created dialogs
    std::map< int /* handle */, StoryDialogParams* >::iterator p_beg = _dialogCache.begin(), p_end = _dialogCache.end();
    for ( ; p_beg != p_end; ++p_beg )
    {
        if ( _p_storyEngine )
            _p_storyEngine->deregisterDialog( p_beg->second->_id );

        delete p_beg->second;
    }
}

bool Story::setup( const std::string& scriptfile )
{
    _scriptFile = scriptfile;

    try
    {
        // load script file and scope all exposed methods
        loadScript( SCRIPT_INTERFACE_NAME, scriptfile, BaseScript< Story >::DEFAULT );

        Params arguments;
        Params returnsvalues;

        // expose method log having the pseudo-signatur: void log( string buffer )
        {
            std::string strbuf;
            arguments.add( strbuf );
            exposeMethod( EXPOSED_METHOD_LOG, &Story::llog, arguments, returnsvalues );
        }

        arguments.clear();
        returnsvalues.clear();

        // expose method for beginning stories having the pseudo-signatur: void begin( story type, story name, owner ID )
        {
            std::string  storytype;
            std::string  storyname;
            unsigned int ownerID = 0;
            arguments.add( storytype );
            arguments.add( storyname );
            arguments.add( ownerID );
            exposeMethod( EXPOSED_METHOD_BEGIN_STORY, &Story::lbeginStory, arguments, returnsvalues );
        }

        arguments.clear();
        returnsvalues.clear();

        // expose method for ending stories having the pseudo-signatur: void end( owner ID, story name )
        {
            unsigned int ownerID = 0;
            std::string  storyname;
            arguments.add( ownerID );
            arguments.add( storyname );
            exposeMethod( EXPOSED_METHOD_CLOSE_STORY, &Story::lcloseStory, arguments, returnsvalues );
        }

        arguments.clear();
        returnsvalues.clear();

        /* expose method for sending an event having the pseudo-signatur:
            void sendEvent(
                           EventType,   ( see StoryEvent )
                           EventFilter,
                           SourceType,
                           SourceID,
                           TargetType,
                           TargetID,
                           uiParam1, ( optional )
                           uiParam2, ( optional )
                           fParam1,  ( optional )
                           fParam2,  ( optional )
                           sParam1,  ( optional )
                           sParam2   ( optional )
                          )
        */
        {
            unsigned int ui  = 0;
            int          sid = 0;
            float        f   = 0.0f;
            std::string  s;
            arguments.add( ui );  // EventType
            arguments.add( ui );  // EventFilter
            arguments.add( ui );  // SourceType
            arguments.add( ui );  // SourceID
            arguments.add( ui );  // TargetType
            arguments.add( ui );  // TargetID
            arguments.add( sid ); // Network ID
            arguments.add( ui );  // uiParam1
            arguments.add( ui );  // uiParam2
            arguments.add( f );   // fParam1
            arguments.add( f );   // fParam2
            arguments.add( s );   // sParam1
            arguments.add( s );   // sParam2
            exposeMethod( EXPOSED_METHOD_SEND_EVENT, &Story::lsendEvent, arguments, returnsvalues );
        }

        // create the dialog's exposed function bindings
        createDialogFuntions();

        // execute the script after exposing methods
        execute();

        // call the initialize function
        arguments.clear();
        callScriptFunction( FCN_INITIALIZE, &arguments );
    }
    catch( const ScriptingException& e )
    {
        storylog_error << "error occured during setting up the story '" << scriptfile << "', " << e.what() << std::endl;
    }

    return true;
}

void Story::createDialogFuntions()
{
    Params arguments;
    Params returnsvalues;

    // expose method for creating a dialog having the pseudo-signatur: int handle dialogCreate()
    {
        int handle = 0;
        returnsvalues.add( handle );
        exposeMethod( EXPOSED_METHOD_DLG_CREATE, &Story::ldialogCreate, arguments, returnsvalues );
    }

    arguments.clear();
    returnsvalues.clear();

    // expose method for destroying a created dialog having the pseudo-signatur: bool dialogDestroy( int handle )
    {
        int          handle = 0;
        bool         ret    = true;
        arguments.add( handle );
        returnsvalues.add( ret );
        exposeMethod( EXPOSED_METHOD_DLG_DESTROY, &Story::ldialogDestoy, arguments, returnsvalues );
    }

    arguments.clear();
    returnsvalues.clear();

    // expose method for setting dialog title having the pseudo-signatur: void dialogSetTitle( int handle, string title )
    {
        int          handle = 0;
        std::string  title;
        arguments.add( handle );
        arguments.add( title );
        exposeMethod( EXPOSED_METHOD_DLG_SET_TITLE, &Story::ldialogSetTitle, arguments, returnsvalues );
    }

    arguments.clear();
    returnsvalues.clear();

    // expose method for setting dialog text having the pseudo-signatur: void dialogSetText( int handle, string title )
    {
        int          handle = 0;
        std::string  text;
        arguments.add( handle );
        arguments.add( text );
        exposeMethod( EXPOSED_METHOD_DLG_SET_TEXT, &Story::ldialogSetText, arguments, returnsvalues );
    }

    arguments.clear();
    returnsvalues.clear();

    // expose method for adding a choice to dialog having the pseudo-signatur: void dialogAddChoice( int handle, string choice text, bool set selection )
    {
        int          handle  = 0;
        std::string  choicetext;
        bool         selected = false;
        arguments.add( handle );
        arguments.add( choicetext );
        arguments.add( selected );
        exposeMethod( EXPOSED_METHOD_DLG_ADD_CHOICE, &Story::ldialogAddChoice, arguments, returnsvalues );
    }

    arguments.clear();
    returnsvalues.clear();

    // expose method for adding a choice to dialog having the pseudo-signatur: void dialogAddStringInput( int handle, string string field name, string default value )
    {
        int          handle = 0;
        std::string  inputtext;
        std::string  defaultvalue;
        arguments.add( handle );
        arguments.add( inputtext );
        arguments.add( defaultvalue );
        exposeMethod( EXPOSED_METHOD_DLG_ADD_STRING_INPUT, &Story::ldialogAddStringIput, arguments, returnsvalues );
    }

    arguments.clear();
    returnsvalues.clear();

    // expose method for opening a created dialog having the pseudo-signatur: bool dialogOpen( int handle, int network session ID, uint source ID )
    {
        int          handle    = 0;
        int          networkID = 0;
        unsigned int sourceID  = 0;
        bool         ret       = true;
        arguments.add( handle );
        arguments.add( networkID );
        arguments.add( sourceID );
        returnsvalues.add( ret );
        exposeMethod( EXPOSED_METHOD_DLG_OPEN, &Story::ldialogOpen, arguments, returnsvalues );
    }

    // exposed methods valid during executing processDialogResults function
    // ######
    arguments.clear();
    returnsvalues.clear();

    // expose method for checking if the player aborted the dialog, pseudo-signatur: bool dialogGetAborted( int handle )
    {
        int          handle = 0;
        bool         ret    = true;
        arguments.add( handle );
        returnsvalues.add( ret );
        exposeMethod( EXPOSED_METHOD_DLG_GET_ABORTED, &Story::ldialogGetAborted, arguments, returnsvalues );
    }

    arguments.clear();
    returnsvalues.clear();

    // expose method for getting the selected choice beginning at 1, pseudo-signatur: unsigned int dialogGetChoice( int handle ), if 0 is returned then it means no choice made!
    {
        int          handle = 0;
        unsigned int choice = 0;
        arguments.add( handle );
        returnsvalues.add( choice );
        exposeMethod( EXPOSED_METHOD_DLG_GET_CHOICE, &Story::ldialogGetChoice, arguments, returnsvalues );
    }

    arguments.clear();
    returnsvalues.clear();

    // expose method for getting the n'th input field value, pseudo-signatur: bool dialogGetStringInput( int handle, unsigned int index ), index must begin at 1
    {
        int          handle = 0;
        unsigned int index  = 0;
        std::string  ret;
        arguments.add( handle );
        arguments.add( index );
        returnsvalues.add( ret );
        exposeMethod( EXPOSED_METHOD_DLG_GET_STR_INPUT, &Story::ldialogGetStringInput, arguments, returnsvalues );
    }
    // ######
}

StoryPtr Story::clone( unsigned int ownerID, const std::string& name )
{
    assert( ownerID && "invalid owner ID!" );

    StoryPtr story     = new Story( getType(), name );
    story->_ownerID    = ownerID;
    // load the script
    story->setup( _scriptFile );

    return story;
}

void Story::processEvent( unsigned int storyTime, const StoryEvent& event )
{
    if ( _freeze )
        return;

    Params arguments;

    // fill in the event parameters
    arguments.add( storyTime );
    arguments.add( event.getType() );
    arguments.add( event.getSourceType() );
    arguments.add( event.getSourceID() );
    arguments.add( event.getTargetType() );
    arguments.add( event.getTargetID() );
    arguments.add( event.getNetworkID() );
    arguments.add( event.getUIParam1() );
    arguments.add( event.getUIParam2() );
    arguments.add( event.getFParam1() );
    arguments.add( event.getFParam2() );
    arguments.add( event.getSParam() );
    
    try
    {
        // call the function, we expect no return value
        callScriptFunction( FCN_PROC_EVENT, &arguments );
    }
    catch ( const ScriptingException& e )
    {
        storylog_error << getName() << ": error on calling script function " << FCN_PROC_EVENT << std::endl;
        storylog_error << "reason : " << e.what() << std::endl;
    }
}

void Story::processDialogResutls( const StoryDialogResults& results )
{
    if ( _freeze )
        return;

    // find the handle of given results._id and pass it to function
    int handle = 0;
    std::map< int /* handle */, StoryDialogParams* >::iterator p_params = _dialogCache.begin(), p_end = _dialogCache.end();
    for ( ; p_params != p_end; ++p_params )
    {
        if ( p_params->second->_id == results._id )
        {
            handle = static_cast< int >( p_params->first );
            break;
        }
    }

    if ( !handle )
    {
        storylog_error << getName() << ": processDialogResutls cannot find the dialog handle " << std::endl;
        return;
    }

    // set the update parameter
    Params arguments;
    arguments.add( handle );

    try
    {
        // set the temporary results pointer which is only valid during the following script function, it is used to make the results available in exposed script functions.
        _p_dialogResults = &results;

        // call the function, we expect no return value
        callScriptFunction( FCN_PROC_DIALOG_RESULTS, &arguments );
    }
    catch ( const ScriptingException& e )
    {
        storylog_error << getName() << ": error on calling script function '" << FCN_PROC_DIALOG_RESULTS << "', story will be freezed!" << std::endl;
        storylog_error << "reason : " << e.what() << std::endl;

        _freeze = true;
    }

    _p_dialogResults = NULL;
}

void Story::update( float deltaTime )
{
    if ( _freeze )
        return;

    // set the update parameter
    Params arguments;
    arguments.add( deltaTime );
    
    try
    {
        // call the function, we expect no return value
        callScriptFunction( FCN_UPDATE, &arguments );
    }
    catch ( const ScriptingException& e )
    {
        storylog_error << getName() << ": error on calling script function '" << FCN_UPDATE << "', story will be freezed!" << std::endl;
        storylog_error << "reason : " << e.what() << std::endl;

        _freeze = true;
    }
}

void Story::llog( const Params& arguments, Params& /*returnvalues*/ )
{
    if ( arguments.getNumPassedParameters() )
    {
        //! TODO: get the lua function name
        //lua_Debug dbg;
        //dbg.name = NULL;
        //lua_getinfo( _p_state, ">S", &dbg );
        //std::string fcnname = dbg.name ? dbg.name : "?fcn";

        std::string str = GET_SCRIPT_PARAMVALUE( arguments, 0, std::string );
        _p_log->enableSeverityLevelPrinting( false );
        storylog_info << "[story]   " << getName() << ": " << str << std::endl;
        _p_log->enableSeverityLevelPrinting( true );
    }
}

void Story::lbeginStory( const Params& arguments, Params& /*returnvalues*/ )
{
    if ( arguments.getNumPassedParameters() < 3 )
    {
        storylog_error << getName() << ": invalid call of function '" << EXPOSED_METHOD_BEGIN_STORY << "' without story type, name, and owner ID arguments!" << std::endl;
        return;
    }

    std::string  storytype = GET_SCRIPT_PARAMVALUE( arguments, 0, std::string );
    std::string  storyname = GET_SCRIPT_PARAMVALUE( arguments, 1, std::string );
    unsigned int ownerID   = GET_SCRIPT_PARAMVALUE( arguments, 2, unsigned int );

    if ( !_p_storyEngine->beginStory( storytype, storyname, ownerID ) )
        storylog_error << getName() << ": could not create story '" << storyname << "' for owner: '" << ownerID << "'" << std::endl;
    else
        storylog_verbose << getName() << ": created story '" << storyname << "' for owner: '" << ownerID << "'" << std::endl;
}

void Story::lcloseStory( const Params& arguments, Params& /*returnvalues*/ )
{
    if ( arguments.getNumPassedParameters() < 2 )
    {
        storylog_error << getName() << ": invalid call of function '" << EXPOSED_METHOD_CLOSE_STORY << "' without owner ID and story name arguments!" << std::endl;
        return;
    }

    unsigned int ownerID   = GET_SCRIPT_PARAMVALUE( arguments, 0, unsigned int );
    std::string  storyname = GET_SCRIPT_PARAMVALUE( arguments, 1, std::string );

    if ( !_p_storyEngine->closeStory( ownerID, storyname ) )
        storylog_error << getName() << ": could not end story '" << storyname << "' for owner: '" << ownerID << "'" << std::endl;
    else
        storylog_verbose << getName() << ": ended story '" << storyname << "' of owner: '" << ownerID << "'" << std::endl;
}

void Story::lsendEvent( const Params& arguments, Params& /*returnvalues*/ )
{
    unsigned int numparams = arguments.getNumPassedParameters();
    if ( numparams < 6 )
    {
        storylog_error << getName() << ": invalid call of function '" << EXPOSED_METHOD_SEND_EVENT << "' it must be called with at least 6 parameters!" << std::endl;
        return;
    }

    // mandatory params
    unsigned int eventType  = GET_SCRIPT_PARAMVALUE( arguments, 0, unsigned int );
    unsigned int filter     = GET_SCRIPT_PARAMVALUE( arguments, 1, unsigned int );
    unsigned int sourceType = GET_SCRIPT_PARAMVALUE( arguments, 2, unsigned int );
    unsigned int sourceID   = GET_SCRIPT_PARAMVALUE( arguments, 3, unsigned int );
    unsigned int targetType = GET_SCRIPT_PARAMVALUE( arguments, 4, unsigned int );
    unsigned int targetID   = GET_SCRIPT_PARAMVALUE( arguments, 5, unsigned int );
    int          networkID  = GET_SCRIPT_PARAMVALUE( arguments, 6, int );

    // optional parameters
    unsigned int uiParam1 = 0;
    unsigned int uiParam2 = 0;
    float        fParam1  = 0.0f;
    float        fParam2  = 0.0f;
    std::string  sParam;
    if ( numparams > 7 )
        uiParam1 = GET_SCRIPT_PARAMVALUE( arguments, 7, unsigned int );
    if ( numparams > 8 )
        uiParam2 = GET_SCRIPT_PARAMVALUE( arguments, 8, unsigned int );
    if ( numparams > 9 )
        fParam1 = GET_SCRIPT_PARAMVALUE( arguments, 9, float );
    if ( numparams > 10 )
        fParam2 = GET_SCRIPT_PARAMVALUE( arguments, 10, float );
    if ( numparams > 11 )
        sParam = GET_SCRIPT_PARAMVALUE( arguments, 11, std::string );

    // enqueue the event, it will be sent out on next update
    StoryEvent event( eventType, sourceType, sourceID, targetType, targetID, filter, networkID, uiParam1, uiParam2, fParam1, fParam2, sParam );
    _p_storyEngine->enqueueEvent( event );
}

void Story::ldialogCreate( const Params& /*arguments*/, Params& returnvalues )
{
    // create new handle
    _dialogHandle++;

    // increase the dialog count used for statistics
    _dialogCount++;

    // create helper object for a new dialog parameter
    StoryDialogParams* p_dialogParams = new StoryDialogParams;

    // request the story engine for a new dialog registration
    p_dialogParams->_id = _p_storyEngine->registerDialog( this );

    _dialogCache[ _dialogHandle ] = p_dialogParams;

    int dialoghandle = static_cast< int >( _dialogHandle );
    SET_SCRIPT_PARAMVALUE( returnvalues, 0, int, dialoghandle );

    storylog_debug << "story " << getName() << ": create dialog " << _dialogHandle << ", total dialogs " << _dialogCount << std::endl;
}

void Story::ldialogDestoy( const Params& arguments, Params& returnvalues )
{
    // get the dialog handle
    int dialoghandle = GET_SCRIPT_PARAMVALUE( arguments, 0, int );

    std::map< int, StoryDialogParams* >::iterator p_dialog = _dialogCache.find( dialoghandle );

    // does the dialog handle exist?
    if ( p_dialog == _dialogCache.end() )
    {
        bool ret = false;
        SET_SCRIPT_PARAMVALUE( returnvalues, 0, bool, ret );
        storylog_warning << "story " << getName() << ": tries to destroy a non-existing dialog, handle " << dialoghandle << std::endl;
        return;
    }

    // remove the dialog from story engine
    _p_storyEngine->deregisterDialog( p_dialog->second->_id );

    // remove the dialog from internal map
    delete p_dialog->second;
    _dialogCache.erase( p_dialog );

    // decrease the dialog count used for statistics
    _dialogCount--;

    bool ret = true;
    SET_SCRIPT_PARAMVALUE( returnvalues, 0, bool, ret );

    storylog_debug << "story " << getName() << ": destroy dialog " << _dialogHandle << ", total dialogs " << _dialogCount << std::endl;
}

void Story::ldialogSetTitle( const Params& arguments, Params& /*returnvalues*/ )
{
    // get the dialog handle
    int dialoghandle = GET_SCRIPT_PARAMVALUE( arguments, 0, int );

    std::map< int, StoryDialogParams* >::iterator p_dialog = _dialogCache.find( dialoghandle );

    // does the dialog handle exist?
    if ( p_dialog == _dialogCache.end() )
    {
        storylog_error << "story " << getName() << ": tries to set title of a non-existing dialog, handle " << dialoghandle << std::endl;
        return;
    }

    // set the dialog title
    std::string title = GET_SCRIPT_PARAMVALUE( arguments, 1, std::string );
    p_dialog->second->_title = title;
}

void Story::ldialogSetText( const Params& arguments, Params& /*returnvalues*/ )
{
    // get the dialog handle
    int dialoghandle = GET_SCRIPT_PARAMVALUE( arguments, 0, int );

    std::map< int, StoryDialogParams* >::iterator p_dialog = _dialogCache.find( dialoghandle );

    // does the dialog handle exist?
    if ( p_dialog == _dialogCache.end() )
    {
        storylog_error << "story " << getName() << ": tries to set text of a non-existing dialog, handle " << dialoghandle << std::endl;
        return;
    }

    // set the dialog title
    std::string text = GET_SCRIPT_PARAMVALUE( arguments, 1, std::string );
    p_dialog->second->_text = text;
}

void Story::ldialogAddChoice( const Params& arguments, Params& /*returnvalues*/ )
{
    // get the dialog handle
    int dialoghandle = GET_SCRIPT_PARAMVALUE( arguments, 0, int );

    std::map< int, StoryDialogParams* >::iterator p_dialog = _dialogCache.find( dialoghandle );

    // does the dialog handle exist?
    if ( p_dialog == _dialogCache.end() )
    {
        storylog_error << "story " << getName() << ": tries to add choice to a non-existing dialog, handle " << dialoghandle << std::endl;
        return;
    }

    std::string choicetext = GET_SCRIPT_PARAMVALUE( arguments, 1, std::string );
    bool        selected   = GET_SCRIPT_PARAMVALUE( arguments, 2, bool );

    StoryDialogParams::ChoiceInput choice = std::make_pair( choicetext, selected );
    p_dialog->second->_choices.push_back( choice );
}

void Story::ldialogAddStringIput( const Params& arguments, Params& /*returnvalues*/ )
{
    // get the dialog handle
    int dialoghandle = GET_SCRIPT_PARAMVALUE( arguments, 0, int );

    std::map< int, StoryDialogParams* >::iterator p_dialog = _dialogCache.find( dialoghandle );

    // does the dialog handle exist?
    if ( p_dialog == _dialogCache.end() )
    {
        storylog_error << "story " << getName() << ": tries to add an input text to a non-existing dialog, handle " << dialoghandle << std::endl;
        return;
    }

    std::string inputtext    = GET_SCRIPT_PARAMVALUE( arguments, 1, std::string );
    std::string defaultvalue = GET_SCRIPT_PARAMVALUE( arguments, 2, std::string );

    StoryDialogParams::TextInput choice = std::make_pair( inputtext, defaultvalue );
    p_dialog->second->_textFields.push_back( choice );
}

void Story::ldialogOpen( const Params& arguments, Params& returnvalues )
{
    // get the dialog handle
    int          dialoghandle = GET_SCRIPT_PARAMVALUE( arguments, 0, int );
    unsigned int networkID    = GET_SCRIPT_PARAMVALUE( arguments, 1, int );
    unsigned int sourceID     = GET_SCRIPT_PARAMVALUE( arguments, 2, unsigned int );

    std::map< int, StoryDialogParams* >::iterator p_dialog = _dialogCache.find( dialoghandle );

    // does the dialog handle exist?
    if ( p_dialog == _dialogCache.end() )
    {
        bool ret = false;
        SET_SCRIPT_PARAMVALUE( returnvalues, 0, bool, ret );
        storylog_warning << "story " << getName() << ": tries to open a non-existing dialog, handle " << dialoghandle << std::endl;
        return;
    }

    // set the source and networking ID and use the story system for opening the dialog
    p_dialog->second->_destNetworkID = networkID;
    p_dialog->second->_sourceID      = sourceID;

    _p_storyEngine->openDialog( *p_dialog->second );

    bool ret = true;
    SET_SCRIPT_PARAMVALUE( returnvalues, 0, bool, ret );
    storylog_debug << "story " << getName() << ": open dialog, handle " << dialoghandle << std::endl;
}

void Story::ldialogGetAborted( const Params& arguments, Params& returnvalues )
{
    if ( !_p_dialogResults )
    {
        storylog_error << getName() << ": trying to call script function " EXPOSED_METHOD_DLG_GET_ABORTED " in an invalid context!" << std::endl;
        storylog_error << "   this function can be used only in " FCN_PROC_DIALOG_RESULTS << std::endl;
        bool ret = true;
        SET_SCRIPT_PARAMVALUE( returnvalues, 0, bool, ret );
        return;
    }

    // get the dialog handle
    int dialoghandle = GET_SCRIPT_PARAMVALUE( arguments, 0, int );

    std::map< int, StoryDialogParams* >::iterator p_dialog = _dialogCache.find( dialoghandle );

    // does the dialog handle exist?
    if ( p_dialog == _dialogCache.end() )
    {
        bool ret = false;
        SET_SCRIPT_PARAMVALUE( returnvalues, 0, bool, ret );
        storylog_warning << "story " << getName() << ": tries to get abort state of a non-existing dialog, handle " << dialoghandle << std::endl;
        return;
    }

    SET_SCRIPT_PARAMVALUE( returnvalues, 0, bool, _p_dialogResults->_dialogAbort );
}

void Story::ldialogGetChoice( const Params& arguments, Params& returnvalues )
{
    if ( !_p_dialogResults )
    {
        storylog_error << getName() << ": trying to call script function " EXPOSED_METHOD_DLG_GET_CHOICE " in an invalid context!" << std::endl;
        storylog_error << "   this function can be used only in " FCN_PROC_DIALOG_RESULTS << std::endl;
        unsigned int ret = 0;
        SET_SCRIPT_PARAMVALUE( returnvalues, 0, unsigned int, ret );
        return;
    }

    // get the dialog handle
    int dialoghandle = GET_SCRIPT_PARAMVALUE( arguments, 0, int );

    std::map< int, StoryDialogParams* >::iterator p_dialog = _dialogCache.find( dialoghandle );

    // does the dialog handle exist?
    if ( p_dialog == _dialogCache.end() )
    {
        unsigned int ret = 0;
        SET_SCRIPT_PARAMVALUE( returnvalues, 0, unsigned int, ret );
        storylog_warning << "story " << getName() << ": tries to get choice selection of a non-existing dialog, handle " << dialoghandle << std::endl;
        return;
    }

    SET_SCRIPT_PARAMVALUE( returnvalues, 0, unsigned int, _p_dialogResults->_choice );
}

void Story::ldialogGetStringInput( const Params& arguments, Params& returnvalues )
{
    if ( !_p_dialogResults )
    {
        storylog_error << getName() << ": trying to call script function " EXPOSED_METHOD_DLG_GET_STR_INPUT " in an invalid context!" << std::endl;
        storylog_error << "   this function can be used only in " FCN_PROC_DIALOG_RESULTS << std::endl;
        std::string ret( "" );
        SET_SCRIPT_PARAMVALUE( returnvalues, 0, std::string, ret );
        return;
    }

    // get the dialog handle
    int dialoghandle = GET_SCRIPT_PARAMVALUE( arguments, 0, int );

    std::map< int, StoryDialogParams* >::iterator p_dialog = _dialogCache.find( dialoghandle );

    // does the dialog handle exist?
    if ( p_dialog == _dialogCache.end() )
    {
        std::string ret( "" );
        SET_SCRIPT_PARAMVALUE( returnvalues, 0, std::string, ret );
        storylog_warning << "story " << getName() << ": tries to get string input value of a non-existing dialog, handle " << dialoghandle << std::endl;
        return;
    }

    unsigned int fieldindex = GET_SCRIPT_PARAMVALUE( arguments, 1, unsigned int );
    if ( !fieldindex || ( fieldindex > _p_dialogResults->_textFields.size() ) )
    {
        std::string ret( "" );
        SET_SCRIPT_PARAMVALUE( returnvalues, 0, std::string, ret );
        storylog_warning << "story " << getName() << ": tries to get get string input value of a non-existing field index (note: indices begin at 1 not 0!), handle " << fieldindex << std::endl;
        return;
    }

    std::string ret( _p_dialogResults->_textFields[ fieldindex - 1 ] );
    SET_SCRIPT_PARAMVALUE( returnvalues, 0, std::string, ret );
}

} // namespace vrc
