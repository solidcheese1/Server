#include "socket.h"
void set_nonblocking(int sock){
    int flags = fcntl(sock,F_GETFL, 0);
    if(flags == -1){
        perror("Can't get socket flags");
    }
    flags = flags | O_NONBLOCK;
    if(fcntl(sock, F_SETFL, flags) == -1){
        perror("Error in setting nonblocking regime");
    }

};

int create_socket(int port, int type){
    int sock = socket(AF_INET, type, 0);
    if (sock < 0) {
        std::cerr << "Cant create socket";
        return -1;
    }
    set_nonblocking(sock);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;
    int opt = 1;
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); 
    if (bind(sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind");
        close(sock);
        return 1;
    }
    return sock;

};

void add_socket_to_epoll(int &epoll_fd, int sock){
    struct epoll_event ev;
    ev.events = EPOLLIN; 
    ev.data.fd = sock;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sock, &ev); 

};