// server.c
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

#define TCP_PORT 1027
#define UDP_PORT 1026
#define MAX_EVENTS 64
#define BUF_SIZE 1024


typedef struct server_state{
    int sock;
    int connected_clients;
    int total_clients;
    int shutdown_flag;

};
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

void get_time_str(char *buf, size_t size) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(buf, size, "%04d-%02d-%02d %02d:%02d:%02d",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
             tm.tm_hour, tm.tm_min, tm.tm_sec);
}

void handle_command(int fd, char *cmd, server_state *state) {
    if (strcmp(cmd, "/time") == 0) {
        char timebuf[64];
        get_time_str(timebuf, sizeof(timebuf));
        send(fd, timebuf, strlen(timebuf), 0);
    } else if (strcmp(cmd, "/stats") == 0) {
        char buf[128];
        snprintf(buf, sizeof(buf), "Total clients: %d, Connected now: %d\n", 
                 state->total_clients, state->connected_clients);
        send(fd, buf, strlen(buf), 0);
    } else if (strcmp(cmd, "/shutdown") == 0) {
        state->shutdown_flag = 1;
    } else {
        const char *msg = "Unknown command\n";
        send(fd, msg, strlen(msg), 0);
    }
}


int main(){
    std::cout << "Programm started" << std::endl;
    struct epoll_event ev,events[MAX_EVENTS];
    server_state server = {0,0,0,0};
    int tcp_listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    int udp_listen_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if((tcp_listen_sock < 0) || (udp_listen_sock < 0)){
        std::cerr << "Cant create socket";
        return -1;
    }
    set_nonblocking(tcp_listen_sock);
    set_nonblocking(udp_listen_sock);
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(TCP_PORT);
    addr.sin_addr.s_addr = INADDR_ANY;
    int opt = 1;
    setsockopt(tcp_listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    setsockopt(udp_listen_sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)); 
    if (bind(tcp_listen_sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind tcp");
        return 1;
    }   
    addr.sin_port = htons(UDP_PORT);
    if (bind(udp_listen_sock, (struct sockaddr*)&addr, sizeof(addr)) == -1) {
        perror("bind udp");
        return 1;
    }

    if (listen(tcp_listen_sock, SOMAXCONN) == -1) {
        perror("listen");
        return 1;
    }


    int epoll_fd = epoll_create1(0); 
    if(epoll_fd == -1){
        perror("epoll_create");
        return 1;
    }
    ev.events = EPOLLIN; 
    ev.data.fd = tcp_listen_sock;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, tcp_listen_sock, &ev); 
    ev.data.fd = udp_listen_sock;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, udp_listen_sock, &ev);
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
                while (true) {
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
            } else if (fd == udp_listen_sock) {
                char buf[BUF_SIZE];
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                ssize_t count = recvfrom(udp_listen_sock, buf, sizeof(buf)-1, 0,
                                         (struct sockaddr*)&client_addr, &client_len);
                if (count > 0) {
                    buf[count] = 0;
                    if (buf[0] == '/') {
                        char reply[256];
                        if (strcmp(buf, "/time") == 0) {
                            std::cout << "recved time" << std::endl;
                            get_time_str(reply, sizeof(reply));
                        } else if (strcmp(buf, "/stats") == 0) {
                            snprintf(reply, sizeof(reply), "Total: %d, Connected: %d",
                                     server.total_clients, server.connected_clients);
                        } else if (strcmp(buf, "/shutdown") == 0) {
                            server.shutdown_flag = 1;
                            strcpy(reply, "Shutting down");
                        } else {
                            strcpy(reply, "Unknown command");
                        }
                        sendto(udp_listen_sock, reply, strlen(reply), 0,
                               (struct sockaddr*)&client_addr, client_len);
                    } else {
                        sendto(udp_listen_sock, buf, count, 0,
                               (struct sockaddr*)&client_addr, client_len);
                    }
                }
            } else {
                char buf[BUF_SIZE];
                ssize_t count = recv(fd, buf, sizeof(buf)-1, 0);
                if (count <= 0) {
                    if (count == 0) {
                        close(fd);
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                        server.connected_clients--;
                    } else if (errno != EAGAIN && errno != EWOULDBLOCK) {
                        perror("recv");
                        close(fd);
                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
                        server.connected_clients--;
                    }
                } else {
                    buf[count] = 0;
                    if (buf[0] == '/') {
                        handle_command(fd, buf, &server);
                    } else {
                        send(fd, buf, count, 0);
                    }
                }
            }
        }
    }

    close(tcp_listen_sock);
    close(udp_listen_sock);
    close(epoll_fd);
    printf("Server shutdown\n");

    return 0;

}
