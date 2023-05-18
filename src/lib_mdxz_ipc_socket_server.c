/*
 * @Author       : lv zhipeng
 * @Date         : 2023-03-13 15:17:47
 * @LastEditors  : lv zhipeng
 * @LastEditTime : 2023-03-17 11:24:14
 * @FilePath     : /LIBS/src/lib_mdxz_ipc_socket_server.c
 * @Description  :
 *
 * Copyright (c) 2023 by MDXZ, All Rights Reserved.
 */

#include "lib_mdxz_ipc_socket_server.h"
#include "uthash.h"
#include <ev.h>

// #define IPC_SERVER_LOCK_FILE "./ipc_server.lock"
#define IPC_SERVER_LOCK_FILE "./ipc_server.lock"
#define IPC_SERVER_IP "127.0.0.1"
#define IPC_SERVER_PORT 20480

int ipc_server_lockfile;
struct flock ipc_server_flock;

typedef struct client_ctx_s
{
    int clientSocket;
    struct ev_io io;
    struct ev_timer timer;
    MdxzAppId_e appId;
    UT_hash_handle hh;
} ipc_client_ctx_t;

ipc_client_ctx_t *ipcClientTable = NULL;

typedef struct ipc_server_ctx_s
{
    struct ev_loop *loop;
    struct ev_io server_watcher;
    int listen_fd;
    ipc_client_ctx_t *clients;
} ipc_server_ctx_t;

static ipc_server_ctx_t g_ipc_server_ctx = {0};

static void accept_cb(EV_P_ struct ev_io *io, int revents);
static void client_cb(EV_P_ struct ev_io *io, int revents);
static MDXZ_ERR_CODE_e ipc_server_handle_msg(char *msg, uint32_t msg_len);
static void client_timeout_cb(EV_P_ struct ev_timer *timer, int revents);
static MDXZ_ERR_CODE_e appID_is_registered(MdxzAppId_e appId, ipc_client_ctx_t *result);

/**
 * @description: Check if IPC server is running.
 * @return       MDXZ_DEV_TRUE     IPC server is running.
 *               MDXZ_DEV_FALSE    IPC server is not running.
 */
MDXZ_DEV_STATUS_e server_is_running()
{
    int fd_lockfile;
    struct flock fl;

    // Try to open the lock file for reading
    fd_lockfile = open(IPC_SERVER_LOCK_FILE, O_RDWR | O_CREAT | O_EXCL, 0666);
    if (fd_lockfile == -1)
    {
        if (errno == EACCES)
        {
            return MDXZ_DEV_TRUE;
        }
        else
        {
            return MDXZ_DEV_TRUE;
        }
    }

    // Try to get a shared lock on the lock file
    fl.l_type = F_RDLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    if (fcntl(fd_lockfile, F_SETLK, &fl) == -1)
    {
        // If getting the lock fails, the server is already running
        close(fd_lockfile);
        return MDXZ_DEV_TRUE;
    }

    // Release the lock on the lock file and close the file
    fl.l_type = F_UNLCK;
    if (fcntl(fd_lockfile, F_SETLK, &fl) == -1)
    {
        perror("Unable to unlock file");
        close(fd_lockfile);
        return MDXZ_DEV_FALSE;
    }
    close(fd_lockfile);

    // If we got here, the lock file existed but the server is not running
    return MDXZ_DEV_FALSE;
}

/**
 * @description: Lock (exclusive) the current IPC server
 * @return       MDXZ_SUCCESS      locked successfully,
 *               MDXZ_FAILURE      if failed to lock, which means the IPC
 *               server has already been initialized in another process and does not need to be initialized again.
 */
MDXZ_ERR_CODE_e ipc_server_lockfile_lock()
{
    int fd_lockfile;
    struct flock fl;

    fd_lockfile = open(IPC_SERVER_LOCK_FILE, O_RDWR | O_EXCL, 0666);
    if (fd_lockfile == -1)
    {
        perror(strerror(errno));
        return MDXZ_FAILURE;
    }
    ipc_server_lockfile = fd_lockfile;
    // Try to get a shared lock on the lock file
    fl.l_type = F_RDLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    if (fcntl(ipc_server_lockfile, F_SETLK, &fl) == -1)
    {
        close(ipc_server_lockfile);
        perror(strerror(errno));

        return MDXZ_FAILURE;
    }
    return MDXZ_SUCCESS;
}

/**
 * @description: Release the current IPC server. Call this interface to release
 *               the server resources when the process exits or crashes.
 * @return       MDXZ_SUCCESS
 *               MDXZ_FAILURE
 */
