
#ifndef SERVER_STATE_HPP
#define SERVER_STATE_HPP
#include "config.hpp"
struct server_state{
    int sock;
    int connected_clients;
    int total_clients;
    int shutdown_flag;

};

struct client_info {
    int fd;
    char outbuf[BUF_SIZE];
    size_t outbuf_len;
    size_t outbuf_sent;
};
#endif