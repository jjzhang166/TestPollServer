//
//  tcp_poll_server.c
//  AQPollServer
//
//  Created by PixBoly on 2018/1/16.
//  Copyright © 2018年 pix. All rights reserved.
//

#include "tcp_poll_server.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <pthread.h>

#define LISTENQ     5
#define OPEN_MAX    1000
#define INFTIM      -1

pthread_mutex_t sendmutex;
pthread_mutex_t broadcastmutex;

/**
 服务器端内部数据，Handler
 */
typedef struct _PollServerData{
    char ip[20];
    int port;
    struct pollfd clientfds[OPEN_MAX];
    int clientcount;
    PollRecvDataFunc recvFunc;
}PollServerData;

// 数据绑定
int socket_bind(int port);
// 处理连接
void handle_connection(PollServerData * data,int num);
// 执行poll
void do_poll(int listenfd,PollServerData * data);


// 内部方法，socket绑定
int socket_bind(int port)
{
    int  listenfd;
    struct sockaddr_in servaddr;
    listenfd = socket(AF_INET,SOCK_STREAM,0);
    if (listenfd == -1)
    {
        perror("socket error:");
        exit(1);
    }
    /* Enable address reuse */
    int on = 1;
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on) );
    
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET,"192.168.1.107",&servaddr.sin_addr);
    servaddr.sin_port = htons(port);
    if (bind(listenfd,(struct sockaddr*)&servaddr,sizeof(servaddr)) == -1)
    {
        perror("bind error: ");
        exit(1);
    }
    return listenfd;
}

// 内部方法，执行poll操作
void do_poll(int listenfd,PollServerData * data)
{
    int  connfd,sockfd;
    struct sockaddr_in cliaddr;
    socklen_t cliaddrlen;
    int maxi;
    int i;
    int nready;
    //添加监听描述符
    data->clientfds[0].fd = listenfd;
    data->clientfds[0].events = POLLIN;
    //初始化客户连接描述符
    for (i = 1;i < OPEN_MAX;i++)
        data->clientfds[i].fd = -1;
    maxi = 0;
    //循环处理
    for ( ; ; )
    {
        //获取可用描述符的个数
        nready = poll(data->clientfds,maxi+1,INFTIM);
        if (nready == -1)
        {
            perror("poll error:");
            exit(1);
        }
        //测试监听描述符是否准备好
        if (data->clientfds[0].revents & POLLIN)
        {
            cliaddrlen = sizeof(cliaddr);
            //接受新的连接
            if ((connfd = accept(listenfd,(struct sockaddr*)&cliaddr,&cliaddrlen)) == -1)
            {
                if (errno == EINTR)
                    continue;
                else
                {
                    perror("accept error:");
                    exit(1);
                }
            }
            fprintf(stdout,"accept a new client: %s:%d,fd:%d\n", inet_ntoa(cliaddr.sin_addr),cliaddr.sin_port,connfd);
            //将新的连接描述符添加到数组中
            for (i = 1;i < OPEN_MAX;i++)
            {
                if (data->clientfds[i].fd < 0)
                {
                    data->clientfds[i].fd = connfd;
                    data->clientcount = i + 1; // 这里放了一个服务器fd
                    break;
                }
            }
            if (i == OPEN_MAX)
            {
                fprintf(stderr,"too many clients.\n");
                exit(1);
            }
            //将新的描述符添加到读描述符集合中
            data->clientfds[i].events = POLLIN;
            //记录客户连接套接字的个数
            maxi = (i > maxi ? i : maxi);
            if (--nready <= 0)
                continue;
        }
        //处理客户连接
        handle_connection(data,maxi);
    }
}

// 处理连接
void handle_connection(PollServerData * data,int num)
{
    int i,n;
    unsigned char buf[MAXLINE];
    memset(buf,0,MAXLINE);
    for (i = 1;i <= num;i++)
    {
        if (data->clientfds[i].fd < 0)
            continue;
        //测试客户描述符是否准备好
        if (data->clientfds[i].revents & POLLIN)
        {
            //接收客户端发送的信息
            n = read(data->clientfds[i].fd,buf,MAXLINE);
            if (n == 0)
            {
                close(data->clientfds[i].fd);
                data->clientfds[i].fd = -1;
                continue;
            }
            // printf("read msg is: ");
//            write(STDOUT_FILENO,buf,n);
            //向客户端发送buf
           // write(data->clientfds[i].fd,buf,n);
            if(NULL != data->recvFunc) {
                data->recvFunc(data->clientfds[i].fd,buf,n);
            }
        }
    }
}


