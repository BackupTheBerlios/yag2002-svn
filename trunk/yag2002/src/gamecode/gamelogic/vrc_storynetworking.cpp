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
 # networking for story system; this is a server side object.
 #
 #   date of creation:  01/14/2008
 #
 #   author:            boto (botorabi at users.sourceforge.net)
 #
 #
 ################################################################*/

#include <vrc_main.h>
#include "vrc_storynetworking.h"
#include "../gamelogic/vrc_storysystem.h"
#include "../storage/vrc_storageserver.h"
#include "../storage/vrc_storageclient.h"
#include <RNReplicaNet/Inc/DataBlock_Function.h>

using namespace RNReplicaNet;

//! Separator used for formatting dialog choice and input text fields for transfer
#define DIALOG_STRING_SEPARATOR  "\250"

namespace vrc
{

StoryNetworking::StoryNetworking()
{
}

StoryNetworking::~StoryNetworking()
{
    // reset the networking object in story system, note that a server connection loss also causes the destruction of this object (via network device)
    if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Client )
        StorySystem::get()->setNetworking( NULL );
}

void StoryNetworking::PostObjectCreate()
{ // a new client has joined, this is called only on client

    // this function is called only on clients
    assert( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Client );

    // set the networking object of story system
    StorySystem::get()->setNetworking( this );
}

void StoryNetworking::sendEvent( const StoryEvent& event )
{
    // copy the event data into networking paket
    tEventData eventdata;
    memset( &eventdata, 0, sizeof( eventdata ) );
    eventdata._eventType      = event.getType();
    eventdata._filter         = event.getFilter();
    eventdata._sourceType     = event.getSourceType();
    eventdata._sourceID       = event.getSourceID();
    eventdata._targetType     = event.getTargetType();
    eventdata._targetID       = event.getTargetID();
    eventdata._sessionID      = event.getNetworkID();
    eventdata._uiParam[ 0 ]   = event.getUIParam1();
    eventdata._uiParam[ 1 ]   = event.getUIParam2();
    eventdata._fParam[ 0 ]    = event.getFParam1();
    eventdata._fParam[ 1 ]    = event.getFParam2();
    strncpy( eventdata._sParam, event.getSParam().c_str(), sizeof( eventdata._sParam ) - 2 );
    // terminate the string by two zeros at the end
    eventdata._sParam[ sizeof( eventdata._sParam ) - 1 ] = 0;
    eventdata._sParam[ sizeof( eventdata._sParam ) - 2 ] = 0;

    if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Client )
    {
        eventdata._sessionCookie = ( unsigned int )( yaf3d::NetworkDevice::get()->getSessionID() );
        eventdata._userID        = StorageClient::get()->getUserID();
    }

    // call the receive event on clients or server
    MASTER_FUNCTION_CALL( RPC_ReceiveEvent( eventdata ) );
}

