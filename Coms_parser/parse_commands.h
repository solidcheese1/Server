// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <time.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <errno.h>
// #include <signal.h>
// #include <netinet/in.h>
// #include <sys/epoll.h>
// #include <sys/socket.h>
// #include <sys/types.h>
// #include <arpa/inet.h>
// #include <iostream>
#include "../server_state.hpp"

void parse_command(int fd, char *buf, char * reply, server_state *state);