/**
 tpc poll 客户端初始化
 
 @param handler 服务端句柄
 @param port    服务端口
 @return 正确返回0，错误返回负值
 */
int tcp_poll_server_init(void ** handler /* out */,int port) {
    int ret = 0;
    if(port <= 0) {
        ret =  -1;
        printf("FUNC tcp_poll_server_init(),error:port <= 0\n");
        goto END;
    }
    *handler = malloc(sizeof(PollServerData));
    if(NULL == *handler) {
        ret = -2;
        printf("FUNC tcp_poll_server_init(),error:malloc err,ret:%d\n",ret);
        goto END;
    }
    PollServerData * pData = *handler;
    pData->port = port;
    
END:
    return ret;
    
}
/**
 启动poll服务器
 
 @param handler 服务器句柄
 @return 正确返回0，错误返回负值
 */
int tcp_poll_server_start(void * handler /* in */)
{
    int ret = 0;
    if(NULL == handler) {
        ret = -1;
        goto END;
    }
    PollServerData * pData = handler;
    
    int  listenfd,connfd,sockfd;
    struct sockaddr_in cliaddr;
    socklen_t cliaddrlen;
    listenfd = socket_bind(pData->port);     // 绑定IP,端口
    listen(listenfd,LISTENQ); //监听fd
    pthread_mutex_init(&sendmutex,NULL);
    pthread_mutex_init(&broadcastmutex, NULL);
    do_poll(listenfd,pData);
    
END:
    return ret;
}

/**
 设置服务器端回调数据
 
 @param handler 服务器句柄
 @param callback 回调函数指针
 @return 正确返回0
 */
int tcp_poll_server_set_recv_callback(void * handler,PollRecvDataFunc callback) {
    int ret = 0;
    if(NULL == handler || NULL == callback) {
        ret = -1;
        goto END;
    }
    PollServerData * pData = handler;
    pData->recvFunc = callback;
    
END:
    return ret;
}

// 服务器发送报文
int tcp_poll_server_send_data(int fd,unsigned char * buff,int bufflen) {
    //    printf("FUNC poll_server_send_data(),bufflen:%d\n",bufflen);
    pthread_mutex_lock(&sendmutex);
    int ret = 0;
    //向客户端发送buf
    write(fd,buff,bufflen);
    pthread_mutex_unlock(&sendmutex);
    return ret;
}

// 广播报文
int tcp_poll_server_broadcast_data(void * handler,unsigned char * buff,int bufflen) {
    printf("broadcast start....\n");
    //    pthread_mutex_lock(&broadcastmutex);
    int ret = 0;
    
    if(NULL == handler || NULL == buff || bufflen <= 0) {
        ret = -1;
        return ret ;
    }
    PollServerData * pData = handler;
    int i = 1; // 第0个为服务器fd
    for (i = 1; i < pData->clientcount; i++) {
        tcp_poll_server_send_data(pData->clientfds[i].fd, buff, bufflen);
    }
    //    pthread_mutex_unlock(&broadcastmutex);
    printf("broadcast end...\n");
    return ret;
}

/**
 服务器关闭
 
 @param handler 服务器句柄
 @return 正确返回0
 */
int tcp_poll_server_close(void * handler) {
    int ret = 0;
    if(NULL == handler) {
        ret = -1;
        goto END;
    }
    PollServerData * pData = handler;
    // 第一个就是服务器的fd
    close(pData->clientfds[0].fd);
END:
    return ret;
    
}

/**
 服务器销毁
 
 @param handler 服务器句柄指针
 @return 正确返回0
 */
int tcp_poll_server_destroy(void ** handler /* in */) {
    int ret = 0;
    if(NULL == handler) {
        return ret;
    }
    if(NULL != *handler) {
        free(*handler);
        *handler = NULL;
    }
    handler = NULL;
    return ret;
}
