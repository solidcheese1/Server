#include "parse_commands.h"
#include "time.h"
#include <cstring>
#include <cstdio>
#include <cstddef>


void get_time_str(char *buf, size_t size) {
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    snprintf(buf, size, "%04d-%02d-%02d %02d:%02d:%02d \n",
             tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
             tm.tm_hour, tm.tm_min, tm.tm_sec);
};

void parse_command(int fd, char *buf, char * reply, server_state *state){
    if (strcmp(buf, "/time") == 0) {
        get_time_str(reply, 256);
    } else if (strcmp(buf, "/stats") == 0) {
        snprintf(reply, 256, "Total: %d, Connected: %d \n",
                    state->total_clients, state->connected_clients);
    } else if (strcmp(buf, "/shutdown") == 0) {
        state->shutdown_flag = 1;
        strcpy(reply, "Shutting down\n");
    } else {
        strcpy(reply, "Unknown command\n");
    }

};

