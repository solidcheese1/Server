#pragma once
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
#include "../server_state.hpp"
#include "../config.hpp"
#include "../Socket/socket.h"
#include "../Coms_parser/parse_commands.h"

int create_tcp_socket();
void accept_tcp_clients(int &tcp_listen_sock, int &epoll_fd, server_state &server);
void close_tcp_client(int &fd, int &epoll_fd, ssize_t &count, server_state *state);
void process_data_tcp(int fd, int &epoll_fd, server_state &server);


