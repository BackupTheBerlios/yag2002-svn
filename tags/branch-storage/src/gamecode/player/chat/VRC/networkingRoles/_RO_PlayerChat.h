/* Auto-generated with RNROLCompiler V2.4.0.0 don't edit */
#ifndef ___RO_PlayerChat_h__
#define ___RO_PlayerChat_h__
#include "RNReplicaNet/Inc/ReplicaObject.h"
#include "RNReplicaNet/Inc/Define_RO.h"

//! Attention, 256 bytes is the maximum what ReplicaNet allows to send in an RPC call
typedef struct _tChatMsg
{
    //! Message text
    unsigned char    _text[ 255 - 2 * sizeof( int ) ];

    //! Used for private message ( whisper ), 0 if publich message, session ID of recipient if private
    int              _recipientID;

    //! Sender session ID
    int              _sessionID;

} tChatMsg;

typedef struct _tChatData
{
    char    _nickname[ 32 ];

    char    _realname[ 32 ];

    int     _sessionID;
    // do we need more initialization data for VRC chat?

} tChatData;

_RO_DEF_ALLOCATEFUNC(ImplChatNetworkingVRC)
class _MAKE_BASE(ImplChatNetworkingVRC);
class _RO_ImplChatNetworkingVRC : public RNReplicaNet::ReplicaObject
{
	_RO_STD_FRAMEWORK(ImplChatNetworkingVRC)
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_RequestJoin)
	void Call_RPC_RequestJoin(tChatData);
	typedef void(tDBFV_RPC_RequestJoin)(tChatData);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_ClientJoined)
	void Call_RPC_ClientJoined(tChatData);
	typedef void(tDBFV_RPC_ClientJoined)(tChatData);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_RequestLeave)
	void Call_RPC_RequestLeave(tChatData);
	typedef void(tDBFV_RPC_RequestLeave)(tChatData);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_ClientLeft)
	void Call_RPC_ClientLeft(tChatData);
	typedef void(tDBFV_RPC_ClientLeft)(tChatData);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_InitializeClient)
	void Call_RPC_InitializeClient(tChatData);
	typedef void(tDBFV_RPC_InitializeClient)(tChatData);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_RequestChangeNickname)
	void Call_RPC_RequestChangeNickname(tChatData);
	typedef void(tDBFV_RPC_RequestChangeNickname)(tChatData);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_ChangedNickname)
	void Call_RPC_ChangedNickname(tChatData);
	typedef void(tDBFV_RPC_ChangedNickname)(tChatData);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_RequestMemberList)
	void Call_RPC_RequestMemberList(tChatData);
	typedef void(tDBFV_RPC_RequestMemberList)(tChatData);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_RecvMemberList)
	void Call_RPC_RecvMemberList(tChatData);
	typedef void(tDBFV_RPC_RecvMemberList)(tChatData);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_PostChatText)
	void Call_RPC_PostChatText(tChatMsg);
	typedef void(tDBFV_RPC_PostChatText)(tChatMsg);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_RecvChatText)
	void Call_RPC_RecvChatText(tChatMsg);
	typedef void(tDBFV_RPC_RecvChatText)(tChatMsg);
	_RO_DEF_REGISTERDATABLOCKS(ImplChatNetworkingVRC)
};
#endif
