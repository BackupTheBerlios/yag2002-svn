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


#define SCRIPT_INTERFACE_NAME           "story"
#define FCN_INITIALIZE                  "initialize"
#define FCN_PROC_EVENT                  "processEvent"
#define FCN_UPDATE                      "update"
#define EXPOSED_METHOD_LOG              "log"
#define EXPOSED_METHOD_BEGIN_STORY      "begin"
#define EXPOSED_METHOD_CLOSE_STORY      "close"


namespace vrc
{

Story::Story( const std::string type, const std::string name ) :
 _ownerID( 0 ),
 _type( type ),
 _name( name ),
 _p_storyEngine( NULL ),
 _p_log( NULL ),
 _freeze( false )
{
    _p_log = StorySystem::get()->getStoryLog();
    _p_storyEngine = StorySystem::get()->getStoryEngine();
}

Story::~Story()
{
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

        // expose method log having the pseudo-signatur: void log( char* string )
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
    arguments.add( event.getFParam1() );
    arguments.add( event.getFParam2() );
    arguments.add( event.getSParam1() );
    arguments.add( event.getSParam2() );
    
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
    std::string str = GET_SCRIPT_PARAMVALUE( arguments, 0, std::string );
    if ( arguments.size() )
    {
        //! TODO: get the lua function name
        //lua_Debug dbg;
        //dbg.name = NULL;
        //lua_getinfo( _p_state, ">S", &dbg );
        //std::string fcnname = dbg.name ? dbg.name : "?fcn";

        _p_log->enableSeverityLevelPrinting( false );
        storylog_info << "[story]   " << getName() << ": " << str << std::endl;
        _p_log->enableSeverityLevelPrinting( true );
    }
}

void Story::lbeginStory( const Params& arguments, Params& /*returnvalues*/ )
{
    if ( arguments.size() < 3 )
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
    if ( arguments.size() < 2 )
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

} // namespace vrc
