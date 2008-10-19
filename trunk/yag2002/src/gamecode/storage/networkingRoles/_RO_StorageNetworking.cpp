/* Auto-generated with RNROLCompiler V2.8.0.0 don't edit */
#include "_RO_StorageNetworking.h"
#ifdef _WIN32
#pragma warning(disable : 4786)
#endif
#include "../vrc_storagenetworking.h"
using namespace vrc;
_RO_DO_STD_FRAMEWORK(StorageNetworking)
#include "RNReplicaNet/Inc/DataBlock_Function.h"
_RO_DO_MEMBERFUNC_PRESTAGE_DEF(StorageNetworking,RPC_RequestAccountInfo)
tAccountInfoData value1
_RO_DO_MEMBERFUNC_PRESTAGE(RPC_RequestAccountInfo)
_RO_DO_MEMBERFUNC_ADDVAR2(value1,tAccountInfoData)
_RO_DO_MEMBERFUNC_POSTSTAGE(RPC_RequestAccountInfo)
_RO_DO_MEMBERFUNC_CRACK_START(StorageNetworking,RPC_RequestAccountInfo)
tAccountInfoData value1;
_RO_DO_MEMBERFUNC_MESSAGE_CRACK(value1)
_RO_DO_MEMBERFUNC_CRACK_END(StorageNetworking,RPC_RequestAccountInfo)
value1
_RO_DO_MEMBERFUNC_CRACK_END2()
_RO_DO_MEMBERFUNC_PRESTAGE_DEF(StorageNetworking,RPC_RequestUpdateAccountInfo)
tAccountInfoData value1
_RO_DO_MEMBERFUNC_PRESTAGE(RPC_RequestUpdateAccountInfo)
_RO_DO_MEMBERFUNC_ADDVAR2(value1,tAccountInfoData)
_RO_DO_MEMBERFUNC_POSTSTAGE(RPC_RequestUpdateAccountInfo)
_RO_DO_MEMBERFUNC_CRACK_START(StorageNetworking,RPC_RequestUpdateAccountInfo)
tAccountInfoData value1;
_RO_DO_MEMBERFUNC_MESSAGE_CRACK(value1)
_RO_DO_MEMBERFUNC_CRACK_END(StorageNetworking,RPC_RequestUpdateAccountInfo)
value1
_RO_DO_MEMBERFUNC_CRACK_END2()
_RO_DO_MEMBERFUNC_PRESTAGE_DEF(StorageNetworking,RPC_AccountInfoResult)
tAccountInfoData value1
_RO_DO_MEMBERFUNC_PRESTAGE(RPC_AccountInfoResult)
_RO_DO_MEMBERFUNC_ADDVAR2(value1,tAccountInfoData)
_RO_DO_MEMBERFUNC_POSTSTAGE(RPC_AccountInfoResult)
_RO_DO_MEMBERFUNC_CRACK_START(StorageNetworking,RPC_AccountInfoResult)
tAccountInfoData value1;
_RO_DO_MEMBERFUNC_MESSAGE_CRACK(value1)
_RO_DO_MEMBERFUNC_CRACK_END(StorageNetworking,RPC_AccountInfoResult)
value1
_RO_DO_MEMBERFUNC_CRACK_END2()
_RO_DO_MEMBERFUNC_PRESTAGE_DEF(StorageNetworking,RPC_RequestContacts)
tUserContacts value1
_RO_DO_MEMBERFUNC_PRESTAGE(RPC_RequestContacts)
_RO_DO_MEMBERFUNC_ADDVAR2(value1,tUserContacts)
_RO_DO_MEMBERFUNC_POSTSTAGE(RPC_RequestContacts)
_RO_DO_MEMBERFUNC_CRACK_START(StorageNetworking,RPC_RequestContacts)
tUserContacts value1;
_RO_DO_MEMBERFUNC_MESSAGE_CRACK(value1)
_RO_DO_MEMBERFUNC_CRACK_END(StorageNetworking,RPC_RequestContacts)
value1
_RO_DO_MEMBERFUNC_CRACK_END2()
_RO_DO_MEMBERFUNC_PRESTAGE_DEF(StorageNetworking,RPC_ContactsResult)
tUserContacts value1
_RO_DO_MEMBERFUNC_PRESTAGE(RPC_ContactsResult)
_RO_DO_MEMBERFUNC_ADDVAR2(value1,tUserContacts)
_RO_DO_MEMBERFUNC_POSTSTAGE(RPC_ContactsResult)
_RO_DO_MEMBERFUNC_CRACK_START(StorageNetworking,RPC_ContactsResult)
tUserContacts value1;
_RO_DO_MEMBERFUNC_MESSAGE_CRACK(value1)
_RO_DO_MEMBERFUNC_CRACK_END(StorageNetworking,RPC_ContactsResult)
value1
_RO_DO_MEMBERFUNC_CRACK_END2()
_RO_DO_BEGINREGISTERDATABLOCKS(StorageNetworking)
_RO_DO_REGISTERBLOCK_FUNCTION_VAR(RPC_RequestAccountInfo)
_RO_DO_DATABLOCK_MAKERELIABLE()
_RO_DO_REGISTERBLOCK_END()
_RO_DO_REGISTERBLOCK_FUNCTION_VAR(RPC_RequestUpdateAccountInfo)
_RO_DO_DATABLOCK_MAKERELIABLE()
_RO_DO_REGISTERBLOCK_END()
_RO_DO_REGISTERBLOCK_FUNCTION_VAR(RPC_AccountInfoResult)
_RO_DO_DATABLOCK_MAKERELIABLE()
_RO_DO_REGISTERBLOCK_END()
_RO_DO_REGISTERBLOCK_FUNCTION_VAR(RPC_RequestContacts)
_RO_DO_DATABLOCK_MAKERELIABLE()
_RO_DO_REGISTERBLOCK_END()
_RO_DO_REGISTERBLOCK_FUNCTION_VAR(RPC_ContactsResult)
_RO_DO_DATABLOCK_MAKERELIABLE()
_RO_DO_REGISTERBLOCK_END()
_RO_DO_ENDREGISTERDATABLOCKS(StorageNetworking)
