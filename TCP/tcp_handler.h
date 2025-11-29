#pragma once
#include "../server_state.hpp"
#include "../config.hpp"
#include "../Socket/socket.h"
#include "../Coms_parser/parse_commands.h"

int create_tcp_socket();
int accept_tcp_clients(int &tcp_listen_sock, int &epoll_fd, server_state &server);
int close_tcp_client(int &fd, int &epoll_fd, server_state *state);
int process_data_tcp(int fd, int &epoll_fd, server_state &server, struct epoll_event &ev);


