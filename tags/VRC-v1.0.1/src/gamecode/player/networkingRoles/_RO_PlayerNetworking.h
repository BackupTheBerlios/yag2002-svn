/* Auto-generated with RNROLCompiler V2.2.0.0 don't edit */
#ifndef ___RO_PlayerNetworking_h__
#define ___RO_PlayerNetworking_h__
#include "RNReplicaNet/Inc/ReplicaObject.h"
#include "RNReplicaNet/Inc/Define_RO.h"
_RO_DEF_ALLOCATEFUNC(PlayerNetworking);

typedef struct _tInitializationData
{
    float   _posX;
    float   _posY;
    float   _posZ;
    float   _rotZ;   
} tInitializationData;

class _MAKE_BASE(PlayerNetworking);
class _RO_PlayerNetworking : public RNReplicaNet::ReplicaObject
{
	_RO_STD_FRAMEWORK(PlayerNetworking);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_Initialize);
	void Call_RPC_Initialize(tInitializationData);
	_RO_DEF_REGISTERDATABLOCKS(PlayerNetworking);
};
#endif
