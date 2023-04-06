#ifndef CLIENT_H
#define CLIENT_H

#include "../common/common.h"

// Declare client data structures and functions here
struct client_data
{
    int sockfd;
    char username[20];
};

void *handle_input(void *arg);
void *handle_output(void *arg);

#endif /* CLIENT_H */
