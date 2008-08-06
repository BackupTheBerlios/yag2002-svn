/* Auto-generated with RNROLCompiler V2.7.0.0 don't edit */
#ifndef ___RO_MailboxNetworking_h__
#define ___RO_MailboxNetworking_h__
#include "RNReplicaNet/Inc/ReplicaObject.h"
#include "RNReplicaNet/Inc/Define_RO.h"
namespace vrc
{

//! Max size of mailbox data buffer
#define VRC_MAILBOXDATA_MAXLEN  ( 20 * 1024 )

//! Type for mail data header
typedef struct _tMailDataHeader
{
    //! Command to perform such as delete, move, etc.
    unsigned int    _cmd;

    //! Unique mail ID
    unsigned int    _mailID;

    //! Status flags
    unsigned int    _status;

    //! Lenth of data buffer
    unsigned int    _dataLen;

} tMailDataHeader;


//! Type for trasfering mail data
typedef struct _tMailData
{
    //! Mail data header
    tMailDataHeader _header;

    //! Data buffer
    char            _p_data[ VRC_MAILBOXDATA_MAXLEN ];

} tMailData;


_RO_DEF_ALLOCATEFUNC(MailboxNetworking)
class _MAKE_BASE(MailboxNetworking);
class _RO_MailboxNetworking : public RNReplicaNet::ReplicaObject
{
	_RO_STD_FRAMEWORK(MailboxNetworking)
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_RequestMailCommand)
	void Call_RPC_RequestMailCommand(tMailData);
	typedef void(tDBFV_RPC_RequestMailCommand)(tMailData);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_Response)
	void Call_RPC_Response(tMailData);
	typedef void(tDBFV_RPC_Response)(tMailData);
	_RO_DEF_REGISTERDATABLOCKS(MailboxNetworking)
};
} // namespace vrc
#endif
