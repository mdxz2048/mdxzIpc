/*
 * @Author       : lv zhipeng
 * @Date         : 2022-04-11 15:52:16
 * @LastEditors  : lv zhipeng
 * @LastEditTime : 2023-05-18 14:31:25
 * @FilePath     : /lib_caeri_ipc/lib_caeri_ipc_test.c
 * @Description  :
 *
 * Copyright (c) 2023 by MDXZ, All Rights Reserved.
 */
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "mdxz_types.h"
#include "lib_mdxz_ipc.h"
#include "signal.h"

typedef struct
{
    uint32_t data0;
    uint32_t data1;
    uint32_t data2;
    uint32_t data3;
    uint32_t data4;
    uint32_t data5;
    uint32_t data6;
    uint32_t data7;
    uint32_t data8;
    uint32_t data9;
} ipcMsgAPayload_t;

typedef struct
{
    MdxzIpcMsgHead_t header;
    ipcMsgAPayload_t payload;
} ipcTestMsgA_t;

void singal_handle()
{
    printf("Recv a signal.\n");
    lib_mdxz_ipc_client_destroy();
    printf("ipc_server_lockfile_unlock(),exit...\n");
    exit(0);
}

MDXZ_ERR_CODE_e lib_mdxz_ipc_test_handle_recv_msg(MdxzIpcRecvMsg_t *recvMsg)
{
    ipcTestMsgA_t *testMsgA = (ipcTestMsgA_t *)recvMsg->cMsgContent;
    printf("[%d] to [%d]\nLEN[%d]\tdata 0~4[0X%x] [0X%x] [0X%x] [0X%x][0X%x] 5~9[0X%x] [0X%x] [0X%x] [0X%x][0X%x]",
           testMsgA->header.eSrcId, testMsgA->header.eDstId, testMsgA->header.i32payloadLen,
           testMsgA->payload.data0, testMsgA->payload.data1, testMsgA->payload.data2, testMsgA->payload.data3, testMsgA->payload.data4,
           testMsgA->payload.data5, testMsgA->payload.data6, testMsgA->payload.data7, testMsgA->payload.data8, testMsgA->payload.data9);

    return MDXZ_SUCCESS;
}

MDXZ_ERR_CODE_e main(int argc, char **argv)
{
#if 0
    if (server_is_running() == MDXZ_DEV_TRUE)
    {
        printf("server_is_running...");
    }
    else
    {
        if (SIG_ERR == signal(SIGINT, singal_handle))
            printf("cant catch singal_handle\n");
        if (MDXZ_SUCCESS == ipc_server_lockfile_lock())
        {
            printf("ipc_server_lockfile_lock()\n");
            while (1)
            {
                printf(".\n");
                sleep(1);
            }
        }
        else
        {
            printf("Cant lock.");
        }
    }
#endif
    if (argc != 3)
    {
        perror("./test eSrcId eDstId");
        return MDXZ_FAILURE;
    }
    MdxzIpcRegisterInfo_t registerInfo = {
        .eSelfId = atoi(argv[1])};
    int32_t pi32ServerSockfd = 0;

    if (MDXZ_SUCCESS == lib_mdxz_ipc_client_init(registerInfo, &pi32ServerSockfd))
    {
        if (SIG_ERR == signal(SIGINT, singal_handle))
            printf("cant catch singal_handle\n");

        ipcTestMsgA_t msg = {0};
        msg.header.eDstId = atoi(argv[2]);
        msg.header.eSrcId = atoi(argv[1]);
        msg.header.eMsgId = MDXZ_APP_IPC_MSG_NORSP;
        msg.header.eMsgType = MDXZ_IPC_TYPE_SYNC;
        msg.header.i32payloadLen = sizeof(ipcMsgAPayload_t);
        msg.payload.data0 = 0x30;
        msg.payload.data1 = 0x31;
        msg.payload.data2 = 0x32;
        msg.payload.data3 = 0x33;
        msg.payload.data4 = 0x34;
        msg.payload.data5 = 0x35;
        msg.payload.data6 = 0x36;
        msg.payload.data7 = 0x37;
        msg.payload.data8 = 0x38;
        msg.payload.data9 = 0x39;
        printf("eSrcId=%d, eDstId=%d\n", msg.header.eSrcId, msg.header.eDstId);
        MdxzIpcRecvMsg_t recvMsg = {0};
        while (1)
        {
            memset(&recvMsg, 0, sizeof(MdxzIpcRecvMsg_t));
            if (msg.header.eDstId < msg.header.eSrcId)
            {

                lib_mdxz_ipc_client_send_no_rsp_msg(&msg, sizeof(ipcTestMsgA_t));
            }
            else
            {
                printf("recv...\n");
                if (lib_mdxz_ipc_client_recv_msg(&recvMsg) == MDXZ_SUCCESS)
                {
                    lib_mdxz_ipc_test_handle_recv_msg(&recvMsg);
                }
            }

            sleep(1);
        }
    }

    return MDXZ_SUCCESS;
}
