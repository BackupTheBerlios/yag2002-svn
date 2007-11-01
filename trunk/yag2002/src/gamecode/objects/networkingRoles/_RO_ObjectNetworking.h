/* Auto-generated with RNROLCompiler V2.7.0.0 don't edit */
#ifndef ___RO_ObjectNetworking_h__
#define ___RO_ObjectNetworking_h__
#include "RNReplicaNet/Inc/ReplicaObject.h"
#include "RNReplicaNet/Inc/Define_RO.h"
namespace vrc
{

typedef struct _tObjectData
{
    //! User ID of picking user
    unsigned int    _userID;

    //! Used for object respawn
    unsigned int    _objectID;

    float           _posX;
    float           _posY;
    float           _posZ;
    float           _rotZ;

    //! Object parameters depending on object type
    unsigned int    _param1;
    unsigned int    _param2;
    unsigned int    _param3;
    unsigned int    _param4;

} tObjectData;


_RO_DEF_ALLOCATEFUNC(ObjectNetworking)
class _MAKE_BASE(ObjectNetworking);
class _RO_ObjectNetworking : public RNReplicaNet::ReplicaObject
{
	_RO_STD_FRAMEWORK(ObjectNetworking)
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_Respawn)
	void Call_RPC_Respawn(tObjectData);
	typedef void(tDBFV_RPC_Respawn)(tObjectData);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_Use)
	void Call_RPC_Use(tObjectData);
	typedef void(tDBFV_RPC_Use)(tObjectData);
	_RO_DEF_REGISTERDATABLOCKS(ObjectNetworking)
};
} // namespace vrc
#endif
