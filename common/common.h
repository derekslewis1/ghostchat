#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#define MAX_CLIENTS 10
#define MAX_USERNAME_LEN 20
#define MAX_MESSAGE_LEN 256

struct client
{
    int sockfd;
    char username[MAX_USERNAME_LEN];
};

extern struct client clients[MAX_CLIENTS];
extern int num_clients;

void send_to_all_clients_except(int sockfd, char *message, int except_sockfd);
void send_to_all_clients(char *username, char *text);
void send_message(int sockfd, char *message);
void broadcast_message(char *message);
int add_client(int sockfd, char *username);
int remove_client(int sockfd);
int find_client_index(int sockfd);

#endif /* COMMON_H */
