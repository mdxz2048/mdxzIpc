/*
 * @Author       : lv zhipeng
 * @Date         : 2023-03-15 14:32:54
 * @LastEditors  : lv zhipeng
 * @LastEditTime : 2023-03-17 08:58:06
 * @FilePath     : /LIBS/src/lib_mdxz_ipc_socket.c
 * @Description  :
 *
 * Copyright (c) 2023 by MDXZ, All Rights Reserved.
 */
#include <stdio.h>
#include "lib_mdxz_ipc_socket.h"
#include "pthread.h"
#include "stdbool.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct
{
    bool createdIPCServerByCurrentAppID;
    bool isIPCInitialized;
    MdxzIpcRegisterInfo_t registerInfo;
    int32_t clientSocket;
} MdxzIpcClientInfo_t;

MdxzIpcClientInfo_t gIpcClient = {
    .createdIPCServerByCurrentAppID = false,
    .isIPCInitialized = false};

static MDXZ_ERR_CODE_e lib_mdxz_ipc_client_register(MdxzIpcRegisterInfo_t registerInfo, uint32_t client_socket);

void *server_thread(void *arg)
{
    ipc_server_start();
    return NULL;
}

MDXZ_ERR_CODE_e lib_mdxz_ipc_client_init(MdxzIpcRegisterInfo_t registerInfo, int32_t *socketFd)
{
    if (gIpcClient.isIPCInitialized == false)
    {
        int ret = -1;
        debug_printf("isIPCInitialized is false.");
        if (server_is_running() == MDXZ_DEV_TRUE)
        {
            debug_printf("server is running...");
            int32_t clientSocket = 0;
            ret = ipc_client_init(registerInfo.eSelfId, &clientSocket);
            if (ret == MDXZ_SUCCESS)
            {
                ret == lib_mdxz_ipc_client_register(registerInfo, clientSocket);
                if (ret == MDXZ_SUCCESS)
                {
                    debug_printf("ipc_client_init() successful.\n");
                    gIpcClient.clientSocket = clientSocket;
                    return MDXZ_SUCCESS;
                }
                else
                {
                    debug_printf("register failed.\n");
                    close(clientSocket);
                    return MDXZ_FAILURE;
                }
            }
            else
            {
                debug_printf("ipc_client_init() failed.\n");
                return MDXZ_FAILURE;
            }
        }
        else
        {
            debug_printf("server is not running...");
            pthread_t thread_to_start_ipc_server;
            gIpcClient.createdIPCServerByCurrentAppID = true;
            // create thread for start ipc server
            ret = pthread_create(&thread_to_start_ipc_server, NULL, server_thread, NULL);
            if (ret != 0)
            {
                debug_printf("Failed to create server thread: %d\n", ret);
                return MDXZ_FAILURE;
            }
            if (MDXZ_SUCCESS == ipc_server_lockfile_lock())
            {
                debug_printf("ipc_server_lockfile_lock()\n");
                // init client
                int32_t clientSocket = 0;
                ret = ipc_client_init(registerInfo.eSelfId, &clientSocket);
                if (ret == MDXZ_SUCCESS)
                {
                    ret == lib_mdxz_ipc_client_register(registerInfo, clientSocket);
                    if (ret == MDXZ_SUCCESS)
                    {
                        debug_printf("ipc_client_init() successful.\n");
                        *socketFd = clientSocket;
                        gIpcClient.clientSocket = clientSocket;
                        gIpcClient.isIPCInitialized = true;
                        return MDXZ_SUCCESS;
                    }
                    else
                    {
                        debug_printf("register failed.\n");
                        close(clientSocket);
                        return MDXZ_FAILURE;
                    }
                    return MDXZ_SUCCESS;
                }
                else
                {
                    debug_printf("ipc_client_init() failed.");
                    return MDXZ_FAILURE;
                }
            }
            else
            {
                debug_printf("Cant lock.");
                ipc_server_stop();
            }
        }
    }
    else
    {
        // IPC in the current process has already been initialized.
        debug_printf("isIPCInitialized is true.");
        *socketFd = gIpcClient.clientSocket;
        return MDXZ_SUCCESS;
    }
}
static MDXZ_ERR_CODE_e lib_mdxz_ipc_client_register(MdxzIpcRegisterInfo_t registerInfo, uint32_t client_socket)
{
    int sendLen = 0;
    sendLen = send(client_socket, &registerInfo, sizeof(MdxzIpcRegisterInfo_t), 0);
    if (sendLen == sizeof(MdxzIpcRegisterInfo_t))
    {
        debug_printf("sendLen = [%d], appId = [%d]", sendLen, registerInfo.eSelfId);

        return MDXZ_SUCCESS;
    }
    else
    {
        perror("register client to server failed.");
        return MDXZ_FAILURE;
    }
}

MDXZ_ERR_CODE_e lib_mdxz_ipc_client_destroy()
{
    int ret = 0;
    if (gIpcClient.createdIPCServerByCurrentAppID == true)
    {
        debug_printf("destroy ip server.\n");
        ret = ipc_server_stop();
        ret = ipc_server_lockfile_unlock();
        if (ret == MDXZ_SUCCESS)
        {
            debug_printf("ipc_server_lockfile_unlock()...");
        }
        else
        {
            debug_printf("ipc_server_lockfile_unlock() failed.");
        }
    }
    close(gIpcClient.clientSocket);
    gIpcClient.isIPCInitialized = false;
    return MDXZ_SUCCESS;
}

MDXZ_ERR_CODE_e lib_mdxz_ipc_client_send_no_rsp_msg(void *ptSendMsg, u_int32_t sendMsgLen)
{
    uint32_t length = 0;
    length = send(gIpcClient.clientSocket, ptSendMsg, sendMsgLen, 0);
    if (length == sendMsgLen)
    {
        return MDXZ_SUCCESS;
    }
    else
    {
        printf("send length is [%d]\n", length);
        return MDXZ_FAILURE;
    }
}

MDXZ_ERR_CODE_e lib_mdxz_ipc_client_recv_msg(MdxzIpcRecvMsg_t *ptRecvMsg)
{
    uint32_t length = 0;
    length = recv(gIpcClient.clientSocket, ptRecvMsg->cMsgContent, MDXZ_IPC_MSG_MAX_LEN, 0);
    if (length == -1)
    {
        ptRecvMsg->u32MsgLen = 0;
        perror("recv() is failed.");
        return MDXZ_FAILURE;
    }
    else
    {
        ptRecvMsg->u32MsgLen = length;
        return MDXZ_SUCCESS;
    }
}
