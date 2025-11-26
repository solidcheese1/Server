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
#include <iostream>
#include "../Coms_parser/parse_commands.h"
#include "../config.hpp"
#include "../server_state.hpp"
#include "../Socket/socket.h"

int create_udp_socket();
void process_data_udp(int udp_listen_sock, server_state &server);