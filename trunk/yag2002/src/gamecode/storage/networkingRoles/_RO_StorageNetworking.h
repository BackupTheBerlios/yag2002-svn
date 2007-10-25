/* Auto-generated with RNROLCompiler V2.7.0.0 don't edit */
#ifndef ___RO_StorageNetworking_h__
#define ___RO_StorageNetworking_h__
#include "RNReplicaNet/Inc/ReplicaObject.h"
#include "RNReplicaNet/Inc/Define_RO.h"
namespace vrc
{

typedef struct _tAccountInfoData
{
    unsigned int    _userID;

    unsigned int    _registrationDate;

    unsigned int    _onlineTime;

    unsigned int    _priviledges;

} tAccountInfoData;


_RO_DEF_ALLOCATEFUNC(StorageNetworking)
class _MAKE_BASE(StorageNetworking);
class _RO_StorageNetworking : public RNReplicaNet::ReplicaObject
{
	_RO_STD_FRAMEWORK(StorageNetworking)
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_RequestAccountInfo)
	void Call_RPC_RequestAccountInfo(tAccountInfoData);
	typedef void(tDBFV_RPC_RequestAccountInfo)(tAccountInfoData);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_AccountInfoResult)
	void Call_RPC_AccountInfoResult(tAccountInfoData);
	typedef void(tDBFV_RPC_AccountInfoResult)(tAccountInfoData);
	_RO_DEF_REGISTERDATABLOCKS(StorageNetworking)
};
} // namespace vrc
#endif
