#include "config.hpp"
#include "Socket/socket.h"
#include "server_state.hpp"
#include "TCP/tcp_handler.h"
#include "UDP/udp_handler.h"

int main(){
    std::cout << "Server started" << std::endl;
    struct epoll_event ev,events[MAX_EVENTS];
    server_state server = {0,0,0,0};
    struct sockaddr_in tcp_addr;
    struct sockaddr_in udp_addr;
    int tcp_listen_sock = create_tcp_socket();
    int udp_listen_sock = create_udp_socket();


    int epoll_fd = epoll_create1(0); 
    if(epoll_fd == -1){
        perror("epoll_create");
        return 1;
    }
    add_socket_to_epoll(epoll_fd,tcp_listen_sock);
    add_socket_to_epoll(epoll_fd,udp_listen_sock);
    while (!server.shutdown_flag) {
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1); 
        if (n == -1) {
            if (errno == EINTR)
                continue;
            perror("epoll_wait");
            break;
        }
        for (int i = 0; i < n; i++) {
            int fd = events[i].data.fd;
            if (fd == tcp_listen_sock) {
                accept_tcp_clients(tcp_listen_sock, epoll_fd, server);

            } else if (fd == udp_listen_sock) {
                process_data_udp(udp_listen_sock, server);
            } else {
                process_data_tcp(fd, epoll_fd, server);
            }
        }
    }

    close(tcp_listen_sock);
    close(udp_listen_sock);
    close(epoll_fd);
    printf("Server shutdown\n");

    return 0;

}
