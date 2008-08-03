/* Auto-generated with RNROLCompiler V2.7.0.0 don't edit */
#ifndef ___RO_MailboxNetworking_h__
#define ___RO_MailboxNetworking_h__
#include "RNReplicaNet/Inc/ReplicaObject.h"
#include "RNReplicaNet/Inc/Define_RO.h"
namespace vrc
{

//! Type for sending requests to mailbox server
typedef struct _tMailRequest
{
    //! Unique user ID
    unsigned int    _userId;

    //! Command to perform such as delete, move, etc.
    unsigned int    _cmd;

    //! Unique mail ID
    unsigned int    _mailId;

    //! String buffer used for some commands
    char            _message[ 64 ];

} tMailRequest;

//! Type for trasfering mail contents
typedef struct _tMailContent
{
    //! Unique mail ID
    unsigned int    _id;

    //! Sender
    char            _from[ 64 ];

    //! Comma separated recipients
    char            _to[ 64 ];

    //! Comma separated copy recipients
    char            _cc[ 64 ];

    //! Timestamp of receipt or send
    char            _date[ 64 ];

    //! Mail subject
    char            _subject[ 64 ];

    //! Mail body
    char            _body[ 512 ];

    //! Mail attributes, see MailboxContent::Attributes
    unsigned int    _attributes;

} tMailContent;


_RO_DEF_ALLOCATEFUNC(MailboxNetworking)
class _MAKE_BASE(MailboxNetworking);
class _RO_MailboxNetworking : public RNReplicaNet::ReplicaObject
{
	_RO_STD_FRAMEWORK(MailboxNetworking)
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_RequestMailCommand)
	void Call_RPC_RequestMailCommand(tMailRequest);
	typedef void(tDBFV_RPC_RequestMailCommand)(tMailRequest);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_RequestSendMail)
	void Call_RPC_RequestSendMail(tMailContent);
	typedef void(tDBFV_RPC_RequestSendMail)(tMailContent);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_Response)
	void Call_RPC_Response(tMailRequest);
	typedef void(tDBFV_RPC_Response)(tMailRequest);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_SendMail)
	void Call_RPC_SendMail(tMailContent);
	typedef void(tDBFV_RPC_SendMail)(tMailContent);
	_RO_DEF_REGISTERDATABLOCKS(MailboxNetworking)
};
} // namespace vrc
#endif
