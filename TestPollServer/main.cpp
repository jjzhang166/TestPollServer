//
//  main.cpp
//  TestPollServer
//
//  Created by PixBoly on 2018/1/15.
//  Copyright © 2018 PixBoly. All rights reserved.
//

#include <iostream>
#include "tcp_poll_server.h"
#define PORT 7777 

int main(int argc, const char * argv[]) {
    // insert code here...
    std::cout << "Hello, World!\n";
    void * serverHandler;
    tcp_poll_server_init(&serverHandler, PORT);
    tcp_poll_server_start(serverHandler);
    return 0;
}
