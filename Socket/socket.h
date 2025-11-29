int set_nonblocking(int sock);
int create_socket(int port, int type);
int add_socket_to_epoll(int &epoll_fd, int sock);