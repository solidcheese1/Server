#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <iostream>
#include "socket.h"

int set_nonblocking(int sock){
    int flags = fcntl(sock,F_GETFL, 0);
    if(flags == -1){
        std::cerr << "Can't get socket flags";
        return -2;
    }
    flags = flags | O_NONBLOCK;
    if(fcntl(sock, F_SETFL, flags) == -1){
        std::cerr << "Error in setting nonblocking regime";
        return -2;
    }
    return 0;

};

int create_socket(int port, int type){
    int sock = socket(AF_INET, type, 0);
    if (sock < 0) {
        std::cerr << "Cant create socket";
        return -1;
    }
    int res = set_nonblocking(sock);
    if(res < 0){
        return res;
    }
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); 
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        std::cerr << "Bind error";
        return -2;
    }
    return sock;

};

int add_socket_to_epoll(int &epoll_fd, int sock){
    struct epoll_event ev;
    ev.events = EPOLLIN; 
    ev.data.fd = sock;
    int res = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &ev); 
    if(res < 0){
        std::cerr << "Adding socket to epoll error";
    }
    return res;

};