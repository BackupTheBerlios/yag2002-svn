/* Auto-generated with RNROLCompiler V2.7.0.0 don't edit */
#include "_RO_MailboxNetworking.h"
#ifdef _WIN32
#pragma warning(disable : 4786)
#endif
#include "../vrc_mailboxnetworking.h"

// Specialise these two templates to allow tMailData to be used with DataBlock_Function
namespace RNReplicaNet
{

template<>
void FunctionAddVariable< vrc::tMailData >( DynamicMessageHelper &message, vrc::tMailData &variable )
{
    message.AddVariable( &variable._header, sizeof( variable._header ) );
    unsigned int len = ( variable._header._dataLen > VRC_MAILBOXDATA_MAXLEN ) ? VRC_MAILBOXDATA_MAXLEN : variable._header._dataLen;
    // limit the data length
    message.AddVariable( variable._p_data, len );
}

template<>
void FunctionGetVariable< vrc::tMailData >( MessageHelper &message, vrc::tMailData &variable )
{
    message.GetVariable( &variable._header, sizeof( variable._header ) );
    // limit the data length
    unsigned int len = ( variable._header._dataLen > VRC_MAILBOXDATA_MAXLEN ) ? VRC_MAILBOXDATA_MAXLEN : variable._header._dataLen;
    message.GetVariable( variable._p_data, len );
}

} // namespace RNReplicaNet


using namespace vrc;

_RO_DO_STD_FRAMEWORK(MailboxNetworking)
#include "RNReplicaNet/Inc/DataBlock_Function.h"
_RO_DO_MEMBERFUNC_PRESTAGE_DEF(MailboxNetworking,RPC_RequestMailCommand)
tMailData value1
_RO_DO_MEMBERFUNC_PRESTAGE(RPC_RequestMailCommand)
_RO_DO_MEMBERFUNC_ADDVAR2(value1,tMailData)
_RO_DO_MEMBERFUNC_POSTSTAGE(RPC_RequestMailCommand)
_RO_DO_MEMBERFUNC_CRACK_START(MailboxNetworking,RPC_RequestMailCommand)
tMailData value1;
_RO_DO_MEMBERFUNC_MESSAGE_CRACK(value1)
_RO_DO_MEMBERFUNC_CRACK_END(MailboxNetworking,RPC_RequestMailCommand)
value1
_RO_DO_MEMBERFUNC_CRACK_END2()
_RO_DO_MEMBERFUNC_PRESTAGE_DEF(MailboxNetworking,RPC_Response)
tMailData value1
_RO_DO_MEMBERFUNC_PRESTAGE(RPC_Response)
_RO_DO_MEMBERFUNC_ADDVAR2(value1,tMailData)
_RO_DO_MEMBERFUNC_POSTSTAGE(RPC_Response)
_RO_DO_MEMBERFUNC_CRACK_START(MailboxNetworking,RPC_Response)
tMailData value1;
_RO_DO_MEMBERFUNC_MESSAGE_CRACK(value1)
_RO_DO_MEMBERFUNC_CRACK_END(MailboxNetworking,RPC_Response)
value1
_RO_DO_MEMBERFUNC_CRACK_END2()
_RO_DO_BEGINREGISTERDATABLOCKS(MailboxNetworking)
_RO_DO_REGISTERBLOCK_FUNCTION_VAR(RPC_RequestMailCommand)
_RO_DO_DATABLOCK_MAKERELIABLE()
_RO_DO_REGISTERBLOCK_END()
_RO_DO_REGISTERBLOCK_FUNCTION_VAR(RPC_Response)
_RO_DO_DATABLOCK_MAKERELIABLE()
_RO_DO_REGISTERBLOCK_END()
_RO_DO_ENDREGISTERDATABLOCKS(MailboxNetworking)