MDXZ_ERR_CODE_e ipc_server_lockfile_unlock()
{
    struct flock fl;

    // Try to get a shared lock on the lock file
    fl.l_type = F_UNLCK;
    fl.l_whence = SEEK_SET;
    fl.l_start = 0;
    fl.l_len = 0;
    if (fcntl(ipc_server_lockfile, F_SETLK, &fl) == -1)
    {
        perror(strerror(errno));
        close(ipc_server_lockfile);
        return MDXZ_FAILURE;
    }
    unlink(IPC_SERVER_LOCK_FILE);

    return MDXZ_SUCCESS;
}

MDXZ_ERR_CODE_e ipc_server_start()
{
    int sockfd, opt = 1;
    struct sockaddr_in serv_addr;

    // Create socket file descriptor
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("socket creation failed");
        return -1;
    }
    g_ipc_server_ctx.listen_fd = sockfd;

    // Set socket options
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt failed");
        return -1;
    }

    // Set server address
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(IPC_SERVER_IP);
    serv_addr.sin_port = htons(IPC_SERVER_PORT);

    // Bind the socket to the server address
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("bind failed");
        return -1;
    }

    // Listen for incoming connections
    if (listen(sockfd, 5) < 0)
    {
        perror("listen failed");
        return -1;
    }

    // Initialize libev loop
    g_ipc_server_ctx.loop = ev_default_loop(0);

    // Create and initialize a watcher for the server socket
    ev_io_init(&g_ipc_server_ctx.server_watcher, accept_cb, sockfd, EV_READ);
    ev_io_start(g_ipc_server_ctx.loop, &g_ipc_server_ctx.server_watcher);

    // Set loop data to point to the hash table
    // ev_set_loop_data(loop, (void *)&client_table);

    // Start the libev loop
    ev_loop(g_ipc_server_ctx.loop, 0);

    // Close the server socket
    close(sockfd);

    return 0;
}

MDXZ_ERR_CODE_e ipc_server_stop()
{
    if (g_ipc_server_ctx.listen_fd < 0)
    {
        return MDXZ_FAILURE;
    }

    // Stop the libev loop

    // Stop the server watcher
    ev_io_stop(g_ipc_server_ctx.loop, &g_ipc_server_ctx.server_watcher);

    // Destroy the libev loop
    ev_loop_destroy(g_ipc_server_ctx.loop);

    // Close the server socket
    close(g_ipc_server_ctx.listen_fd);

    g_ipc_server_ctx.listen_fd = -1;

    return MDXZ_SUCCESS;
}
static MDXZ_ERR_CODE_e ipc_server_handle_msg(char *msg, uint32_t msg_len)
{
    int ret = 0;
    MdxzIpcMsgHead_t *header = (MdxzIpcMsgHead_t *)msg;
    printf("[SERVER] [%d] to [%d]: payload length is [%d] Byte.\n", header->eSrcId, header->eDstId, header->i32payloadLen);
    char *content = msg + sizeof(MdxzIpcMsgHead_t);
    printf("[SERVER] DATA: %x %x %x\n", *content, *(content + 4), *(content + 8));
    ipc_client_ctx_t result = {0};

    if (appID_is_registered(header->eDstId, &result) == MDXZ_SUCCESS)
    {
        if (&result)
        {
            printf("[SERVER] appId=%d, clientSocket=%d\n", result.appId, result.clientSocket);

            ret = send(result.clientSocket, msg, msg_len, 0);
            if (ret == msg_len)
            {
                printf("[SERVER] send length is [%d]\n", ret);
                return MDXZ_SUCCESS;
            }
            else
            {
                perror("send error");
                return MDXZ_FAILURE;
            }
        }
        else
            printf("[SERVER] result is NULL.\n");
        // printf("[SERVER] %d is registered.\n",header->eDstId);
    }
    else
    {
        printf("[%d] to [%d]:The eDstId[%d] is not registered.\n", header->eSrcId, header->eDstId, header->eDstId);
    }
}

