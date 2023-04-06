#ifndef SERVER_H
#define SERVER_H

#include "../common/common.h"

struct client_thread_data
{
    int sockfd;
    struct sockaddr_in cli_addr;
    char username[MAX_USERNAME_LEN];
};

void *handle_client(void *arg);
extern int remove_client(int sockfd);

#endif /* SERVER_H */