void StoryNetworking::sendOpenDialog( const StoryDialogParams& params )
{ // this method is called only by server

    assert( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server );

    tDialogData dialog;
    dialog._sessionID = params._destNetworkID;
    dialog._id        = params._id;
    dialog._sourceID  = params._sourceID;

    memset( &dialog._p_title, 0, sizeof( dialog._p_title ) );
    memset( &dialog._p_text, 0, sizeof( dialog._p_text ) );
    memset( &dialog._p_choiceText, 0, sizeof( dialog._p_choiceText ) );
    memset( &dialog._p_inputFieldText, 0, sizeof( dialog._p_inputFieldText ) );
    memset( &dialog._p_inputFieldTextDefaults, 0, sizeof( dialog._p_inputFieldTextDefaults ) );

    // copy title and text
    memcpy( dialog._p_title, params._title.c_str(), std::min( params._title.length(), sizeof( dialog._p_title ) ) );
    memcpy( dialog._p_text, params._text.c_str(), std::min( params._text.length(), sizeof( dialog._p_text ) ) );

    // format the choices into proper string ready for transfer
    std::stringstream choicestr;
    unsigned int      choicedefault = 0;
    std::vector< StoryDialogParams::ChoiceInput >::const_iterator p_choice = params._choices.begin(), p_choiceEnd = params._choices.end();
    for ( unsigned int index = 0; p_choice != p_choiceEnd; ++p_choice, ++index )
    {
        choicestr << p_choice->first << DIALOG_STRING_SEPARATOR; // append a string separator sign
        if ( p_choice->second )
            choicedefault = index;
    }

    if ( choicestr.str().length() > sizeof( dialog._p_choiceText ) )
        log_error << "StoryNetworking: dialog choice strings excees the max length " << sizeof( dialog._p_choiceText ) << std::endl;

    memcpy( dialog._p_choiceText, choicestr.str().c_str(), std::min( choicestr.str().length(), sizeof( dialog._p_choiceText ) - 1 ) );
    dialog._p_choiceText[ sizeof( dialog._p_choiceText ) - 1 ] = 0;
    dialog._choiceDefault = choicedefault;

    // format the input text fields into proper string ready for transfer
    std::stringstream inputfieldstr;
    std::stringstream inputfielddeafultstr;
    std::vector< StoryDialogParams::TextInput >::const_iterator p_textinput = params._textFields.begin(), p_textinputEnd = params._textFields.end();
    for ( ; p_textinput != p_textinputEnd; ++p_textinput )
    {
        inputfieldstr << p_textinput->first << DIALOG_STRING_SEPARATOR;       // append a string separator sign
        inputfielddeafultstr << p_textinput->second << DIALOG_STRING_SEPARATOR;
    }

    if ( inputfieldstr.str().length() > sizeof( dialog._p_inputFieldText ) )
        log_error << "StoryNetworking: dialog input text strings excees the max length " << sizeof( dialog._p_inputFieldText ) << std::endl;

    if ( inputfielddeafultstr.str().length() > sizeof( dialog._p_inputFieldTextDefaults ) )
        log_error << "StoryNetworking: dialog input text value strings excees the max length " << sizeof( dialog._p_inputFieldTextDefaults ) << std::endl;

    memcpy( dialog._p_inputFieldText, inputfieldstr.str().c_str(), std::min( inputfieldstr.str().length(), sizeof( dialog._p_inputFieldText ) - 1 ) );
    memcpy( dialog._p_inputFieldTextDefaults, inputfielddeafultstr.str().c_str(), std::min( inputfielddeafultstr.str().length(), sizeof( dialog._p_inputFieldTextDefaults ) - 1 ) );
    dialog._p_inputFieldText[ sizeof( dialog._p_inputFieldText ) - 1 ] = 0;
    dialog._p_inputFieldTextDefaults[ sizeof( dialog._p_inputFieldTextDefaults ) - 1 ] = 0;

    // send out the network message to client now
    NOMINATED_REPLICAS_FUNCTION_CALL( 1, &params._destNetworkID, RPC_OpenDialog( dialog ) );
}

void StoryNetworking::sendDialogResults( const StoryDialogResults& results )
{ // this method is called on clients

    assert( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Client );

    tDialogResultsData dialogresults;

    dialogresults._sessionID   = results._destNetworkID;
    dialogresults._sourceID    = results._sourceID;
    dialogresults._id          = results._id;
    dialogresults._choice      = results._choice;
    dialogresults._dialogAbort = results._dialogAbort;

    memset( dialogresults._p_inputFieldValues, 0, sizeof( dialogresults._p_inputFieldValues ) );

    std::stringstream inputfieldvalues;
    std::vector< std::string >::const_iterator p_textinput = results._textFields.begin(), p_textinputEnd = results._textFields.end();
    for ( ; p_textinput != p_textinputEnd; ++p_textinput )
    {
        inputfieldvalues << *p_textinput << DIALOG_STRING_SEPARATOR;       // append a string separator sign
    }

    if ( inputfieldvalues.str().length() > sizeof( results._textFields ) )
        log_error << "StoryNetworking: dialog result's input text strings excees the max length " << sizeof( results._textFields ) << std::endl;

    memcpy( dialogresults._p_inputFieldValues, inputfieldvalues.str().c_str(), std::min( inputfieldvalues.str().length(), sizeof( dialogresults._p_inputFieldValues ) - 1 ) );
    dialogresults._p_inputFieldValues[ sizeof( dialogresults._p_inputFieldValues ) - 1 ] = 0;

    // send out the network message to server now
    MASTER_FUNCTION_CALL( RPC_ReceiveDialogResults( dialogresults ) );
}

void StoryNetworking::RPC_ReceiveEvent( tEventData eventdata )
{ // this is called on server or clients

    // let the storage server validate the client first!
    if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server )
    {
        if ( !StorageServer::get()->validateClient( eventdata._userID, eventdata._sessionCookie ) )
        {
            log_info << "*** StoryNetworking: receiving event from unauthorized client" << std::endl;
            return;
        }
    }

    // terminate the string by two zeros at the end
    eventdata._sParam[ sizeof( eventdata._sParam ) - 1 ] = 0;
    eventdata._sParam[ sizeof( eventdata._sParam ) - 2 ] = 0;
    // create the event
    StoryEvent event(
                        eventdata._eventType,
                        eventdata._sourceType,
                        eventdata._sourceID,
                        eventdata._targetType,
                        eventdata._targetID,
                        eventdata._filter,
                        eventdata._sessionID,
                        eventdata._uiParam[ 0 ],
                        eventdata._uiParam[ 1 ],
                        eventdata._fParam[ 0 ],
                        eventdata._fParam[ 1 ],
                        eventdata._sParam
                     );

    // feed in the event into story system
    if ( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server )
        StorySystem::get()->sendEvent( event );
    else
        StorySystem::get()->receiveEvent( event );
}

