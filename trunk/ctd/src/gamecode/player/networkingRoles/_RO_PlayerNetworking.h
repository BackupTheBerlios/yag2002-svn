/* Auto-generated with RNROLCompiler V2.0.0.0 don't edit */
#ifndef ___RO_PlayerNetworking_h__
#define ___RO_PlayerNetworking_h__
#include "RNReplicaNet/Inc/ReplicaObject.h"
#include "RNReplicaNet/Inc/Define_RO.h"

typedef struct _tChatMsg 
{
    char    _text[ 256 ];
} tChatMsg;

_RO_DEF_ALLOCATEFUNC(PlayerNetworking);
class _MAKE_BASE(PlayerNetworking);
class _RO_PlayerNetworking : public RNReplicaNet::ReplicaObject
{
	_RO_STD_FRAMEWORK(PlayerNetworking);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_AddChatText);
	void Call_RPC_AddChatText(tChatMsg);
	_RO_DEF_REGISTERDATABLOCKS(PlayerNetworking);
};
#endif
