#include "tcp_handler.h"


int create_tcp_socket(){
    int tcp_listen_sock = create_socket(TCP_PORT, SOCK_STREAM);
    if(tcp_listen_sock < 0){
        return tcp_listen_sock;
    }
    if (listen(tcp_listen_sock, SOMAXCONN) == -1) {
        perror("listen");
        return -1;
    }
    return tcp_listen_sock;
};


void accept_tcp_clients(int &tcp_listen_sock, int &epoll_fd, server_state &server){
    while (true) {
        struct epoll_event ev;
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(tcp_listen_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break; 
            else {
                perror("accept");
                break;
            }
        }

        set_nonblocking(client_fd);
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = client_fd;
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
        
        server.connected_clients++;
        server.total_clients++;
    }

};


void close_tcp_client(int &fd, int &epoll_fd, ssize_t &count, server_state *state){
    if (count == 0) {
        close(fd);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
        state -> connected_clients--;
    } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
        perror("recv");
        close(fd);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
        state -> connected_clients--;
    }

};

void process_data_tcp(int fd, int &epoll_fd, server_state &server){
    char buf[BUF_SIZE];
    ssize_t count = recv(fd, buf, sizeof(buf)-1, 0);
    if (count <= 0) {
        if (count == 0) {
            close_tcp_client(fd, epoll_fd, count, &server);
        }
    } else {
        buf[count] = 0;
        if (buf[0] == '/') {
            parse_command_tcp(fd, buf, &server);
        } else {
            send(fd, buf, count, 0);
        }
    }

};

