#include "udp_handler.h"
#include <vector>
std::vector<char*> send_again; //вектор пакетов, которые не были отправлены успешно с первой попытки
std::vector<struct sockaddr_in> adresses; //вектор соответствующих адресов

int create_udp_socket(){
    int udp_listen_sock = create_socket(UDP_PORT, SOCK_DGRAM);
    return udp_listen_sock;

};

void add_to_send_again(char* reply, struct sockaddr_in &client_addr){
    char *resend = new char[strlen(reply)];
    memcpy(resend, reply, strlen(reply));
    send_again.push_back(resend);
    adresses.push_back(client_addr);

};
int send_data(int udp_listen_sock, int &epoll_fd, char* reply, struct sockaddr_in &client_addr, int flag){
    socklen_t client_len = sizeof(client_addr);
    ssize_t sent = sendto(udp_listen_sock, reply, strlen(reply), 0,(struct sockaddr*)&client_addr, client_len);
    if (sent == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == ENOBUFS) {
            if(flag == 0){
                epoll_event ev{};
                ev.events = EPOLLIN | EPOLLOUT;
                ev.data.fd = udp_listen_sock;
                int res = epoll_ctl(epoll_fd, EPOLL_CTL_MOD, udp_listen_sock, &ev);
                if(res < 0){
                    std::cerr << "error in epoll";
                    return -2;
                }
                add_to_send_again(reply, client_addr);
            }
            return -1;
        }else {
            std::cerr <<"udp send";
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, udp_listen_sock, nullptr);
            return -2;
        }
    }
    return 0;

};
void clean_send_again(){
    for(int i = 0; i < send_again.size(); i ++){
        delete[] send_again[i];
    }
}

int process_data_udp(int udp_listen_sock, int &epoll_fd,  server_state &server, struct epoll_event &ev){
    int res = 0;
    if(ev.events & EPOLLOUT){
        for(int i = 0; i < send_again.size(); i++){
            if(send_again[i] == nullptr){
                continue;
            }
            res = send_data(udp_listen_sock, epoll_fd, send_again[i], adresses[i], 1);
            if(res == -2){
                clean_send_again();
                return -2;
            }else if(res == -1){
                break;
            }else{
                delete[] send_again[i];
                send_again[i] = nullptr;
            }
        }
        if(res == 0){
            send_again.clear();
        }
    }
    
    if(ev.events & EPOLLIN){
        char buf[BUF_SIZE];
        char reply[256];
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        ssize_t count = recvfrom(udp_listen_sock, buf, sizeof(buf)-1, 0,
                                    (struct sockaddr*)&client_addr, &client_len);
        if (count > 0) {
            buf[count] = 0;
            if (buf[0] == '/') {
                parse_command(udp_listen_sock, buf, reply, &server);
                if(res == 0){
                    socklen_t client_len = sizeof(client_addr);
                    res = send_data(udp_listen_sock, epoll_fd, reply, client_addr, 0);
                    if(res == -2){
                        clean_send_again();
                        return -2;
                    }
                }else{
                    add_to_send_again(reply, client_addr);
                }
            } else {
                if(res == 0){
                    res = send_data(udp_listen_sock, epoll_fd, buf, client_addr, 0);
                    if(res == -2){
                        clean_send_again();
                        return -2;
                    }
                }else{
                    add_to_send_again(buf, client_addr);
                }
            }
        }
    }   
    return 0;

};