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

_RO_DEF_ALLOCATEFUNC(StoryNetworking)
class _MAKE_BASE(StoryNetworking);
class _RO_StoryNetworking : public RNReplicaNet::ReplicaObject
{
	_RO_STD_FRAMEWORK(StoryNetworking)
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_ReceiveEvent)
	void Call_RPC_ReceiveEvent(tEventData);
	typedef void(tDBFV_RPC_ReceiveEvent)(tEventData);
	_RO_DEF_REGISTERDATABLOCKS(StoryNetworking)
};
} // namespace vrc
#endif
