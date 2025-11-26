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
#include "../server_state.hpp"

void parse_command_udp(int fd, char *buf, server_state *state, struct sockaddr_in &client_addr);
void parse_command_tcp(int fd, char *cmd, server_state *state);
