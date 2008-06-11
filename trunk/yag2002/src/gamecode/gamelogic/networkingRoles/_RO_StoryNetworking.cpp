/* Auto-generated with RNROLCompiler V2.7.0.0 don't edit */
#include "_RO_StoryNetworking.h"
#ifdef _WIN32
#pragma warning(disable : 4786)
#endif
#include "../vrc_storynetworking.h"

using namespace vrc;
_RO_DO_STD_FRAMEWORK(StoryNetworking)
#include "RNReplicaNet/Inc/DataBlock_Function.h"
_RO_DO_MEMBERFUNC_PRESTAGE_DEF(StoryNetworking,RPC_ReceiveEvent)
tEventData value1
_RO_DO_MEMBERFUNC_PRESTAGE(RPC_ReceiveEvent)
_RO_DO_MEMBERFUNC_ADDVAR2(value1,tEventData)
_RO_DO_MEMBERFUNC_POSTSTAGE(RPC_ReceiveEvent)
_RO_DO_MEMBERFUNC_CRACK_START(StoryNetworking,RPC_ReceiveEvent)
tEventData value1;
_RO_DO_MEMBERFUNC_MESSAGE_CRACK(value1)
_RO_DO_MEMBERFUNC_CRACK_END(StoryNetworking,RPC_ReceiveEvent)
value1
_RO_DO_MEMBERFUNC_CRACK_END2()
_RO_DO_MEMBERFUNC_PRESTAGE_DEF(StoryNetworking,RPC_OpenDialog)
tDialogData value1
_RO_DO_MEMBERFUNC_PRESTAGE(RPC_OpenDialog)
_RO_DO_MEMBERFUNC_ADDVAR2(value1,tDialogData)
_RO_DO_MEMBERFUNC_POSTSTAGE(RPC_OpenDialog)
_RO_DO_MEMBERFUNC_CRACK_START(StoryNetworking,RPC_OpenDialog)
tDialogData value1;
_RO_DO_MEMBERFUNC_MESSAGE_CRACK(value1)
_RO_DO_MEMBERFUNC_CRACK_END(StoryNetworking,RPC_OpenDialog)
value1
_RO_DO_MEMBERFUNC_CRACK_END2()
_RO_DO_MEMBERFUNC_PRESTAGE_DEF(StoryNetworking,RPC_ReceiveDialogResults)
tDialogResultsData value1
_RO_DO_MEMBERFUNC_PRESTAGE(RPC_ReceiveDialogResults)
_RO_DO_MEMBERFUNC_ADDVAR2(value1,tDialogResultsData)
_RO_DO_MEMBERFUNC_POSTSTAGE(RPC_ReceiveDialogResults)
_RO_DO_MEMBERFUNC_CRACK_START(StoryNetworking,RPC_ReceiveDialogResults)
tDialogResultsData value1;
_RO_DO_MEMBERFUNC_MESSAGE_CRACK(value1)
_RO_DO_MEMBERFUNC_CRACK_END(StoryNetworking,RPC_ReceiveDialogResults)
value1
_RO_DO_MEMBERFUNC_CRACK_END2()
_RO_DO_BEGINREGISTERDATABLOCKS(StoryNetworking)
_RO_DO_REGISTERBLOCK_FUNCTION_VAR(RPC_ReceiveEvent)
_RO_DO_DATABLOCK_MAKERELIABLE()
_RO_DO_REGISTERBLOCK_END()
_RO_DO_REGISTERBLOCK_FUNCTION_VAR(RPC_OpenDialog)
_RO_DO_DATABLOCK_MAKERELIABLE()
_RO_DO_REGISTERBLOCK_END()
_RO_DO_REGISTERBLOCK_FUNCTION_VAR(RPC_ReceiveDialogResults)
_RO_DO_DATABLOCK_MAKERELIABLE()
_RO_DO_REGISTERBLOCK_END()
_RO_DO_ENDREGISTERDATABLOCKS(StoryNetworking)
