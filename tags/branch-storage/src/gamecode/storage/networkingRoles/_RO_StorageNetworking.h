/* Auto-generated with RNROLCompiler V2.7.0.0 don't edit */
#ifndef ___RO_StorageNetworking_h__
#define ___RO_StorageNetworking_h__
#include "RNReplicaNet/Inc/ReplicaObject.h"
#include "RNReplicaNet/Inc/Define_RO.h"
namespace vrc
{
typedef struct _tAuthData
{
    char    _login[ 64 ];

    char    _passwd[ 64 ];

} tAuthData;

_RO_DEF_ALLOCATEFUNC(StorageNetworking)
class _MAKE_BASE(StorageNetworking);
class _RO_StorageNetworking : public RNReplicaNet::ReplicaObject
{
	_RO_STD_FRAMEWORK(StorageNetworking)
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_Authentify)
	void Call_RPC_Authentify(tAuthData);
	typedef void(tDBFV_RPC_Authentify)(tAuthData);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_AuthentificationResult)
	void Call_RPC_AuthentificationResult(bool);
	typedef void(tDBFV_RPC_AuthentificationResult)(bool);
	_RO_DEF_REGISTERDATABLOCKS(StorageNetworking)
};
} // namespace vrc
#endif
