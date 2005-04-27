/* Auto-generated with RNROLCompiler V2.0.0.0 don't edit */
#include "_RO_PlayerNetworking.h"
#ifdef _WIN32
#pragma warning(disable : 4786)
#endif
#include "../ctd_playernetworking.h"
_RO_DO_STD_FRAMEWORK(PlayerNetworking);
#include "RNReplicaNet/Inc/DataBlock_Function.h"
_RO_DO_MEMBERFUNC_PRESTAGE_DEF(PlayerNetworking,RPC_AddChatText)
tChatMsg value1
_RO_DO_MEMBERFUNC_PRESTAGE(RPC_AddChatText);
_RO_DO_MEMBERFUNC_ADDVAR(value1);
_RO_DO_MEMBERFUNC_POSTSTAGE(RPC_AddChatText);
_RO_DO_MEMBERFUNC_CRACK_START(PlayerNetworking,RPC_AddChatText)
tChatMsg value1;
_RO_DO_MEMBERFUNC_MESSAGE_CRACK(value1);
_RO_DO_MEMBERFUNC_CRACK_END(PlayerNetworking,RPC_AddChatText)
value1
_RO_DO_MEMBERFUNC_CRACK_END2()
#include "RNReplicaNet/Inc/DataBlock_Predict_Float.h"
#include "RNReplicaNet/Inc/DataBlock_NData.h"
_RO_DO_BEGINREGISTERDATABLOCKS(PlayerNetworking);
_RO_DO_REGISTERBLOCK_NDATA_VAR(_p_playerName);
_RO_DO_DATABLOCK_MAKERELIABLE();
_RO_DO_REGISTERBLOCK_END();
_RO_DO_REGISTERBLOCK_NDATA_VAR(_p_animFileName);
_RO_DO_DATABLOCK_MAKERELIABLE();
_RO_DO_REGISTERBLOCK_END();
_RO_DO_REGISTERBLOCK_PREDICT_FLOAT_VAR(_positionX);
_RO_DO_REGISTERBLOCK_PREDICT_FLOAT_SETMINERROR(0.1f);
_RO_DO_REGISTERBLOCK_PREDICT_FLOAT_SETMINDISTANCE(6.0f);
_RO_DO_REGISTERBLOCK_PREDICT_FLOAT_SETMAXERROR(0.5f);
_RO_DO_REGISTERBLOCK_PREDICT_FLOAT_SETMAXDISTANCE(10.0f);
_RO_DO_REGISTERBLOCK_PREDICT_FLOAT_SETQUADRATIC();
_RO_DO_REGISTERBLOCK_END();
_RO_DO_REGISTERBLOCK_PREDICT_FLOAT_VAR(_positionY);
_RO_DO_REGISTERBLOCK_PREDICT_FLOAT_SETMINERROR(0.1f);
_RO_DO_REGISTERBLOCK_PREDICT_FLOAT_SETMINDISTANCE(6.0f);
_RO_DO_REGISTERBLOCK_PREDICT_FLOAT_SETMAXERROR(0.5f);
_RO_DO_REGISTERBLOCK_PREDICT_FLOAT_SETMAXDISTANCE(10.0f);
_RO_DO_REGISTERBLOCK_PREDICT_FLOAT_SETQUADRATIC();
_RO_DO_REGISTERBLOCK_END();
_RO_DO_REGISTERBLOCK_PREDICT_FLOAT_VAR(_positionZ);
_RO_DO_REGISTERBLOCK_PREDICT_FLOAT_SETMINERROR(0.1f);
_RO_DO_REGISTERBLOCK_PREDICT_FLOAT_SETMINDISTANCE(6.0f);
_RO_DO_REGISTERBLOCK_PREDICT_FLOAT_SETMAXERROR(0.5f);
_RO_DO_REGISTERBLOCK_PREDICT_FLOAT_SETMAXDISTANCE(10.0f);
_RO_DO_REGISTERBLOCK_PREDICT_FLOAT_SETQUADRATIC();
_RO_DO_REGISTERBLOCK_END();
_RO_DO_REGISTERBLOCK_PREDICT_FLOAT_VAR(_yaw);
_RO_DO_REGISTERBLOCK_PREDICT_FLOAT_SETMINERROR(0.1f);
_RO_DO_REGISTERBLOCK_PREDICT_FLOAT_SETMAXERROR(0.2f);
_RO_DO_REGISTERBLOCK_PREDICT_FLOAT_SETLINEAR();
_RO_DO_REGISTERBLOCK_END();
_RO_DO_REGISTERBLOCK_NDATA_VAR(_cmdAnimFlags);
_RO_DO_DATABLOCK_MAKERELIABLE();
_RO_DO_REGISTERBLOCK_END();
_RO_DO_REGISTERBLOCK_FUNCTION_VAR(RPC_AddChatText);
_RO_DO_DATABLOCK_MAKERELIABLE();
_RO_DO_REGISTERBLOCK_END();
_RO_DO_ENDREGISTERDATABLOCKS(PlayerNetworking);