static void accept_cb(EV_P_ struct ev_io *io, int revents)
{
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = accept(io->fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd != -1)
    {
        MdxzIpcRegisterInfo_t registerInfo = {0};
        int ret = recv(client_fd, &registerInfo, sizeof(MdxzIpcRegisterInfo_t), 0);
        if (ret == sizeof(MdxzIpcRegisterInfo_t))
        {
            printf("sizeof(MdxzIpcRegisterInfo_t) = %ld\n", sizeof(MdxzIpcRegisterInfo_t));
            printf("new client %d\n", client_fd);
            ipc_client_ctx_t *client = calloc(1, sizeof(ipc_client_ctx_t));
            client->clientSocket = client_fd;
            client->appId = registerInfo.eSelfId;

            ev_io_init(&client->io, client_cb, client_fd, EV_READ);
            ev_timer_init(&client->timer, client_timeout_cb, 10.0, 10);
            client->io.data = client;
            client->timer.data = client;
            HASH_ADD_INT(ipcClientTable, appId, client);
            debug_printf("Registration is successful.The appID is [%d],socket is [%d]\n", client->appId, client->clientSocket);

            ev_io_start(g_ipc_server_ctx.loop, &client->io);
            ev_timer_start(g_ipc_server_ctx.loop, &client->timer);
        }
        else
        {
            close(client_fd);
            debug_printf("Registration is unsuccessful.len is [%d], socket is [%d]\n", ret, client_fd);
        }
    }
}

static MDXZ_ERR_CODE_e socket_is_registered(int32_t socket, ipc_client_ctx_t *result)
{

    for (result = ipcClientTable; result != NULL; result = result->hh.next)
    {
        if (result->clientSocket == socket)
        {
            return MDXZ_SUCCESS;
        }
    }
    return MDXZ_FAILURE;
}

static MDXZ_ERR_CODE_e appID_is_registered(MdxzAppId_e appId, ipc_client_ctx_t *result)
{
    ipc_client_ctx_t *findResult = NULL;
    HASH_FIND_INT(ipcClientTable, &appId, findResult);
    if (result)
    {
        // 找到了对应的客户端
        *result = *findResult;
        debug_printf("rsult appId=%d, clientSocket=%d", result->appId, result->clientSocket);
        return MDXZ_SUCCESS;
    }
    else
    {
        // 没有找到对应的客户端
        debug_printf("no find");
        return MDXZ_FAILURE;
    }
}
static void client_cb(EV_P_ struct ev_io *io, int revents)
{
    ipc_client_ctx_t *client = (ipc_client_ctx_t *)io->data;

    if ((revents & EV_READ) != 0)
    {
        char recv_buf[MDXZ_IPC_MSG_MAX_LEN] = {0};
        int n = recv(client->clientSocket, recv_buf, MDXZ_IPC_MSG_MAX_LEN, 0);
        if (n <= 0)
        {
            printf("client %d disconnected\n", client->clientSocket);
            ev_io_stop(g_ipc_server_ctx.loop, io);
            ev_timer_stop(g_ipc_server_ctx.loop, &client->timer);
            close(client->clientSocket);
            HASH_DEL(ipcClientTable, client);
            free(client);
        }
        else if (n < sizeof(MdxzIpcMsgHead_t))
        {
            printf("recv msg too short.len = %d\n", n);
        }
        else
        {
            MdxzIpcMsgHead_t *header = (MdxzIpcMsgHead_t *)recv_buf;
            ipc_client_ctx_t *result = NULL;
            // if (appID_is_registered(header->appID, result) == MDXZ_SUCCESS)
            if (socket_is_registered(client->clientSocket, result) == MDXZ_SUCCESS)
            {
                ipc_server_handle_msg(recv_buf, n);
            }
            else
            {
                debug_printf("The socket[%d] is not registered\n", client->clientSocket);
                ev_io_stop(g_ipc_server_ctx.loop, io);
                ev_timer_stop(g_ipc_server_ctx.loop, &client->timer);
                close(client->clientSocket);
                HASH_DEL(ipcClientTable, client);
                free(client);
            }
        }
    }
}

static void client_timeout_cb(EV_P_ struct ev_timer *timer, int revents)
{
    ipc_client_ctx_t *client = (ipc_client_ctx_t *)timer->data;
    if (client != NULL)
    {
        printf("client %d timeout\n", client->clientSocket);
        // ev_io_stop(g_ipc_server_ctx.loop, &client->io);
        // ev_timer_stop(g_ipc_server_ctx.loop, &client->timer);
        // close(client->clientSocket);
        // HASH_DEL(ipcClientTable, client);
        // free(client);
    }
}

MDXZ_ERR_CODE_e ipc_client_init(MdxzAppId_e eSelfId, uint32_t *client_socket)
{
    int sockfd;
    struct sockaddr_in addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("Failed to create socket");
        return MDXZ_FAILURE;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(IPC_SERVER_PORT);
    addr.sin_addr.s_addr = inet_addr(IPC_SERVER_IP);

    inet_pton(AF_INET, IPC_SERVER_IP, &(addr.sin_addr));

    if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        debug_printf("Failed to connect to server");
        close(sockfd);
        return MDXZ_FAILURE;
    }
    // Register App ID to server.

    *client_socket = sockfd;
    return MDXZ_SUCCESS;
}
