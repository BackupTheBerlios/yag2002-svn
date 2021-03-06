/* Auto-generated with RNROLCompiler V2.8.0.0 don't edit */
#ifndef ___RO_StorageNetworking_h__
#define ___RO_StorageNetworking_h__
#include "RNReplicaNet/Inc/ReplicaObject.h"
#include "RNReplicaNet/Inc/Define_RO.h"
namespace vrc
{

//! Max data length for transfering generic data
#define VRC_SNDATA_MAXLEN   2000

typedef struct _tAccountInfoData
{
    //! Status flags
    enum Status
    {
        eLoggedIn       = 0x01,
        eBanned         = 0x10
    };

    //! Priviledge flags
    enum Privileges
    {
        eModerator      = 0x02
    };

    unsigned int    _sessionCookie;

    unsigned int    _userID;

    char            _p_nickName[ 64 ];

    char            _p_registrationDate[ 64 ];

    char            _p_lastLogin[ 64 ];

    char            _p_onlineTime[ 64 ];

    char            _p_userDescription[ 256 ];

    unsigned int    _priviledges;

    unsigned int    _status;

} tAccountInfoData;


//! Type for trasfering user contacts
typedef struct _tUserContacts
{
    // Command types
    enum
    {
        eUnknown         = 0x00,
        eRequestContacts = 0x10,
        eUpdateContacts  = 0x20,
        eRestuls         = 0x40,
        eError           = 0x80
    };

    //! Session cookie
    unsigned int    _sessionCookie;

    // !User id
    unsigned int    _userID;

    //! Command
    unsigned int    _cmd;

    //! Length of data
    unsigned int    _dataLen;

    //! Data buffer
    char            _p_data[ VRC_SNDATA_MAXLEN ];

} tUserContacts;


_RO_DEF_ALLOCATEFUNC(StorageNetworking)
class _MAKE_BASE(StorageNetworking);
class _RO_StorageNetworking : public RNReplicaNet::ReplicaObject
{
	_RO_STD_FRAMEWORK(StorageNetworking)
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_RequestAccountInfo)
	void Call_RPC_RequestAccountInfo(tAccountInfoData);
	typedef void(tDBFV_RPC_RequestAccountInfo)(tAccountInfoData);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_RequestUpdateAccountInfo)
	void Call_RPC_RequestUpdateAccountInfo(tAccountInfoData);
	typedef void(tDBFV_RPC_RequestUpdateAccountInfo)(tAccountInfoData);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_AccountInfoResult)
	void Call_RPC_AccountInfoResult(tAccountInfoData);
	typedef void(tDBFV_RPC_AccountInfoResult)(tAccountInfoData);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_RequestContacts)
	void Call_RPC_RequestContacts(tUserContacts);
	typedef void(tDBFV_RPC_RequestContacts)(tUserContacts);
	_RO_DO_DEFBLOCK_FUNCTION_VAR(RPC_ContactsResult)
	void Call_RPC_ContactsResult(tUserContacts);
	typedef void(tDBFV_RPC_ContactsResult)(tUserContacts);
	_RO_DEF_REGISTERDATABLOCKS(StorageNetworking)
};
} // namespace vrc
#endif
