/* Auto-generated with RNROLCompiler V2.4.0.0 don't edit */
#ifndef ___RO_NetworkNotification_h__
#define ___RO_NetworkNotification_h__
#include "RNReplicaNet/Inc/ReplicaObject.h"
#include "RNReplicaNet/Inc/Define_RO.h"

//! Struct for notification string
typedef struct _tNotificationData
{
    //! Timout for auto-destruction of messagebox. if 0 then the default timeout is taken
    float   _destructionTimeout;
    //! Notification text
    char    _text[ 256 ];
    //! Notification title
    char    _title[ 32 ];

} tNotificationData;

_RO_DEF_ALLOCATEFUNC(NetworkNotificationNetworking)
class _MAKE_BASE(NetworkNotificationNetworking);
class _RO_NetworkNotificationNetworking : public RNReplicaNet::ReplicaObject
{
	_RO_STD_FRAMEWORK(NetworkNotificationNetworking)
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_RecvNotification)
	void Call_RPC_RecvNotification(tNotificationData);
	typedef void(tDBFV_RPC_RecvNotification)(tNotificationData);
	_RO_DEF_REGISTERDATABLOCKS(NetworkNotificationNetworking)
};
#endif
