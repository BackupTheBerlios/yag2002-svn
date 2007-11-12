/* Auto-generated with RNROLCompiler V2.7.0.0 don't edit */
#ifndef ___RO_ObjectNetworking_h__
#define ___RO_ObjectNetworking_h__
#include "RNReplicaNet/Inc/ReplicaObject.h"
#include "RNReplicaNet/Inc/Define_RO.h"
namespace vrc
{

//! Action data struct
typedef struct _tActionData
{
    //! Session cookie
    unsigned int    _sessionCookie;

    //! Unique user ID
    unsigned int    _userID;

    //! Action type
    unsigned int    _actionType;

    //! Action result set by server
    int             _actionResult;

    //! Generic parameter filed used for different action types
    unsigned int    _paramUint[ 10 ];

    //! Generic parameter filed used for different action types
    float           _paramFloat[ 10 ];

    //! Generic parameter filed used for different action types
    char            _p_paramString[ 128 ];

} tActionData;


_RO_DEF_ALLOCATEFUNC(ObjectNetworking)
class _MAKE_BASE(ObjectNetworking);
class _RO_ObjectNetworking : public RNReplicaNet::ReplicaObject
{
	_RO_STD_FRAMEWORK(ObjectNetworking)
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_RequestAction)
	void Call_RPC_RequestAction(tActionData);
	typedef void(tDBFV_RPC_RequestAction)(tActionData);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_ActionResult)
	void Call_RPC_ActionResult(tActionData);
	typedef void(tDBFV_RPC_ActionResult)(tActionData);
	_RO_DEF_REGISTERDATABLOCKS(ObjectNetworking)
};
} // namespace vrc
#endif
