/* Auto-generated with RNROLCompiler V2.7.0.0 don't edit */
#ifndef ___RO_ObjectNetworking_h__
#define ___RO_ObjectNetworking_h__
#include "RNReplicaNet/Inc/ReplicaObject.h"
#include "RNReplicaNet/Inc/Define_RO.h"
namespace vrc
{
_RO_DEF_ALLOCATEFUNC(ObjectNetworking)
class _MAKE_BASE(ObjectNetworking);
class _RO_ObjectNetworking : public RNReplicaNet::ReplicaObject
{
	_RO_STD_FRAMEWORK(ObjectNetworking)
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_RequestUse)
	void Call_RPC_RequestUse(unsigned int /*clientID*/ );
	typedef void(tDBFV_RPC_RequestUse)(unsigned int /*clientID*/ );
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_Use)
	void Call_RPC_Use(unsigned int /*clientID*/ );
	typedef void(tDBFV_RPC_Use)(unsigned int /*clientID*/ );
	_RO_DEF_REGISTERDATABLOCKS(ObjectNetworking)
};
} // namespace vrc
#endif