void StoryNetworking::RPC_OpenDialog( tDialogData dialog )
{ // this method is called on clients only

    assert( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Client );

    // dialogs open only on triggering client
    // note: actually, the server calls this rpc on the right client; this is only a further check.
    if ( dialog._sessionID != yaf3d::NetworkDevice::get()->getSessionID() )
    {
        log_warning << "StoryNetworking: rpc open dialog has been called on wrong client!" << std::endl;
        return;
    }

    // terminate the strings for being on safe side
    dialog._p_title[ sizeof( dialog._p_title ) - 1 ] = 0;
    dialog._p_text[ sizeof( dialog._p_text ) - 1 ] = 0;
    dialog._p_choiceText[ sizeof( dialog._p_choiceText ) - 1 ] = 0;
    dialog._p_inputFieldText[ sizeof( dialog._p_inputFieldText ) - 1 ] = 0;
    dialog._p_inputFieldTextDefaults[ sizeof( dialog._p_inputFieldTextDefaults ) - 1 ] = 0;

    StoryDialogParams params;
    params._id            = dialog._id;
    params._destNetworkID = dialog._sessionID;
    params._sourceID      = dialog._sourceID;
    params._title         = dialog._p_title;
    params._text          = dialog._p_text;

    // reconstruct the choices
    std::vector< std::string > choices;

    yaf3d::explode( dialog._p_choiceText, DIALOG_STRING_SEPARATOR, &choices );
    
    std::vector< std::string >::const_iterator p_choice = choices.begin(), p_choiceEnd = choices.end();
    for ( unsigned int index = 0; p_choice != p_choiceEnd; ++p_choice, ++index )
    {
        params._choices.push_back( std::make_pair( *p_choice, index == dialog._choiceDefault ) );
    }

    // reconstruct the input fields
    std::vector< std::string > inputfields;
    std::vector< std::string > inputfielddefaults;

    yaf3d::explode( dialog._p_inputFieldText, DIALOG_STRING_SEPARATOR, &inputfields );
    yaf3d::explode( dialog._p_inputFieldTextDefaults, DIALOG_STRING_SEPARATOR, &inputfielddefaults );

    if ( inputfields .size() != inputfielddefaults.size() )
        log_error << "StoryNetworking: received a dialog open request with different count of input field / default value counts " << inputfields .size() << ", " << inputfielddefaults .size() << std::endl;

    std::vector< std::string >::const_iterator p_inputfield = inputfields.begin(), p_inputfieldEnd = inputfields.end();
    std::vector< std::string >::const_iterator p_inputfielddefault = inputfielddefaults.begin(), p_inputfielddefaultEnd = inputfielddefaults.end();
    for ( ; ( p_inputfield != p_inputfieldEnd ) && ( p_inputfielddefault != p_inputfielddefaultEnd ); ++p_inputfield, ++p_inputfielddefault )
    {
        params._textFields.push_back( std::make_pair( *p_inputfield, *p_inputfielddefault ) );
    }

    // pass the dialog open request to story system
    StorySystem::get()->receiveOpenDialog( params );
}

void StoryNetworking::RPC_ReceiveDialogResults( tDialogResultsData dialogresults )
{ // this method is called on server

    assert( yaf3d::GameState::get()->getMode() == yaf3d::GameState::Server );

    StoryDialogResults results;

    results._destNetworkID = dialogresults._sessionID;
    results._sourceID      = dialogresults._sourceID;
    results._id            = dialogresults._id;
    results._choice        = dialogresults._choice;
    results._dialogAbort   = dialogresults._dialogAbort;

    std::vector< std::string > inputvalues;

    yaf3d::explode( dialogresults._p_inputFieldValues, DIALOG_STRING_SEPARATOR, &inputvalues );

    std::vector< std::string >::const_iterator p_inputfield = inputvalues.begin(), p_inputfieldEnd = inputvalues.end();
    for ( ; p_inputfield != p_inputfieldEnd; ++p_inputfield )
    {
        results._textFields.push_back( *p_inputfield );
    }

    // pass the dialog results to story system
    StorySystem::get()->receiveDialogResults( results );
}

} // namespace vrc
