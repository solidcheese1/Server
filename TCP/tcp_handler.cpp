#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include "tcp_handler.h"
#include "../config.hpp"
#include "../server_state.hpp"
#include <map>

std::map<int,client_info> Clients;

int send_data(client_info* client, int &epoll_fd, server_state &server) {
    while (client->outbuf_sent < client->outbuf_len) {
        ssize_t sent = send(client->fd, client->outbuf + client->outbuf_sent, client->outbuf_len - client->outbuf_sent, 0);
        if (sent == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // Сокет не готов, нужно ждать EPOLLOUT
                epoll_event ev{};
                ev.events = EPOLLIN | EPOLLOUT; 
                ev.data.fd = client->fd;
                int res = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client->fd, &ev);
                if(res == -1){
                    std::cerr << "error in epoll" << std::endl;
                    return -2;

                }
                // сдвигаем неотправленные данные в начало буфера
                memcpy(client->outbuf, client->outbuf + client->outbuf_sent, client->outbuf_len-client->outbuf_sent);
                client->outbuf_len = client->outbuf_len-client->outbuf_sent;
                client->outbuf_sent = 0;
                return -1;;
            } else {
                std::cerr << "send tcp error";
                int res = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client->fd, nullptr);
                if(res == -1){
                    std::cerr << "error in epoll" << std::endl;
                    return -2;

                }
                close(client->fd); 
                server.connected_clients --;
                Clients.erase(client->fd);
                return -1;
            }
        }
        client->outbuf_sent += (size_t)sent;
    }

    client->outbuf_len = 0;
    client->outbuf_sent = 0;
    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = client->fd;
    int res = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, client->fd, &ev);
    if(res == -1){
        std::cerr << "error in epoll" << std::endl;
        return -2;
    }
    return 0;
};

int process_data_tcp(int fd, int &epoll_fd, server_state &server, struct epoll_event &ev){
    if(ev.events & EPOLLOUT){
        int res = send_data(&Clients[fd], epoll_fd, server);
        if(res == -2){
            return -2;
        }
    }
    if(ev.events & EPOLLIN){
        char buf[BUF_SIZE];
        ssize_t count = recv(fd, buf, sizeof(buf)-1, 0);
        if (count <= 0) {
            int res = close_tcp_client(fd, epoll_fd, &server);
            if(res == -2){
                return res;
            }
            if(count < 0){
                std::cerr << "error in tcp recv";
            }
        } else {
            buf[count] = 0;
            client_info * client;
            if (buf[0] == '/') {
                char answer[BUF_SIZE];
                parse_command(fd, buf, answer, &server);
                if(server.shutdown_flag != 1){

                    if(Clients.count(fd) == 0){
                        client_info new_client;
                        new_client.fd = fd;
                        new_client.outbuf_len = 0;
                        new_client.outbuf_sent = 0;
                        Clients[fd] = new_client;
                        
                    }
                    client = &Clients[fd];
                    memcpy(client->outbuf + client -> outbuf_len, answer, strlen(answer)); // copy new data to end of buffer
                    client -> outbuf_len = client -> outbuf_len + strlen(answer);
                    int res = send_data(&Clients[fd], epoll_fd, server);
                    if(res == -2){
                        return res;
                    }

                }
            } else {
                if(Clients.count(fd) == 0){
                    client_info new_client;
                    new_client.fd = fd;
                    new_client.outbuf_len = 0;
                    new_client.outbuf_sent = 0;
                    Clients[fd] = new_client;
                        
                }
                client = &Clients[fd];
                memcpy(client->outbuf + client -> outbuf_len, buf, count); // copy new data to end of buffer
                client -> outbuf_len = client -> outbuf_len + count;
                int res = send_data(&Clients[fd], epoll_fd, server);
                if(res == -2){
                    return res;
                }
            }
        }
    }
    return 0;  

};



int create_tcp_socket(){
    int tcp_listen_sock = create_socket(TCP_PORT, SOCK_STREAM);
    if(tcp_listen_sock < 0){
        std::cerr << "tcp socket creation error";
        return tcp_listen_sock;
    }
    if (listen(tcp_listen_sock, SOMAXCONN) == -1) {
        std::cerr << "socket listen error";
        return -1;
    }
    return tcp_listen_sock;
};


int accept_tcp_clients(int &tcp_listen_sock, int &epoll_fd, server_state &server){
    while (true) {
        struct epoll_event ev;
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(tcp_listen_sock, (struct sockaddr*)&client_addr, &client_len);
        if (client_fd == -1) {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break; 
            else {
                std::cerr << "accept client error";
                return -2;
            }
        }


        int res = set_nonblocking(client_fd);
        if(res < 0){
            std::cerr << "Error in setting nonblocking regime(client)";
            close(client_fd);
        }
        ev.events = EPOLLIN;
        ev.data.fd = client_fd;
        res  = epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev);
        if(res < 0){
            std::cerr << "error in epoll";
            return -2;
        }
        
        server.connected_clients++;
        server.total_clients++;
    }
    return 0;

};


int close_tcp_client(int &fd, int &epoll_fd, server_state *state){
    
    close(fd);
    int res = epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
    if(res < 0){
        return -2;
    }
    state -> connected_clients--;
    return 0; 
};


