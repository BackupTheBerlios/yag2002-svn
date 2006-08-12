/* Auto-generated with RNROLCompiler V2.4.0.0 don't edit */
#include "_RO_NetworkNotification.h"
#ifdef _WIN32
#pragma warning(disable : 4786)
#endif
#include "../vrc_networknotification.h"

_RO_DO_STD_FRAMEWORK(NetworkNotificationNetworking)
#include "RNReplicaNet/Inc/DataBlock_NData.h"
#include "RNReplicaNet/Inc/DataBlock_Function.h"
_RO_DO_MEMBERFUNC_PRESTAGE_DEF(NetworkNotificationNetworking,RPC_RecvNotification)
tNotificationData value1
_RO_DO_MEMBERFUNC_PRESTAGE(RPC_RecvNotification)
_RO_DO_MEMBERFUNC_ADDVAR2(value1,tNotificationData)
_RO_DO_MEMBERFUNC_POSTSTAGE(RPC_RecvNotification)
_RO_DO_MEMBERFUNC_CRACK_START(NetworkNotificationNetworking,RPC_RecvNotification)
tNotificationData value1;
_RO_DO_MEMBERFUNC_MESSAGE_CRACK(value1)
_RO_DO_MEMBERFUNC_CRACK_END(NetworkNotificationNetworking,RPC_RecvNotification)
value1
_RO_DO_MEMBERFUNC_CRACK_END2()
_RO_DO_BEGINREGISTERDATABLOCKS(NetworkNotificationNetworking)
_RO_DO_REGISTERBLOCK_NDATA_VAR(_p_versionInfo)
_RO_DO_DATABLOCK_MAKERELIABLE()
_RO_DO_REGISTERBLOCK_END()
_RO_DO_REGISTERBLOCK_NDATA_VAR(_p_message)
_RO_DO_DATABLOCK_MAKERELIABLE()
_RO_DO_REGISTERBLOCK_END()
_RO_DO_REGISTERBLOCK_FUNCTION_VAR(RPC_RecvNotification)
_RO_DO_DATABLOCK_MAKERELIABLE()
_RO_DO_REGISTERBLOCK_END()
_RO_DO_ENDREGISTERDATABLOCKS(NetworkNotificationNetworking)
