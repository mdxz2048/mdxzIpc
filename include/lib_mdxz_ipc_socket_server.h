#ifndef __LIB_MDXZ_IPC_SOCKET_SERVER_H_
#define __LIB_MDXZ_IPC_SOCKET_SERVER_H_

#ifdef __cplusplus
extern "C"
{
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <errno.h>
#include "mdxz_types.h"
#include "mdxz_common_app_id.h"
#include "mdxz_common_msg_map.h"

#define MDXZ_IPC_MSG_MAX_LEN     (2048)

//typedef MdxzErrorCode_e (*pFunIpcAsyncCb)(const UCHAR *pcRspMsg, UINT32 u32RspMsgLen);

typedef enum {
    MDXZ_IPC_TYPE_SYNC,
    MDXZ_IPC_TYPE_ASYNC,
    MDXZ_IPC_TYPE_ASYNC_RSP
} MdxzIpcMsgType_e;

typedef struct {
    MdxzAppId_e eSelfId; //must be filled
} MdxzIpcRegisterInfo_t;

typedef struct {
    MdxzAppId_e eSrcId; //must be filled
    MdxzAppId_e eDstId; //must be filled
    MdxzIpcMsg_e eMsgId; //must be filled
    MdxzIpcMsgType_e eMsgType;
    //pFunIpcAsyncCb pFunAsyncCb;
    int32_t i32Reserve;
    int32_t i32payloadLen; //must be filled
} MdxzIpcMsgHead_t;

// typedef struct {
//     UCHAR cMsgContent[MDXZ_IPC_MSG_MAX_LEN];
//     UINT32 u32MsgLen;
// } MdxzIpcRecvMsg_t;

// // 定义哈希表结构体，用来存放进程头信息
// typedef struct {
//     MdxzAppId_e app_id; // 进程ID
//     int sockfd; // 进程对应的socket
//     UT_hash_handle hh; // 定义哈希表元素
// } MdxzIpcProcess;



// MdxzErrorCode_e MdxzIpcRecvMsg(INT32 i32ServerSockfd, MdxzIpcRecvMsg_t *ptRecvMsg);
// MdxzErrorCode_e MdxzIpcRspMsg(UCHAR *pcRspMsg, UINT32 u32RspMsgLen);
// MdxzErrorCode_e MdxzIpcSendSyncMsg(UCHAR *pcSendMsg, UINT32 u32SendMsgLen, UCHAR *pcRecvMsg, UINT32 *pu32RecvMsgLen, UINT32 u32TimeoutSec);
// MdxzErrorCode_e MdxzIpcSendNoRspMsg(UCHAR *pcSendMsg, UINT32 u32SendMsgLen);
// MdxzErrorCode_e MdxzIpcSaveDataToFile(const UCHAR *pcFileName, const VOID *pvData, UINT32 u32DataLen);
// MdxzErrorCode_e MdxzIpcGetDataFromFile(const UCHAR *pcFileName, VOID *pvBuff, UINT32 u32DataLen);
MDXZ_DEV_STATUS_e server_is_running();
MDXZ_ERR_CODE_e ipc_server_lockfile_lock();
MDXZ_ERR_CODE_e ipc_server_lockfile_unlock();
MDXZ_ERR_CODE_e ipc_server_start();
MDXZ_ERR_CODE_e ipc_server_stop();
MDXZ_ERR_CODE_e ipc_client_init(MdxzAppId_e eSelfId,uint32_t *client_socket);

#ifdef __cplusplus
}
#endif

#endif