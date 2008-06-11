/* Auto-generated with RNROLCompiler V2.7.0.0 don't edit */
#ifndef ___RO_StoryNetworking_h__
#define ___RO_StoryNetworking_h__
#include "RNReplicaNet/Inc/ReplicaObject.h"
#include "RNReplicaNet/Inc/Define_RO.h"
namespace vrc
{

//! Event data struct
typedef struct _tEventData
{
    //! Session cookie
    unsigned int    _sessionCookie;

    //! Session ID
    unsigned int    _sessionID;

    //! Unique user ID
    unsigned int    _userID;

    //! Source type triggering the event ( e.g. player )
    unsigned int    _sourceType;

    //! Event's source instance, this specefies an instance of a source type ( ( e.g. player ID )
    unsigned int    _sourceID;

    //! Event target, the event target specifies the target type.
    unsigned int    _targetType;

    //! Event's target instance, this specefies an instance of a target type
    unsigned int    _targetID;

    //! Event type
    unsigned int    _eventType;

    //! Event filter
    unsigned int    _filter;

    //! Event's generic parameters
    unsigned int    _uiParam[ 2 ];
    float           _fParam[ 2 ];
    char            _sParam[ 128 ];

} tEventData;

//! Dialog data struct
typedef struct _tDialogData
{
    //! Session ID of sending client
    int             _sessionID;

    //! ID identifying the dialog trigger, e.g. an actor ID
    unsigned int    _sourceID;

    //! Unique dialog ID
    unsigned int    _id;

    //! Index of default choise
    unsigned int    _choiceDefault;

    //! Dialog title
    char            _p_title[ 64 ];

    //! Dialog text
    char            _p_text[ 512 ];

    //! Choice strings
    char            _p_choiceText[ 128 ];

    //! Input field strings
    char            _p_inputFieldText[ 128 ];

    //! Input fields' default values
    char            _p_inputFieldTextDefaults[ 64 ];

} tDialogData;

//! Dialog resutls data struct
typedef struct _tDialogResultsData
{
    //! Session ID of sending client
    int             _sessionID;

    //! ID identifying the dialog trigger, e.g. an actor ID
    unsigned int    _sourceID;

    //! Unique dialog ID
    unsigned int    _id;

    //! Has the dialog been aborted?
    bool            _dialogAbort;

    //! Index of selected choice
    unsigned int    _choice;

    //! Input fields' values
    char            _p_inputFieldValues[ 64 ];

} tDialogResultsData;

_RO_DEF_ALLOCATEFUNC(StoryNetworking)
class _MAKE_BASE(StoryNetworking);
class _RO_StoryNetworking : public RNReplicaNet::ReplicaObject
{
	_RO_STD_FRAMEWORK(StoryNetworking)
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_ReceiveEvent)
	void Call_RPC_ReceiveEvent(tEventData);
	typedef void(tDBFV_RPC_ReceiveEvent)(tEventData);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_OpenDialog)
	void Call_RPC_OpenDialog(tDialogData);
	typedef void(tDBFV_RPC_OpenDialog)(tDialogData);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_ReceiveDialogResults)
	void Call_RPC_ReceiveDialogResults(tDialogResultsData);
	typedef void(tDBFV_RPC_ReceiveDialogResults)(tDialogResultsData);
	_RO_DEF_REGISTERDATABLOCKS(StoryNetworking)
};
} // namespace vrc
#endif
