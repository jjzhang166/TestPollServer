//
//  tcp_poll_server.h
//  AQPollServer
//  服务器端tcp poll封装
//  Created by PixBoly on 2018/1/16.
//  Copyright © 2018年 pix. All rights reserved.
//

#ifndef tcp_poll_server_h
#define tcp_poll_server_h

// 最大缓冲区大小

#define MAXLINE     1024
#ifdef __cplusplus
extern "C"
{
#endif
    
/**
 tcp 接收报文回调
  回调方法指针类型
 @param fd 回调数据客户端fd
 @param buf 回调数据buff指针
 @param buflen 回调数据长度
 */
typedef void (*PollRecvDataFunc)(int fd,unsigned char * buf,int buflen);
    
/**
 tpc poll 客户端初始化

 @param handler 服务端句柄
 @param port    服务端口
 @return 正确返回0，错误返回负值
 */
int tcp_poll_server_init(void ** handler /* out */,int port);

/**
    启动poll服务器

 @param handler 服务器句柄
 @return 正确返回0，错误返回负值
 */
int tcp_poll_server_start(void * handler /* in */);


/**
 设置服务器端回调数据

 @param handler 服务器句柄
 @param callback 回调函数指针
 @return 正确返回0
 */
int tcp_poll_server_set_recv_callback(void * handler,PollRecvDataFunc callback);


/**
 服务端发报文

 @param fd 所要发送的客户端fd
 @param buff 所有发送数据buff
 @param bufflen 所要发送数据的长度
 @return 发送成功返回0
 */
int tcp_poll_server_send_data(int fd,unsigned char * buff,int bufflen);


/**
 服务器端广播报文

 @param handler 服务器句柄
 @param buff    数据
 @param bufflen 数据长度
 @return 正确返回0
 */
int tcp_poll_server_broadcast_data(void * handler,unsigned char * buff,int bufflen);


/**
 服务器关闭

 @param handler 服务器句柄
 @return 正确返回0
 */
int tcp_poll_server_close(void * handler/* in */);


/**
 服务器销毁

 @param handler 服务器句柄指针
 @return 正确返回0
 */
int tcp_poll_server_destroy(void ** handler /* in */);
    
#ifdef __cplusplus
}
#endif


#endif /* tcp_poll_server_h */
