#include "parse_commands.h"


void get_time_str(char *buf, size_t size) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(buf, size, "%04d-%02d-%02d %02d:%02d:%02d",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
             tm.tm_hour, tm.tm_min, tm.tm_sec);
};

void parse_command_udp(int fd, char *buf, server_state *state, struct sockaddr_in &client_addr){
    char reply[256];
    std::cout << "In handle command udp " << std::endl;
    if (strcmp(buf, "/time") == 0) {
        std::cout << "recved time" << std::endl;
        get_time_str(reply, sizeof(reply));
    } else if (strcmp(buf, "/stats") == 0) {
        snprintf(reply, sizeof(reply), "Total: %d, Connected: %d",
                    state->total_clients, state->connected_clients);
    } else if (strcmp(buf, "/shutdown") == 0) {
        state->shutdown_flag = 1;
        strcpy(reply, "Shutting down");
    } else {
        strcpy(reply, "Unknown command");
    }
    socklen_t client_len = sizeof(client_addr);
    sendto(fd, reply, strlen(reply), 0,
            (struct sockaddr*)&client_addr, client_len);

};

void parse_command_tcp(int fd, char *cmd, server_state *state) {
    if (strcmp(cmd, "/time") == 0) {
        char timebuf[64];
        get_time_str(timebuf, sizeof(timebuf));
        send(fd, timebuf, strlen(timebuf), 0);
    } else if (strcmp(cmd, "/stats") == 0) {
        char buf[128];
        snprintf(buf, sizeof(buf), "Total clients: %d, Connected now: %d\n", 
                 state->total_clients, state->connected_clients);
        send(fd, buf, strlen(buf), 0);
    } else if (strcmp(cmd, "/shutdown") == 0) {
        state->shutdown_flag = 1;
    } else {
        const char *msg = "Unknown command\n";
        send(fd, msg, strlen(msg), 0);
    }
};
