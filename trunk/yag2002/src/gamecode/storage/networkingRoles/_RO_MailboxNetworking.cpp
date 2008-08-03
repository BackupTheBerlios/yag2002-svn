/* Auto-generated with RNROLCompiler V2.7.0.0 don't edit */
#include "_RO_MailboxNetworking.h"
#ifdef _WIN32
#pragma warning(disable : 4786)
#endif
#include "../vrc_mailboxnetworking.h"

using namespace vrc;
_RO_DO_STD_FRAMEWORK(MailboxNetworking)
#include "RNReplicaNet/Inc/DataBlock_Function.h"
_RO_DO_MEMBERFUNC_PRESTAGE_DEF(MailboxNetworking,RPC_RequestMailCommand)
tMailRequest value1
_RO_DO_MEMBERFUNC_PRESTAGE(RPC_RequestMailCommand)
_RO_DO_MEMBERFUNC_ADDVAR2(value1,tMailRequest)
_RO_DO_MEMBERFUNC_POSTSTAGE(RPC_RequestMailCommand)
_RO_DO_MEMBERFUNC_CRACK_START(MailboxNetworking,RPC_RequestMailCommand)
tMailRequest value1;
_RO_DO_MEMBERFUNC_MESSAGE_CRACK(value1)
_RO_DO_MEMBERFUNC_CRACK_END(MailboxNetworking,RPC_RequestMailCommand)
value1
_RO_DO_MEMBERFUNC_CRACK_END2()
_RO_DO_MEMBERFUNC_PRESTAGE_DEF(MailboxNetworking,RPC_RequestSendMail)
tMailContent value1
_RO_DO_MEMBERFUNC_PRESTAGE(RPC_RequestSendMail)
_RO_DO_MEMBERFUNC_ADDVAR2(value1,tMailContent)
_RO_DO_MEMBERFUNC_POSTSTAGE(RPC_RequestSendMail)
_RO_DO_MEMBERFUNC_CRACK_START(MailboxNetworking,RPC_RequestSendMail)
tMailContent value1;
_RO_DO_MEMBERFUNC_MESSAGE_CRACK(value1)
_RO_DO_MEMBERFUNC_CRACK_END(MailboxNetworking,RPC_RequestSendMail)
value1
_RO_DO_MEMBERFUNC_CRACK_END2()
_RO_DO_MEMBERFUNC_PRESTAGE_DEF(MailboxNetworking,RPC_Response)
tMailRequest value1
_RO_DO_MEMBERFUNC_PRESTAGE(RPC_Response)
_RO_DO_MEMBERFUNC_ADDVAR2(value1,tMailRequest)
_RO_DO_MEMBERFUNC_POSTSTAGE(RPC_Response)
_RO_DO_MEMBERFUNC_CRACK_START(MailboxNetworking,RPC_Response)
tMailRequest value1;
_RO_DO_MEMBERFUNC_MESSAGE_CRACK(value1)
_RO_DO_MEMBERFUNC_CRACK_END(MailboxNetworking,RPC_Response)
value1
_RO_DO_MEMBERFUNC_CRACK_END2()
_RO_DO_MEMBERFUNC_PRESTAGE_DEF(MailboxNetworking,RPC_SendMail)
tMailContent value1
_RO_DO_MEMBERFUNC_PRESTAGE(RPC_SendMail)
_RO_DO_MEMBERFUNC_ADDVAR2(value1,tMailContent)
_RO_DO_MEMBERFUNC_POSTSTAGE(RPC_SendMail)
_RO_DO_MEMBERFUNC_CRACK_START(MailboxNetworking,RPC_SendMail)
tMailContent value1;
_RO_DO_MEMBERFUNC_MESSAGE_CRACK(value1)
_RO_DO_MEMBERFUNC_CRACK_END(MailboxNetworking,RPC_SendMail)
value1
_RO_DO_MEMBERFUNC_CRACK_END2()
_RO_DO_BEGINREGISTERDATABLOCKS(MailboxNetworking)
_RO_DO_REGISTERBLOCK_FUNCTION_VAR(RPC_RequestMailCommand)
_RO_DO_DATABLOCK_MAKERELIABLE()
_RO_DO_REGISTERBLOCK_END()
_RO_DO_REGISTERBLOCK_FUNCTION_VAR(RPC_RequestSendMail)
_RO_DO_DATABLOCK_MAKERELIABLE()
_RO_DO_REGISTERBLOCK_END()
_RO_DO_REGISTERBLOCK_FUNCTION_VAR(RPC_Response)
_RO_DO_DATABLOCK_MAKERELIABLE()
_RO_DO_REGISTERBLOCK_END()
_RO_DO_REGISTERBLOCK_FUNCTION_VAR(RPC_SendMail)
_RO_DO_DATABLOCK_MAKERELIABLE()
_RO_DO_REGISTERBLOCK_END()
_RO_DO_ENDREGISTERDATABLOCKS(MailboxNetworking)
