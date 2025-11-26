#include "udp_handler.h"
int create_udp_socket(){
    int udp_listen_sock = create_socket(UDP_PORT, SOCK_DGRAM);
    return udp_listen_sock;

};

void process_data_udp(int udp_listen_sock, server_state &server){
    char buf[BUF_SIZE];
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    ssize_t count = recvfrom(udp_listen_sock, buf, sizeof(buf)-1, 0,
                                (struct sockaddr*)&client_addr, &client_len);
    if (count > 0) {
        buf[count] = 0;
        if (buf[0] == '/') {
            parse_command_udp(udp_listen_sock,buf, &server,client_addr );
        } else {
            sendto(udp_listen_sock, buf, count, 0,
                    (struct sockaddr*)&client_addr, client_len);
        }
    }

};