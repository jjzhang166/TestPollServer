//
//  poll_server.h
//  TestPollServer
//
//  Created by PixBoly on 2018/1/15.
//  Copyright © 2018年 PixBoly. All rights reserved.
//

#ifndef poll_server_h
#define poll_server_h

#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif
//函数声明
//创建套接字并进行绑定
int socket_bind(const char* ip,int port);
//IO多路复用poll
void do_poll(int listenfd);
//处理多个连接
void handle_connection(struct pollfd *connfds,int num);

int test_poll(void);

#ifdef __cplusplus
}
#endif
#endif /* poll_server_h */
