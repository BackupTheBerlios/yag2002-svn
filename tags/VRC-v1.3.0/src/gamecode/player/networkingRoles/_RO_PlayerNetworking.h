/* Auto-generated with RNROLCompiler V2.7.0.0 don't edit */
#ifndef ___RO_PlayerNetworking_h__
#define ___RO_PlayerNetworking_h__
#include "RNReplicaNet/Inc/ReplicaObject.h"
#include "RNReplicaNet/Inc/Define_RO.h"

typedef struct _tInitializationData
{
    float   _posX;
    float   _posY;
    float   _posZ;
    float   _rotZ;
    char    _ip[ 24 ];
} tInitializationData;

_RO_DEF_ALLOCATEFUNC(PlayerNetworking)
class _MAKE_BASE(PlayerNetworking);
class _RO_PlayerNetworking : public RNReplicaNet::ReplicaObject
{
	_RO_STD_FRAMEWORK(PlayerNetworking)
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_ServerGrantsAccess)
	void Call_RPC_ServerGrantsAccess(tInitializationData);
	typedef void(tDBFV_RPC_ServerGrantsAccess)(tInitializationData);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_RequestInitialization)
	void Call_RPC_RequestInitialization();
	typedef void(tDBFV_RPC_RequestInitialization)();
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_Initialize)
	void Call_RPC_Initialize(tInitializationData);
	typedef void(tDBFV_RPC_Initialize)(tInitializationData);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_EnableVoiceChat)
	void Call_RPC_EnableVoiceChat(bool);
	typedef void(tDBFV_RPC_EnableVoiceChat)(bool);
	_RO_DEF_REGISTERDATABLOCKS(PlayerNetworking)
};
#endif
