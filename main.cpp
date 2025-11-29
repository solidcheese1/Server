#include "config.hpp"
#include "Socket/socket.h"
#include "server_state.hpp"
#include "TCP/tcp_handler.h"
#include "UDP/udp_handler.h"


int main(){
    std::cout << "Server started" << std::endl;
    int ret = 0;
    struct epoll_event ev,events[MAX_EVENTS];
    server_state server = {0,0,0,0};
    struct sockaddr_in tcp_addr;
    struct sockaddr_in udp_addr;
    int tcp_listen_sock = create_tcp_socket();
    int udp_listen_sock = create_udp_socket();
    if(tcp_listen_sock < 0 || udp_listen_sock < 0){
        return -2;
    }


    int epoll_fd = epoll_create1(0); 
    if(epoll_fd == -1){
        std::cerr <<"epoll_create";
        ret = -2;
        server.shutdown_flag = 1;

    }
    if((add_socket_to_epoll(epoll_fd,tcp_listen_sock) < 0) || (add_socket_to_epoll(epoll_fd,udp_listen_sock) < 0)){
        ret = -2;
        server.shutdown_flag = 1;

    }
    
    while (!server.shutdown_flag) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1); 
        if (n == -1) {
            if (errno == EINTR)
                continue;
            std::cerr << "epoll_wait";
            ret = -2;
            break;
        }
        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;
            if (fd == tcp_listen_sock) {
                int res = accept_tcp_clients(tcp_listen_sock, epoll_fd, server);
                if(res == -2){
                    ret = -2;
                    break;
                }

            } else if (fd == udp_listen_sock) {
                int res = process_data_udp(udp_listen_sock, epoll_fd,server, events[i]);
                if(res == -2){
                    ret = -2;
                    break;
                }
            } else {
                int res = process_data_tcp(fd, epoll_fd, server, events[i]);
                if(res == -2){
                    ret = -2;
                    break;
                }
            }
        }
    }

    close(tcp_listen_sock);
    close(udp_listen_sock);
    close(epoll_fd);
    clean_send_again();
    std::cout << "Server shutdown\n";

    return ret;

}
