#ifndef __LIB_MDXZ_IPC_SOCKET_H_
#define __LIB_MDXZ_IPC_SOCKET_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "mdxz_types.h"
#include "mdxz_common_app_id.h"
#include "mdxz_common_msg_map.h"
#include "lib_mdxz_ipc_socket_server.h"

#include "uthash.h" // 使用哈希表需要包含此头文件

#define MDXZ_IPC_MSG_MAX_LEN     (2048)

//typedef MdxzErrorCode_e (*pFunIpcAsyncCb)(const UCHAR *pcRspMsg, Uint32_t u32RspMsgLen);





typedef struct {
    int32_t u32MsgLen;
    char cMsgContent[MDXZ_IPC_MSG_MAX_LEN];
} MdxzIpcRecvMsg_t;

// // 定义哈希表结构体，用来存放进程头信息
// typedef struct {
//     MdxzAppId_e app_id; // 进程ID
//     int sockfd; // 进程对应的socket
//     UT_hash_handle hh; // 定义哈希表元素
// } MdxzIpcProcess;



MDXZ_ERR_CODE_e lib_mdxz_ipc_client_init(MdxzIpcRegisterInfo_t registerInfo, int32_t *pi32ServerSockfd);
MDXZ_ERR_CODE_e lib_mdxz_ipc_client_destroy();
MDXZ_ERR_CODE_e lib_mdxz_ipc_client_recv_msg(MdxzIpcRecvMsg_t *ptRecvMsg);
// MdxzErrorCode_e MdxzIpcRspMsg(UCHAR *pcRspMsg, Uint32_t u32RspMsgLen);
// MdxzErrorCode_e MdxzIpcSendSyncMsg(UCHAR *pcSendMsg, Uint32_t u32SendMsgLen, UCHAR *pcRecvMsg, Uint32_t *pu32RecvMsgLen, Uint32_t u32TimeoutSec);
MDXZ_ERR_CODE_e lib_mdxz_ipc_client_send_no_rsp_msg(void *ptSendMsg, u_int32_t sendMsgLen);
// MdxzErrorCode_e MdxzIpcSaveDataToFile(const UCHAR *pcFileName, const VOID *pvData, Uint32_t u32DataLen);
// MdxzErrorCode_e MdxzIpcGetDataFromFile(const UCHAR *pcFileName, VOID *pvBuff, Uint32_t u32DataLen);


#ifdef __cplusplus
}
#endif

#endif