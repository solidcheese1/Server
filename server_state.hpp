
#ifndef SERVER_STATE_HPP
#define SERVER_STATE_HPP
struct server_state{
    int sock;
    int connected_clients;
    int total_clients;
    int shutdown_flag;

};
#endif